#!/usr/bin/env python3
"""How close is DKR? For each DKR function whose "Official Name" is a JFG
runlink name at an address that is still INCLUDE_ASM in src/, compare DKR's
compiled code (the objects dkrscan.py left in build/match/dkr/) with the ROM
there: masked word differences plus the size difference, lowest score first.

Run dkrscan.py first. The output contains runlink names read from your ROM:
never commit or paste it. Needs the ROM and `make extract`.
"""
import argparse
import glob
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import common as C  # noqa: E402
from dkrscan import official_names, outdir  # noqa: E402


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0],
                                 epilog="example: tools/match/near.py --max 50")
    ap.add_argument("--dkr", default=C.DKR_DEFAULT, help="DKR decomp clone (default %(default)s)")
    ap.add_argument("--only", action="append", default=[], metavar="TEXT",
                    help="only DKR src files whose name contains TEXT (repeatable)")
    ap.add_argument("--max", type=int, help="only rows with score <= MAX")
    args = ap.parse_args()
    dkr = os.path.abspath(args.dkr)
    if not os.path.isdir(dkr + "/src"):
        sys.exit("near: %s is not a DKR decomp clone (use --dkr)" % dkr)

    rom = C.read_rom()
    addr = C.runlink_addrs()
    jsize = {}
    for p in glob.glob(C.path("asm", "nonmatchings", "*", "func_*.s")):
        m = re.search(r"nonmatching \S+, (0x[0-9A-Fa-f]+)", open(p).read())
        if m:
            jsize[int(os.path.basename(p)[5:13], 16)] = int(m.group(1), 16)
    csrc = {}
    for f in glob.glob(C.path("src", "**", "*.c"), recursive=True):
        for a in re.findall(r'INCLUDE_ASM\("[^"]+", func_([0-9A-Fa-f]{8})\)', open(f).read()):
            csrc[int(a, 16)] = f
    out = outdir()
    rows = []
    nobj = 0
    for c in sorted(glob.glob(dkr + "/src/*.c")):
        base = os.path.basename(c)
        if args.only and not any(t in base for t in args.only):
            continue
        o = out + "/" + base + ".o"
        if not os.path.exists(o):
            continue
        nobj += 1
        offm = official_names(open(c, errors="ignore").read())
        text, relocs, dump = C.text_and_relocs(o)
        for off, size, name in C.text_functions(dump):
            on = offm.get(name)
            if not on or on not in addr:
                continue
            a = addr[on]
            if a not in csrc:
                continue  # already C, or not in a C file
            js = jsize.get(a, 0)
            ro = C.main_rom_offset(a)
            bad = 0
            for i in range(0, min(size, js) // 4 * 4, 4):
                mk = C.reloc_mask(relocs.get(off + i, ("", ""))[0])
                if (C.u32(text, off + i) & mk) != (C.u32(rom, ro + i) & mk):
                    bad += 1
            score = bad + abs(size - js) // 4
            if args.max is None or score <= args.max:
                rows.append((score, base, name, on, size, js, bad, os.path.relpath(csrc[a], C.ROOT)))
    if not nobj:
        sys.exit("near: no DKR objects in %s (run dkrscan.py first)" % os.path.relpath(out, C.ROOT))
    for r in sorted(rows):
        print("score %4d %-18s %-34s %-26s dkr %5d jfg %5d bad %4d  %s" % r)


if __name__ == "__main__":
    main()
