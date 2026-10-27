#!/usr/bin/env python3
"""Write placeholder assembly for every INCLUDE_ASM / INCLUDE_RODATA in src/.

Without the ROM there is no asm/ directory, so asm-processor has nothing to
put where a C file says INCLUDE_ASM. For the ROM-free compile check (CI),
this writes a stand-in for each file those lines name:

  func_XXXXXXXX (any function)   glabel, `jr $ra; nop`, endlabel
  data / rodata / bss (a module's sections)   one word in that section
  INCLUDE_RODATA D_XXXXXXXX      dlabel and one word (.rodata)

The C around them is then compiled by the real Makefile rules (IDO 5.3 with
the project's flags, through asm-processor). The objects are only good for
checking that the C compiles: they are not linked and match nothing.

It refuses to run when asm/ already exists (a tree with the real, extracted
assembly) unless that asm/ was written by this script.

Usage: tools/ci_stub_asm.py [--src src] [--root .]
"""
import argparse
import os
import re
import sys

INCLUDE_RE = re.compile(r'^\s*INCLUDE_(ASM|RODATA)\(\s*"([^"]+)"\s*,\s*(\w+)\s*\);', re.M)
MARKER = "asm/CI_STUBS"


def stub(kind, name):
    if kind == "RODATA":
        return "dlabel %s\n    .word 0\nenddlabel %s\n" % (name, name)
    if name in ("data", "rodata"):
        return ".section .%s\n    .word 0\n" % name
    if name == "bss":
        return ".section .bss\n    .space 4\n"
    return "glabel %s\n    jr $ra\n    nop\nendlabel %s\n" % (name, name)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--src", default="src")
    ap.add_argument("--root", default=".")
    args = ap.parse_args()
    asm = os.path.join(args.root, "asm")
    marker = os.path.join(args.root, MARKER)
    if os.path.exists(asm) and not os.path.exists(marker):
        sys.exit("ci_stub_asm: %s exists and is not a stub tree; run this in a clean checkout "
                 "(no ROM, no make extract)" % asm)
    os.makedirs(asm, exist_ok=True)
    open(marker, "w").write("placeholder assembly from tools/ci_stub_asm.py, not the game's\n")
    n = 0
    for d, _, fs in os.walk(args.src):
        for f in sorted(fs):
            if not f.endswith(".c"):
                continue
            for kind, folder, name in INCLUDE_RE.findall(open(os.path.join(d, f)).read()):
                if not folder.startswith("asm/"):
                    sys.exit("ci_stub_asm: %s/%s: INCLUDE_%s outside asm/: %s" % (d, f, kind, folder))
                path = os.path.join(args.root, folder, name + ".s")
                os.makedirs(os.path.dirname(path), exist_ok=True)
                with open(path, "w") as out:
                    out.write(stub(kind, name))
                n += 1
    print("ci_stub_asm: %d placeholder files under %s" % (n, asm))


if __name__ == "__main__":
    main()
