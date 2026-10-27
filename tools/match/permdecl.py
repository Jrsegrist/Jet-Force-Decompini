#!/usr/bin/env python3
"""Write a var.py variants file with one variant per ordering of a set of
local declarations: TEMPLATE holds a '=== TEMPLATE' block (the function with
the slot @1@ where the declarations go); each variant puts the declarations,
one per line, in the slot in another order. Other text is kept as it is.
n declarations give n! variants (keep n small).
"""
import argparse
import itertools
import sys


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0],
                                 epilog="example: tools/match/permdecl.py tmpl.txt vars.txt 's32 i; s32 j; f32 x'")
    ap.add_argument("template", help="file with a '=== TEMPLATE' block containing @1@")
    ap.add_argument("out", help="variants file to write (for var.py / buildvar.py)")
    ap.add_argument("decls", nargs="+", help="declarations separated by ';' (one or more arguments)")
    ap.add_argument("--indent", default="    ", help="indent of each declaration (default 4 spaces)")
    args = ap.parse_args()

    decls = [d.strip() for d in " ".join(args.decls).split(";") if d.strip()]
    t = open(args.template).read()
    if "=== TEMPLATE\n" not in t:
        sys.exit("permdecl: %s has no '=== TEMPLATE' block" % args.template)
    body = t.split("=== TEMPLATE\n", 1)[1].split("\n=== ", 1)[0]
    if "@1@" not in body:
        sys.exit("permdecl: the template has no @1@ slot")
    res = []
    for n, p in enumerate(itertools.permutations(decls)):
        s = body.replace("@1@", "\n".join("%s%s;" % (args.indent, d) for d in p))
        res.append("=== p%02d\n%s\n" % (n, s.rstrip("\n")))
    with open(args.out, "w") as f:
        f.write("".join(res))
    print(len(res), "variants")


if __name__ == "__main__":
    main()
