#!/usr/bin/env python3
"""Apply the rename map names.txt to the whole tree.

    tools/rename.py                 rename (then build: names never change code)
    tools/rename.py --dry-run       what would change, per file
    tools/rename.py --check         every old name still in the tree; exit 1 if any
    tools/rename.py --which NAME    old <-> new for one symbol
    tools/rename.py FILE...         only these files (e.g. a branch's new files)
    tools/rename.py --aliases OUT OBJ...   the linker aliases (the Makefile runs this)

names.txt (repository root) has one rename per line, `#` starts a comment:

    func_80012345 = cam_shake_update       a main-segment function or global
    D_800A1234 = gShakeTimer               (old = an address name)
    func_m001_00000 = drops_reset          a module's static function or data
    field Particle.unk4 = lifetime         a struct field (renamed by hand)
    renamed cam_shake_update = cam_shake_tick   a name replaced later

Old names are splat's address names: func_XXXXXXXX / D_XXXXXXXX in the main
segment, func_mNNN_XXXXX / D_mNNN_XXXXX in module NNN. A module's exported
symbols keep their address names (the runlink build places them by it), so
only `static` module symbols may be renamed. A name that is replaced later
is edited on its address line and gets a `renamed previous = new` line;
rename.py rewrites the previous name too, and --check reports it.

What it rewrites, whole-word, in every tracked text file (and every new file
git does not ignore): C sources and headers, undefined_syms, CREDITS, the
READMEs and docs. Left alone: tools/ and lib/, names.txt itself,
symbol_addrs.*.txt and the yaml (splat names the generated assembly by
address there), and every INCLUDE_ASM / INCLUDE_RODATA line (those name the
assembly files splat writes, which keep their address names). A `#define new
new` line left behind is removed. It is safe to re-run: a renamed name is
left alone.

Struct fields are renamed by hand (the compiler reports a use that was
missed); --check verifies that the struct no longer has the old member and
has the new one.

The assembly still uses the old names (or the ROM's own names, aliased to the
address names at extract time). --aliases writes the linker script lines that
tie the two together, given the main-segment C objects: `old = new;` when C
or undefined_syms defines the new name, `PROVIDE(new = old);` when assembly
defines the symbol and C only refers to it.
"""
import argparse
import os
import re
import subprocess
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
NAMES = "names.txt"
ADDR_NAME = re.compile(r"(?:func|D)_[0-9A-F]{8}$")
MOD_NAME = re.compile(r"(?:func|D)_m(\d{3})_[0-9A-F]{5}$")
IDENT = re.compile(r"[A-Za-z_]\w*$")
ROW = re.compile(r"^\s*(\w+)\s*=\s*(\w+)\s*$")
FIELD_ROW = re.compile(r"^\s*field\s+(\w+)\.(\w+)\s*=\s*(\w+)\s*$")
RENAMED_ROW = re.compile(r"^\s*renamed\s+(\w+)\s*=\s*(\w+)\s*$")
ASM_LINE = re.compile(r"^\s*INCLUDE_(?:ASM|RODATA)\s*\(")

EXCLUDE_PREFIXES = ("tools/", "lib/", "asm/", "assets/", "build/", "build_nm/", ".github/")
EXCLUDE_FILES = (NAMES, "jfg.us.yaml")
EXCLUDE_RE = re.compile(r"(^|/)symbol_addrs[^/]*\.txt$")
BINARY_EXT = (".png", ".bin", ".z64", ".n64", ".v64", ".o", ".zip", ".jpg", ".gif", ".pdf")


def is_old_name(name):
    return bool(ADDR_NAME.match(name) or MOD_NAME.match(name))


def load(path=None):
    """(pairs, fields, renamed): [(old, new, line)], [(type, old, new, line)], [(previous, new, line)].
    Exits with a message on a malformed or inconsistent map."""
    path = path or os.path.join(ROOT, NAMES)
    pairs, fields, renamed, errors = [], [], [], []
    if not os.path.exists(path):
        return pairs, fields, renamed
    for n, line in enumerate(open(path), 1):
        line = line.split("#", 1)[0]
        if not line.strip():
            continue
        m = FIELD_ROW.match(line)
        if m:
            fields.append(m.groups() + (n,))
            continue
        m = RENAMED_ROW.match(line)
        if m:
            renamed.append(m.groups() + (n,))
            continue
        m = ROW.match(line)
        if m:
            pairs.append(m.groups() + (n,))
            continue
        errors.append("%s:%d: not `old = new`, `field Type.old = new` or `renamed old = new`" % (NAMES, n))
    seen_old, seen_new = {}, {}
    for old, new, n in pairs:
        if not is_old_name(old):
            errors.append("%s:%d: %s is not an address name (func_/D_ + 8 hex digits, or func_mNNN_/D_mNNN_ "
                          "+ 5)" % (NAMES, n, old))
        if is_old_name(new) or not IDENT.match(new):
            errors.append("%s:%d: the new name %s is not a name of our own" % (NAMES, n, new))
        if old in seen_old:
            errors.append("%s:%d: %s is renamed twice (line %d)" % (NAMES, n, old, seen_old[old]))
        if new in seen_new:
            errors.append("%s:%d: %s is the new name of two symbols (line %d)" % (NAMES, n, new, seen_new[new]))
        seen_old[old], seen_new[new] = n, n
    seen_prev = {}
    for prev, new, n in renamed:
        if prev in seen_old or prev in seen_new:
            errors.append("%s:%d: %s is still an address line's name; rename it there" % (NAMES, n, prev))
        if prev in seen_prev:
            errors.append("%s:%d: %s is renamed twice (line %d)" % (NAMES, n, prev, seen_prev[prev]))
        if is_old_name(new):
            errors.append("%s:%d: the new name %s is an address name" % (NAMES, n, new))
        seen_prev[prev] = n
    seen_field = {}
    for typ, old, new, n in fields:
        key = (typ, old)
        if key in seen_field:
            errors.append("%s:%d: field %s.%s is renamed twice (line %d)" % (NAMES, n, typ, old, seen_field[key]))
        seen_field[key] = n
    if errors:
        sys.exit("\n".join(errors))
    return pairs, fields, renamed


def mapping(pairs, renamed):
    m = {o: n for o, n, _ in pairs}
    m.update({p: n for p, n, _ in renamed})
    return m


def orig(name, pairs=None):
    """The address name of NAME (itself if it is one or is not in the map)."""
    if pairs is None:
        pairs = load()[0]
    for o, n, _ in pairs:
        if n == name:
            return o
    return name


def git_files(root):
    out = subprocess.run(["git", "ls-files", "-z", "--cached", "--others", "--exclude-standard"], cwd=root,
                         capture_output=True, check=True).stdout
    files = []
    for rel in out.decode("utf-8", "surrogateescape").split("\0"):
        if not rel or rel.startswith(EXCLUDE_PREFIXES) or rel in EXCLUDE_FILES or EXCLUDE_RE.search(rel):
            continue
        if rel.lower().endswith(BINARY_EXT):
            continue
        p = os.path.join(root, rel)
        if os.path.isfile(p) and not os.path.islink(p):
            files.append(rel)
    return sorted(set(files))


def read_text(path):
    data = open(path, "rb").read()
    if b"\0" in data:
        return None
    return data.decode("utf-8", "surrogateescape")


def token_re(words):
    words = sorted(set(words), key=lambda w: (-len(w), w))
    return re.compile(r"(?<![\w$])(" + "|".join(map(re.escape, words)) + r")(?![\w$])")


IDENTITY_DEFINE = re.compile(r"^[ \t]*#[ \t]*define[ \t]+(\w+)[ \t]+\1[ \t]*(?:/\*[^\n]*?\*/[ \t]*|//[^\n]*)?\r?\n",
                             re.M)


def rename_text(text, pat, table):
    """TEXT with every old name replaced (INCLUDE_ASM/INCLUDE_RODATA lines kept), and the count."""
    count = 0
    out = []
    for line in text.splitlines(True):
        if not ASM_LINE.match(line):
            line, n = pat.subn(lambda m: table[m.group(1)], line)
            count += n
        out.append(line)
    text = "".join(out)
    if count:
        news = set(table.values())
        text = IDENTITY_DEFINE.sub(lambda m: "" if m.group(1) in news else m.group(0), text)
    return text, count


def module_static_errors(pairs, root):
    """A renamed module symbol must be static in its module's C file."""
    errors = []
    cache = {}
    for old, new, n in pairs:
        m = MOD_NAME.match(old)
        if not m:
            continue
        src = os.path.join(root, "src", "modules", "m%s.c" % m.group(1))
        if src not in cache:
            cache[src] = read_text(src) if os.path.exists(src) else ""
        for name in (old, new):
            if exported_decl(cache[src], name):
                errors.append("%s:%d: %s is not static in %s (exported module symbols keep their "
                              "address names)" % (NAMES, n, name, os.path.relpath(src, root)))
                break
    return errors


def exported_decl(text, name):
    """True when TEXT declares NAME at file scope without `static` (or `extern`)."""
    for m in re.finditer(r"^(\S[^\n;{}=(\[]*?)\b%s\s*[\[(=;]" % re.escape(name), text, re.M):
        decl = m.group(1)
        if re.match(r"(?:(?:static|extern|typedef|return|if|while|for|case)\b|#|/|\*|INCLUDE_)", decl):
            continue
        if re.match(r"[A-Za-z_]", decl):
            return True
    return False


def struct_body(text, typ):
    """The member list of struct TYP in TEXT, or None."""
    for pat in (r"\bstruct\s+%s\s*\{" % re.escape(typ), r"typedef\s+struct\s*\w*\s*\{"):
        for m in re.finditer(pat, text):
            depth, i = 0, m.end() - 1
            while i < len(text):
                if text[i] == "{":
                    depth += 1
                elif text[i] == "}":
                    depth -= 1
                    if depth == 0:
                        break
                i += 1
            body = text[m.end():i]
            if pat.startswith("typedef"):
                if not re.match(r"\s*%s\s*;" % re.escape(typ), text[i + 1:]):
                    continue
            return body
    return None


def check_fields(fields, files, root):
    problems = []
    texts = {}
    # (a file may define its own view of a struct with fewer fields: each
    # definition must have lost the old member, and one must have the new)
    for typ, old, new, n in fields:
        found = has_new = False
        for rel in files:
            if not rel.endswith((".c", ".h")):
                continue
            if rel not in texts:
                texts[rel] = read_text(os.path.join(root, rel)) or ""
            body = struct_body(texts[rel], typ)
            if body is None:
                continue
            found = True
            body = re.sub(r"/\*.*?\*/|//[^\n]*", "", body, flags=re.S)
            if re.search(r"\b%s\b" % re.escape(old), body):
                problems.append("%s: struct %s still has %s (now %s)" % (rel, typ, old, new))
            if re.search(r"\b%s\b" % re.escape(new), body):
                has_new = True
        if not found:
            problems.append("%s:%d: struct %s not found" % (NAMES, n, typ))
        elif not has_new:
            problems.append("%s:%d: no struct %s has a field %s" % (NAMES, n, typ, new))
    return problems


def defined_names(objs):
    """Global symbols the objects define."""
    names = set()
    if not objs:
        return names
    cross = os.environ.get("CROSS", "mips-linux-gnu-")
    out = subprocess.run([cross + "nm", "-g", "--defined-only"] + list(objs), capture_output=True, text=True,
                         check=True).stdout
    for line in out.splitlines():
        parts = line.split()
        if len(parts) == 3:
            names.add(parts[2])
    return names


def script_names(path):
    names = set()
    if os.path.exists(path):
        for line in open(path):
            line = re.sub(r"/\*.*?\*/|//.*", "", line)
            m = re.match(r"\s*(?:PROVIDE\s*\(\s*)?(\w+)\s*=", line)
            if m:
                names.add(m.group(1))
    return names


def write_aliases(out, objs, pairs, undefined_syms):
    defined = defined_names(objs) | script_names(undefined_syms)
    lines = ["/* Generated by tools/rename.py --aliases from names.txt: the assembly's",
             " * address names for the symbols C now names. Do not edit. */"]
    for old, new, _ in pairs:
        if MOD_NAME.match(old):
            continue  # module statics: no symbol outside their object
        if new in defined:
            lines.append("%s = %s;" % (old, new))
        else:
            lines.append("PROVIDE(%s = %s);" % (new, old))
    text = "\n".join(lines) + "\n"
    if not (os.path.exists(out) and open(out).read() == text):
        os.makedirs(os.path.dirname(out) or ".", exist_ok=True)
        with open(out, "w") as f:
            f.write(text)


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("files", nargs="*", help="only these files (or, with --aliases, the C objects)")
    ap.add_argument("--dry-run", action="store_true")
    ap.add_argument("--check", action="store_true", help="report old names still in the tree; exit 1 if any")
    ap.add_argument("--which", metavar="NAME")
    ap.add_argument("--aliases", metavar="OUT", help="write the linker aliases for the C objects given")
    ap.add_argument("--version", default="us")
    ap.add_argument("--root", default=ROOT, help=argparse.SUPPRESS)
    args = ap.parse_args(argv)
    root = os.path.abspath(args.root)

    pairs, fields, renamed = load(os.path.join(root, NAMES))
    table = mapping(pairs, renamed)
    if args.which:
        n = args.which
        if n in table:
            print("%s = %s" % (n, table[n]))
            return 0
        for o, nn, _ in pairs:
            if nn == n:
                print("%s = %s" % (o, n))
                return 0
        for t, o, nn, _ in fields:
            if n in (o, nn, "%s.%s" % (t, o), "%s.%s" % (t, nn)):
                print("field %s.%s = %s" % (t, o, nn))
                return 0
        print("%s: not in %s" % (n, NAMES))
        return 1
    if args.aliases:
        write_aliases(args.aliases, args.files, pairs, os.path.join(root, "undefined_syms.%s.txt" % args.version))
        return 0

    errors = module_static_errors(pairs, root)
    if errors:
        sys.exit("\n".join(errors))
    if args.files:
        files = [os.path.relpath(os.path.abspath(f), root) for f in args.files]
    else:
        files = git_files(root)

    if args.check:
        stale = 0
        if table:
            pat = token_re(table)
            for rel in files:
                text = read_text(os.path.join(root, rel))
                if text is None:
                    continue
                for i, line in enumerate(text.splitlines(), 1):
                    if ASM_LINE.match(line):
                        continue
                    for m in pat.finditer(line):
                        print("%s:%d: %s (now %s)" % (rel, i, m.group(1), table[m.group(1)]))
                        stale += 1
        problems = check_fields(fields, files, root)
        for p in problems:
            print(p)
        stale += len(problems)
        print("rename.py --check: %d old name%s in %d files (%d renames, %d fields)" % (
            stale, "" if stale == 1 else "s", len(files), len(table), len(fields)))
        return 1 if stale else 0

    if not table:
        print("rename.py: %s has no renames" % NAMES)
        return 0
    pat = token_re(table)
    total = changed = 0
    for rel in files:
        path = os.path.join(root, rel)
        text = read_text(path)
        if text is None:
            continue
        new_text, n = rename_text(text, pat, table)
        if n:
            total += n
            changed += 1
            print("  %-50s %d" % (rel, n))
            if not args.dry_run:
                with open(path, "wb") as f:
                    f.write(new_text.encode("utf-8", "surrogateescape"))
    print("rename.py: %d replacement%s in %d file%s%s" % (total, "" if total == 1 else "s", changed,
                                                       "" if changed == 1 else "s",
                                                       " (dry run)" if args.dry_run else ""))
    if fields:
        problems = check_fields(fields, files, root)
        if problems:
            print("fields still to rename by hand:")
            for p in problems:
                print("  " + p)
    return 0


if __name__ == "__main__":
    sys.exit(main())
