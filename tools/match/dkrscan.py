#!/usr/bin/env python3
"""Find DKR functions in JFG: compile every src/*.c of the Diddy Kong Racing
decompilation (a local clone, --dkr) with IDO 5.3 -O2 -mips1 and look for each
function anywhere in JFG's main segment (exact match, relocated fields
masked); a function whose "Official Name" is a JFG runlink name but does not
match exactly is listed as 'named' (see near.py for how close it is).

Writes the objects and build/match/dkr/cands.json (the good candidates).
The output can contain runlink names read from your ROM: never commit or
paste it. Needs the ROM and `make extract`.
"""
import argparse
import bisect
import collections
import glob
import json
import os
import re
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import common as C  # noqa: E402

OFFICIAL_RE = re.compile(r"Official [Nn]ame:\s*(\w+)[^\n]*\n(?:[^\n]*\n){0,3}?\s*(?:UNUSED\s+|static\s+)?"
                         r"[\w\s\*]+?\b(\w+)\s*\(")
DEFS = ["-D_LANGUAGE_C", "-DF3DDKR_GBI", "-D_MIPS_SZLONG=32", "-DNDEBUG", "-D_FINALROM", "-DVERSION_us_v77",
        "-DBUILD_VERSION=VERSION_I", "-DTARGET_N64", "-DANTI_TAMPER=1", "-DCIC_ID=6103"]


def outdir():
    return C.workdir("dkr")


def official_names(src):
    """DKR function name -> its 'Official Name' comment."""
    return {m.group(2): m.group(1) for m in OFFICIAL_RE.finditer(src)}


def write_asset_stub(dkr, out):
    """An asset_enums.h (generated in DKR's build) giving every ASSET_* identifier a value."""
    files = glob.glob(dkr + "/src/**/*.[ch]", recursive=True) + glob.glob(dkr + "/include/*.h")
    idents, tys, defined = set(), set(), set()
    for fn in files:
        t = open(fn, errors="ignore").read()
        idents |= set(re.findall(r"\b(ASSET_[A-Z0-9_]+)\b", t))
        tys |= set(re.findall(r"\b(Asset\w*Enum)\b", t))
    for h in glob.glob(dkr + "/include/*.h") + glob.glob(dkr + "/src/*.h"):
        t = open(h, errors="ignore").read()
        defined |= set(re.findall(r"\b(ASSET_[A-Z0-9_]+)\s*[,=]", t))
        defined |= set(re.findall(r"#define\s+(ASSET_[A-Z0-9_]+)", t))
    os.makedirs(out + "/inc", exist_ok=True)
    with open(out + "/inc/asset_enums.h", "w") as f:
        f.write("#ifndef ASSET_ENUMS_H\n#define ASSET_ENUMS_H\nenum { ASSET_ENUM_DUMMY_ = 0,\n")
        for i, n in enumerate(sorted(idents - defined)):
            f.write("  %s = %d,\n" % (n, i + 1))
        f.write("};\n")
        for t in sorted(tys):
            f.write("typedef int %s;\n" % t)
        f.write("#endif\n")


def masked_funcs(o):
    text, relocs, dump = C.text_and_relocs(o)
    res = []
    for off, size, name in C.text_functions(dump):
        if size < 8:
            continue
        ws, ms = [], []
        for i in range(off, off + size, 4):
            mk = C.reloc_mask(relocs.get(i, ("", ""))[0])
            ws.append(C.u32(text, i) & mk)
            ms.append(mk)
        while ws and ws[-1] == 0:
            ws.pop()
            ms.pop()
        res.append((name, off, size, ws, ms))
    return res


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0],
                                 epilog="example: tools/match/dkrscan.py --only math_util --only camera")
    ap.add_argument("--dkr", default=C.DKR_DEFAULT, help="DKR decomp clone (default %(default)s)")
    ap.add_argument("--only", action="append", default=[], metavar="TEXT",
                    help="only DKR src files whose name contains TEXT (repeatable)")
    args = ap.parse_args()
    dkr = os.path.abspath(args.dkr)
    if not os.path.isdir(dkr + "/src"):
        sys.exit("dkrscan: %s is not a DKR decomp clone (use --dkr)" % dkr)

    out = outdir()
    write_asset_stub(dkr, out)
    rom = C.read_rom()
    main_seg = rom[C.MAIN_ROM:C.MAIN_ROM + (C.MAIN_END - C.MAIN_VRAM)]
    mw = [C.u32(main_seg, i) for i in range(0, len(main_seg) - 3, 4)]
    addr = C.runlink_addrs()
    byaddr = {v: k for k, v in addr.items()}
    segs = []
    for m in re.finditer(r"\[0x([0-9A-Fa-f]+), (c|asm)(?:, (\w+))?\]", open(C.path("jfg.%s.yaml" % C.VERSION)).read()):
        o = int(m.group(1), 16)
        if C.MAIN_ROM <= o < C.main_rom_offset(C.MAIN_END):
            segs.append((o - C.MAIN_ROM + C.MAIN_VRAM, m.group(3) or "%X" % o))
    segs.sort()
    seg_starts = [s[0] for s in segs]

    def jfile(a):
        i = bisect.bisect_right(seg_starts, a) - 1
        return segs[i][1] if i >= 0 else "?"
    index = collections.defaultdict(list)
    for i, w in enumerate(mw):
        index[w].append(i)

    def find(ws, ms):
        if ms[0] == 0xFFFFFFFF:
            cands = index.get(ws[0], [])
        else:
            cands = [i for i in range(len(mw)) if (mw[i] & ms[0]) == ws[0]]
        return [C.MAIN_VRAM + 4 * i for i in cands
                if i + len(ws) <= len(mw) and all((mw[i + j] & ms[j]) == ws[j] for j in range(len(ws)))]

    inc = ["-I", out + "/inc", "-I", dkr]
    for d in ("include", "include/libc", "include/PR", "include/sys", "src", "libultra", "libultra/src/os",
              "libultra/src/io", "libultra/src/audio", "src/hasm", "build"):
        inc += ["-I", dkr + "/" + d]
    rows, cands = [], []
    summary = collections.OrderedDict()
    for c in sorted(glob.glob(dkr + "/src/*.c")):
        base = os.path.basename(c)
        if args.only and not any(t in base for t in args.only):
            continue
        offm = official_names(open(c, errors="ignore").read())
        o = out + "/" + base + ".o"
        r = subprocess.run([C.path(C.IDO_CC), "-c", "-G", "0", "-non_shared", "-Xcpluscomm", "-nostdinc",
                            "-Wab,-r4300_mul", "-O2", "-mips1", "-woff", "516,649,838,712,581"] + DEFS + inc +
                           ["-o", o, c], capture_output=True, text=True)
        if r.returncode != 0:
            print("COMPILE FAIL", base, [x for x in r.stderr.splitlines() if "Error" in x][:2])
            continue
        st = summary.setdefault(base, collections.Counter())
        for name, off, size, ws, ms in masked_funcs(o):
            st["funcs"] += 1
            hits = find(ws, ms) if ws else []
            off_name = offm.get(name)
            ja = addr.get(off_name) if off_name else None
            if hits:
                if (ja in hits) or (len(hits) == 1 and size >= 24):
                    h0 = ja if ja in hits else hits[0]
                    cands.append({"dkr_file": base, "dkr_func": name, "official": off_name, "addr": h0,
                                  "jfg_file": jfile(h0), "size": size})
                st["exact"] += 1
                h = hits[0]
                rows.append((base, name, off_name or "", size, "%08X" % h, jfile(h),
                             "EXACT" + ("" if len(hits) == 1 else "x%d" % len(hits)), byaddr.get(h, "")))
            elif ja:
                st["named_nomatch"] += 1
                rows.append((base, name, off_name, size, "%08X" % ja, jfile(ja), "named", ""))
    with open(out + "/cands.json", "w") as f:
        json.dump(cands, f, indent=1)
    byfile = collections.defaultdict(list)
    for c in cands:
        byfile[c["jfg_file"]].append(c)
    print("=== good candidates per JFG file")
    for f in sorted(byfile, key=lambda f: -len(byfile[f])):
        print("%-16s %3d  %s" % (f, len(byfile[f]), " ".join("%s@%08X" % (c["dkr_func"], c["addr"])
                                                           for c in byfile[f])))
    print()
    for r in rows:
        print("%-20s %-36s %-28s %5d %s %-6s %-6s %s" % r)
    print()
    for k, v in summary.items():
        print("%-22s funcs=%3d exact=%3d named_nomatch=%3d" % (k, v["funcs"], v["exact"], v["named_nomatch"]))


if __name__ == "__main__":
    main()
