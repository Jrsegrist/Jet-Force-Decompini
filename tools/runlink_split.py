#!/usr/bin/env python3
"""Split Jet Force Gemini's runlink modules into assembly files.

Each of the 157 run-time linked modules becomes asm/modules/mNNN.s with its
.text (disassembled), .data and .bss, and relocated fields written
symbolically so that the assembler emits ordinary ELF relocations. The
module's runlink relocation tables are not copied: tools/runlink_build.py
recreates them from the assembled objects. See docs/runlink.md.

For work in C, every module is also split per function into
asm/nonmatchings/modules/mNNN/func_mNNN_XXXXX.s (for INCLUDE_ASM), with
data.s, rodata.s and bss.s (the module's sections, as far as it has them)
and functions.txt (the function list) next to them.

Symbol references in the generated assembly are by address, so that C written
from it never contains a name from the ROM's name table:
  func_XXXXXXXX / D_XXXXXXXX   a main-segment symbol (text / data or bss)
  func_mNNN_XXXXX / D_mNNN_XXXXX
                  a symbol exported by module NNN at offset XXXXX
  rl_mNNN_XXXXX   module NNN's own code or data at offset XXXXX that is
                  not referred to through the global symbol table (runlink
                  "kind 1"; or the target of a "kind 2" jump)
The runlink names themselves only appear in comments. When two runlink
symbols share an address and a reference uses the second one, the runlink
name is used instead (tools/runlink_build.py accepts both).

The relocation entry order of the original tables is written to
build/runlink/order/mNNN.txt (an IDO-compiled module reproduces it from its
own object; the assembler sorts relocations differently). Everything
written here is derived from the ROM and is not committed.

Usage: runlink_split.py ROM [--version us]
"""
import argparse
import os
import struct

import rabbitizer

LAYOUT = {
    "us": dict(symtab=0x1ED0270, modtab=0x1ED2780, modules=0x1ED3B20,
               nameoffs=0x1FEB040, names=0x1FED550,
               text=0x80000450, data=0x800A0660, bss=0x800B0B50),
}
SEC_RODATA, SEC_DATA, SEC_BSS = 4093, 4094, 4095
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


def read_modtab(rom, L):
    nmods = (L["modules"] - L["modtab"]) // 32
    return [struct.unpack_from(">IIIIIHHii", rom, L["modtab"] + 32 * m) for m in range(nmods)]


def address_name(L, mods, mod, off):
    """The address-based name of runlink symbol (mod, off)."""
    if mod == 0:
        addr = L["text"] + off
        return ("func_%08X" if addr < L["data"] else "D_%08X") % addr
    if mod in (SEC_RODATA, SEC_DATA):
        return "D_%08X" % (L["data"] + off)
    if mod == SEC_BSS:
        return "D_%08X" % (L["bss"] + off)
    tsize = mods[mod - 1][2]
    return ("func_m%03d_%05X" if off < tsize else "D_m%03d_%05X") % (mod, off)


def symbol_names(L, mods, syms):
    """Name to use for each symbol index: by address, unless an earlier
    symbol has the same address (then the runlink name)."""
    first = {}
    out = []
    for i, (mod, off, name) in enumerate(syms):
        key = (mod if mod not in (SEC_RODATA, SEC_DATA) else SEC_DATA, off)
        if not (mod <= len(mods) or mod in (SEC_RODATA, SEC_DATA, SEC_BSS)):
            print("symbol %d (%s): unknown module %d" % (i, name, mod))
            out.append(name)
        elif key in first:
            out.append(name)
        else:
            first[key] = i
            out.append(address_name(L, mods, mod, off))
    return out


def find_jump_tables(image, tsize, dsize, lo_info, rel32):
    """IDO switch jump tables: a `jr $reg` (not $ra) whose register was
    loaded by a `lw` through a %lo into the module's data block. The table
    is the run of R_MIPS_32 words there that point into .text, as long as
    the `sltiu` bound before the jump says (or until the next table).
    Returns ({table offset: [case offsets]}, {jr offset: [case offsets]},
    {%lo offset: table offset})."""
    words = [u32(image, o) for o in range(0, tsize, 4)]
    found = []  # (jr offset, lw offset, table offset, bound or None)
    for o in range(0, tsize, 4):
        w = words[o // 4]
        rs = (w >> 21) & 31
        if (w & 0xFC1FFFFF) != 0x00000008 or rs == 31:
            continue
        lw = bound = None
        for p in range(o - 4, max(-4, o - 0x60), -4):
            pw = words[p // 4]
            if pw == 0x03E00008:
                break
            if lw is None and p in lo_info and pw >> 26 == 0x23 and (pw >> 16) & 31 == rs:
                lw = p
            elif lw is not None and pw >> 26 == 0x0B and bound is None:  # sltiu
                bound = pw & 0xFFFF
                break
        if lw is None:
            continue
        t = lo_info[lw][1]
        if tsize <= t < tsize + dsize:
            found.append((o, lw, t, bound))
    starts = set(t for _, _, t, _ in found)
    tables, jumps, los = {}, {}, {}
    for o, lw, t, bound in found:
        if t not in tables:
            cases = []
            k = t
            while k in rel32 and rel32[k] < tsize and (k == t or k not in starts):
                if bound is not None and len(cases) == bound:
                    break
                cases.append(rel32[k])
                k += 4
            tables[t] = cases
        if tables[t]:
            jumps[o] = tables[t]
            los[lw] = t
    return {t: c for t, c in tables.items() if c}, jumps, los


def find_functions(image, tsize, starts, jumps=None):
    """Function start offsets: the known entry points plus every place
    after a `jr $ra` (and its delay slot) where no earlier branch or jump
    table of the current function reaches past it. Returns (sorted starts,
    end of code)."""
    jumps = jumps or {}
    words = [u32(image, o) for o in range(0, tsize, 4)]
    end = tsize
    while end > 0 and words[end // 4 - 1] == 0:
        end -= 4
    if 0 < end < tsize:
        last = rabbitizer.Instruction(words[end // 4 - 1], end - 4)
        if last.isValid() and (last.isBranch() or last.isJump()):
            end += 4  # its delay slot is a nop
    found = set(s for s in starts if s < end)
    found.add(0)
    reach = 0  # furthest branch target of the current function
    o = 0
    while o < end:
        if o in found:
            reach = o
        w = words[o // 4]
        ins = rabbitizer.Instruction(w, o)
        if ins.isValid() and ins.isBranch():
            reach = max(reach, o + 4 + (sx16(w & 0xFFFF) << 2))
        if o in jumps:
            reach = max([reach] + jumps[o])
        if w == 0x03E00008 and reach <= o + 4:
            n = o + 8
            while n < end and words[n // 4] == 0:
                n += 4
            if n < end:
                found.add(n)
        o += 4
    return sorted(found), end


def split_module(rom, L, syms, names, mods, m, asm_dir, nm_root, build_dir):
    me = m + 1
    w0, off, tsize, dsize, bsize, r1, r2, init, fini = mods[m]
    base = L["modules"] + off
    image = rom[base:base + tsize + dsize]
    end = tsize + dsize + bsize
    tabs = []
    for tstart, tlen in ((tsize + dsize, r1), (tsize + dsize + r1, r2)):
        tabs.append([struct.unpack_from(">II", rom, base + tstart + 8 * i) for i in range(tlen // 8)])

    labels = {}  # module offset -> [(name, is_global, comment)]
    exported = {}  # module offset -> [runlink names]
    for i, (mod, soff, name) in enumerate(syms):
        if mod == me:
            exported.setdefault(soff, []).append(name)
            nm = names[i]
            lst = labels.setdefault(soff, [])
            if nm not in lst:
                lst.append(nm)
    prefix = "rl_m%03d_" % me

    def local(o):
        nm = prefix + "%05X" % o
        lst = labels.setdefault(o, [])
        if nm not in lst:
            lst.append(nm)
        return nm

    sub = {}
    order = []
    secbases = set()
    jal_targets = set()
    lo_info = {}  # %lo offset -> (its %hi's offset or None, target module offset)
    rel32 = {}  # R_MIPS_32 offset -> target module offset (kind 1 only)
    for ti, entries in enumerate(tabs):
        last_hi = {}
        for word, x in entries:
            o, typ, kind = x >> 8, (x >> 4) & 0xF, x & 0xF
            order.append("%d 0x%X %d" % (ti + 1, o, typ))
            if kind == 0:
                sym = names[word]
            elif kind == 1:
                sym = local(word)
                secbases.add(word)
            elif kind == 2:
                sym = None
            else:
                raise SystemExit("m%03d: unknown relocation kind %d" % (me, kind))
            ins = u32(image, o)
            if typ == R_MIPS_26:
                tgt = (ins & 0x3FFFFFF) << 2
                if kind == 2:
                    # the loader ignores the entry's word; runlink_build.py
                    # recomputes it
                    sub[o] = (typ, local(tgt))
                    jal_targets.add(tgt)
                else:
                    sub[o] = (typ, expr(sym, tgt))
                    if syms[word][0] == me:
                        jal_targets.add(syms[word][1] + tgt)
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
                if kind == 1:
                    lo_info[o] = (ho, word + a)
            elif typ == R_MIPS_32:
                a = ins - 0x100000000 if ins & 0x80000000 else ins
                sub[o] = (typ, expr(sym, a))
                if kind == 1:
                    rel32[o] = word + a
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
    jtbls, jtbl_jumps, jtbl_los = find_jump_tables(image, tsize, dsize, lo_info, rel32)
    jtargets = set(t for ts in jtbls.values() for t in ts)

    starts = set(k for k in exported if k < tsize) | jal_targets
    for v in (init, fini):
        if v >= 0:
            starts.add(v)
    funcs, code_end = find_functions(image, tsize, starts, jtbl_jumps)
    if code_end == 0:
        funcs = []
    fnames = {}
    for f in funcs:
        fnames[f] = "func_m%03d_%05X" % (me, f)

    def emit_labels(o, functype, macros=False):
        """Labels at module offset o. With macros, the prelude.inc forms
        that asm-processor accepts (jlabel in code, dlabel in data)."""
        out = []
        if o in exported:
            out.append("/* %s */" % ", ".join(exported[o]))
        for nm in labels.get(o, []):
            if o in fnames and nm == fnames[o]:
                continue
            if macros:
                out.append("%s %s" % ("jlabel" if functype else "dlabel", nm))
                continue
            out.append(".global %s" % nm)
            if functype:
                out.append(".type %s, @function" % nm)
            out.append("%s:" % nm)
        if o in btargets or o in jtargets:
            out.append(".L%05X:" % o)
        return out

    # In the per-function files, a jump table is read through its own local
    # label jtbl_mNNN_XXXXX (defined in rodata.s), and its entries are the
    # .L labels of the cases, as m2c expects. gas turns both into
    # section-relative relocations, which give the same runlink entries as
    # the rl_ section-start names used in asm/modules/mNNN.s.
    jname = {t: "jtbl_m%03d_%05X" % (me, t) for t in jtbls}
    nm_sub = {}
    for lo, t in jtbl_los.items():
        old = sub[lo][1]
        nm_sub[lo] = "%%lo(%s)" % jname[t]
        ho = lo_info[lo][0]
        if ho is not None and sub[ho][1] == "%hi" + old[3:]:
            nm_sub[ho] = "%%hi(%s)" % jname[t]
    jentry = {}
    for t, cases in jtbls.items():
        for i, c in enumerate(cases):
            jentry[t + 4 * i] = ".L%05X" % c

    def text_line(o, macros=False):
        w = u32(image, o)
        ins = rabbitizer.Instruction(w, o)
        if macros and o in nm_sub:
            s = ins.disassemble(immOverride=nm_sub[o])
        elif o in sub:
            typ, e = sub[o][0], sub[o][1]
            s = ".word %s" % e if typ == R_MIPS_32 else ins.disassemble(immOverride=e)
        elif o in branch:
            s = ins.disassemble(immOverride=".L%05X" % branch[o])
        elif ins.isValid() and ins.isImplemented() and not ins.isJType():
            s = ins.disassemble()
        else:
            s = ".word 0x%08X" % w
        return "    /* %05X %08X */ %s" % (o, w, s)

    # .data / .rodata split: kind-1 references point at section starts
    # (IDO refers to static data through section symbols), so a referenced
    # offset in the data block other than its start is where .rodata begins.
    ro = sorted(k for k in secbases if tsize < k < tsize + dsize)
    rodata_start = ro[0] if ro else None

    def data_lines(lo, hi, macros=False):
        out = []
        o = lo
        while o < hi:
            inner = [k for k in (o + 1, o + 2, o + 3) if k in labels]
            if inner and o not in sub:
                for k in range(o, o + 4):
                    out.extend(emit_labels(k, False, macros))
                    out.append("    .byte 0x%02X" % image[k])
            else:
                out.extend(emit_labels(o, False, macros))
                if macros and o in jname:
                    out.append("dlabel %s, local" % jname[o])
                elif macros and o - 4 in jentry and o not in jentry and o not in labels:
                    out.append(".LD%05X:  /* end of the jump table */" % o)
                e = sub[o][1] if o in sub else "0x%08X" % u32(image, o)
                if macros and o in jentry:
                    e = jentry[o]
                out.append("    /* %05X */ .word %s" % (o, e))
            o += 4
        return out

    def bss_lines(macros=False):
        out = []
        pos = tsize + dsize
        for k in sorted(k for k in labels if tsize + dsize <= k <= end):
            if k > pos:
                out.append("    .space 0x%X" % (k - pos))
                pos = k
            out.extend(emit_labels(k, False, macros))
        if end > pos:
            out.append("    .space 0x%X" % (end - pos))
        bad = [k for k in labels if k > end]
        if bad:
            raise SystemExit("m%03d: labels past the end: %s" % (me, bad))
        return out

    head = [
        "/* Runlink module %d, generated by tools/runlink_split.py from the ROM. */" % me,
        ".set noat",
        ".set noreorder",
        ".set gp=64",
        "",
    ]
    lines = list(head)
    for nm, v in (("__rl_init", init), ("__rl_fini", fini)):
        if v >= 0:
            lines.append(".global %s" % nm)
            lines.append(".set %s, %s" % (nm, fnames.get(v) or local(v)))

    lines.append("\n.section .text")
    for o in range(0, tsize, 4):
        if o in fnames:
            lines.append(".global %s" % fnames[o])
            lines.append(".type %s, @function" % fnames[o])
            lines.append("%s:" % fnames[o])
        lines += emit_labels(o, True)
        lines.append(text_line(o))
    lines.append("\n.section .data")
    lines += data_lines(tsize, tsize + dsize)
    lines.append("\n.section .bss")
    lines += bss_lines()
    with open("%s/m%03d.s" % (asm_dir, me), "w") as f:
        f.write("\n".join(lines) + "\n")

    # per-function files for INCLUDE_ASM, and the module's data
    nm_dir = "%s/m%03d" % (nm_root, me)
    os.makedirs(nm_dir, exist_ok=True)
    for old in os.listdir(nm_dir):
        os.remove(os.path.join(nm_dir, old))
    bounds = funcs + [code_end]
    flist = []
    for a, b in zip(bounds, bounds[1:]):
        out = ["glabel %s" % fnames[a]]
        for o in range(a, b, 4):
            out += emit_labels(o, True, True)
            out.append(text_line(o, True))
        if b == code_end and code_end < tsize:
            out.append("    /* padding to 16 bytes follows (IDO adds it) */")
        out.append("endlabel %s" % fnames[a])
        with open("%s/%s.s" % (nm_dir, fnames[a]), "w") as f:
            f.write("\n".join(out) + "\n")
        flist.append("%s 0x%X 0x%X" % (fnames[a], a, b - a))
    dend = rodata_start if rodata_start is not None else tsize + dsize
    for fn, sec, lo, hi in (("data", ".data", tsize, dend), ("rodata", ".rodata", dend, tsize + dsize)):
        if hi > lo:
            with open("%s/%s.s" % (nm_dir, fn), "w") as f:
                f.write("\n".join([".section %s" % sec] + data_lines(lo, hi, True)) + "\n")
    if bsize:
        with open("%s/bss.s" % nm_dir, "w") as f:
            f.write("\n".join([".section .bss"] + bss_lines(True)) + "\n")
    with open("%s/functions.txt" % nm_dir, "w") as f:
        f.write("# name offset size (module %d: text 0x%X, data 0x%X, bss 0x%X%s)\n" % (
            me, tsize, dsize, bsize,
            ", .rodata from 0x%X" % rodata_start if rodata_start is not None else ""))
        f.write("\n".join(flist) + ("\n" if flist else ""))
    if len(ro) > 1:
        print("m%03d: more than one section start in the data block: %s" % (me, [hex(k) for k in ro]))
    return len(funcs), len(jtbls)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("rom")
    ap.add_argument("--version", default="us")
    ap.add_argument("--asm-dir", default="asm/modules")
    ap.add_argument("--nonmatchings-dir", default="asm/nonmatchings/modules")
    ap.add_argument("--build-dir", default="build/runlink")
    args = ap.parse_args()
    L = LAYOUT[args.version]
    rom = open(args.rom, "rb").read()

    rabbitizer.config.pseudos_enablePseudos = False
    rabbitizer.config.regNames_userFpcCsr = False  # $31, not $FpcCsr, for gas

    syms = read_symbols(rom, L)
    mods = read_modtab(rom, L)
    names = symbol_names(L, mods, syms)
    os.makedirs(args.asm_dir, exist_ok=True)
    os.makedirs(args.build_dir + "/order", exist_ok=True)
    with open(args.build_dir + "/symbols.txt", "w") as f:
        for i, (mod, off, name) in enumerate(syms):
            f.write("%d %d 0x%X %s\n" % (i, mod, off, name))
    nfuncs = njtbls = 0
    for m in range(len(mods)):
        nf, nj = split_module(rom, L, syms, names, mods, m, args.asm_dir, args.nonmatchings_dir,
                              args.build_dir)
        nfuncs += nf
        njtbls += nj
    print("runlink_split: %d modules, %d functions, %d jump tables" % (len(mods), nfuncs, njtbls))


if __name__ == "__main__":
    main()
