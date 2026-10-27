#!/usr/bin/env python3
"""Probe many variants of one function: each variant replaces FUNC's
definition (or its INCLUDE_ASM line) in a scratch copy of SRC, which is run
through probe.py; one result line per variant.

VARIANTS has blocks headed '=== label'. A block's body is one of
  - a full C definition (with any declarations it needs before it);
  - replacement lines 'old ||| new' (every non-blank line has ' ||| '),
    applied once each, in order, to FUNC's current definition ('\\n' in
    old/new is a newline);
  - nothing: FUNC as it is in SRC (a baseline).
An '=== PRE' block holds lines 'old --- new' (or 'old ||| new') applied to
the whole of SRC before every variant. permdecl.py writes VARIANTS files.
SRC itself is never changed.
"""
import argparse
import os
import re
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import common as C  # noqa: E402


def find_def(src, name):
    """(start, end) of NAME's definition in SRC (with a comment right above it),
    or of its INCLUDE_ASM line; None if neither is there."""
    m = re.search(r"^(?![ \t])[^\n;{}]*\b%s\s*\([^;{]*\)\s*\{" % re.escape(name), src, re.M)
    if not m:
        m2 = re.search(r'^INCLUDE_ASM\("[^"]+", %s\);\n' % re.escape(name), src, re.M)
        return (m2.start(), m2.end()) if m2 else None
    start = m.start()
    cm = re.search(r"(/\*(?:(?!\*/).)*\*/\n)$", src[:start], re.S)
    if cm:
        start = cm.start()
    i = m.end()
    depth = 1
    while depth:
        c = src[i]
        if c == "{":
            depth += 1
        elif c == "}":
            depth -= 1
        i += 1
    if src[i:i + 1] == "\n":
        i += 1
    return start, i


def split_pair(line, seps):
    for s in seps:
        if s in line:
            a, b = line.split(s, 1)
            return a.replace("\\n", "\n"), b.replace("\\n", "\n")
    return None


def read_variants(path):
    """([(old, new) PRE replacements], [(label, kind, body)]) with kind
    'def', 'rep' (body = list of (old, new)) or 'base'."""
    pre, variants = [], []
    for b in re.split(r"^=== ", open(path).read(), flags=re.M):
        if not b.strip():
            continue
        head, _, body = b.partition("\n")
        label = head.strip()
        lines = [x for x in body.splitlines() if x.strip()]
        if label == "PRE":
            pre += [p for p in (split_pair(x, (" --- ", " ||| ")) for x in lines) if p]
        elif not lines:
            variants.append((label, "base", None))
        elif all(" ||| " in x for x in lines):
            variants.append((label, "rep", [split_pair(x, (" ||| ",)) for x in lines]))
        else:
            variants.append((label, "def", body))
    return pre, variants


def apply_variant(src, func, kind, body):
    """SRC with FUNC replaced as the variant says, or an error string."""
    rng = find_def(src, func)
    if rng is None:
        return None, "FUNC NOT FOUND"
    if kind == "base":
        return src, None
    if kind == "rep":
        fn = src[rng[0]:rng[1]]
        for a, c in body:
            if a not in fn:
                return None, "NOT FOUND: %r" % a[:60]
            fn = fn.replace(a, c, 1)
        return src[:rng[0]] + fn + src[rng[1]:], None
    return src[:rng[0]] + body.rstrip("\n") + "\n" + src[rng[1]:], None


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0],
                                 epilog="example: tools/match/var.py src/gzip.c func_80001234 vars.txt --stop")
    ap.add_argument("src", help="the C file")
    ap.add_argument("func", help="the function (C name or func_XXXXXXXX)")
    ap.add_argument("variants", help="the variants file")
    ap.add_argument("--name", help="probe line to report (default FUNC, else every DIFF line)")
    ap.add_argument("--show", metavar="LABEL", help="print the full probe output (with disassembly) for LABEL")
    ap.add_argument("--stop", action="store_true", help="stop at the first variant that MATCHes")
    ap.add_argument("--tag", default="", help="own scratch directories (parallel runs)")
    ap.usage = "%(prog)s [-h] [--name N] [--show LABEL] [--stop] [--tag T] src func variants [cc flags]"
    args, cflags = ap.parse_known_args()

    srcp = os.path.abspath(args.src)
    src0 = open(srcp).read()
    pre, variants = read_variants(args.variants)
    for a, c in pre:
        src0 = src0.replace(a, c)
    tmp = os.path.join(C.workdir("var", args.tag), os.path.basename(srcp))
    want = args.name or args.func
    try:
        for label, kind, body in variants:
            src, err = apply_variant(src0, args.func, kind, body)
            if err:
                print("%-12s %s" % (label, err))
                continue
            with open(tmp, "w") as f:
                f.write(src)
            show = args.show == label
            cmd = [sys.executable, os.path.join(HERE, "probe.py"), "--tag", args.tag or "var", "--no-syms"]
            cmd += ["--show"] if show else []
            r = subprocess.run(cmd + [tmp] + cflags, capture_output=True, text=True, cwd=C.ROOT)
            out = r.stdout + r.stderr
            lines = [x for x in out.splitlines() if " DIFF " in x or " MATCH (" in x]
            sel = [x for x in lines if x.split()[0] == want] or [x for x in lines if " DIFF " in x]
            if r.returncode:
                print("%-12s COMPILE FAIL %s" % (label, out[-600:].replace("\n", " | ")))
            else:
                print("%-12s %s" % (label, " ; ".join(sel) if sel else "ALL MATCH"))
            if show:
                print(out)
            sys.stdout.flush()
            if args.stop and sel and " MATCH (" in sel[0]:
                break
    finally:
        if os.path.exists(tmp):
            os.remove(tmp)


if __name__ == "__main__":
    main()
