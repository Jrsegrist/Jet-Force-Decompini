#!/usr/bin/env python3
"""Self-tests for the build tools, on synthetic inputs (no ROM).

  runlink   Two small made-up modules are assembled and linked into runlink
            images by tools/runlink_build.py. A synthetic "ROM" is put
            together from those images, a symbol table and a name pool (all
            invented here), and split again by tools/runlink_split.py. The
            module assembly it writes, and separately the per-function files
            (INCLUDE_ASM form) put back together, must rebuild to the same
            images and module table byte for byte. The modules cover the
            relocation kinds (calls inside and outside the module, %hi/%lo
            and .word references to own and foreign data, a switch jump
            table, bss, an init function, an alias).
  syms      tools/runlink_syms.py on the same synthetic ROM.
  diff      tools/runlink_diff.py finds functions in the module map.
  progress  tools/progress.py on a tiny linked ELF, its map, a module map and
            C files with INCLUDE_ASM lines.
  rename    tools/rename.py on a made-up tree: the rewrite, --check, --which,
            re-running, bad maps, stale field uses, and the linker aliases
            (linked).

Needs mips-linux-gnu-as / ld / objdump (binutils-mips-linux-gnu) and the
Python packages of requirements.txt (rabbitizer). Run from the repository
root: tools/ci_test_tools.py [-v]
"""
import contextlib
import io
import os
import re
import shutil
import struct
import subprocess
import sys
import tempfile
import unittest

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.join(ROOT, "tools"))

import progress  # noqa: E402
import rename  # noqa: E402
import runlink_build  # noqa: E402
import runlink_diff  # noqa: E402
import runlink_split  # noqa: E402
import runlink_syms  # noqa: E402

CROSS = os.environ.get("CROSS", "mips-linux-gnu-")
ASFLAGS = ["-EB", "-march=vr4300", "-mtune=vr4300", "-mabi=32", "-G", "0", "--no-warn"]

HEAD = ".set noat\n.set noreorder\n.set gp=64\n"

# Module 1: an exported function that calls a local helper and a main-segment
# function, reads its own static data and an exported variable, and a switch
# with a jump table in .rodata; .data holds a pointer into its own .text.
MOD1 = HEAD + r"""
.section .text
.global ModOneEntry
.type ModOneEntry, @function
ModOneEntry:
    addiu $sp, $sp, -24
    sw    $ra, 20($sp)
    jal   helper
    nop
    jal   TestMainFunc
    nop
    lui   $a0, %hi(mydata)
    lw    $a0, %lo(mydata)($a0)
    lui   $a1, %hi(ModOneVar)
    lw    $a1, %lo(ModOneVar)($a1)
    lw    $ra, 20($sp)
    jr    $ra
    addiu $sp, $sp, 24

.type helper, @function
helper:
    jr    $ra
    move  $v0, $zero

.global ModOneSwitch
.type ModOneSwitch, @function
ModOneSwitch:
    sltiu $at, $a0, 3
    beqz  $at, .Ldefault
    sll   $t6, $a0, 2
    lui   $at, %hi(jtbl)
    addu  $at, $at, $t6
    lw    $t6, %lo(jtbl)($at)
    jr    $t6
    nop
.Lcase0:
    jr    $ra
    li    $v0, 1
.Lcase1:
    jr    $ra
    li    $v0, 2
.Lcase2:
    jr    $ra
    li    $v0, 3
.Ldefault:
    jr    $ra
    move  $v0, $zero

.section .data
mydata:
    .word 0x12345678
    .word helper
.global ModOneVar
ModOneVar:
    .word 7

.section .rodata
jtbl:
    .word .Lcase0
    .word .Lcase1
    .word .Lcase2
"""

# Module 2: calls module 1's export, main functions by name, by address and
# through an alias, uses main data and its own bss, and has an init function.
MOD2 = HEAD + r"""
.global __rl_init
.set __rl_init, ModTwoEntry

.section .text
.global ModTwoEntry
.type ModTwoEntry, @function
ModTwoEntry:
    addiu $sp, $sp, -24
    sw    $ra, 20($sp)
    jal   ModOneEntry
    nop
    jal   func_80000550
    nop
    jal   TestAlias
    nop
    lui   $t0, %hi(TestMainVar)
    lw    $t0, %lo(TestMainVar)($t0)
    lui   $t1, %hi(counter)
    sw    $t0, %lo(counter)($t1)
    lw    $ra, 20($sp)
    jr    $ra
    addiu $sp, $sp, 24

.section .data
    .word TestMainFunc
    .word 0

.section .bss
counter:
    .space 0x20
"""

L_TEST = dict(text=0x80000450, data=0x800A0660, bss=0x800B0B50, bss_end=0x800B1000)


def run(cmd, **kw):
    r = subprocess.run(cmd, capture_output=True, text=True, **kw)
    if r.returncode:
        raise AssertionError("%s failed (%d):\n%s%s" % (" ".join(cmd), r.returncode, r.stdout, r.stderr))
    return r.stdout


def assemble(src_text, obj, include=None):
    s = obj[:-2] + ".s"
    with open(s, "w") as f:
        f.write(src_text)
    cmd = [CROSS + "as"] + ASFLAGS + (["-I", include] if include else []) + ["-o", obj, s]
    run(cmd)
    return obj


def call_main(fn, argv):
    old = sys.argv
    sys.argv = ["tool"] + argv
    out = io.StringIO()
    try:
        with contextlib.redirect_stdout(out):
            fn()
    finally:
        sys.argv = old
    return out.getvalue()


class RunlinkRoundTrip(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.tmp = tempfile.mkdtemp(prefix="jfg_ci_rl_")
        t = cls.tmp
        o1 = assemble(MOD1, os.path.join(t, "m001.o"))
        o2 = assemble(MOD2, os.path.join(t, "m002.o"))

        # module offsets of the exported symbols, laid out as runlink_build does
        def offset(obj, name):
            e = runlink_build.Elf(obj)
            tsize = runlink_build.align(e.section(".text")[1], 16)
            base = {e.byname.get(".text"): 0, e.byname.get(".data"): tsize}
            s = next(s for s in e.syms if s["name"] == name)
            return base[s["shndx"]] + s["value"]

        # index, module, offset, name (invented names, none from any game)
        # (index 0 is unused: a word 0 would hide a wrong kind-2 word)
        cls.syms = [
            (0, 0x200, "TestMainOther"),
            (0, 0x100, "TestMainFunc"),
            (1, offset(o1, "ModOneEntry"), "ModOneEntry"),
            (1, offset(o1, "ModOneVar"), "ModOneVar"),
            (1, offset(o1, "ModOneSwitch"), "ModOneSwitch"),
            (2, offset(o2, "ModTwoEntry"), "ModTwoEntry"),
            (runlink_build.SEC_DATA, 0x10, "TestMainVar"),
            (runlink_build.SEC_BSS, 0x40, "TestMainBss"),
            (0, 0x100, "TestMainFuncAlias"),
        ]
        cls.symfile = os.path.join(t, "symbols.txt")
        with open(cls.symfile, "w") as f:
            for i, (m, o, n) in enumerate(cls.syms):
                f.write("%d %d 0x%X %s\n" % (i, m, o, n))
        cls.aliases = os.path.join(t, "aliases.txt")
        with open(cls.aliases, "w") as f:
            f.write("TestAlias = TestMainFunc; /* an alias */\nD_800A0670 = TestMainVar;\n")
        cls.ref_mods, cls.ref_tab = cls.build(t, "ref", [o1, o2])

        # the synthetic ROM: symbol table, module table, images, names
        nsyms, nmods = len(cls.syms), len(cls.ref_tab) // 32
        L = dict(L_TEST, main_imports=0x40, symtab=0x100)
        L["modtab"] = L["symtab"] + 4 * nsyms
        L["modules"] = L["modtab"] + 32 * nmods
        L["nameoffs"] = runlink_build.align(L["modules"] + len(cls.ref_mods), 16)
        L["names"] = L["nameoffs"] + 4 * nsyms
        pool = b""
        offs = []
        for _, _, n in cls.syms:
            offs.append(len(pool))
            pool += n.encode() + b"\0"
        rom = bytearray(L["names"] + len(pool) + 16)
        for i, (m, o, _) in enumerate(cls.syms):
            struct.pack_into(">I", rom, L["symtab"] + 4 * i, (m << 20) | o)
            struct.pack_into(">I", rom, L["nameoffs"] + 4 * i, offs[i])
        rom[L["modtab"]:L["modtab"] + len(cls.ref_tab)] = cls.ref_tab
        rom[L["modules"]:L["modules"] + len(cls.ref_mods)] = cls.ref_mods
        rom[L["names"]:L["names"] + len(pool)] = pool
        cls.L = L
        cls.rom = os.path.join(t, "test.z64")
        open(cls.rom, "wb").write(rom)
        runlink_split.LAYOUT["test"] = L
        runlink_syms.LAYOUT["test"] = L
        cls.split_out = call_main(runlink_split.main, [
            cls.rom, "--version", "test", "--asm-dir", os.path.join(t, "asm/modules"),
            "--nonmatchings-dir", os.path.join(t, "asm/nonmatchings/modules"),
            "--build-dir", os.path.join(t, "runlink")])

    @classmethod
    def tearDownClass(cls):
        shutil.rmtree(cls.tmp, ignore_errors=True)

    @classmethod
    def build(cls, t, tag, objs, order=None):
        mods, tab = os.path.join(t, tag + "_modules.bin"), os.path.join(t, tag + "_modtab.bin")
        argv = ["--symbols", cls.symfile, "--aliases", cls.aliases, "--out-modules", mods,
                "--out-table", tab]
        if order:
            argv += ["--order", order]
        call_main(runlink_build.main, argv + objs)
        return open(mods, "rb").read(), open(tab, "rb").read()

    def test_table(self):
        tab = [struct.unpack_from(">IIIIIHHii", self.ref_tab, 32 * m) for m in range(2)]
        # module 1: no bss, both relocation tables used, no init
        self.assertEqual(tab[0][0], 0)
        self.assertEqual(tab[0][4], 0)
        self.assertGreater(tab[0][5], 0)
        self.assertGreater(tab[0][6], 0)
        self.assertEqual((tab[0][7], tab[0][8]), (-1, -1))
        # module 2: bss rounded to 16, init at ModTwoEntry
        self.assertEqual(tab[1][4], 0x20)
        self.assertEqual(tab[1][7], self.syms[5][1])
        # the images are back to back: text, data block, the two tables
        self.assertEqual(tab[1][1], tab[0][2] + tab[0][3] + tab[0][5] + tab[0][6])

    def entries(self, m):
        """[(table, module offset, type, kind, word)] of module m (1-based)."""
        _, off, t, d, b, r1, r2, _, _ = struct.unpack_from(">IIIIIHHii", self.ref_tab, 32 * (m - 1))
        out = []
        base = off + t + d
        for tn, (start, n) in enumerate(((base, r1), (base + r1, r2))):
            for i in range(n // 8):
                word, x = struct.unpack_from(">II", self.ref_mods, start + 8 * i)
                out.append((tn + 1, x >> 8, (x >> 4) & 0xF, x & 0xF, word))
        return out

    def test_relocation_values(self):
        e1 = self.entries(1)
        img = self.ref_mods
        tsize = struct.unpack_from(">I", self.ref_tab, 8)[0]
        # a kind-2 entry repeats the word of the kind-0 entry before it
        last = None
        for tn, o, typ, kind, word in e1:
            if kind == 0:
                last = word
            elif kind == 2:
                self.assertEqual(word, last)
        # jal helper: the jump field holds helper's module offset
        jal = [o for tn, o, typ, kind, w in e1 if kind == 2]
        self.assertEqual(len(jal), 1)
        self.assertEqual((struct.unpack_from(">I", img, jal[0])[0] & 0x3FFFFFF) << 2, 0x34)
        # jal TestMainFunc: kind 0, symbol index 1, in table 1
        self.assertIn((1, 0x10, 4, 0, 1), e1)
        # lui/lw mydata: kind 1, word = the start of .data (the end of .text)
        self.assertIn((2, 0x18, 5, 1, tsize), e1)
        self.assertIn((2, 0x1C, 6, 1, tsize), e1)
        # ModOneVar: kind 0 with its own symbol index (3), in table 2
        self.assertIn((2, 0x20, 5, 0, 3), e1)
        # the jump table's three words point at .text (kind 1, word 0)
        self.assertEqual(sum(1 for tn, o, typ, k, w in e1 if typ == 2 and k == 1 and w == 0), 4)
        # module 2 calls module 1's export by index (2), from table 1
        self.assertIn((1, 0x8, 4, 0, 2), self.entries(2))

    def test_relocation_kinds(self):
        seen = set()
        for m in (1, 2):
            for tn, o, typ, kind, word in self.entries(m):
                seen.add((m, tn, typ, kind))
        R26, HI, LO, W32 = 4, 5, 6, 2
        for want in [(1, 1, R26, 0),  # jal TestMainFunc
                     (1, 2, R26, 2),  # jal helper (inside the module)
                     (1, 2, HI, 1), (1, 2, LO, 1),  # mydata, jtbl
                     (1, 2, HI, 0),  # ModOneVar (own export, by index)
                     (1, 2, W32, 1),  # .word helper, the jump table
                     (2, 1, R26, 0), (2, 1, HI, 0), (2, 1, W32, 0),
                     (2, 2, LO, 1)]:  # counter (bss)
            self.assertIn(want, seen)

    def test_split_finds_functions_and_jump_table(self):
        self.assertIn("2 modules, 4 functions, 1 jump tables", self.split_out)
        fl = open(os.path.join(self.tmp, "asm/nonmatchings/modules/m001/functions.txt")).read()
        self.assertEqual(re.findall(r"^func_m001_(\w+)", fl, re.M), ["00000", "00034", "0003C"])
        # the jump table, with its three cases as labels
        ro = open(os.path.join(self.tmp, "asm/nonmatchings/modules/m001/rodata.s")).read()
        self.assertIn("dlabel jtbl_m001_", ro)
        self.assertEqual(len(re.findall(r"\.word \.L[0-9A-F]{5}$", ro, re.M)), 3)

    def test_split_uses_no_names(self):
        """The generated assembly names symbols by address; runlink names
        appear only in comments."""
        for d, _, fs in os.walk(os.path.join(self.tmp, "asm")):
            for f in fs:
                if not f.endswith(".s"):
                    continue
                t = re.sub(r"/\*.*?\*/", "", open(os.path.join(d, f)).read(), flags=re.S)
                for n in ("ModOneEntry", "ModOneVar", "ModTwoEntry", "TestMainFunc", "TestMainVar"):
                    self.assertNotIn(n, t, "%s in %s" % (n, f))

    def test_rebuild_from_module_asm(self):
        t = self.tmp
        objs = [assemble(open(os.path.join(t, "asm/modules/m%03d.s" % m)).read(),
                         os.path.join(t, "re_m%03d.o" % m)) for m in (1, 2)]
        mods, tab = self.build(t, "re", objs, order=os.path.join(t, "runlink/order"))
        self.assertEqual(tab, self.ref_tab)
        self.assertEqual(mods, self.ref_mods)

    def test_rebuild_from_function_files(self):
        """The INCLUDE_ASM pieces (one file per function, data.s, rodata.s,
        bss.s) assembled together give the same module."""
        t = self.tmp
        objs = []
        for m in (1, 2):
            d = os.path.join(t, "asm/nonmatchings/modules/m%03d" % m)
            src = ['.include "prelude.inc"', ".section .text"]
            for line in open(os.path.join(d, "functions.txt")):
                if not line.startswith("#"):
                    src.append(open(os.path.join(d, line.split()[0] + ".s")).read())
            for part in ("data", "rodata", "bss"):
                p = os.path.join(d, part + ".s")
                if os.path.exists(p):
                    src.append(open(p).read())
            if m == 2:
                src.append(".global __rl_init\n.set __rl_init, func_m002_00000")
            objs.append(assemble("\n".join(src) + "\n", os.path.join(t, "fn_m%03d.o" % m),
                                 include=os.path.join(ROOT, "include")))
        mods, tab = self.build(t, "fn", objs, order=os.path.join(t, "runlink/order"))
        self.assertEqual(tab, self.ref_tab)
        self.assertEqual(mods, self.ref_mods)

    def test_runlink_syms(self):
        t = self.tmp
        out, mods, al = (os.path.join(t, n) for n in ("syms.txt", "mods.txt", "aliases_out.txt"))
        call_main(runlink_syms.main, [self.rom, out, "--version", "test", "--modules", mods,
                                      "--aliases", al])
        s = open(out).read()
        self.assertIn("TestMainFunc = 0x80000550; // type:func filename:func_80000550", s)
        self.assertIn("TestMainVar = 0x800A0670;\n", s)
        self.assertIn("TestMainBss = 0x800B0B90;\n", s)
        self.assertIn("// alias: TestMainFuncAlias = 0x80000550; (same as TestMainFunc)", s)
        self.assertNotIn("ModOneEntry", s)
        a = open(al).read()
        self.assertIn("PROVIDE(func_80000550 = TestMainFunc);", a)
        self.assertIn("PROVIDE(TestMainFunc = func_80000550);", a)
        self.assertIn("PROVIDE(TestMainFuncAlias = TestMainFunc);", a)
        lines = [x for x in open(mods) if not x.startswith("#")]
        self.assertEqual(len(lines), 2)
        self.assertTrue(lines[0].split()[1] == "0x%07X" % self.L["modules"])

    def test_runlink_diff_find(self):
        mapfile = os.path.join(self.tmp, "modules.map")
        # (the last build's map: the object from the split assembly)
        name = "func_m001_%05X" % self.syms[4][1]
        hit = runlink_diff.find(name, mapfile)
        self.assertIsNotNone(hit)
        self.assertEqual(hit[0], 1)
        self.assertEqual(hit[3], self.syms[4][1])
        self.assertIsNone(runlink_diff.find("nothing", mapfile))


class Progress(unittest.TestCase):
    def test_progress(self):
        t = tempfile.mkdtemp(prefix="jfg_ci_prog_")
        try:
            for d in ("src", "src/modules", "build/src", "build/asm", "build/runlink"):
                os.makedirs(os.path.join(t, d))
            fn = ".global {0}\n.type {0}, @function\n{0}:\n" + "    nop\n" * 2 + "{1}" + ".size {0}, . - {0}\n"
            game = HEAD + ".section .text\n" + fn.format("func_80000450", "") + \
                fn.format("func_80000458", "    nop\n" * 2)
            lib = HEAD + ".section .text\n" + fn.format("func_80084200", "    nop\n" * 6)
            assemble(game, os.path.join(t, "build/src/game.o"))
            assemble(lib, os.path.join(t, "build/asm/lib.o"))
            with open(os.path.join(t, "src/game.c"), "w") as f:
                f.write('/* INCLUDE_ASM("asm/nonmatchings/game", func_80000450); */\n'
                        'INCLUDE_ASM("asm/nonmatchings/game", func_80000458);\n')
            with open(os.path.join(t, "link.ld"), "w") as f:
                f.write("SECTIONS {\n  .game 0x80000450 : { build/src/game.o(.text) }\n"
                        "  .lib 0x80084200 : { build/asm/lib.o(.text) }\n"
                        "  /DISCARD/ : { *(.reginfo) *(.MIPS.abiflags) }\n}\n")
            run([CROSS + "ld", "-T", "link.ld", "-Map", "build/jfg.us.map", "-o", "build/jfg.us.elf",
                 "build/src/game.o", "build/asm/lib.o"], cwd=t)
            with open(os.path.join(t, "src/modules/m001.c"), "w") as f:
                f.write('INCLUDE_ASM("asm/nonmatchings/modules/m001", func_m001_00040);\n')
            with open(os.path.join(t, "build/runlink/modules.map"), "w") as f:
                f.write("# module image_offset text data bss object\n"
                        "m001 0x000000 0x00100 0x00010 0x00000 build/src/modules/m001.o\n"
                        "  func_m001_00000 0x00000 0x40\n  func_m001_00040 0x00040 0xC0\n"
                        "m002 0x000110 0x00010 0x00000 0x00000 build/asm/modules/m002.o\n"
                        "  func_m002_00000 0x00000 0x10\n")
            cwd = os.getcwd()
            os.chdir(t)
            try:
                out = call_main(progress.main, ["--files", "--modules"])
            finally:
                os.chdir(cwd)
        finally:
            shutil.rmtree(t, ignore_errors=True)

        def row(label):
            m = re.search(r"^%s\s+(\d+)/(\d+)\s+\S+\s+(\d+)/(\d+)" % re.escape(label), out, re.M)
            self.assertIsNotNone(m, "%s not in:\n%s" % (label, out))
            return tuple(int(x) for x in m.groups())

        self.assertEqual(row("main: game"), (1, 2, 8, 24))
        self.assertEqual(row("main: libraries"), (0, 1, 0, 32))
        self.assertEqual(row("main: total"), (1, 3, 8, 56))
        self.assertEqual(row("runlink modules"), (1, 3, 0x40, 0x110))
        self.assertIn("(0 of 2 modules all C, 1 partly)", out)
        self.assertEqual(row("total"), (2, 6, 72, 328))
        # names: the one C function (func_80000450) still has its address name
        self.assertRegex(out, r"(?m)^names\s+0/1\s")


RENAME_NAMES = """\
# a made-up rename map
func_80001000 = cam_shake
D_800A0000 = gShakeTimer
func_80002000 = shake_tick
func_80003000 = asm_helper
func_m001_00000 = drops_reset
field Shake.unk4 = timer
renamed shake_update = shake_tick
"""
RENAME_C = """\
#include "a.h"
#define gShakeTimer D_800A0000
extern s32 D_800A0000;
INCLUDE_ASM("asm/nonmatchings/a", func_80001000);
void func_80001000(Shake *s) { s->unk4 = D_800A0000; shake_update(); func_80003000(); }
void func_80001000_b(void) { /* another symbol: func_80001000x */ }
"""
RENAME_C_AFTER = """\
#include "a.h"
extern s32 gShakeTimer;
INCLUDE_ASM("asm/nonmatchings/a", func_80001000);
void cam_shake(Shake *s) { s->unk4 = gShakeTimer; shake_tick(); asm_helper(); }
void func_80001000_b(void) { /* another symbol: func_80001000x */ }
"""
RENAME_MOD = """\
static void func_m001_00000(void) {
}
void func_m001_00040(void) {
    func_m001_00000();
}
"""


class Rename(unittest.TestCase):
    """tools/rename.py on a made-up tree (a git repository in a temporary directory)."""

    def setUp(self):
        self.t = tempfile.mkdtemp(prefix="jfg_ci_ren_")
        t = self.t
        os.makedirs(os.path.join(t, "src", "modules"))
        files = {
            "names.txt": RENAME_NAMES,
            "src/a.c": RENAME_C,
            "src/a.h": "typedef struct Shake {\n    /* 0x00 */ s32 unk0;\n    /* 0x04 */ s32 unk4;\n} Shake;\n"
                       "void func_80002000(void);\n#define shake_update func_80002000\n",
            "src/modules/m001.c": RENAME_MOD,
            "undefined_syms.us.txt": "D_800A0000 = 0x800A0000;\n",
            "symbol_addrs.us.txt": "func_80001000 = 0x80001000; // type:func\n",
            "README.md": "func_80001000 shakes the camera.\n",
        }
        for rel, text in files.items():
            with open(os.path.join(t, rel), "w") as f:
                f.write(text)
        run(["git", "init", "-q"], cwd=t)
        run(["git", "add", "-A"], cwd=t)

    def tearDown(self):
        shutil.rmtree(self.t, ignore_errors=True)

    def rename(self, *argv):
        out = io.StringIO()
        code = None
        with contextlib.redirect_stdout(out):
            try:
                code = rename.main(["--root", self.t] + list(argv))
            except SystemExit as e:
                code = e.code
        return code, out.getvalue()

    def read(self, rel):
        with open(os.path.join(self.t, rel)) as f:
            return f.read()

    def test_apply_check_rerun(self):
        code, out = self.rename("--check")
        self.assertEqual(code, 1)
        code, out = self.rename()
        self.assertEqual(code, 0, out)
        self.assertEqual(self.read("src/a.c"), RENAME_C_AFTER)
        # (the `#define shake_tick shake_tick` the rename leaves is dropped)
        self.assertTrue(self.read("src/a.h").endswith("} Shake;\nvoid shake_tick(void);\n"), self.read("src/a.h"))
        self.assertIn("static void drops_reset(void)", self.read("src/modules/m001.c"))
        self.assertIn("    drops_reset();", self.read("src/modules/m001.c"))
        self.assertIn("void func_m001_00040(void)", self.read("src/modules/m001.c"))
        self.assertEqual(self.read("undefined_syms.us.txt"), "gShakeTimer = 0x800A0000;\n")
        # splat's symbol file keeps the address name; docs are renamed
        self.assertEqual(self.read("symbol_addrs.us.txt"), "func_80001000 = 0x80001000; // type:func\n")
        self.assertEqual(self.read("README.md"), "cam_shake shakes the camera.\n")
        # safe to re-run
        code, out = self.rename()
        self.assertIn("0 replacements", out)
        # the field is still to be renamed by hand
        code, out = self.rename("--check")
        self.assertEqual(code, 1)
        self.assertIn("struct Shake still has unk4", out)
        for rel, old, new in (("src/a.h", "s32 unk4;", "s32 timer;"), ("src/a.c", "s->unk4", "s->timer")):
            text = self.read(rel).replace(old, new)
            with open(os.path.join(self.t, rel), "w") as f:
                f.write(text)
        code, out = self.rename("--check")
        self.assertEqual(code, 0, out)
        self.assertIn("0 old names", out)

    def write(self, rel, text):
        with open(os.path.join(self.t, rel), "w") as f:
            f.write(text)

    def test_field_uses(self):
        # the field is renamed in its struct and in the code the compiler sees,
        # but a NON_MATCHING body still has the old name
        self.rename()
        self.write("src/a.h", self.read("src/a.h").replace("s32 unk4;", "s32 timer;"))
        self.write("src/a.c", self.read("src/a.c").replace("s->unk4", "s->timer")
                   + "#ifdef NON_MATCHING\nvoid f(Shake *s) { s->unk4 = 0; }\n#endif\n")
        code, out = self.rename("--check")
        self.assertEqual(code, 1, out)
        self.assertIn("src/a.c:7: STALE unk4 (Shake.unk4 = timer)", out)
        # with another struct that has an unk4, the use may be that struct's:
        # listed (--field-uses, and after a rename), not an error
        self.write("src/b.h", "typedef struct Other {\n    s16 unk4;\n} Other;\n")
        run(["git", "add", "src/b.h"], cwd=self.t)
        code, out = self.rename("--check")
        self.assertEqual(code, 0, out)
        self.assertIn("1 `.old`/`->old` use", out)
        code, out = self.rename("--field-uses")
        self.assertEqual(code, 0, out)
        self.assertIn("src/a.c:7: unk4 (Shake.unk4 = timer): void f(Shake *s) { s->unk4 = 0; }", out)
        code, out = self.rename()
        self.assertIn("src/a.c:7: unk4 (Shake.unk4 = timer)", out)

    def test_which(self):
        self.assertEqual(self.rename("--which", "func_80001000"), (0, "func_80001000 = cam_shake\n"))
        self.assertEqual(self.rename("--which", "cam_shake"), (0, "func_80001000 = cam_shake\n"))
        self.assertEqual(self.rename("--which", "shake_update"), (0, "shake_update = shake_tick\n"))
        self.assertEqual(self.rename("--which", "Shake.unk4"), (0, "field Shake.unk4 = timer\n"))
        self.assertEqual(self.rename("--which", "nothing")[0], 1)

    def test_bad_maps(self):
        for text, msg in (("func_m001_00040 = drops_main\n", "not static"),
                          ("func_80001000 = a\nfunc_80002000 = a\n", "new name of two symbols"),
                          ("func_80001000 = a\nfunc_80001000 = b\n", "renamed twice"),
                          ("cam_shake = a\n", "not an address name"),
                          ("func_80001000 = D_80001000\n", "not a name of our own")):
            with open(os.path.join(self.t, "names.txt"), "w") as f:
                f.write(text)
            code, out = self.rename("--dry-run")
            self.assertIn(msg, str(code), text)

    def test_aliases(self):
        # C defines cam_shake; undefined_syms defines gShakeTimer (after the
        # rename); assembly defines func_80002000 and func_80003000
        self.rename()
        obj = assemble(HEAD + ".section .text\n.global cam_shake\ncam_shake:\n    nop\n",
                       os.path.join(self.t, "a.o"))
        out = os.path.join(self.t, "build", "aliases.txt")
        code, _ = self.rename("--aliases", out, obj)
        self.assertEqual(code, 0)
        lines = [ln for ln in self.read(out).splitlines() if not ln.startswith(("/*", " *"))]
        self.assertEqual(lines, ["func_80001000 = cam_shake;", "D_800A0000 = gShakeTimer;",
                                 "PROVIDE(shake_tick = func_80002000);", "PROVIDE(asm_helper = func_80003000);"])
        # and the linker takes them: asm calls the address name, C the new one
        asm = assemble(HEAD + ".section .text\n.global func_80002000\nfunc_80002000:\n    jal func_80001000\n"
                       "    nop\n.global func_80003000\nfunc_80003000:\n    jal shake_tick\n    nop\n"
                       "    lw $2, %lo(D_800A0000)($0)\n", os.path.join(self.t, "b.o"))
        with open(os.path.join(self.t, "link.ld"), "w") as f:
            f.write("SECTIONS {\n  .text 0x80001000 : { *(.text) }\n"
                    "  /DISCARD/ : { *(.reginfo) *(.MIPS.abiflags) }\n}\n")
        run([CROSS + "ld", "-T", "link.ld", "-T", out, "-T", "undefined_syms.us.txt", "-o", "x.elf", obj, asm],
            cwd=self.t)
        syms = run([CROSS + "objdump", "-t", "x.elf"], cwd=self.t)
        addr = {m.group(2): m.group(1) for m in re.finditer(r"^([0-9a-f]{8}) .* (\w+)$", syms, re.M)}
        self.assertEqual(addr["func_80001000"], addr["cam_shake"])
        self.assertEqual(addr["shake_tick"], addr["func_80002000"])
        self.assertEqual(addr["D_800A0000"], "800a0000")


if __name__ == "__main__":
    # the tools leave files to the garbage collector; unittest's default
    # warning filter would print a ResourceWarning for each
    unittest.main(warnings="ignore")
