#!/usr/bin/env python3
"""Generate splat symbol files from Jet Force Gemini's runlink tables.

To link its run-time "runlink" modules the ROM carries a global symbol table
(one word per symbol: module index << 20 | offset) and a parallel table of
name offsets into a string pool. The names are never used: everything this
script writes for people and tools to read names symbols by address
(func_XXXXXXXX, D_XXXXXXXX). It writes, from the user's own ROM:

  OUT_SYMBOL_ADDRS   a splat symbol_addrs file: every main-segment (module 0)
                     symbol by its address name, with type:func for code
  --modules F        the module table, one line per module
  --aliases F        build-internal: the linker aliases between the address
                     names and the names the project's own licensed sources
                     use (DKR's, libultra's, names.txt, undefined_syms), for
                     the symbols where the ROM's table is the only record of
                     which address such a name stands for. Only names that
                     occur in the --used-in files are written, so the file
                     holds no name that only the ROM has. Not for reading.
  --hide-strings DIR rewrite, in DIR's .s files, every .ascii/.asciz string
                     that contains a name from the ROM's table (one not in
                     the --used-in files) as .byte lines: the same bytes,
                     without the name in readable form

All of it is ROM-derived, generated at extract (or build) time, and never
committed. tools/check_rom_names.py checks that no generated file has a name.

Usage: runlink_syms.py ROM [OUT_SYMBOL_ADDRS] [--modules F] [--aliases F]
                       [--used-in PATH ...] [--hide-strings DIR ...]
"""
import argparse
import os
import re
import struct
import sys

# US 1.0 (NJFE) layout. These are the ROM addresses the runlink loader in
# main uses (see "ROM layout" in README.md).
LAYOUT = {
    "us": {
        "main_imports": 0x1ECF220,  # main's relocations against module symbols
        "symtab": 0x1ED0270,        # global symbol table
        "modtab": 0x1ED2780,        # module table, 0x20 bytes per module
        "modules": 0x1ED3B20,       # module images (text, data, relocations)
        "nameoffs": 0x1FEB040,      # name offsets, one word per symbol
        "names": 0x1FED550,         # name string pool
        # main (module 0) sections, filled in at run time by the loader
        "text": 0x80000450,
        "data": 0x800A0660,
        "bss": 0x800B0B50,
        "bss_end": 0x801059A0,
    }
}

SEC_RODATA, SEC_DATA, SEC_BSS = 4093, 4094, 4095
SEC_ABS = 4092  # a fixed address in low memory: 0x80000000 + offset

# the project's own licensed sources of names (see README.md, "Names")
USED_IN = ["src", "include", "lib/ultralib/src", "lib/ultralib/include", "undefined_syms.us.txt",
           "names.txt", "symbol_addrs.us.txt"]
WORD = re.compile(r"[A-Za-z_]\w+")
ADDR_NAME = re.compile(r"(?:func|D)_(?:[0-9A-Fa-f]{8}|m\d{3}_[0-9A-Fa-f]{5})$")


def u32(rom, off):
    return struct.unpack_from(">I", rom, off)[0]


def cstr(rom, off):
    end = rom.index(b"\0", off)
    return rom[off:end].decode("ascii")


def read_table(rom, L):
    """[(module, offset, name)] of every symbol, in table order."""
    nsyms = (L["modtab"] - L["symtab"]) // 4
    if (L["names"] - L["nameoffs"]) // 4 != nsyms:
        sys.exit("symbol and name tables disagree in length")
    out = []
    for i in range(nsyms):
        v = u32(rom, L["symtab"] + 4 * i)
        out.append((v >> 20, v & 0xFFFFF, cstr(rom, L["names"] + u32(rom, L["nameoffs"] + 4 * i))))
    return out


def rom_names(rom_path, version="us"):
    """The set of names in the ROM's table (for the checks; never print them)."""
    with open(rom_path, "rb") as f:
        rom = f.read()
    return set(n for _, _, n in read_table(rom, LAYOUT[version]))


def words_of(paths):
    """Every identifier-like word in the given files and directory trees."""
    words = set()
    for base in paths:
        files = [base] if os.path.isfile(base) else [
            os.path.join(d, f) for d, sub, fs in os.walk(base) for f in fs]
        for p in files:
            try:
                with open(p, errors="ignore") as fh:
                    words |= set(WORD.findall(fh.read()))
            except OSError:
                pass
    return words


def main_address(L, mod, off):
    """The main-segment address of symbol (mod, off), or None for a module's."""
    if mod == 0:
        return L["text"] + off
    if mod in (SEC_RODATA, SEC_DATA):
        return L["data"] + off
    if mod == SEC_BSS:
        return L["bss"] + off
    if mod == SEC_ABS:
        return 0x80000000 + off
    return None


def address_name(L, addr):
    return ("func_%08X" if L["text"] <= addr < L["data"] else "D_%08X") % addr


def read_modtab(rom, L):
    nmods = (L["modules"] - L["modtab"]) // 32
    return [struct.unpack_from(">IIIIIHHii", rom, L["modtab"] + 32 * m) for m in range(nmods)]


def write_aliases(path, table, L, mods, used):
    """The build-internal alias linker script (see the module docstring)."""
    out = ["/* build-internal: not for reading. Generated by tools/runlink_syms.py: the",
           " * address names of symbols that the project's licensed sources name. */"]
    linked = set()  # address names already tied to a name in both directions
    for mod, off, name in table:
        if name not in used or ADDR_NAME.match(name):
            continue
        addr = main_address(L, mod, off)
        if addr is None:
            if not 1 <= mod <= len(mods):
                continue
            plain = ("func_m%03d_%05X" if off < mods[mod - 1][2] else "D_m%03d_%05X") % (mod, off)
            out.append("PROVIDE(%s = %s);" % (name, plain))  # read by runlink_build.py
            continue
        plain = address_name(L, addr)
        out.append("PROVIDE(%s = %s);" % (name, plain))
        if plain not in linked:
            linked.add(plain)
            out.append("PROVIDE(%s = %s);" % (plain, name))
    text = "\n".join(out) + "\n"
    if not (os.path.exists(path) and open(path).read() == text):
        os.makedirs(os.path.dirname(path) or ".", exist_ok=True)
        with open(path, "w") as f:
            f.write(text)
    return len(out) - 2


STRING_RE = re.compile(r'^(?P<pre>.*?)\.(?P<kind>asciz|ascii)\s+"(?P<s>(?:[^"\\]|\\.)*)"\s*$')
ESC = {"n": 10, "t": 9, "r": 13, "b": 8, "f": 12, "v": 11, "a": 7, "e": 27, "\\": 92, '"': 34, "'": 39}


def unescape(s):
    """The bytes of a gas string literal's body."""
    out = bytearray()
    i = 0
    while i < len(s):
        c = s[i]
        if c != "\\":
            out += c.encode("latin-1")
            i += 1
            continue
        n = s[i + 1]
        if n in "01234567":
            j = i + 1
            while j < len(s) and j < i + 4 and s[j] in "01234567":
                j += 1
            out.append(int(s[i + 1:j], 8) & 0xFF)
            i = j
        elif n in "xX":
            j = i + 2
            while j < len(s) and s[j] in "0123456789abcdefABCDEF":
                j += 1
            out.append(int(s[i + 2:j], 16) & 0xFF)
            i = j
        elif n in ESC:
            out.append(ESC[n])
            i += 2
        else:
            raise ValueError("unknown escape \\%s" % n)
    return bytes(out)


def hide_strings(dirs, names):
    """Rewrite the strings that contain one of NAMES as .byte lines. Returns
    (strings rewritten, files changed)."""
    nstr = nfiles = 0
    for base in dirs:
        for d, sub, fs in os.walk(base):
            for f in fs:
                if not f.endswith(".s"):
                    continue
                p = os.path.join(d, f)
                lines = open(p, errors="surrogateescape").read().split("\n")
                changed = False
                for k, line in enumerate(lines):
                    m = STRING_RE.match(line)
                    if not m or not set(WORD.findall(m.group("s"))) & names:
                        continue
                    b = unescape(m.group("s")) + (b"\0" if m.group("kind") == "asciz" else b"")
                    rows = [", ".join("0x%02X" % x for x in b[i:i + 16]) for i in range(0, len(b), 16)]
                    pad = " " * len(m.group("pre"))
                    lines[k] = "\n".join(("%s.byte %s" % (m.group("pre") if i == 0 else pad, r))
                                         for i, r in enumerate(rows))
                    changed = True
                    nstr += 1
                if changed:
                    nfiles += 1
                    with open(p, "w", errors="surrogateescape") as fh:
                        fh.write("\n".join(lines))
    return nstr, nfiles


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("rom")
    ap.add_argument("out", nargs="?", help="the splat symbol_addrs file to write")
    ap.add_argument("--version", default="us")
    ap.add_argument("--modules", help="also write a module list here")
    ap.add_argument("--aliases", help="also write the build-internal linker aliases here")
    ap.add_argument("--used-in", nargs="*", metavar="PATH",
                    help="files and directories with the names allowed in --aliases and "
                         "--hide-strings (default: the project's licensed sources)")
    ap.add_argument("--hide-strings", nargs="*", default=[], metavar="DIR",
                    help="rewrite strings in DIR's .s files that contain a ROM name")
    args = ap.parse_args()

    L = LAYOUT[args.version]
    rom = open(args.rom, "rb").read()
    table = read_table(rom, L)
    mods = read_modtab(rom, L)
    used = None
    if args.aliases or args.hide_strings:
        used = words_of(args.used_in if args.used_in is not None else
                        [p for p in USED_IN if os.path.exists(p)])

    if args.out:
        seen = set()
        lines = []
        for mod, off, _ in table:
            addr = main_address(L, mod, off)
            if addr is None or addr in seen or addr < L["text"]:
                continue  # a module's symbol, a second one at the address, or low memory
            seen.add(addr)
            # per-function asm files are named by address (func_XXXXXXXX.s)
            suffix = " // type:func filename:func_%08X" % addr if addr < L["data"] else ""
            lines.append("%s = 0x%08X;%s" % (address_name(L, addr), addr, suffix))
        with open(args.out, "w") as f:
            f.write("// generated by tools/runlink_syms.py from the ROM's runlink tables (addresses only)\n")
            f.write("\n".join(sorted(lines, key=lambda x: x.split(" = ")[1])) + "\n")
        print("runlink_syms: %d main symbols" % len(seen))

    if args.aliases:
        n = write_aliases(args.aliases, table, L, mods, used)
        print("runlink_syms: %d build-internal aliases" % n)

    if args.hide_strings:
        rom_only = set(n for _, _, n in table) - used
        print("runlink_syms: %d strings rewritten as bytes in %d files" % hide_strings(args.hide_strings,
                                                                                      rom_only))

    if args.modules:
        nmods = len(mods)
        with open(args.modules, "w") as f:
            f.write("# idx rom_start text data bss relocs1 relocs2 init fini\n")
            for m in range(nmods):
                e = mods[m]
                f.write("%3d 0x%07X 0x%05X 0x%05X 0x%05X 0x%04X 0x%04X %d %d\n" % (
                    m + 1, L["modules"] + e[1], e[2], e[3], e[4], e[5], e[6], e[7], e[8]))


if __name__ == "__main__":
    main()
