#!/usr/bin/env python3
"""Local pre-push check: no name that exists only in the ROM is committed.

The ROM carries about 2,400 symbol names for its run-time module linker.
They are read from your ROM at extract time (build/runlink/symbols.txt,
build/syms/runlink_syms.us.txt) and must never be committed: committed code
names things by address, by a name from the DKR decompilation (CC0), by a
libultra name, or by a descriptive name of our own. CI cannot run this check
(it has no ROM, so it does not know the names); run it before every push,
after `make extract`:

    tools/check_names.py --dkr PATH_TO_DKR_CLONE [--since REV] [--allow NAME ...]

It reads every word of
  - every tracked file (the work tree),
  - every commit message and every added line of every commit (all of
    `git log`, or only REV..HEAD with --since), since pushed history is
    public too,
and fails when one of them is a runlink name that neither the DKR
decompilation's source (--dkr, a clone of DavidSM64/Diddy-Kong-Racing) nor
lib/ultralib contains. The names it finds are printed: only on your own
terminal, never commit or paste them anywhere.
"""
import argparse
import os
import re
import subprocess
import sys

WORD = re.compile(r"[A-Za-z_]\w+")
REF_EXT = (".c", ".h", ".s", ".txt", ".inc", ".ld", ".md", ".py", ".yaml")


def rom_names(version):
    names = set()
    p = "build/runlink/symbols.txt"
    if os.path.exists(p):
        for line in open(p):
            parts = line.split()
            if len(parts) == 4:
                names.add(parts[3])
    p = "build/syms/runlink_syms.%s.txt" % version
    if os.path.exists(p):
        for line in open(p):
            m = re.match(r"(?://\s*alias:\s*)?(\w+) = ", line)
            if m:
                names.add(m.group(1))
    return names


def ref_words(dirs):
    words = set()
    for base in dirs:
        for d, sub, fs in os.walk(base):
            sub[:] = [s for s in sub if s != ".git"]
            for f in fs:
                if f.endswith(REF_EXT):
                    try:
                        with open(os.path.join(d, f), errors="ignore") as fh:
                            words |= set(WORD.findall(fh.read()))
                    except OSError:
                        pass
    return words


def git(*args):
    return subprocess.run(["git"] + list(args), capture_output=True, text=True, errors="ignore",
                          check=True).stdout


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--dkr", required=True, help="a clone of the DKR decompilation")
    ap.add_argument("--since", help="only check history after this revision (default: all)")
    ap.add_argument("--version", default="us")
    ap.add_argument("--allow", action="append", default=[], metavar="NAME",
                    help="a name the user has approved although DKR and ultralib lack it")
    args = ap.parse_args()

    names = rom_names(args.version)
    if len(names) < 100:
        sys.exit("check_names: no ROM names found; run `make extract` first")
    if not os.path.isdir(os.path.join(args.dkr, "src")):
        sys.exit("check_names: %s does not look like the DKR decompilation" % args.dkr)
    allowed = ref_words([args.dkr, "lib/ultralib"]) | set(args.allow)

    found = {}
    for f in git("ls-files", "-z").split("\0"):
        if f and os.path.isfile(f) and os.path.getsize(f) < 4_000_000:
            with open(f, errors="ignore") as fh:
                for w in set(WORD.findall(fh.read())) & names:
                    found.setdefault(w, set()).add(f)
    rng = ["%s..HEAD" % args.since] if args.since else ["HEAD"]
    hist = git("log", "-p", "--no-ext-diff", "--format=COMMIT %h%n%B", *rng)
    commit = None
    for line in hist.splitlines():
        if line.startswith("COMMIT "):
            commit = line.split()[1]
            continue
        if line.startswith("-"):
            continue  # removed lines were checked when they were added
        for w in set(WORD.findall(line)) & names:
            found.setdefault(w, set()).add("commit %s" % commit)

    bad = sorted(w for w in found if w not in allowed)
    print("check_names: %d ROM names known, %d used (all also in DKR/ultralib: %d)" % (
        len(names), len(found), len(found) - len(bad)))
    if bad:
        print("check_names: %d ROM-only name(s) are committed (do not push):" % len(bad))
        for w in bad:
            print("  %s: %s" % (w, ", ".join(sorted(found[w])[:5])))
        sys.exit(1)
    print("check_names: OK")


if __name__ == "__main__":
    main()
