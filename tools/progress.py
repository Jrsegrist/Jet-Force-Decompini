#!/usr/bin/env python3
"""Decompilation progress: functions (and bytes) built from C, per segment.

A function counts as decompiled when it is compiled from C: a function in
src/ that is not pulled in with INCLUDE_ASM, or an ultralib object. Everything
assembled from asm/ (or INCLUDE_ASM'd) counts as not decompiled, including
hand-written assembly. Reads build/jfg.us.elf and build/jfg.us.map.

Usage: tools/progress.py [--version us] [--files]
"""
import argparse
import collections
import glob
import os
import re
import subprocess
import sys

# main segment regions by VRAM (ROM 0x84E00 = VRAM 0x80084200: libraries)
REGIONS = [
    ("main: game", 0x80000400, 0x80084200),
    ("main: libraries", 0x80084200, 0x800A0660),
]


def functions(elf):
    """(addr, size, name) of every function symbol in the ELF."""
    out = subprocess.run(["mips-linux-gnu-objdump", "-t", elf], capture_output=True,
                         text=True, check=True).stdout
    funcs = {}
    for m in re.finditer(r"^([0-9a-f]{8}) (.{7}) (\S+)\s+([0-9a-f]{8}) (\S+)$", out, re.M):
        addr, flags, sect, size, name = int(m.group(1), 16), m.group(2), m.group(3), int(m.group(4), 16), m.group(5)
        if "F" not in flags or name.startswith(".") or name.endswith(".NON_MATCHING"):
            continue
        if size == 0:
            continue  # a label inside another function (secondary entry)
        # keep one name per address (the larger size wins for aliases)
        if addr not in funcs or size > funcs[addr][0]:
            funcs[addr] = (size, name)
    return funcs


def text_objects(mapfile):
    """[(start, end, object path)] for every .text input section in the map."""
    objs = []
    text = open(mapfile).read()
    for m in re.finditer(r"^ \.text\s+0x([0-9a-f]+)\s+0x([0-9a-f]+) (\S+)$", text, re.M):
        start, size, obj = int(m.group(1), 16), int(m.group(2), 16), m.group(3)
        if size:
            objs.append((start, start + size, obj))
    return sorted(objs)


def include_asm_addrs(src):
    t = open(src).read()
    t = re.sub(r"/\*.*?\*/", "", t, flags=re.S)
    t = re.sub(r"//[^\n]*", "", t)
    return {int(a, 16) for a in re.findall(r'^\s*INCLUDE_ASM\(\s*"[^"]*"\s*,\s*func_([0-9A-Fa-f]{8})\s*\)', t, re.M)}


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--version", default="us")
    ap.add_argument("--files", action="store_true", help="also list each file")
    args = ap.parse_args()
    elf = "build/jfg.%s.elf" % args.version
    mapfile = "build/jfg.%s.map" % args.version
    if not os.path.exists(elf):
        sys.exit("build first (make)")

    funcs = functions(elf)
    objs = text_objects(mapfile)
    asm_cache = {}

    def classify(addr):
        for start, end, obj in objs:
            if start <= addr < end:
                if obj.startswith("build/src/"):
                    src = obj[len("build/"):-2] + ".c"
                    if not os.path.exists(src):
                        return obj, False
                    if src not in asm_cache:
                        asm_cache[src] = include_asm_addrs(src)
                    return obj, addr not in asm_cache[src]
                return obj, False
        return None, False

    totals = collections.OrderedDict((r[0], [0, 0, 0, 0]) for r in REGIONS)
    per_file = collections.defaultdict(lambda: [0, 0, 0, 0])
    for addr, (size, name) in sorted(funcs.items()):
        region = next((r[0] for r in REGIONS if r[1] <= addr < r[2]), None)
        if region is None:
            continue
        obj, is_c = classify(addr)
        t = totals[region]
        f = per_file[obj]
        for row in (t, f):
            row[0] += 1
            row[1] += size
            if is_c:
                row[2] += 1
                row[3] += size

    def pct(a, b):
        return 100.0 * a / b if b else 0.0

    print("%-18s %16s %22s" % ("segment", "functions in C", "bytes in C"))
    all_ = [0, 0, 0, 0]
    for region, (n, sz, nc, szc) in totals.items():
        print("%-18s %6d/%-5d %5.1f%% %8d/%-7d %5.1f%%" % (region, nc, n, pct(nc, n), szc, sz, pct(szc, sz)))
        all_ = [a + b for a, b in zip(all_, (n, sz, nc, szc))]
    n, sz, nc, szc = all_
    print("%-18s %6d/%-5d %5.1f%% %8d/%-7d %5.1f%%" % ("main: total", nc, n, pct(nc, n), szc, sz, pct(szc, sz)))
    # runlink modules: all assembly so far; count functions by their returns
    mods = sorted(glob.glob("asm/modules/m*.s"))
    if mods:
        nret = tbytes = 0
        for p in mods:
            t = open(p).read()
            text = t.split(".section .data")[0]
            nret += len(re.findall(r"\bjr\s+\$ra\b", text))
            tbytes += 4 * len(re.findall(r"^\s+/\* [0-9A-F]{5} [0-9A-F]{8} \*/", text, re.M))
        print("%-18s %6d/%-5d %5.1f%% %8d/%-7d %5.1f%%   (%d modules; functions counted by jr $ra)" % (
            "runlink modules", 0, nret, 0.0, 0, tbytes, 0.0, len(mods)))
    if args.files:
        print()
        for obj, (n, sz, nc, szc) in sorted(per_file.items(), key=lambda kv: kv[0] or ""):
            print("%-40s %4d/%-4d %7d/%-7d" % (obj, nc, n, szc, sz))


if __name__ == "__main__":
    main()
