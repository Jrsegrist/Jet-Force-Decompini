#!/usr/bin/env python3
"""Split Jet Force Gemini's runlink modules into assembly files.

Each of the 157 run-time linked modules becomes asm/modules/mNNN.s with its
.text (disassembled), .data and .bss, and relocated fields written
symbolically so that the assembler emits ordinary ELF relocations. The
module's runlink relocation tables are not copied: tools/runlink_build.py
recreates them from the assembled objects. See docs/runlink.md.

Symbol references in the generated assembly:
  name            a global symbol from the ROM's runlink symbol table
                  (external, or exported by this module)
  rl_mNNN_XXXX    this module's own data or code at module offset XXXX
                  (runlink "kind 1": the loader adds the module base)
  __rlk2_<index>  marker for a "kind 2" jump: the instruction already holds
                  the module-relative target; the entry also records a
                  symbol index that the loader does not use

The relocation entry order of the original tables is written to
build/runlink/order/mNNN.txt (an IDO-compiled module would reproduce it
from its own object; the assembler sorts relocations differently).
Everything written here is derived from the ROM and is not committed.

Usage: runlink_split.py ROM [--version us]
"""
import argparse
import os
import struct

import rabbitizer

LAYOUT = {
    "us": dict(symtab=0x1ED0270, modtab=0x1ED2780, modules=0x1ED3B20,
               nameoffs=0x1FEB040, names=0x1FED550),
}
R_MIPS_32, R_MIPS_26, R_MIPS_HI16, R_MIPS_LO16 = 2, 4, 5, 6


def u32(b, o):
    return struct.unpack_from(">I", b, o)[0]


def sx16(v):
    return v - 0x10000 if v & 0x8000 else v


def expr(sym, addend):
    if addend == 0:
        return sym
    return "%s %s 0x%X" % (sym, "+" if addend > 0 else "-", abs(addend))


def read_symbols(rom, L):
    nsyms = (L["modtab"] - L["symtab"]) // 4
    syms = []
    for i in range(nsyms):
        v = u32(rom, L["symtab"] + 4 * i)
        no = u32(rom, L["nameoffs"] + 4 * i)
        end = rom.index(b"\0", L["names"] + no)
        syms.append((v >> 20, v & 0xFFFFF, rom[L["names"] + no:end].decode("ascii")))
    return syms


def split_module(rom, L, syms, m, asm_dir, build_dir):
    me = m + 1
    w0, off, tsize, dsize, bsize, r1, r2, init, fini = struct.unpack_from(">IIIIIHHii", rom, L["modtab"] + 32 * m)
    base = L["modules"] + off
    image = rom[base:base + tsize + dsize]
    end = tsize + dsize + bsize
    tabs = []
    for tstart, tlen in ((tsize + dsize, r1), (tsize + dsize + r1, r2)):
        tabs.append([struct.unpack_from(">II", rom, base + tstart + 8 * i) for i in range(tlen // 8)])

    labels = {}  # module offset -> [names]
    for mod, soff, name in syms:
        if mod == me:
            labels.setdefault(soff, []).append(name)
    prefix = "rl_m%03d_" % me

    def local(o):
        nm = prefix + "%X" % o
        lst = labels.setdefault(o, [])
        if nm not in lst:
            lst.append(nm)
        return nm

    sub = {}
    order = []
    for ti, entries in enumerate(tabs):
        last_hi = {}
        for word, x in entries:
            o, typ, kind = x >> 8, (x >> 4) & 0xF, x & 0xF
            order.append("%d 0x%X %d" % (ti + 1, o, typ))
            if kind == 0:
                sym = syms[word][2]
            elif kind == 1:
                sym = local(word)
            elif kind == 2:
                sym = "__rlk2_%d" % word
            else:
                raise SystemExit("m%03d: unknown relocation kind %d" % (me, kind))
            ins = u32(image, o)
            if typ == R_MIPS_26:
                sub[o] = (typ, expr(sym, (ins & 0x3FFFFFF) << 2))
                if kind == 2:
                    local((ins & 0x3FFFFFF) << 2)
            elif typ == R_MIPS_HI16:
                last_hi[(word, kind)] = o
                sub[o] = (typ, None, sym)
            elif typ == R_MIPS_LO16:
                ho = last_hi.get((word, kind))
                hi = (u32(image, ho) & 0xFFFF) if ho is not None else 0
                a = (hi << 16) + sx16(ins & 0xFFFF)
                sub[o] = (typ, "%%lo(%s)" % expr(sym, a))
                if ho is not None and sub[ho][1] is None:
                    sub[ho] = (R_MIPS_HI16, "%%hi(%s)" % expr(sym, a))
            elif typ == R_MIPS_32:
                a = ins - 0x100000000 if ins & 0x80000000 else ins
                sub[o] = (typ, expr(sym, a))
            else:
                raise SystemExit("m%03d: unknown relocation type %d" % (me, typ))
    for o, v in list(sub.items()):
        if v[1] is None:  # a %hi with no %lo after it
            hi = u32(image, o) & 0xFFFF
            sub[o] = (R_MIPS_HI16, "%%hi(%s)" % expr(v[2], hi << 16))
    with open("%s/order/m%03d.txt" % (build_dir, me), "w") as f:
        f.write("\n".join(order) + ("\n" if order else ""))

    branch = {}
    for o in range(0, tsize, 4):
        if o in sub:
            continue
        w = u32(image, o)
        ins = rabbitizer.Instruction(w, o)
        if ins.isValid() and ins.isBranch():
            branch[o] = o + 4 + (sx16(w & 0xFFFF) << 2)
    btargets = set(branch.values())

    def emit_labels(o, functype):
        out = []
        for nm in labels.get(o, []):
            out.append(".global %s" % nm)
            if functype:
                out.append(".type %s, @function" % nm)
            out.append("%s:" % nm)
        if o in btargets:
            out.append(".L%05X:" % o)
        return out

    lines = [
        "/* Runlink module %d, generated by tools/runlink_split.py from the ROM. */" % me,
        ".set noat",
        ".set noreorder",
        ".set gp=64",
        "",
    ]
    for nm, v in (("__rl_init", init), ("__rl_fini", fini)):
        if v >= 0:
            tgt = labels.get(v) or [local(v)]
            lines.append(".global %s" % nm)
            lines.append(".set %s, %s" % (nm, tgt[0]))

    lines.append("\n.section .text")
    for o in range(0, tsize, 4):
        lines += emit_labels(o, True)
        w = u32(image, o)
        ins = rabbitizer.Instruction(w, o)
        if o in sub:
            typ, e = sub[o][0], sub[o][1]
            s = ".word %s" % e if typ == R_MIPS_32 else ins.disassemble(immOverride=e)
        elif o in branch:
            s = ins.disassemble(immOverride=".L%05X" % branch[o])
        elif ins.isValid() and ins.isImplemented() and not ins.isJType():
            s = ins.disassemble()
        else:
            s = ".word 0x%08X" % w
        lines.append("    /* %05X %08X */ %s" % (o, w, s))

    lines.append("\n.section .data")
    o = tsize
    while o < tsize + dsize:
        inner = [k for k in (o + 1, o + 2, o + 3) if k in labels]
        if inner and o not in sub:
            for k in range(o, o + 4):
                lines += emit_labels(k, False)
                lines.append("    .byte 0x%02X" % image[k])
        else:
            lines += emit_labels(o, False)
            e = sub[o][1] if o in sub else "0x%08X" % u32(image, o)
            lines.append("    /* %05X */ .word %s" % (o, e))
        o += 4

    lines.append("\n.section .bss")
    pos = tsize + dsize
    for k in sorted(k for k in labels if tsize + dsize <= k <= end):
        if k > pos:
            lines.append("    .space 0x%X" % (k - pos))
            pos = k
        lines += emit_labels(k, False)
    if end > pos:
        lines.append("    .space 0x%X" % (end - pos))
    bad = [k for k in labels if k > end]
    if bad:
        raise SystemExit("m%03d: labels past the end: %s" % (me, bad))
    with open("%s/m%03d.s" % (asm_dir, me), "w") as f:
        f.write("\n".join(lines) + "\n")


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("rom")
    ap.add_argument("--version", default="us")
    ap.add_argument("--asm-dir", default="asm/modules")
    ap.add_argument("--build-dir", default="build/runlink")
    args = ap.parse_args()
    L = LAYOUT[args.version]
    rom = open(args.rom, "rb").read()

    rabbitizer.config.pseudos_enablePseudos = False
    rabbitizer.config.regNames_userFpcCsr = False  # $31, not $FpcCsr, for gas

    syms = read_symbols(rom, L)
    os.makedirs(args.asm_dir, exist_ok=True)
    os.makedirs(args.build_dir + "/order", exist_ok=True)
    with open(args.build_dir + "/symbols.txt", "w") as f:
        for i, (mod, off, name) in enumerate(syms):
            f.write("%d %d 0x%X %s\n" % (i, mod, off, name))
    nmods = (L["modules"] - L["modtab"]) // 32
    for m in range(nmods):
        split_module(rom, L, syms, m, args.asm_dir, args.build_dir)
    print("runlink_split: %d modules" % nmods)


if __name__ == "__main__":
    main()
