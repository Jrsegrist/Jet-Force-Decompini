#!/usr/bin/env python3
"""Check that no tracked file is ROM data or derived from it.

This repository contains no game data: no ROM, nothing extracted from it
(assembly, assets, binaries, the linker script splat writes) and no long
tables of numbers lifted from it. This script looks at every file git tracks
(not the submodules) and fails when one of them:

  - lies where extracted or built files go (asm/, assets/, build/, ...) or
    has a ROM, binary or object file extension (.z64, .bin, .o, .ld, ...);
  - is binary (has a NUL byte or is not UTF-8 text), or is larger than
    --max-size bytes;
  - is a .s file (the hand-written assembly the build needs comes from the
    ROM at extract time; only the include/*.inc macro files are tracked);
  - contains disassembly as splat or tools/runlink_split.py write it: lines
    with the `/* ROM VRAM WORD */` or `/* OFFSET WORD */` comments, or
    `glabel` / `dlabel` lines outside the macro files;
  - has a run of more than --max-table numeric literals in a row (C data
    tables written out as numbers; the longest legitimate ones in src/ are
    far shorter).

No ROM is needed. Usage: tools/ci_rom_free.py [--max-size N] [--max-table N]
Exit status 1 when something is found.
"""
import argparse
import os
import re
import subprocess
import sys

BAD_DIRS = ("asm/", "assets/", "build/", "build_nm/", "expected/", ".venv/")
BAD_EXT = (".z64", ".n64", ".v64", ".rom", ".bin", ".o", ".elf", ".ld", ".map", ".a",
           ".zip", ".7z", ".gz", ".png", ".jpg", ".bmp", ".wav", ".aiff", ".aifc")
# the assembler macro files (written for this project, no ROM content)
ASM_OK = ("include/macro.inc", "include/prelude.inc")

DISASM_RE = re.compile(
    r"/\*\s*[0-9A-F]{5,7}\s+80[0-9A-F]{6}\s+[0-9A-F]{8}\s*\*/"  # splat: ROM VRAM word
    r"|/\*\s*[0-9A-F]{5}\s+[0-9A-F]{8}\s*\*/"                   # runlink_split: offset word
    r"|^\s*/\*\s*[0-9A-F]{5}\s*\*/\s*\.(?:word|byte|half)\b")   # runlink_split data
LABEL_RE = re.compile(r"^\s*(?:glabel|dlabel|jlabel)\s+\w+", re.M)
# a numeric literal followed by a separator (C initialisers, .word lists)
NUM = r"(?:0[xX][0-9A-Fa-f]+|\d+(?:\.\d*)?(?:[eE][-+]?\d+)?)[uUlLfF]*"
TABLE_RE = re.compile(r"(?:-?\s*%s\s*(?:,|\.word|\.byte|\.half)\s*)+(?:-?\s*%s)?" % (NUM, NUM))


def tracked_files():
    out = subprocess.run(["git", "ls-files", "-s", "-z"], capture_output=True, check=True).stdout
    files = []
    for entry in out.split(b"\0"):
        if not entry:
            continue
        meta, path = entry.split(b"\t", 1)
        if meta.startswith(b"160000"):
            continue  # a submodule (gitlink)
        files.append(path.decode("utf-8", "replace"))
    return files


def longest_run(text):
    best = 0
    for m in TABLE_RE.finditer(text):
        n = len(re.findall(NUM, m.group(0)))
        best = max(best, n)
    return best


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--max-size", type=int, default=256 * 1024)
    ap.add_argument("--max-table", type=int, default=256)
    ap.add_argument("--verbose", "-v", action="store_true", help="print the longest table per file")
    args = ap.parse_args()

    problems = []
    files = tracked_files()
    longest = (0, None)
    for f in files:
        low = f.lower()
        if low.startswith(BAD_DIRS) or "/build/" in low:
            problems.append("%s: extracted/build files are never tracked" % f)
        if low.endswith(BAD_EXT) or os.path.basename(low).startswith("baserom"):
            problems.append("%s: ROM, binary or generated file type" % f)
        if not os.path.isfile(f):
            continue  # deleted in the work tree
        size = os.path.getsize(f)
        if size > args.max_size:
            problems.append("%s: %d bytes (more than %d)" % (f, size, args.max_size))
            continue
        data = open(f, "rb").read()
        if b"\0" in data:
            problems.append("%s: binary (NUL bytes)" % f)
            continue
        try:
            text = data.decode("utf-8")
        except UnicodeDecodeError:
            problems.append("%s: not UTF-8 text" % f)
            continue
        if low.endswith((".s", ".asm")) or (low.endswith(".inc") and f not in ASM_OK):
            problems.append("%s: assembly file (extracted assembly is never tracked)" % f)
        if f != "tools/ci_rom_free.py":
            n = sum(1 for line in text.splitlines() if DISASM_RE.search(line))
            if n:
                problems.append("%s: %d line(s) of disassembly" % (f, n))
            if f not in ASM_OK and LABEL_RE.search(text) and not low.endswith((".py", ".md")):
                problems.append("%s: glabel/dlabel lines (extracted assembly)" % f)
        run = longest_run(text)
        if run > longest[0]:
            longest = (run, f)
        if args.verbose and run >= 16:
            print("  %5d numbers in a row: %s" % (run, f))
        if run > args.max_table:
            problems.append("%s: a table of %d numbers (more than %d)" % (f, run, args.max_table))

    print("ci_rom_free: %d tracked files checked; longest number table %d (%s)" % (
        len(files), longest[0], longest[1]))
    for p in problems:
        print("  " + p)
    if problems:
        print("ci_rom_free: %d problem(s)" % len(problems))
        sys.exit(1)
    print("ci_rom_free: OK, no ROM-derived files")


if __name__ == "__main__":
    main()
