#!/usr/bin/env python3
"""The module probe: compile runlink-module C files (game flags plus
-I include/modules, as modules.mk does) and compare each function's text with
the module image in the ROM, relocated fields masked; then the object's .data
and .rodata (each padded to 16 bytes) with the module's data block.

A function named func_mNNN_XXXXX is compared at module offset XXXXX, any
other at its own object offset. Needs the ROM and `make extract` (it reads
asm/nonmatchings/modules/mNNN/functions.txt for the .rodata start).
"""
import argparse
import os
import re
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import common as C  # noqa: E402
sys.path.insert(0, C.TOOLS)
import runlink_build  # noqa: E402
import runlink_split  # noqa: E402

MOD_CFLAGS = C.GAME_CFLAGS + ["-I", "include/modules"]


def probe_file(f, m, me, rom, img, img_full, ent, outdir, verbose, dis):
    o = os.path.join(outdir, os.path.basename(f) + ".o")
    r = C.compile_game(f, o, cflags=MOD_CFLAGS)
    if r.returncode:
        print("== %s: compile failed\n%s%s" % (f, r.stdout[-3000:], r.stderr[-3000:]))
        return
    e = runlink_build.Elf(o)
    text, tsize, ti, _ = e.section(".text")
    mask = {}
    for off, typ, symi in e.rels(ti):
        mask[off] = C.reloc_mask(typ) if typ in (4, 5, 6) else 0
    funcs = sorted((s["value"], s["name"]) for s in e.syms
                   if s["shndx"] == ti and s["type"] == 2 and not s["name"].startswith("__"))
    bounds = [v for v, _ in funcs] + [tsize]
    res = []
    for i, (v, name) in enumerate(funcs):
        end = bounds[i + 1]
        base = int(name.rsplit("_", 1)[1], 16) if name.startswith("func_m%03d_" % me) else v
        bad = []
        for k in range(v, end, 4):
            a = C.u32(text, k)
            rr = base + k - v
            b = C.u32(img, rr) if rr + 4 <= len(img) else None
            mk = mask.get(k, 0xFFFFFFFF)
            if b is None or (a & mk) != (b & mk):
                bad.append(rr)
        res.append("%s %s" % (name, "MATCH" if not bad else "%d diff (first %X)" % (len(bad), bad[0])))
        if bad and verbose:
            for k in range(v, end, 4):
                a = C.u32(text, k)
                rr = base + k - v
                b = C.u32(img, rr) if rr + 4 <= len(img) else 0
                print("  %s %05X %-38s %s" % ("*" if rr in bad else " ", rr, dis(b, rr), dis(a, rr)))
    print("== %s (text 0x%X vs rom 0x%X): %s" % (f, tsize, ent[2], "; ".join(res)))

    # data block: the object's .data then .rodata (each 16-aligned) vs the ROM's
    ft = C.need(C.path("asm", "nonmatchings", "modules", m, "functions.txt"), "run `make extract`")
    rf = re.search(r"\.rodata from 0x([0-9A-Fa-f]+)", open(ft).readline())
    dstart = ent[2]
    rstart = int(rf.group(1), 16) if rf else dstart + ent[3]
    romsec = {".data": img_full[dstart:rstart], ".rodata": img_full[rstart:dstart + ent[3]]}
    if not rf and e.section(".rodata")[1] != 0:
        ds = (e.section(".data")[1] + 15) & ~15
        romsec = {".data": img_full[dstart:dstart + ds], ".rodata": img_full[dstart + ds:dstart + ent[3]]}
    for sname in (".data", ".rodata"):
        sd, ssz, si, _ = e.section(sname)
        want = romsec[sname]
        if si is None and not want:
            continue
        smask = {}
        if si is not None:
            for off, typ, symi in e.rels(si):
                smask[off] = 0
        pad = (len(sd) + 15) & ~15
        sd = sd + b"\0" * (pad - len(sd))
        bad = []
        for k in range(0, max(len(sd), len(want)), 4):
            a, b = sd[k:k + 4], want[k:k + 4]
            if len(a) < 4 or len(b) < 4:
                bad.append(k)
                continue
            mk = smask.get(k, 0xFFFFFFFF)
            if (struct.unpack(">I", a)[0] & mk) != (struct.unpack(">I", b)[0] & mk):
                bad.append(k)
        if bad:
            print("   %s: size 0x%X vs rom 0x%X, %d words differ, first at +0x%X (rom %s, C %s)" % (
                sname, len(sd), len(want), len(bad), bad[0], want[bad[0]:bad[0] + 4].hex(),
                sd[bad[0]:bad[0] + 4].hex()))
        else:
            print("   %s: MATCH (0x%X bytes)" % (sname, len(sd)))


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0],
                                 epilog="example: tools/match/modprobe.py m020 src/modules/m020.c")
    ap.add_argument("module", help="the module, mNNN")
    ap.add_argument("files", nargs="+", help="C file(s) for that module")
    ap.add_argument("-v", "--verbose", action="store_true", help="disassemble functions that differ (ROM | C)")
    ap.add_argument("--out", default=os.environ.get("PROBE_DIR"),
                    help="scratch directory (default $PROBE_DIR, else build/match/modprobe)")
    args = ap.parse_args()
    if not re.match(r"m\d{3}$", args.module):
        ap.error("module must look like m020")
    me = int(args.module[1:])
    outdir = args.out or C.workdir("modprobe")
    os.makedirs(outdir, exist_ok=True)
    outdir = os.path.abspath(outdir)
    L = runlink_split.LAYOUT[C.VERSION]
    rom = C.read_rom()
    ent = struct.unpack_from(">IIIIIHHii", rom, L["modtab"] + 32 * (me - 1))
    start = L["modules"] + ent[1]
    img = rom[start:start + ent[2]]
    img_full = rom[start:start + ent[2] + ent[3]]
    dis = C.disassembler(pseudos=False)
    for f in args.files:
        probe_file(os.path.abspath(f), args.module, me, rom, img, img_full, ent, outdir, args.verbose, dis)


if __name__ == "__main__":
    main()
