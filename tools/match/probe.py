#!/usr/bin/env python3
"""The main-segment probe: compile one C file with the game flags (through
asm-processor, as the Makefile does) and compare each C function word by word
with the ROM at the function's address, relocated fields masked.

A function is placed by its func_XXXXXXXX name, by `func_XXXXXXXX = cname;` in
undefined_syms.us.txt, or by a runlink name (read from your ROM). Functions
whose address has an INCLUDE_ASM line in the file are skipped. Afterwards it
lists the address the ROM gives every symbol the object references
(SYMBOL MISMATCH = wrong extern) and warns when the object has its own
.data/.rodata/.bss. Needs the ROM and `make extract`.

The masks hide jal targets and %hi/%lo values: a call to the wrong function or
a wrong data offset still MATCHes. Run the full build (`make check`) after.
"""
import argparse
import os
import re
import subprocess
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import common as C  # noqa: E402


def sx(v):
    return v - 0x10000 if v & 0x8000 else v


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0],
                                 epilog="example: tools/match/probe.py src/gzip.c")
    ap.add_argument("file", help="the C file (main segment)")
    ap.add_argument("--show", action="store_true", help="disassemble every function that differs (both sides)")
    ap.add_argument("--tag", default="", help="own scratch directory build/match/probe-TAG (parallel runs)")
    ap.add_argument("--no-syms", action="store_true", help="skip the symbol address list")
    ap.usage = "%(prog)s [-h] [--show] [--tag TAG] [--no-syms] file [cc flags: -O1 -mips2 -DFOO ...]"
    args, cflags = ap.parse_known_args()

    src = os.path.abspath(args.file)
    rom = C.read_rom()
    addr = C.symbol_addrs()
    obj = os.path.join(C.workdir("probe", args.tag), "p.o")
    r = C.compile_game(src, obj, cflags)
    if r.returncode:
        print(r.stdout[-3000:], r.stderr[-3000:])
        sys.exit(1)
    text, relocs, dump = C.text_and_relocs(obj)
    syms = C.text_functions(dump)
    asm_addrs = {int(x, 16) for x in re.findall(r'^INCLUDE_ASM\("[^"]*", func_([0-9A-Fa-f]{8})\);',
                                                open(src).read(), re.M)}
    dis = C.disassembler() if args.show else None

    def place(name):
        mm = re.match(r"func_([0-9A-Fa-f]{8})$", name)
        return int(mm.group(1), 16) if mm else addr.get(name)

    nok = nbad = 0
    placed = {}  # object offset -> (address, size)
    for off, size, name in sorted(syms):
        a = place(name)
        if a is None:
            print("%-30s ?? no address" % name)
            continue
        if a in asm_addrs:
            continue
        placed[off] = (a, size)
        ro = C.main_rom_offset(a)
        bad = []
        for i in range(0, size, 4):
            mk = C.reloc_mask(relocs.get(off + i, ("", ""))[0])
            if (C.u32(text, off + i) & mk) != (C.u32(rom, ro + i) & mk):
                bad.append(i)
        if not bad:
            nok += 1
            print("%-30s MATCH (%d bytes)" % (name, size))
            continue
        nbad += 1
        print("%-30s DIFF %d/%d words, first at +0x%X" % (name, len(bad), size // 4, bad[0]))
        if dis:
            for i in range(0, size, 4):
                x, y = C.u32(text, off + i), C.u32(rom, ro + i)
                print("   +%04X %08X %08X %s %-34s | %-34s %s" % (
                    i, x, y, "*" if i in bad else " ", dis(x, a + i)[:34], dis(y, a + i)[:34],
                    relocs.get(off + i, ("", ""))[1]))
    print("match", nok, "diff", nbad)

    hd = subprocess.run([C.OBJDUMP, "-h", obj], capture_output=True, text=True).stdout
    for m in re.finditer(r"^\s+\d+ (\.rodata|\.data|\.bss|\.late_rodata)\s+([0-9a-f]{8})", hd, re.M):
        if int(m.group(2), 16):
            print("SECTION %s size 0x%X  <-- C object has its own data!" % (m.group(1), int(m.group(2), 16)))
    if args.no_syms:
        return

    # the address of each referenced symbol, from the ROM words at its relocations
    def romword(o_off):
        for off, (a, size) in placed.items():
            if off <= o_off < off + size:
                return C.u32(rom, C.main_rom_offset(a) + o_off - off)
        return None
    found, pend = {}, {}
    for o_off in sorted(relocs):
        typ, sym = relocs[o_off]
        w = romword(o_off)
        x = C.u32(text, o_off)
        if w is None:
            continue
        if typ == "R_MIPS_26":
            found.setdefault(sym, set()).add(0x80000000 | ((w & 0x3FFFFFF) << 2))
        elif typ == "R_MIPS_HI16":
            pend[sym] = (w & 0xFFFF, x & 0xFFFF)
        elif typ == "R_MIPS_LO16" and sym in pend:
            hi_r, hi_o = pend[sym]
            target = (hi_r << 16) + sx(w & 0xFFFF)
            addend = (hi_o << 16) + sx(x & 0xFFFF)
            found.setdefault(sym, set()).add((target - addend) & 0xFFFFFFFF)
    und = set(re.findall(r"^[0-9a-f]{8} .{7} \*UND\*\s+[0-9a-f]{8} (\S+)$",
                         subprocess.run([C.OBJDUMP, "-t", obj], capture_output=True, text=True).stdout, re.M))
    print("--- symbol addresses from the ROM (undefined in the object):")
    for s in sorted(found):
        v = found[s]
        tag = "" if s in und else "   (defined/local in object!)"
        mm = re.match(r"[DL]_([0-9A-Fa-f]{8})$", s) or re.match(r"func_([0-9A-Fa-f]{8})$", s)
        exp = int(mm.group(1), 16) if mm else addr.get(s)
        if exp is not None and s in und and any(a != exp for a in v):
            tag += "   <-- SYMBOL MISMATCH (expected 0x%08X)" % exp
        print("%s = %s;%s" % (s, " / ".join("0x%08X" % a for a in sorted(v)), tag))


if __name__ == "__main__":
    main()
