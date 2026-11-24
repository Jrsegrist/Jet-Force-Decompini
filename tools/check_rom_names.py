#!/usr/bin/env python3
"""Check that no generated file holds a name from the ROM's runlink name table.

The ROM's name table is never used, not even as a research aid: the
assembly, the per-function files, the symbol files and everything else that
`make extract` and `make` write name symbols by address (func_XXXXXXXX,
D_XXXXXXXX, func_mNNN_XXXXX, rl_mNNN_XXXXX, jtbl_...) or by a name from the
project's own licensed sources. This reads the name table from your ROM when
it runs and looks for every whole-word occurrence of one of its names in the
text files under asm/, build/ and src/ (binary files, such as objects and
ROM images, are skipped). A name is allowed when it also occurs in
undefined_syms.us.txt, names.txt, symbol_addrs.us.txt, lib/ultralib or the
DKR decompilation's src/ and include/ (--dkr).

It prints only counts and file paths, never a name, and exits 1 on any hit.
`make check` runs it when a DKR clone is at DKR_DIR (default ~/jfg_refs/dkr).

    tools/check_rom_names.py --dkr PATH_TO_DKR_CLONE [DIR ...]
"""
import argparse
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import runlink_syms  # noqa: E402

ROOT = os.path.dirname(HERE)
ADDR_NAME = re.compile(r"(?:func|D|rl)_(?:[0-9A-Fa-f]{8}|m\d{3}_[0-9A-Fa-f]{5})$|jtbl_\w+$")
LICENSED = ["undefined_syms.us.txt", "names.txt", "symbol_addrs.us.txt", "lib/ultralib"]
SCAN = ["asm", "build", "src"]


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("--dkr", required=True, help="a clone of the DKR decompilation")
    ap.add_argument("--version", default="us")
    ap.add_argument("dirs", nargs="*", default=SCAN, help="where to look (default: %s)" % " ".join(SCAN))
    args = ap.parse_args()
    os.chdir(ROOT)

    rom = "baserom.%s.z64" % args.version
    if not os.path.exists(rom):
        sys.exit("check_rom_names: %s is missing" % rom)
    if not os.path.isdir(os.path.join(args.dkr, "src")):
        sys.exit("check_rom_names: %s does not look like the DKR decompilation" % args.dkr)
    names = runlink_syms.rom_names(rom, args.version)
    allowed = runlink_syms.words_of([p for p in LICENSED if os.path.exists(p)] +
                                    [os.path.join(args.dkr, "src"), os.path.join(args.dkr, "include")])
    bad = set(n for n in names if n not in allowed and not ADDR_NAME.match(n))

    nfiles = nskipped = 0
    hits = {}  # path -> (distinct names, occurrences)
    for base in args.dirs:
        for d, sub, fs in os.walk(base):
            sub[:] = [s for s in sub if s != ".git"]
            for f in fs:
                p = os.path.join(d, f)
                try:
                    with open(p, "rb") as fh:
                        data = fh.read()
                except OSError:
                    continue
                if b"\0" in data:
                    nskipped += 1
                    continue
                nfiles += 1
                ws = [w for w in runlink_syms.WORD.findall(data.decode("latin-1")) if w in bad]
                if ws:
                    hits[p] = (len(set(ws)), len(ws))
    print("check_rom_names: %d names in the ROM's table, %d of them only there; %d text files scanned "
          "in %s (%d binary files skipped)" % (len(names), len(bad), nfiles, " ".join(args.dirs), nskipped))
    if hits:
        print("check_rom_names: %d occurrence(s) in %d file(s) (paths only):" % (
            sum(o for _, o in hits.values()), len(hits)))
        for p in sorted(hits):
            print("  %s: %d name(s), %d occurrence(s)" % ((p,) + hits[p]))
        sys.exit(1)
    print("check_rom_names: OK, 0 occurrences")


if __name__ == "__main__":
    main()
