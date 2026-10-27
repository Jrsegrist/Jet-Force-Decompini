#!/usr/bin/env python3
"""Like var.py, but each variant goes through the full build (`make`, then
`make check`), which catches what the probe masks: %hi/%lo offsets, .bss
order, jal targets. The variant is written into SRC itself; SRC is restored
afterwards (also on error or Ctrl-C). Optionally disassembles START..END of
the linked ELF for each variant. Needs the ROM and `make extract`.
"""
import argparse
import os
import re
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import common as C  # noqa: E402
from var import apply_variant, read_variants  # noqa: E402


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0],
                                 epilog="example: tools/match/buildvar.py src/gzip.c func_80001234 vars.txt "
                                        "0x80001234 0x80001300")
    ap.add_argument("src", help="the C file (in this checkout)")
    ap.add_argument("func", help="the function (C name or func_XXXXXXXX)")
    ap.add_argument("variants", help="the variants file (var.py format)")
    ap.add_argument("start", nargs="?", help="disassemble from this address ...")
    ap.add_argument("end", nargs="?", help="... to this one")
    ap.add_argument("-j", "--jobs", default=str(os.cpu_count() or 4), help="make -j (default: CPU count)")
    args = ap.parse_args()

    path = os.path.abspath(args.src)
    if not path.startswith(C.ROOT + os.sep):
        ap.error("SRC must be inside this checkout (%s)" % C.ROOT)
    with open(path) as f:
        src0 = f.read()
    pre, variants = read_variants(args.variants)
    base = src0
    for a, c in pre:
        base = base.replace(a, c)
    try:
        for label, kind, body in variants:
            src, err = apply_variant(base, args.func, kind, body)
            if err:
                print("=====", label, err)
                continue
            with open(path, "w") as f:
                f.write(src)
            mk = subprocess.run(["make", "-j" + args.jobs], capture_output=True, text=True, cwd=C.ROOT)
            # (the default target may run the check too: its failure is reported below)
            errs = [x for x in (mk.stdout + mk.stderr).splitlines()
                    if "error" in x.lower() and "check]" not in x][:5]
            ck = subprocess.run(["make", "check"], capture_output=True, text=True, cwd=C.ROOT)
            out = (ck.stdout + ck.stderr).strip().splitlines() or [""]
            status = [x for x in out if re.search(r": (OK|FAILED)$", x)] or out[-1:]
            print("=====", label, " | ".join(errs + status[:1]))
            if args.start and args.end:
                d = subprocess.run([C.OBJDUMP, "-d", "--start-address=" + args.start, "--stop-address=" + args.end,
                                    C.path("build", "jfg.%s.elf" % C.VERSION)], capture_output=True, text=True).stdout
                print("\n".join(x for x in d.splitlines() if re.match(r"\s*[0-9a-f]{8}:", x)))
            sys.stdout.flush()
    finally:
        with open(path, "w") as f:
            f.write(src0)


if __name__ == "__main__":
    main()
