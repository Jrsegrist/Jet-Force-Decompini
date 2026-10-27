"""Shared helpers for the matching tools in tools/match/ (not a tool itself).

Paths come from this file's location, so the tools work from any checkout or
worktree. Scratch output goes to build/match/ (build/ is gitignored).
"""
import os
import re
import struct
import subprocess
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
TOOLS = os.path.join(ROOT, "tools")
VERSION = "us"
ROM_NAME = "baserom.%s.z64" % VERSION
DKR_DEFAULT = os.path.expanduser("~/jfg_refs/dkr")

# main code segment: ROM 0x1000 is loaded at 0x80000400; game and library
# code end at 0x800A0660 (data follows)
MAIN_ROM, MAIN_VRAM, MAIN_END = 0x1000, 0x80000400, 0x800A0660

# The game flags, as in the Makefile (CFLAGS/OPT_FLAGS/MIPSISET, ASFLAGS);
# keep the two in step. Paths are relative to ROOT (commands run there).
GAME_CFLAGS = ["-G", "0", "-non_shared", "-Xcpluscomm", "-nostdinc", "-Wab,-r4300_mul",
               "-woff", "516,608,649,838,712",
               "-D_LANGUAGE_C", "-D_MIPS_SZLONG=32", "-D_FINALROM", "-DNDEBUG", "-DBUILD_VERSION=VERSION_I",
               "-I", "include", "-I", "src", "-I", "lib/ultralib/include", "-I", "lib/ultralib/include/PR",
               "-I", "lib/ultralib/include/compiler/ido"]
ASFLAGS = ["-EB", "-march=vr4300", "-mtune=vr4300", "-mabi=32", "-G", "0", "-I", "include", "-I", ".",
           "--no-warn"]
IDO_CC = "tools/ido5.3/cc"
ASM_PROC = "tools/asm-processor/build.py"
OBJDUMP = "mips-linux-gnu-objdump"
OBJCOPY = "mips-linux-gnu-objcopy"


def path(*parts):
    return os.path.join(ROOT, *parts)


def workdir(name, tag=""):
    """build/match/NAME[-TAG], created; TAG keeps parallel runs apart."""
    d = path("build", "match", name + ("-" + tag if tag else ""))
    os.makedirs(d, exist_ok=True)
    return d


def need(p, hint):
    if not os.path.exists(p):
        sys.exit("%s: %s is missing (%s)" % (os.path.basename(sys.argv[0]), os.path.relpath(p, ROOT), hint))
    return p


def read_rom():
    with open(need(path(ROM_NAME), "put your ROM at the repository root"), "rb") as f:
        return f.read()


def main_rom_offset(vram):
    return vram - MAIN_VRAM + MAIN_ROM


def runlink_addrs():
    """Runlink name -> address, read from your ROM by `make extract`. Analysis
    only: these names must never be committed (tools/check_names.py)."""
    addr = {}
    p = need(path("build", "syms", "runlink_syms.%s.txt" % VERSION), "run `make extract`")
    for line in open(p):
        m = re.match(r"(\w+) = 0x([0-9A-Fa-f]+);", line)
        if m:
            addr[m.group(1)] = int(m.group(2), 16)
    return addr


def symbol_addrs():
    """Every name the probe can place: runlink names, undefined_syms.us.txt
    (NAME = 0x...; and func_XXXXXXXX = cname;)."""
    addr = runlink_addrs()
    for line in open(path("undefined_syms.%s.txt" % VERSION)):
        m = re.match(r"\s*(\w+)\s*=\s*0x([0-9A-Fa-f]+);", line)
        if m:
            addr.setdefault(m.group(1), int(m.group(2), 16))
        m = re.match(r"\s*func_([0-9A-Fa-f]{8})\s*=\s*(\w+);", line)
        if m:
            addr[m.group(2)] = int(m.group(1), 16)
    return addr


def reloc_mask(rtype):
    """Bits of an instruction word that a relocation of type RTYPE leaves alone."""
    if rtype in ("R_MIPS_26", 4):
        return 0xFC000000
    if rtype in ("R_MIPS_HI16", "R_MIPS_LO16", 5, 6):
        return 0xFFFF0000
    return 0xFFFFFFFF


def compile_game(src, obj, extra=(), cflags=None):
    """Compile SRC to OBJ with the game flags through asm-processor (like the
    Makefile). EXTRA may override -O and -mips. Returns the CompletedProcess."""
    extra = list(extra)
    opt = [x for x in extra if x.startswith("-O")] or ["-O2"]
    mips = [x for x in extra if x.startswith("-mips")] or ["-mips1"]
    rest = [x for x in extra if not (x.startswith("-O") or x.startswith("-mips"))]
    cmd = [sys.executable, ASM_PROC, "--asm-prelude", "include/prelude.inc", IDO_CC, "--",
           "mips-linux-gnu-as"] + ASFLAGS + ["--", "-c"] + (cflags or GAME_CFLAGS) + rest + opt + mips + \
        ["-o", obj, src]
    return subprocess.run(cmd, capture_output=True, text=True, cwd=ROOT)


def text_and_relocs(obj):
    """(.text bytes, {offset: (reloc type, symbol)}, objdump -t -r text) of OBJ."""
    dump = subprocess.run([OBJDUMP, "-t", "-r", "-j", ".text", obj], capture_output=True, text=True).stdout
    relocs = {}
    for m in re.finditer(r"^([0-9a-f]{8}) (R_MIPS_\w+)\s+(\S+)", dump, re.M):
        relocs[int(m.group(1), 16)] = (m.group(2), m.group(3))
    subprocess.run([OBJCOPY, "-O", "binary", "-j", ".text", obj, obj + ".text"], check=True)
    with open(obj + ".text", "rb") as f:
        text = f.read()
    return text, relocs, dump


def text_functions(dump):
    """(offset, size, name) of each sized function symbol in an objdump -t listing."""
    res = []
    for m in re.finditer(r"^([0-9a-f]{8}) (.{7}) \.text\s+([0-9a-f]{8}) (\S+)$", dump, re.M):
        if "F" in m.group(2) and int(m.group(3), 16) > 0 and not m.group(4).startswith("."):
            res.append((int(m.group(1), 16), int(m.group(3), 16), m.group(4)))
    return res


def u32(b, o):
    return struct.unpack_from(">I", b, o)[0]


def disassembler(pseudos=True):
    """A one-line disassembler (rabbitizer from the venv), or one that prints nothing."""
    try:
        import rabbitizer
    except ImportError:
        return lambda w, va: ""
    rabbitizer.config.pseudos_enablePseudos = pseudos

    def dis(w, va):
        try:
            return rabbitizer.Instruction(w, va).disassemble().replace("\t", " ")
        except Exception:
            return "?"
    return dis
