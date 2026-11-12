#!/usr/bin/env python3
"""Set up a decomp-permuter workspace for one function, with JFG's compiler
command, and read back what the permuter found.

  perm.py import FILE.c FUNC [--asm NAME] [--replace DEF.c] [--tag TAG]
  perm.py best build/perm/FUNC [-n 3]
  perm.py diff build/perm/FUNC [CANDIDATE.c]

`import` preprocesses FILE.c with IDO itself (game flags, -DNON_MATCHING, and
-I include/modules for src/modules/*.c, as modules.mk builds them), keeps only
FUNC's body (the other functions become prototypes), and assembles FUNC's
original assembly (the INCLUDE_ASM line's .s file, or --asm NAME) as the
target. Symbols the assembly names differently from the C (runlink names,
rl_mNNN_XXXXX vs func_mNNN_XXXXX) are renamed to the C's spelling when they
are the same address, so a score of 0 is reachable. The workspace is
build/perm/FUNC[-TAG] (gitignored): base.c, target.s/.o, compile.sh,
settings.toml (compiler_type = "ido").

Then run the permuter (tools/permuter, a submodule; it needs `toml`, see
tools/match/README.md):

  python3 tools/permuter/permuter.py build/perm/FUNC -j4 --best-only

`best` prints the best outputs' scores and FUNC's text from the best one, as
a unified diff against base.c.

A lower score is not a match. Read the candidate, make sure it means the same
as before, put it in the real file and confirm with probe.py/modprobe.py and
the full build. The target .s holds names read from your ROM: never commit or
paste anything under build/perm/.
"""
import argparse
import difflib
import glob
import os
import re
import shutil
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import common as C  # noqa: E402

PERMUTER = C.path("tools", "permuter")
MOD_RE = re.compile(r"\b(?:func|rl|D)_(m\d{3})_([0-9A-Fa-f]{5})\b")
HEX_RE = re.compile(r"^(?:func|D)_([0-9A-F]{8})$")
REF_RE = re.compile(r"(\bjal\s+|\bj\s+|%hi\(|%lo\()([A-Za-z_]\w*)")


def skip_literal(src, i):
    """Index just past the string or char constant starting at SRC[i]."""
    q, j = src[i], i + 1
    while j < len(src) and src[j] != q and src[j] != "\n":
        j += 2 if src[j] == "\\" else 1
    return j + 1


def strip_other_fns(src, keep):
    """SRC with every function definition but KEEP's turned into a prototype
    (headers on several lines included)."""
    out, i, n, last, depth = [], 0, len(src), 0, 0
    while i < n:
        c = src[i]
        if c in "\"'":
            i = skip_literal(src, i)
            continue
        if c == "{" and depth == 0:
            head = src[last:i]
            h = re.sub(r"^\s*#.*$", "", head, flags=re.M)
            m = re.search(r"\b(\w+)\s*\(", h)
            if m and h.rstrip().endswith(")") and "=" not in h:
                j, d = i, 0
                while j < n:
                    if src[j] in "\"'":
                        j = skip_literal(src, j)
                        continue
                    d += {"{": 1, "}": -1}.get(src[j], 0)
                    if d == 0:
                        break
                    j += 1
                out.append(src[last:j + 1] if m.group(1) == keep else head.rstrip() + ";")
                i = last = j + 1
                continue
        if c == "{":
            depth += 1
        elif c == "}":
            depth -= 1
        elif c == ";" and depth == 0:
            out.append(src[last:i + 1])
            last = i + 1
        i += 1
    out.append(src[last:])
    return "".join(out)


def find_def(src, name):
    """(start, end) of NAME's definition in SRC, or None."""
    m = re.search(r"^[^\n;{}#]*\b%s\s*\([^;{}]*\)\s*\{" % re.escape(name), src, re.M)
    if not m:
        return None
    i, depth = m.end() - 1, 0
    while i < len(src):
        if src[i] == "{":
            depth += 1
        elif src[i] == "}":
            depth -= 1
            if depth == 0:
                return m.start(), i + 1
        i += 1
    return None


def strip_line_comments(src):
    """Drop // comments (IDO's -E ignores -Xcpluscomm and trips on an
    apostrophe in one); strings, char constants and /* */ are left alone."""
    out, i, n = [], 0, len(src)
    while i < n:
        c = src[i]
        if c in "\"'":
            j = i + 1
            while j < n and src[j] != c and src[j] != "\n":
                j += 2 if src[j] == "\\" else 1
            out.append(src[i:j + 1])
            i = j + 1
        elif src.startswith("/*", i):
            j = src.find("*/", i + 2)
            j = n if j < 0 else j + 2
            out.append(src[i:j])
            i = j
        elif src.startswith("//", i):
            j = src.find("\n", i)
            i = n if j < 0 else j
        else:
            out.append(c)
            i += 1
    return "".join(out)


def key_of(name, symtab):
    m = MOD_RE.fullmatch(name)
    if m:
        return (m.group(1), int(m.group(2), 16))
    if name in symtab:
        return symtab[name]
    m = HEX_RE.match(name)
    if m:
        return int(m.group(1), 16)
    return None


def cmd_import(a):
    src_path = os.path.relpath(os.path.abspath(a.file), C.ROOT)
    mod = re.match(r"src/modules/(m\d{3})\.c$", src_path) or (a.module and re.match(r"(m\d{3})$", a.module))
    cflags = list(C.GAME_CFLAGS) + (["-I", "include/modules"] if mod else [])
    src = open(C.path(src_path)).read()

    # the target assembly: the INCLUDE_ASM line of --asm NAME (default FUNC)
    asm_name = a.asm or a.func
    inc = re.search(r'INCLUDE_ASM\("([^"]+)",\s*%s\)' % re.escape(asm_name), src)
    if inc:
        asm_path = C.path(inc.group(1), asm_name + ".s")
    else:
        hits = glob.glob(C.path("asm", "nonmatchings", "**", asm_name + ".s"), recursive=True)
        if len(hits) > 1:
            # a module function with .rodata also has a copy in mNNN/ro/ (with its own .rodata)
            hits = [h for h in hits if os.sep + "ro" + os.sep not in h]
        if len(hits) != 1:
            sys.exit("perm.py: no INCLUDE_ASM line for %s in %s (use --asm NAME)" % (asm_name, src_path))
        asm_path = hits[0]
    C.need(asm_path, "run `make extract`")

    ws = C.path("build", "perm", a.func + ("-" + a.tag if a.tag else ""))
    if os.path.exists(ws):
        if not a.force:
            sys.exit("perm.py: %s exists (--force replaces it)" % os.path.relpath(ws, C.ROOT))
        shutil.rmtree(ws)
    os.makedirs(ws)

    if a.replace:
        new = open(a.replace).read()
        span = find_def(src, a.func)
        if span:
            src = src[:span[0]] + new.strip() + "\n" + src[span[1]:]
        else:
            src += "\n" + new
    copy = os.path.join(ws, "orig.c")
    with open(copy, "w") as f:
        f.write(strip_line_comments(src))

    # preprocess with IDO's own cpp, so headers see the compiler's macros
    r = subprocess.run([C.IDO_CC, "-E"] + cflags + ["-I", os.path.dirname(src_path), "-DNON_MATCHING",
                                                    "-DPERMUTER", copy],
                       capture_output=True, text=True, cwd=C.ROOT)
    if r.returncode:
        sys.exit("perm.py: preprocessing failed\n" + r.stderr[-3000:])
    pre = "\n".join(l for l in r.stdout.splitlines() if not l.startswith("#") or l.startswith("#pragma"))
    pre = re.sub(r"^\s*INCLUDE_(?:ASM|RODATA)\([^)]*\);?[ \t]*$", "", pre, flags=re.M)
    base = strip_other_fns(pre, a.func)
    # statics become plain functions: IDO rejects a static that is only
    # declared, and drops an unreferenced static FUNC (a call is a jal either way)
    base = re.sub(r"^(\s*)static\s+(?=[^=;{]*\()", r"\1", base, flags=re.M)
    if not find_def(base, a.func):
        sys.exit("perm.py: no C definition of %s after preprocessing (is it NON_MATCHING C?)" % a.func)
    with open(os.path.join(ws, "base.c"), "w") as f:
        f.write(base.rstrip("\n") + "\n")

    # target: the original assembly, with symbols renamed to the C's spelling
    symtab = C.symbol_addrs()
    for line in open(C.path("symbol_addrs.%s.txt" % C.VERSION)):  # splat's names (sqrtf ...)
        m = re.match(r"\s*(\w+)\s*=\s*0x([0-9A-Fa-f]+);", line)
        if m:
            symtab.setdefault(m.group(1), int(m.group(2), 16))
    want = {}
    for ident in set(re.findall(r"\b[A-Za-z_]\w*\b", base)):
        k = key_of(ident, symtab)
        if k is not None:
            want.setdefault(k, ident)
    asm = open(asm_path).read()
    asm = re.sub(r"^\s*nonmatching .*$", "", asm, flags=re.M)
    for j in set(re.findall(r"^\s*jlabel\s+(\w+)", asm, re.M)):
        asm = re.sub(r"^\s*jlabel\s+%s\s*$" % j, ".Ljl_%s:" % j, asm, flags=re.M)
        asm = re.sub(r"\b%s\b" % j, ".Ljl_%s" % j, asm)
        asm = asm.replace(".Ljl_.Ljl_", ".Ljl_")
    renamed = []

    def ren(m):
        k = key_of(m.group(2), symtab)
        new = want.get(k) if k is not None else None
        if new and new != m.group(2):
            renamed.append(new)
            return m.group(1) + new
        return m.group(0)
    asm = REF_RE.sub(ren, asm)
    g = re.search(r"^\s*glabel\s+(\w+)", asm, re.M)
    if g and g.group(1) != a.func:
        asm = re.sub(r"\b%s\b" % re.escape(g.group(1)), a.func, asm)
    with open(os.path.join(ws, "target.s"), "w") as f:
        f.write(open(C.path("include", "prelude.inc")).read() + "\n.section .text\n" + asm)
    r = subprocess.run(["mips-linux-gnu-as"] + C.ASFLAGS + ["-o", os.path.join(ws, "target.o"),
                                                            os.path.join(ws, "target.s")],
                       capture_output=True, text=True, cwd=C.ROOT)
    if r.returncode:
        sys.exit("perm.py: assembling the target failed\n" + r.stderr[-3000:])

    flags = " ".join("'%s'" % x for x in cflags + ["-O2", "-mips1"])
    with open(os.path.join(ws, "compile.sh"), "w") as f:
        f.write("#!/bin/bash\n# compile.sh IN.c -o OUT.o (called by the permuter)\nset -e\n"
                "IN=\"$(realpath \"$1\")\"\nOUT=\"$(realpath -m \"$3\")\"\ncd '%s'\n"
                "'%s' -c %s -o \"$OUT\" \"$IN\"\n" % (C.ROOT, C.path(C.IDO_CC), flags))
    os.chmod(os.path.join(ws, "compile.sh"), 0o755)
    with open(os.path.join(ws, "settings.toml"), "w") as f:
        f.write('func_name = "%s"\ncompiler_type = "ido"\n' % a.func)

    # check that base.c compiles and has the function
    o = os.path.join(ws, "base.o")
    r = subprocess.run(["bash", os.path.join(ws, "compile.sh"), os.path.join(ws, "base.c"), "-o", o],
                       capture_output=True, text=True)
    if r.returncode:
        sys.exit("perm.py: base.c does not compile\n" + (r.stdout + r.stderr)[-3000:])
    print("%s: target %s, %d symbol(s) renamed%s" % (os.path.relpath(ws, C.ROOT), os.path.relpath(asm_path, C.ROOT),
                                                    len(renamed), " (module %s)" % mod.group(1) if mod else ""))
    print("run: python3 tools/permuter/permuter.py %s -j4 --best-only" % os.path.relpath(ws, C.ROOT))


def cmd_best(a):
    ws = a.dir
    outs = []
    for d in glob.glob(os.path.join(ws, "output-*")):
        m = re.match(r"output-(\d+)-(\d+)$", os.path.basename(d))
        if m:
            outs.append((int(m.group(1)), int(m.group(2)), d))
    if not outs:
        sys.exit("perm.py: no output-* in %s yet" % ws)
    outs.sort()
    func = re.search(r'func_name\s*=\s*"(\w+)"', open(os.path.join(ws, "settings.toml")).read()).group(1)
    for s, n, d in outs[:a.n]:
        print("score %d: %s" % (s, d))
    base = open(os.path.join(ws, "base.c")).read()
    best = open(os.path.join(outs[0][2], "source.c")).read()
    sb, sc = find_def(base, func), find_def(best, func)
    old = base[sb[0]:sb[1]] if sb else base
    new = best[sc[0]:sc[1]] if sc else best
    sys.stdout.writelines(difflib.unified_diff(old.splitlines(True), new.splitlines(True), "base.c",
                                               os.path.join(outs[0][2], "source.c")))


def cmd_diff(a):
    """The permuter's view (its simplified objdump) of target.o vs a candidate."""
    sys.path.insert(0, PERMUTER)
    from src.objdump import objdump, get_arch  # tools/permuter (MIT)
    ws = a.dir
    c = a.source or os.path.join(ws, "base.c")
    o = os.path.join(ws, "diff_cand.o")
    r = subprocess.run(["bash", os.path.join(ws, "compile.sh"), c, "-o", o], capture_output=True, text=True)
    if r.returncode:
        sys.exit("perm.py: compile failed\n" + (r.stdout + r.stderr)[-3000:])
    t = os.path.join(ws, "target.o")
    arch = get_arch(t)
    rows = [[ln.row for ln in objdump(x, arch, stack_differences=False, ign_branch_targets=True)]
            for x in (t, o)]
    sys.stdout.writelines(difflib.unified_diff([x + "\n" for x in rows[0]], [x + "\n" for x in rows[1]],
                                               "target", c, n=a.context))


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = ap.add_subparsers(dest="cmd", required=True)
    p = sub.add_parser("import", help="make build/perm/FUNC from FILE.c")
    p.add_argument("file", help="src/X.c or src/modules/mNNN.c")
    p.add_argument("func", help="the function (its NON_MATCHING or plain C body)")
    p.add_argument("--asm", help="name of the target .s/INCLUDE_ASM (default FUNC; for statics)")
    p.add_argument("--replace", help="file with a definition of FUNC to use instead of FILE's")
    p.add_argument("--module", help="mNNN: module flags for a FILE outside src/modules (a scratch copy)")
    p.add_argument("--tag", default="", help="workspace suffix (build/perm/FUNC-TAG)")
    p.add_argument("--force", action="store_true", help="replace an existing workspace")
    p = sub.add_parser("best", help="best scores and the best candidate's diff")
    p.add_argument("dir")
    p.add_argument("-n", type=int, default=5)
    p = sub.add_parser("diff", help="target vs base.c (or a candidate) as the permuter sees them")
    p.add_argument("dir")
    p.add_argument("source", nargs="?", help="candidate .c (default base.c)")
    p.add_argument("-c", "--context", type=int, default=2)
    a = ap.parse_args()
    {"import": cmd_import, "best": cmd_best, "diff": cmd_diff}[a.cmd](a)


if __name__ == "__main__":
    main()
