#!/usr/bin/env python3
"""Build Jet Force Gemini's runlink module images and module table.

Input: one relocatable MIPS ELF object per module (m001.o ... in module
order), the runlink symbol table (index and name of every global symbol,
written by tools/runlink_split.py), and optionally the original relocation
entry order per module. Output: the module images (text, data, the two
relocation tables, back to back) and the 0x20-byte-per-module table that
points at them. See docs/runlink.md for the format.

How ELF relocations map to runlink entries:
  undefined symbol S                 -> table 1, kind 0, word = index of S
  __rlk2_<index> (undefined marker)  -> table 2, kind 2, word = index
  rl_mNNN_<off> defined here         -> table 2, kind 1, word = its module offset
  other symbol defined here          -> table 2, kind 0, word = index of S
  section symbol + addend            -> table 2, kind 1, word = section's module
                                        offset (what IDO objects contain)
The entry is (word, offset << 8 | type << 4 | kind), type being the ELF
MIPS relocation type (R_MIPS_32, R_MIPS_26, R_MIPS_HI16, R_MIPS_LO16).

Usage: runlink_build.py --symbols symbols.txt [--order DIR] --out-modules F
       --out-table F m001.o m002.o ...
"""
import argparse
import os
import re
import struct


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
            secs.append(dict(name=name, type=typ, off=off, size=size, link=link, info=info, entsize=entsize))
        strtab = secs[shstrndx]
        for s in secs:
            s["name"] = self._str(d, strtab["off"] + s["name"])
        self.d, self.secs = d, secs
        self.byname = {s["name"]: i for i, s in enumerate(secs)}
        self.syms = []
        for s in secs:
            if s["type"] == 2:  # SHT_SYMTAB
                st = secs[s["link"]]
                for i in range(s["size"] // 16):
                    nm, val, size, info, other, shndx = struct.unpack_from(">IIIBBH", d, s["off"] + 16 * i)
                    self.syms.append(dict(name=self._str(d, st["off"] + nm), value=val, bind=info >> 4,
                                          type=info & 0xF, shndx=shndx))

    @staticmethod
    def _str(d, o):
        return d[o:d.index(b"\0", o)].decode("ascii")

    def section(self, name):
        i = self.byname.get(name)
        if i is None:
            return b"", 0, None
        s = self.secs[i]
        data = b"" if s["type"] == 8 else self.d[s["off"]:s["off"] + s["size"]]  # SHT_NOBITS
        return data, s["size"], i

    def rels(self, target_index):
        out = []
        for s in self.secs:
            if s["type"] == 9 and s["info"] == target_index:  # SHT_REL
                for i in range(s["size"] // 8):
                    off, info = struct.unpack_from(">II", self.d, s["off"] + 8 * i)
                    out.append((off, info & 0xFF, info >> 8))
        return out


def build_module(path, me, index, order):
    e = Elf(path)
    text, tsize, ti = e.section(".text")
    data, dsize, di = e.section(".data")
    _, bsize, bi = e.section(".bss")
    secbase = {ti: 0, di: tsize, bi: tsize + dsize}
    local_re = re.compile(r"rl_m%03d_([0-9A-F]+)$" % me)
    t1, t2 = [], []
    for sidx, base in ((ti, 0), (di, tsize)):
        if sidx is None:
            continue
        for off, typ, symi in e.rels(sidx):
            s = e.syms[symi]
            name = s["name"]
            if name.startswith("__rlk2_"):
                tab, kind, word = t2, 2, int(name[7:])
            elif s["shndx"] == 0:
                if name not in index:
                    raise SystemExit("m%03d: %s is not in the runlink symbol table" % (me, name))
                tab, kind, word = t1, 0, index[name]
            elif s["type"] == 3:  # STT_SECTION
                tab, kind, word = t2, 1, secbase[s["shndx"]]
            elif local_re.match(name):
                tab, kind, word = t2, 1, secbase[s["shndx"]] + s["value"]
            else:
                tab, kind, word = t2, 0, index[name]
            tab.append((base + off, typ, kind, word))
    if order is not None:
        rank = {k: i for i, k in enumerate(order)}
        for ti_, tab in ((1, t1), (2, t2)):
            tab.sort(key=lambda r: rank.get((ti_, r[0], r[1]), len(rank)))
    init = fini = -1
    for s in e.syms:
        if s["name"] in ("__rl_init", "__rl_fini") and s["shndx"] != 0:
            v = secbase.get(s["shndx"], 0) + s["value"]
            if s["name"] == "__rl_init":
                init = v
            else:
                fini = v
    out = bytearray(text + data)
    for tab in (t1, t2):
        for off, typ, kind, word in tab:
            out += struct.pack(">II", word, (off << 8) | (typ << 4) | kind)
    return bytes(out), (tsize, dsize, bsize, 8 * len(t1), 8 * len(t2), init, fini)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--symbols", required=True)
    ap.add_argument("--order")
    ap.add_argument("--out-modules", required=True)
    ap.add_argument("--out-table", required=True)
    ap.add_argument("objects", nargs="+")
    args = ap.parse_args()
    index = {}
    for line in open(args.symbols):
        i, mod, off, name = line.split()
        index.setdefault(name, int(i))
    mods = bytearray()
    table = bytearray()
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
        img, (t, d, b, r1, r2, init, fini) = build_module(path, me, index, order)
        table += struct.pack(">IIIIIHHii", 0, len(mods), t, d, b, r1, r2, init, fini)
        mods += img
    # the images are followed by zero padding up to a 16-byte boundary
    mods += b"\0" * (-len(mods) % 16)
    open(args.out_modules, "wb").write(mods)
    open(args.out_table, "wb").write(table)


if __name__ == "__main__":
    main()
