#!/usr/bin/env python3
"""Look for library code among the main segment's remaining asm functions:
compile every lib/ultralib/src/*/*.c for each SDK version (I/J/K/L) x
-O1/-O2/-O3 x -mips2/-mips1, and report every object whose whole .text
(relocated fields masked, trailing padding dropped) appears in the ROM at the
start of a function that is still asm (asm/nonmatchings/*/func_*.s).

Objects are cached in build/match/ulgame/ (delete it after changing
ultralib). Needs the ROM and `make extract`.
"""
import argparse
import glob
import os
import re
import subprocess
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import common as C  # noqa: E402

UL = C.path("lib", "ultralib")


def ul_compile(c, d, v, opt, mp, o):
    cmd = [C.path(C.IDO_CC), "-c", "-Wab,-r4300_mul", "-G", "0", "-nostdinc", "-Xcpluscomm",
           "-woff", "516,649,838,712", "-D_MIPS_SZLONG=32", "-DBUILD_VERSION=VERSION_%s" % v,
           '-DBUILD_VERSION_STRING="2.0%s"' % v, "-DNDEBUG", "-D_FINALROM", "-non_shared", "-DF3DEX_GBI_2",
           "-I", UL + "/include", "-I", UL + "/include/compiler/ido", "-I", UL + "/include/PR",
           "-I", "%s/src/%s" % (UL, d), "-I", UL, opt, mp, "-o32", "-o", o, c]
    return subprocess.run(cmd, capture_output=True).returncode == 0


def masked_words(o):
    if not os.path.exists(o + ".rel"):
        dump = subprocess.run([C.OBJDUMP, "-r", "-j", ".text", o], capture_output=True, text=True).stdout
        with open(o + ".rel", "w") as f:
            f.write(dump)
        subprocess.run([C.OBJCOPY, "-O", "binary", "-j", ".text", o, o + ".text"])
    if not os.path.exists(o + ".text"):
        return None, 0
    relocs = {int(m.group(1), 16): m.group(2)
              for m in re.finditer(r"^([0-9a-f]{8}) (R_MIPS_\w+)", open(o + ".rel").read(), re.M)}
    text = open(o + ".text", "rb").read()
    tl = len(text)
    while tl >= 4 and text[tl - 4:tl] == b"\0\0\0\0":
        tl -= 4
    ws = []
    for j in range(tl // 4):
        mk = C.reloc_mask(relocs.get(4 * j))
        ws.append((C.u32(text, 4 * j) & mk, mk))
    return ws, len(text)


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0],
                                 epilog="example: tools/match/ulgame.py --only io/ --opt 2 --mips 1")
    ap.add_argument("--only", action="append", default=[], metavar="TEXT",
                    help="only ultralib files whose dir/name contains TEXT (repeatable), e.g. io/pimgr")
    ap.add_argument("--versions", default="I,J,K,L", help="SDK versions (default I,J,K,L)")
    ap.add_argument("--opt", default="1,2,3", help="-O levels (default 1,2,3)")
    ap.add_argument("--mips", default="2,1", help="-mips levels (default 2,1)")
    ap.add_argument("--start", action="append", default=[], metavar="ADDR",
                    help="only these function starts (hex, repeatable; default every asm function)")
    args = ap.parse_args()

    rom = C.read_rom()
    main_seg = rom[C.MAIN_ROM:C.MAIN_ROM + (C.MAIN_END - C.MAIN_VRAM)]
    mw = struct.unpack(">%dI" % (len(main_seg) // 4), main_seg)
    if args.start:
        starts = {int(a, 16) for a in args.start}
    else:
        starts = {int(os.path.basename(p)[5:13], 16)
                  for p in glob.glob(C.path("asm", "nonmatchings", "*", "func_*.s"))}
        if not starts:
            sys.exit("ulgame: no asm/nonmatchings/*/func_*.s (run `make extract`)")
    print(len(starts), "starts")
    combos = [(v, "-O" + o.lstrip("-O"), "-mips" + mp.lstrip("-mips"))
              for v in args.versions.split(",") for o in args.opt.split(",") for mp in args.mips.split(",")]
    out = C.workdir("ulgame")
    seen = set()
    for c in sorted(glob.glob("%s/src/*/*.c" % UL)):
        d, f = os.path.basename(os.path.dirname(c)), os.path.basename(c)[:-2]
        key = "%s/%s" % (d, f)
        if args.only and not any(t in key for t in args.only):
            continue
        for v, opt, mp in combos:
            od = "%s/%s%s%s" % (out, v, opt[1:], mp[1:])
            os.makedirs(od, exist_ok=True)
            o = "%s/%s_%s.o" % (od, d, f)
            if not os.path.exists(o) and not os.path.exists(o + ".fail"):
                if not ul_compile(c, d, v, opt, mp, o):
                    open(o + ".fail", "w").close()
                    continue
            if not os.path.exists(o):
                continue
            ws, tsize = masked_words(o)
            if not ws or len(ws) < 2:
                continue
            n = len(ws)
            for a in sorted(starts):
                i = (a - C.MAIN_VRAM) // 4
                if i < 0 or i + n > len(mw):
                    continue
                if all((mw[i + j] & ws[j][1]) == ws[j][0] for j in range(n)):
                    if (key, a) in seen:
                        break
                    seen.add((key, a))
                    hd = subprocess.run([C.OBJDUMP, "-h", o], capture_output=True, text=True).stdout
                    secs = {m.group(1): int(m.group(2), 16)
                            for m in re.finditer(r"^\s+\d+ (\.\w+)\s+([0-9a-f]{8})", hd, re.M)}
                    print("%08X rom %05X %-24s %s %s %s text %X data %X ro %X bss %X" % (
                        a, C.main_rom_offset(a), key, v, opt, mp, tsize,
                        secs.get(".data", 0), secs.get(".rodata", 0), secs.get(".bss", 0)))
                    sys.stdout.flush()
                    break
    print("done")


if __name__ == "__main__":
    main()
