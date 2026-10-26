#!/usr/bin/env python3
"""Build Jet Force Gemini's runlink module images and module table.

Input: one relocatable MIPS ELF object per module (m001.o ... in module
order), either assembled from asm/modules/mNNN.s or compiled from
src/modules/mNNN.c with IDO (plus asm-processor for INCLUDE_ASM); the runlink
symbol table (index, module, offset and name of every global symbol, written
by tools/runlink_split.py); and optionally the original relocation entry
order per module. Output: the module images (text, data, the two relocation
tables, back to back), the 0x20-byte-per-module table that points at them,
and a map of the functions in each module. See docs/runlink.md.

Module layout: .text, then .data and .rodata (the module's data block),
then .bss (allocated by the loader).

How a relocation's symbol becomes a runlink entry:
  section symbol (IDO: static data and functions)
  local symbol defined in the object
  rl_mNNN_XXXXX (NNN = this module, defined or not)
      -> table 2, kind 1, word = module offset of the section start, symbol
         or XXXXX; for R_MIPS_26, kind 2 instead: the jump field gets the
         target offset and the word repeats the previous kind-0 entry's
         (the loader ignores it; the original linker left it like that)
  func_XXXXXXXX / D_XXXXXXXX       main-segment symbol at that address
  func_mNNN_XXXXX / D_mNNN_XXXXX   symbol exported by module NNN at XXXXX
  a runlink symbol name
  a name given an address or another name in an --aliases file
  another global symbol defined in the object (by its module offset)
      -> kind 0, word = the global symbol's index; table 1 if the symbol is
         outside this module, table 2 if it is this module's own
The entry is (word, offset << 8 | type << 4 | kind), type being the ELF
MIPS relocation type (R_MIPS_32, R_MIPS_26, R_MIPS_HI16, R_MIPS_LO16).

Usage: runlink_build.py --symbols symbols.txt [--order DIR] [--aliases F ...]
       --out-modules F --out-table F [--out-map F] m001.o m002.o ...
"""
import argparse
import os
import re
import struct

LAYOUT = {
    "us": dict(text=0x80000450, data=0x800A0660, bss=0x800B0B50),
}
SEC_RODATA, SEC_DATA, SEC_BSS = 4093, 4094, 4095
R_MIPS_26 = 4
SHN_UNDEF, SHN_ABS, SHN_COMMON = 0, 0xFFF1, 0xFFF2
STB_LOCAL = 0
STT_FUNC, STT_SECTION = 2, 3

ADDR_RE = re.compile(r"(?:func|D)_([0-9A-Fa-f]{8})$")
MOD_RE = re.compile(r"(?:func|D)_m(\d{3})_([0-9A-Fa-f]+)$")
LOCAL_RE = re.compile(r"rl_m(\d{3})_([0-9A-Fa-f]+)$")
ALIAS_RE = re.compile(r"^\s*(?:PROVIDE\s*\(\s*)?([A-Za-z_.$][\w.$]*)\s*=\s*([\w.$]+)\s*\)?\s*;")


class Elf:
    def __init__(self, path):
        d = open(path, "rb").read()
        if d[:4] != b"\x7fELF" or d[4] != 1 or d[5] != 2:
            raise SystemExit("%s: not a 32-bit big-endian ELF" % path)
        (shoff,) = struct.unpack_from(">I", d, 0x20)
        shentsize, shnum, shstrndx = struct.unpack_from(">HHH", d, 0x2E)
        secs = []
        for i in range(shnum):
            name, typ, flags, addr, off, size, link, info, align, entsize = struct.unpack_from(
                ">IIIIIIIIII", d, shoff + i * shentsize)
            secs.append(dict(name=name, type=typ, off=off, size=size, link=link, info=info,
                             align=align, entsize=entsize))
        strtab = secs[shstrndx]
        for s in secs:
            s["name"] = self._str(d, strtab["off"] + s["name"])
        self.path, self.d, self.secs = path, d, secs
        self.byname = {s["name"]: i for i, s in enumerate(secs)}
        self.syms = []
        for s in secs:
            if s["type"] == 2:  # SHT_SYMTAB
                st = secs[s["link"]]
                for i in range(s["size"] // 16):
                    nm, val, size, info, other, shndx = struct.unpack_from(">IIIBBH", d, s["off"] + 16 * i)
                    self.syms.append(dict(name=self._str(d, st["off"] + nm), value=val, size=size,
                                          bind=info >> 4, type=info & 0xF, shndx=shndx))

    @staticmethod
    def _str(d, o):
        return d[o:d.index(b"\0", o)].decode("ascii")

    def section(self, name):
        i = self.byname.get(name)
        if i is None:
            return b"", 0, None, 1
        s = self.secs[i]
        data = b"" if s["type"] == 8 else self.d[s["off"]:s["off"] + s["size"]]  # SHT_NOBITS
        return data, s["size"], i, max(1, s["align"])

    def rels(self, target_index):
        out = []
        for s in self.secs:
            if s["type"] == 9 and s["info"] == target_index:  # SHT_REL
                for i in range(s["size"] // 8):
                    off, info = struct.unpack_from(">II", self.d, s["off"] + 8 * i)
                    out.append((off, info & 0xFF, info >> 8))
            elif s["type"] == 4 and s["info"] == target_index:  # SHT_RELA
                raise SystemExit("%s: RELA relocations are not supported" % self.path)
        return out


class Symbols:
    """The runlink global symbol table and the names that resolve into it."""

    def __init__(self, path, layout, alias_files):
        self.L = layout
        self.byname = {}
        self.byaddr = {}
        self.mod = []
        for line in open(path):
            i, mod, off, name = line.split()
            i, mod, off = int(i), int(mod), int(off, 16)
            self.mod.append(mod)
            self.byname.setdefault(name, i)
            self.byaddr.setdefault(self.key(mod, off), i)
        self.aliases = {}
        for p in alias_files:
            for line in open(p):
                line = re.sub(r"/\*.*?\*/|//.*", "", line)
                m = ALIAS_RE.match(line)
                if m:
                    a, b = m.groups()
                    if re.match(r"0x[0-9A-Fa-f]+$", b):
                        self.aliases.setdefault(a, int(b, 16))
                    else:
                        # `func_X = name;` also tells where `name` is
                        self.aliases.setdefault(a, b)
                        if ADDR_RE.match(a) or MOD_RE.match(a):
                            self.aliases.setdefault(b, a)

    def key(self, mod, off):
        if mod == 0:
            return (0, self.L["text"] + off)
        if mod in (SEC_RODATA, SEC_DATA):
            return (0, self.L["data"] + off)
        if mod == SEC_BSS:
            return (0, self.L["bss"] + off)
        return (mod, off)

    def lookup(self, name, depth=0):
        """Index of the runlink symbol that `name` stands for, or None."""
        if name in self.byname:
            return self.byname[name]
        m = ADDR_RE.match(name)
        if m:
            return self.byaddr.get((0, int(m.group(1), 16)))
        m = MOD_RE.match(name)
        if m:
            return self.byaddr.get((int(m.group(1)), int(m.group(2), 16)))
        a = self.aliases.get(name)
        if a is not None and depth < 8:
            if isinstance(a, int):
                return self.byaddr.get((0, a))
            return self.lookup(a, depth + 1)
        return None


def align(n, a):
    return (n + a - 1) // a * a


def build_module(path, me, symbols, order):
    e = Elf(path)
    text, tsize, ti, _ = e.section(".text")
    data, dsize, di, _ = e.section(".data")
    rodata, rsize, ri, ralign = e.section(".rodata")
    _, bsize, bi, _ = e.section(".bss")
    for s in e.secs:
        if s["name"] in (".sdata", ".sbss", ".lit4", ".lit8") and s["size"]:
            raise SystemExit("%s: section %s is not supported (build with -G 0)" % (path, s["name"]))
    tsize_p = align(tsize, 16)
    text = text + b"\0" * (tsize_p - tsize)
    rbase = align(tsize_p + dsize, min(ralign, 16)) if rsize else tsize_p + dsize
    block = data + b"\0" * (rbase - tsize_p - dsize) + rodata
    dtotal = len(block)
    secbase = {ti: 0, di: tsize_p, ri: rbase, bi: tsize_p + dtotal}
    image = bytearray(text + block)

    t1, t2 = [], []
    for sidx, base in ((ti, 0), (di, tsize_p), (ri, rbase)):
        if sidx is None:
            continue
        for off, typ, symi in e.rels(sidx):
            s = e.syms[symi]
            name = s["name"]
            where = base + off
            defined = s["shndx"] not in (SHN_UNDEF, SHN_ABS, SHN_COMMON)
            if s["shndx"] == SHN_COMMON:
                raise SystemExit("%s: %s is a common symbol; define it (or make it static)" % (path, name))
            local = None  # module offset of a kind-1/kind-2 target base
            idx = None
            if s["type"] == STT_SECTION:
                local = secbase[s["shndx"]]
            elif defined and s["bind"] == STB_LOCAL:
                local = secbase[s["shndx"]] + s["value"]
            else:
                m = LOCAL_RE.match(name)
                if m and int(m.group(1)) == me:
                    local = int(m.group(2), 16)
                else:
                    idx = symbols.lookup(name)
                    if idx is None and defined:
                        idx = symbols.byaddr.get((me, secbase[s["shndx"]] + s["value"]))
                    if idx is None:
                        raise SystemExit("m%03d (%s): %s is not in the runlink symbol table%s" % (
                            me, path, name, " (a module's own unexported symbols must be static)"
                            if defined else ""))
            if local is not None:
                if typ == R_MIPS_26:
                    w = struct.unpack_from(">I", image, where)[0]
                    tgt = local + ((w & 0x3FFFFFF) << 2)
                    struct.pack_into(">I", image, where, (w & 0xFC000000) | ((tgt >> 2) & 0x3FFFFFF))
                    t2.append([where, typ, 2, None])
                else:
                    t2.append([where, typ, 1, local])
            else:
                tab = t2 if symbols.mod[idx] == me else t1
                tab.append([where, typ, 0, idx])
    if order is not None:
        rank = {k: i for i, k in enumerate(order)}
        for tn, tab in ((1, t1), (2, t2)):
            tab.sort(key=lambda r: rank.get((tn, r[0], r[1]), len(rank)))
    last = None
    for r in t1 + t2:
        if r[2] == 0:
            last = r[3]
        elif r[2] == 2:
            if last is None:
                raise SystemExit("m%03d: a jump inside the module comes before any global "
                                 "symbol reference; its runlink word is undefined" % me)
            r[3] = last
    init = fini = -1
    funcs = []
    for s in e.syms:
        if s["shndx"] in (SHN_UNDEF, SHN_ABS, SHN_COMMON) or s["shndx"] not in secbase:
            continue
        v = secbase[s["shndx"]] + s["value"]
        if s["name"] == "__rl_init":
            init = v
        elif s["name"] == "__rl_fini":
            fini = v
        elif (s["shndx"] == ti and s["type"] == STT_FUNC and "." not in s["name"]
              and not s["name"].startswith(("rl_m", "__"))):
            funcs.append((v, s["size"], s["name"]))
    funcs.sort()
    fmap = []
    seen = set()
    starts = sorted(set(v for v, _, _ in funcs))
    for v, size, name in funcs:
        if v in seen:
            continue
        seen.add(v)
        nxt = next((x for x in starts if x > v), tsize)
        fmap.append((name, v, size or (nxt - v)))
    out = bytes(image)
    for tab in (t1, t2):
        for off, typ, kind, word in tab:
            out += struct.pack(">II", word, (off << 8) | (typ << 4) | kind)
    return out, (tsize_p, dtotal, bsize, 8 * len(t1), 8 * len(t2), init, fini), fmap


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--symbols", required=True)
    ap.add_argument("--order")
    ap.add_argument("--aliases", nargs="*", default=[],
                    help="linker-script style files of `name = name_or_address;` lines "
                         "(default: undefined_syms.VERSION.txt)")
    ap.add_argument("--version", default="us")
    ap.add_argument("--out-modules", required=True)
    ap.add_argument("--out-table", required=True)
    ap.add_argument("--out-map", help="default: modules.map next to --out-modules")
    ap.add_argument("objects", nargs="+")
    args = ap.parse_args()
    if not args.aliases:
        args.aliases = ["undefined_syms.%s.txt" % args.version]
    if not args.out_map:
        args.out_map = os.path.join(os.path.dirname(args.out_modules), "modules.map")
    symbols = Symbols(args.symbols, LAYOUT[args.version], [p for p in args.aliases if os.path.exists(p)])
    mods = bytearray()
    table = bytearray()
    maplines = ["# module image_offset text data bss object",
                "#   function module_offset size"]
    for n, path in enumerate(args.objects):
        me = n + 1
        order = None
        if args.order:
            p = os.path.join(args.order, "m%03d.txt" % me)
            if os.path.exists(p):
                order = []
                for line in open(p):
                    t, o, ty = line.split()
                    order.append((int(t), int(o, 16), int(ty)))
        img, (t, d, b, r1, r2, init, fini), fmap = build_module(path, me, symbols, order)
        table += struct.pack(">IIIIIHHii", 0, len(mods), t, d, b, r1, r2, init, fini)
        maplines.append("m%03d 0x%06X 0x%05X 0x%05X 0x%05X %s" % (me, len(mods), t, d, b, path))
        for name, v, size in fmap:
            maplines.append("  %s 0x%05X 0x%X" % (name, v, size))
        mods += img
    # the images are followed by zero padding up to a 16-byte boundary
    mods += b"\0" * (-len(mods) % 16)
    open(args.out_modules, "wb").write(mods)
    open(args.out_table, "wb").write(table)
    if args.out_map:
        open(args.out_map, "w").write("\n".join(maplines) + "\n")


if __name__ == "__main__":
    main()
