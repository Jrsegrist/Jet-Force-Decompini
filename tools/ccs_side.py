#!/usr/bin/env python3
"""ccs_side.py: one function three ways, side by side: IDO's `cc -S` output (ugen's code, before as1
schedules it), the ROM's instructions (its asm/nonmatchings .s) and our final object code.

The function's C is compiled twice with the project's real command for its object (read from
`make -n -B`; game flags, or the module flags for src/modules): once with `cc -S`, once through
asm-processor as the build does. A NON_MATCHING body is installed in place of its #ifdef block (the
other functions keep their INCLUDE_ASM, line numbers stay those of the file); --nm compiles the
whole file with -DNON_MATCHING instead.

The listing is grouped by our object's basic blocks. Columns: the cc -S instruction that produced each
object word (with its C line), our object, the ROM. ROM and object are aligned by opcode; each cc -S
instruction is matched to the words as1 made of it (pseudo-ops expanded: la, li, lw sym, blt K, ...).
Marks:
  *        ROM and object differ here (R obj/rom = a register, I = an immediate, B = a branch target,
           OP = another instruction, +obj / +rom = a word only one side has)
  -        a cc -S instruction as1 removed: "move fwd" (copy forwarded into its uses), "mask folded"
           (an `and 255`/`and 0xFFFF` after lbu/lhu: as1 loads straight into the and's register),
           "load fwd from the store" (a reload of what was just stored), "nop", or "dropped"
  +        an object word with no cc -S instruction (as1's nops and expansions)
  fwd a>b  as1 rewrote a cc -S source register a into b in this word
  as1 retargeted a>b   as1 wrote this word's result to b instead of a (a folded mask)
  load forwarded from the store of r   as1 replaced a reload with a copy of r
  1st      the first use of a t-register in that column (c: cc -S order, o: object, r: ROM)
Then a stack-slot map for both sides: each $sp offset, the C value it holds where that can be
inferred (saved registers, parameter homes, `x = ...` stores, `&x` arguments, the variable a spilled
register last held), and the first instruction that uses it; and a summary.

examples:
  .venv/bin/python3 tools/ccs_side.py func_m061_0043C
  .venv/bin/python3 tools/ccs_side.py func_8003733C --only-diff
  .venv/bin/python3 tools/ccs_side.py weapon_ammo_capacity --summary
  .venv/bin/python3 tools/ccs_side.py func_8003733C --file build/match/try.c   (a scratch copy)
Needs `make extract` (the .s files). Scratch output goes to build/match/ccs_side[-TAG]/.
"""
import argparse
import difflib
import glob
import os
import re
import shlex
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, "match"))
import common as C  # noqa: E402

ROOT = C.ROOT
GPR = ["zero", "at", "v0", "v1", "a0", "a1", "a2", "a3", "t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7",
       "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7", "t8", "t9", "k0", "k1", "gp", "sp", "fp", "ra"]
GNUM = {n: i for i, n in enumerate(GPR)}
GNUM["s8"] = 30
TREGS = {"r%d" % i for i in (8, 9, 10, 11, 12, 13, 14, 15, 24, 25)}
CALLEE_SAVED = {"r%d" % i for i in list(range(16, 24)) + [30, 31]} | {"f%d" % i for i in range(20, 32)}
SP = "r29"
NM_RE = re.compile(r'(#ifdef NON_MATCHING\n)((?:(?!#ifdef NON_MATCHING).)*?)(#else\nINCLUDE_ASM\("[^"]+", (\w+)\);\n#endif\n)',
                   re.S)
COND_BR = {"beq", "bne", "blez", "bgtz", "bltz", "bgez", "bltzal", "bgezal", "bc1t", "bc1f", "beql", "bnel",
           "blezl", "bgtzl", "bltzl", "bgezl", "bc1tl", "bc1fl", "b", "bal"}
LOADS = {"lb", "lbu", "lh", "lhu", "lw", "lwl", "lwr", "lwc1", "ldc1", "ld"}
STORES = {"sb", "sh", "sw", "swl", "swr", "swc1", "sdc1", "sd"}
CCS_BR = {"beq", "bne", "blt", "ble", "bgt", "bge", "bltu", "bleu", "bgtu", "bgeu", "beqz", "bnez", "bltz",
          "blez", "bgtz", "bgez", "b", "bc1t", "bc1f", "bal", "bltzal", "bgezal", "beql", "bnel"}


def rname(r):
    if r is None:
        return "?"
    if r[0] == "r" and r[1:].isdigit():
        return GPR[int(r[1:])]
    if r[0] == "f":
        return "f" + r[1:]
    return r.split(":", 1)[-1]


def canon(tok):
    """'$t6' / '$14' / '$f4' / '$fp' -> 'r14', 'f4', 'r30'; anything else None"""
    if not tok.startswith("$"):
        return None
    t = tok[1:]
    if t in GNUM:
        return "r%d" % GNUM[t]
    if t.isdigit():
        return "r%d" % int(t) if int(t) < 32 else None
    m = re.fullmatch(r"f(\d+)", t)
    if m:
        return "f" + m.group(1)
    return "c:" + t


def num(s):
    s = s.strip()
    try:
        return int(s, 0)
    except ValueError:
        try:
            return int(s)
        except ValueError:
            pass
    try:
        return float(s)
    except ValueError:
        return None


def sx16(v):
    return v - 0x10000 if v & 0x8000 else v


# ---------------------------------------------------------------- machine code (ROM and object)

class MIns:
    """One machine word: mn, ops (display strings), regs (canonical, operand order), imm, target."""

    def __init__(self, idx, off, word, dis, vram):
        self.idx, self.off, self.word = idx, off, word
        self.mask = 0xFFFFFFFF
        self.sym = None
        self.target = None
        self.ccs = None  # (ccs index, template) matched
        txt = dis(word, vram + off)
        if word == 0:
            txt = "nop"
        p = txt.split(None, 1)
        self.mn = p[0] if p else "?"
        self.ops = [o.strip() for o in p[1].split(",")] if len(p) > 1 else []
        self.is_branch = self.mn in COND_BR
        self.is_jump = self.mn in ("j", "jal", "jr", "jalr")
        self.is_call = self.mn in ("jal", "jalr")
        if self.is_branch and self.ops:
            self.target = off + 4 + (sx16(word & 0xFFFF) << 2)
            self.ops[-1] = "+0x%03X" % self.target
        if self.mn in ("j", "jal"):
            self.jfield = (word & 0x3FFFFFF) << 2
        self.regs, self.imm = [], None
        for k, o in enumerate(self.ops):
            if self.is_branch and k == len(self.ops) - 1:
                continue
            m = re.fullmatch(r"(-?(?:0x[0-9A-Fa-f]+|\d+))?\((\$\w+)\)", o)
            if m:
                self.imm = int(m.group(1), 0) if m.group(1) else 0
                self.regs.append(canon(m.group(2)))
            elif o.startswith("$"):
                self.regs.append(canon(o))
            elif self.mn not in ("j", "jal"):
                v = num(o)
                if v is not None:
                    self.imm = v
        self.sp_off = None
        if len(self.regs) >= 2 and self.regs[-1] == SP and (self.mn in LOADS or self.mn in STORES):
            self.sp_off, self.sp_kind = self.imm, "store" if self.mn in STORES else "load"
        elif self.mn == "addiu" and len(self.regs) == 2 and self.regs[1] == SP and self.regs[0] != SP:
            self.sp_off, self.sp_kind = self.imm, "addr"

    def set_reloc(self, kind, sym):
        """kind: 'hi', 'lo' or 'j'"""
        self.sym = sym
        if kind == "j":
            self.mask = 0xFC000000
            self.ops = [sym]
            return
        self.mask = 0xFFFF0000
        for k in range(len(self.ops) - 1, -1, -1):
            o = self.ops[k]
            m = re.fullmatch(r"(-?(?:0x[0-9A-Fa-f]+|\d+))?\((\$\w+)\)", o)
            if m:
                self.ops[k] = "%%%s(%s)(%s)" % (kind, sym, m.group(2))
                break
            if not o.startswith("$"):
                self.ops[k] = "%%%s(%s)" % (kind, sym)
                break
        if self.sp_off is not None and kind == "lo":
            self.sp_off = None

    def text(self):
        return ("%-7s %s" % (self.mn, ", ".join(self.ops))).strip()

    def dest(self):
        if self.mn in STORES or self.is_branch or self.is_jump or not self.regs:
            return None
        if self.mn in ("mult", "multu", "div", "divu"):
            return None
        if self.mn in ("mtc1", "ctc1"):
            return self.regs[1] if len(self.regs) > 1 else None
        return self.regs[0]


def read_rom_s(path, dis):
    """the ROM's words for the function from its .s: [MIns], vram of the first word"""
    out = []
    first = None
    vram = 0
    for line in open(path):
        m = re.match(r"\s*/\*([^*]*)\*/\s*(\S+)\s*(.*?)\s*$", line)
        if not m:
            continue
        toks = m.group(1).split()
        if not toks or not re.fullmatch(r"[0-9A-Fa-f]{8}", toks[-1]) or m.group(2).startswith("."):
            continue
        w = int(toks[-1], 16)
        if first is None:
            first = toks
            vram = int(toks[1], 16) if len(toks) >= 3 else int(toks[0], 16)
        ins = MIns(len(out), 4 * len(out), w, dis, vram)
        args = m.group(3)
        hm = re.search(r"%(hi|lo)\(([^)]*)\)", args)
        if hm:
            ins.set_reloc(hm.group(1), hm.group(2))
        elif m.group(2) in ("jal", "j") and args and not args.startswith((".", "$")):
            ins.set_reloc("j", args.split()[0])
        out.append(ins)
    return out, vram


def read_obj(obj, cname, dis, vram):
    text, relocs, dump = C.text_and_relocs(obj)
    funcs = C.text_functions(dump)
    found = [(o, s) for o, s, n in funcs if n == cname]
    if not found:
        return None, "symbol %s not in the object (functions: %s)" % (cname, ", ".join(n for _, _, n in funcs)[:400])
    st, size = found[0]
    byoff = {o: n for o, s, n in funcs}
    out = []
    for k in range(st, st + size, 4):
        ins = MIns(len(out), k - st, C.u32(text, k), dis, vram)
        r = relocs.get(k)
        if r:
            typ, sym = r
            if typ == "R_MIPS_26":
                if sym.startswith("."):
                    sym = byoff.get(ins.jfield, "%s+0x%X" % (sym, ins.jfield))
                ins.set_reloc("j", sym)
            elif typ in ("R_MIPS_HI16", "R_MIPS_LO16"):
                if sym.startswith(".") and typ == "R_MIPS_LO16":
                    sym = "%s+0x%X" % (sym, ins.word & 0xFFFF)
                ins.set_reloc("hi" if typ == "R_MIPS_HI16" else "lo", sym)
        out.append(ins)
    while len(out) > 1 and out[-1].word == 0 and out[-2].mn == "jr":
        # (IDO pads nothing, but an object's last function may be followed by alignment nops)
        if len(out) >= 3 and out[-3].mn == "jr":
            out.pop()
        else:
            break
    return out, None


def frame_of(ins):
    for x in ins[:40]:
        if x.mn == "addiu" and x.regs[:2] == [SP, SP] and isinstance(x.imm, int) and x.imm < 0:
            return -x.imm
    return 0


# ---------------------------------------------------------------- cc -S

class CIns:
    def __init__(self, idx, mn, ops, line, labels, prologue):
        self.idx, self.mn, self.ops, self.line, self.labels, self.prologue = idx, mn, ops, line, labels, prologue
        self.tpls = []
        self.label_op = None
        if mn in CCS_BR and ops:
            self.label_op = len(ops) - 1
        elif mn == "j" and ops and canon(ops[0]) is None:
            self.label_op = 0
        self.regs = []
        for k, o in enumerate(ops):
            if k == self.label_op:
                continue
            m = re.fullmatch(r"(.*)\((\$\w+)\)", o)
            if m:
                self.regs.append(canon(m.group(2)))
            elif canon(o):
                self.regs.append(canon(o))
        self.sp_off = None
        if mn in LOADS | STORES | {"l.s", "s.s", "l.d", "s.d"} and len(ops) == 2:
            m = re.fullmatch(r"(-?\d+)\(\$sp\)", ops[1])
            if m:
                self.sp_off, self.sp_kind = int(m.group(1)), "store" if mn[0] == "s" else "load"
        elif mn in ("addu", "addiu") and len(ops) == 3 and ops[1] == "$sp" and ops[0] != "$sp" and num(ops[2]) is not None:
            self.sp_off, self.sp_kind = num(ops[2]), "addr"

    def is_br(self):
        return self.mn in CCS_BR or self.mn in ("j", "jal", "jr", "jalr")

    def dest(self):
        if self.mn in STORES or self.mn in ("s.s", "s.d") or self.is_br() or not self.ops:
            return None
        if self.mn in ("mtc1", "ctc1"):
            return canon(self.ops[1]) if len(self.ops) > 1 else None
        if self.mn in ("mult", "multu") or (self.mn in ("div", "divu") and len(self.ops) == 2):
            return None
        return canon(self.ops[0])

    def text(self):
        ops = []
        for k, o in enumerate(self.ops):
            if k == self.label_op:
                ops.append(o)
                continue
            ops.append(re.sub(r"\$(\d+|f\d+)\b", lambda m: "$" + rname(canon(m.group(0))) if canon(m.group(0)) else m.group(0), o))
        return ("%-7s %s" % (self.mn, ", ".join(ops))).strip()


def parse_ccs(path, cname):
    ins = []
    frame = None
    on = data = False
    line = None
    pending = []
    prologue = True
    for raw in open(path):
        s = raw.strip()
        if not on:
            if re.match(r"\.ent\s+%s\b" % re.escape(cname), s):
                on = True
            continue
        if re.match(r"\.end\s+%s\b" % re.escape(cname), s):
            break
        if not s or s.startswith("#"):
            continue
        if s.startswith("."):
            d = s.split()[0]
            if d in (".rdata", ".data", ".sdata", ".bss", ".lit4", ".lit8", ".rodata", ".sbss"):
                data = True
            elif d == ".text":
                data = False
            elif d == ".loc":
                p = s.split()
                if len(p) >= 3 and p[2].isdigit():
                    line = int(p[2])
            elif d == ".frame":
                p = s[len(".frame"):].split(",")
                if len(p) >= 2 and num(p[1]) is not None:
                    frame = num(p[1])
                prologue = False
            continue
        if data:
            continue
        m = re.match(r"^(\$?[\w.$]+):\s*(.*)$", s)
        if m and not raw[:1].isspace():
            pending.append(m.group(1))
            s = m.group(2).strip()
            if not s:
                continue
        p = s.split(None, 1)
        ops = [o.strip() for o in p[1].split(",")] if len(p) > 1 else []
        ins.append(CIns(len(ins), p[0], ops, line, pending, prologue))
        pending = []
    return ins, frame


class Tpl:
    def __init__(self, ci, mn, regs=None, imm=None, must=(), sym=None):
        self.ci = ci
        self.mn = {mn} if isinstance(mn, str) else set(mn)
        self.regs, self.imm, self.must, self.sym = regs, imm, tuple(must), sym
        self.primary = False
        self.obj = None
        self.note = None


def s16(v):
    return isinstance(v, int) and -0x8000 <= v <= 0x7FFF


def u16(v):
    return isinstance(v, int) and 0 <= v <= 0xFFFF


def expand(ci):
    """the machine instructions as1 makes of one cc -S instruction (approximately)"""
    mn, ops = ci.mn, ci.ops
    R = canon
    out = []

    def T(*a, **k):
        t = Tpl(ci, *a, **k)
        out.append(t)
        return t

    def li(reg, k):
        if s16(k):
            T("addiu", [reg, "r0"], k)
        elif u16(k):
            T("ori", [reg, "r0"], k)
        elif isinstance(k, int):
            T("lui", [reg], (k >> 16) & 0xFFFF)
            if k & 0xFFFF:
                T("ori", [reg, reg], k & 0xFFFF)
        else:
            T({"lui", "addiu", "ori"}, [reg])

    def mem(o):
        m = re.fullmatch(r"(.*)\((\$\w+)\)", o)
        e, base = (m.group(1), R(m.group(2))) if m else (o, None)
        v = num(e) if e else 0
        return e, base, v

    imm_ops = {"addu": "addiu", "add": "addi", "and": "andi", "or": "ori", "xor": "xori", "slt": "slti",
               "sltu": "sltiu"}
    if mn == "nop":
        T("nop")
    elif mn == "move" and len(ops) == 2:
        T({"or", "addu"}, [R(ops[0]), R(ops[1]), "r0"])
    elif mn == "li" and len(ops) == 2:
        li(R(ops[0]), num(ops[1]))
    elif mn in ("li.s", "li.d"):
        fd = R(ops[0])
        T({"lui", "ori", "addiu"})
        if mn == "li.d":
            T({"mtc1", "lwc1"}, None, must=("f%d" % (int(fd[1:]) + 1),))
        T({"mtc1", "lwc1"}, None, must=(fd,))
    elif mn == "la" and len(ops) == 2:
        e, base, v = mem(ops[1])
        if base and v is not None:
            T("addiu", [R(ops[0]), base], v)
        else:
            T("lui", [R(ops[0])])
            T("addiu", [R(ops[0]), R(ops[0])])
            if base:
                T("addu", None, must=(R(ops[0]), base))
    elif mn in ("neg", "negu") and len(ops) == 2:
        T({"subu", "sub"}, [R(ops[0]), "r0", R(ops[1])])
    elif mn == "not" and len(ops) == 2:
        T("nor", [R(ops[0]), R(ops[1]), "r0"])
    elif mn in ("subu", "sub") and len(ops) in (2, 3) and num(ops[-1]) is not None:
        d = R(ops[0])
        s = R(ops[1]) if len(ops) == 3 else d
        k = num(ops[-1])
        if s16(-k):
            T("addiu" if mn == "subu" else "addi", [d, s], -k)
        else:
            li("r1", k)
            T(mn, [d, s, "r1"])
    elif mn in imm_ops and len(ops) in (2, 3) and num(ops[-1]) is not None:
        d = R(ops[0])
        s = R(ops[1]) if len(ops) == 3 else d
        k = num(ops[-1])
        fits = u16(k) if mn in ("and", "or", "xor") else s16(k)
        if fits:
            T(imm_ops[mn], [d, s], k)
        else:
            li("r1", k)
            T(mn, [d, s, "r1"])
    elif mn in ("sll", "srl", "sra", "rol", "ror") and len(ops) == 3:
        if canon(ops[2]):
            T(mn + "v", [R(ops[0]), R(ops[1]), R(ops[2])])
        elif mn in ("rol", "ror"):
            T({"sll", "srl", "or"}, None, must=(R(ops[0]),))
            T({"sll", "srl", "or"}, None, must=(R(ops[0]),))
        else:
            T(mn, [R(ops[0]), R(ops[1])], num(ops[2]))
    elif mn in ("mul", "mulo", "mulou") and len(ops) == 3:
        d, s = R(ops[0]), R(ops[1])
        k = num(ops[2])
        if k is None:
            T({"mult", "multu"}, None, must=(s, R(ops[2])))
            T("mflo", [d])
        elif isinstance(k, int) and k > 0 and k & (k - 1) == 0:
            T("sll", [d, s], k.bit_length() - 1)
        else:
            any_ = {"sll", "addu", "subu", "mult", "multu", "mflo", "addiu", "ori", "lui"}
            T(any_, None, must=(s,))
            T(any_, None, must=(d,))
    elif mn in ("div", "divu", "rem", "remu") and len(ops) == 3:
        d, s = R(ops[0]), R(ops[1])
        if canon(ops[2]):
            T({"div", "divu"}, None, must=(s, R(ops[2])))
            T("mfhi" if mn.startswith("rem") else "mflo", [d])
        else:
            any_ = {"div", "divu", "sra", "srl", "addu", "and", "andi", "mult", "multu", "mfhi", "mflo", "sll",
                    "subu", "addiu", "bgez", "bltz", "ori", "lui"}
            T(any_, None, must=(s,))
            T(any_, None, must=(d,))
    elif mn in ("seq", "sne", "sgt", "sge", "sle", "sgtu", "sgeu", "sleu") and len(ops) == 3:
        any_ = {"xor", "xori", "sltiu", "sltu", "slt", "slti", "subu", "addiu", "ori"}
        T(any_, None, must=(R(ops[1]),))
        T(any_, None, must=(R(ops[0]),))
    elif mn in ("trunc.w.s", "trunc.w.d", "round.w.s", "round.w.d", "ceil.w.s", "ceil.w.d", "floor.w.s",
                "floor.w.d") and len(ops) >= 2:
        for _ in range(2):
            T("cfc1")
        for _ in range(2):
            T("ctc1")
        T({"ori", "xori"})
        T({"ori", "xori"})
        T("cvt.w." + mn[-1], [R(ops[0]), R(ops[1])])
    elif mn in ("l.s", "s.s", "l.d", "s.d") or mn in LOADS or mn in STORES or mn in ("ulw", "usw", "ulh", "ulhu", "ush"):
        real = {"l.s": "lwc1", "s.s": "swc1", "l.d": "lwc1", "s.d": "swc1"}.get(mn, mn)
        rt = R(ops[0]) if ops else None
        e, base, v = mem(ops[1]) if len(ops) > 1 else ("", None, 0)
        dbl = mn in ("l.d", "s.d")
        if mn in ("ulw", "usw", "ulh", "ulhu", "ush"):
            T({"lwl", "lwr", "swl", "swr", "lb", "lbu", "sb", "sll", "srl", "or"}, None, must=(rt,))
            T({"lwl", "lwr", "swl", "swr", "lb", "lbu", "sb", "sll", "srl", "or"}, None, must=(rt,))
        elif v is not None and base:
            if dbl:
                pair = {rt, "f%d" % (int(rt[1:]) + 1)}
                T(real, [pair, base])
                T(real, [pair, base])
            else:
                T(real, [rt, base], v)
        else:
            gpr_load = real in LOADS and rt and rt[0] == "r"
            T("lui", [{rt, "r1"} if gpr_load else "r1"])
            if base:
                T("addu", None, must=(base,))
            if dbl:
                pair = {rt, "f%d" % (int(rt[1:]) + 1)}
                T(real, [pair, None])
                T(real, [pair, None])
            else:
                T(real, [rt, None])
    elif mn in ("beq", "bne") and len(ops) == 3:
        a = R(ops[0])
        b = R(ops[1])
        k = num(ops[1])
        if b:
            T(mn, [a, b])
        elif k == 0:
            T(mn, [a, "r0"])
        else:
            li("r1", k)
            T(mn, [a, "r1"])
    elif mn in ("beqz", "bnez") and ops:
        T(mn[:3], [R(ops[0]), "r0"])
    elif mn in ("blt", "bge", "bgt", "ble", "bltu", "bgeu", "bgtu", "bleu") and len(ops) == 3:
        a = R(ops[0])
        b = R(ops[1])
        k = 0 if b == "r0" else num(ops[1])
        uns = mn.endswith("u")
        base = mn[:-1] if uns else mn
        if b and b != "r0":
            slt = "sltu" if uns else "slt"
            if base in ("blt", "bge"):
                T(slt, ["r1", a, b])
            else:
                T(slt, ["r1", b, a])
            T("bne" if base in ("blt", "bgt") else "beq", ["r1", "r0"])
        elif k == 0 and not uns:
            T({"blt": "bltz", "bge": "bgez", "bgt": "bgtz", "ble": "blez"}[base], [a])
        elif k == 0:
            if base == "bgt":
                T("bne", [a, "r0"])
            elif base == "ble":
                T("beq", [a, "r0"])
            elif base == "bge":
                T("beq", ["r0", "r0"])
        else:
            kk = k if base in ("blt", "bge") else (k + 1 if isinstance(k, int) else k)
            slti = "sltiu" if uns else "slti"
            if s16(kk):
                T(slti, ["r1", a], kk)
            else:
                li("r1", kk)
                T("sltu" if uns else "slt", ["r1", a, "r1"])
            T("bne" if base in ("blt", "ble") else "beq", ["r1", "r0"])
    elif mn == "b":
        T({"beq", "b"}, ["r0", "r0"])
    elif mn in ("bltz", "bgez", "blez", "bgtz") and ops:
        T(mn, [R(ops[0])])
    elif mn in ("bc1t", "bc1f"):
        T(mn)
    elif mn == "j" and ops:
        if canon(ops[0]):
            T("jr", [R(ops[0])])
        else:
            T("j")
    elif mn == "jal" and ops:
        if canon(ops[0]):
            T("jalr", None, must=(R(ops[0]),))
        else:
            T("jal", sym=ops[0])
    elif mn == "jalr" and ops:
        T("jalr", None, must=(R(ops[-1]),))
    elif mn in ("mfc1.d", "mtc1.d") and len(ops) == 2:
        T(mn[:-2], None, must=(R(ops[0]),))
        T(mn[:-2], None)
    elif mn in ("abs", "abs.s", "abs.d") and mn == "abs":
        T({"bgez", "sra", "xor", "subu", "or"}, None, must=(R(ops[0]),))
        T({"bgez", "sra", "xor", "subu", "or"}, None, must=(R(ops[0]),))
    else:
        regs = [canon(o) for o in ops if canon(o)]
        imm = None
        for o in ops:
            if not canon(o) and num(o) is not None:
                imm = num(o)
        T(mn, regs or None, imm)
    out[-1].primary = True
    return out


# ---------------------------------------------------------------- segments and matching

def obj_segments(ins):
    """segment index per word: split at branch targets and after the delay slot of branches/jumps/calls"""
    targets = {x.target for x in ins if x.target is not None}
    seg = []
    sigs = []
    cur = 0
    cut_after = None
    sig = "fall"
    for i, x in enumerate(ins):
        if i and (x.off in targets or cut_after == i):
            sigs.append(sig)
            sig = "fall"
            cur += 1
        seg.append(cur)
        if x.is_branch or x.is_jump:
            cut_after = i + 2
            sig = branch_sig(x)
    sigs.append(sig)
    return seg, sigs


def branch_sig(x):
    if x.mn == "jal":
        return "jal:" + (x.sym or "?")
    if x.mn in ("jalr",):
        return "jalr"
    if x.mn == "jr":
        return "jr:ra" if x.regs[:1] == ["r31"] else "jr"
    if x.mn == "j" or (x.mn in ("beq", "b") and x.regs[:2] == ["r0", "r0"]):
        return "b"
    return "br"


def ccs_segments(cins):
    seg = []
    sigs = []
    cur = 0
    sig = "fall"
    started = False
    for ci in cins:
        if ci.labels and started:
            sigs.append(sig)
            sig = "fall"
            cur += 1
            started = False
        seg.append(cur)
        started = True
        if ci.is_br():
            if ci.mn == "jal":
                sig = "jal:" + ci.ops[0] if not canon(ci.ops[0]) else "jalr"
            elif ci.mn == "j":
                sig = "jr:ra" if ci.ops[0] == "$31" else ("jr" if canon(ci.ops[0]) else "b")
            elif ci.mn == "jr":
                sig = "jr"
            elif ci.mn == "b":
                sig = "b"
            else:
                sig = "br"
            sigs.append(sig)
            sig = "fall"
            cur += 1
            started = False
    sigs.append(sig)
    return seg, sigs


def map_segments(osigs, csigs):
    """obj segment -> ccs segment"""
    sm = difflib.SequenceMatcher(None, osigs, csigs, autojunk=False)
    mp = {}
    for tag, i1, i2, j1, j2 in sm.get_opcodes():
        for k in range(i2 - i1):
            if tag == "equal":
                mp[i1 + k] = j1 + k
            elif j2 > j1:
                mp[i1 + k] = j1 + min(k * (j2 - j1) // max(1, i2 - i1), j2 - j1 - 1)
            else:
                mp[i1 + k] = max(0, j1 - 1)
    return mp


def score(o, t, oseg_c, tseg, ofrac, tfrac):
    if o.mn not in t.mn:
        return None
    s = 1.0
    if t.regs is not None:
        dest_pos = not (o.mn in STORES or o.is_branch or o.is_jump)
        for k, r in enumerate(t.regs):
            if r is None:
                continue
            if k >= len(o.regs):
                s -= 1
                continue
            ok = o.regs[k] in r if isinstance(r, set) else o.regs[k] == r
            if ok:
                s += 3 if k == 0 else 2
            else:
                s -= 3 if (k == 0 and dest_pos) else 1
    for r in t.must:
        s += 2 if r in o.regs else -2
    if t.imm is not None and o.imm is not None and o.mask == 0xFFFFFFFF and not o.is_branch:
        s += 2 if t.imm == o.imm else -2
    if t.sym is not None and o.sym is not None:
        s += 4 if t.sym == o.sym else -4
    d = abs(tseg - oseg_c)
    s += {0: 4, 1: 2, 2: 1}.get(d, -0.05 * min(d, 40))
    s -= 0.5 * abs(ofrac - tfrac)
    return s


def match_ccs(oins, oseg, omap, cins, cseg):
    tpls = []
    for ci in cins:
        ci.tpls = expand(ci)
        tpls += ci.tpls
    byname = {}
    for t in tpls:
        for m in t.mn:
            byname.setdefault(m, []).append(t)
    nO, nC = max(1, len(oins)), max(1, len(cins))
    cand = []
    for o in oins:
        oc = omap.get(oseg[o.idx], 0)
        for t in byname.get(o.mn, ()):
            sc = score(o, t, oc, cseg[t.ci.idx], o.idx / nO, t.ci.idx / nC)
            if sc is not None and sc >= 2.5:
                cand.append((-sc, o.idx, t.ci.idx, id(t), o, t))
    cand.sort(key=lambda x: x[:4])
    for _, _, _, _, o, t in cand:
        if o.ccs is None and t.obj is None:
            o.ccs = t
            t.obj = o


def prior_store(ci, cins):
    """the cc -S store to the same address as load CI in its segment, or None"""
    if ci.mn not in LOADS and ci.mn not in ("l.s", "l.d") or len(ci.ops) != 2 or ci.labels:
        return None
    base = re.fullmatch(r"(.*)\((\$\w+)\)", ci.ops[1])
    breg = canon(base.group(2)) if base else None
    for p in range(ci.idx - 1, max(-1, ci.idx - 16), -1):
        q = cins[p]
        if q.is_br():
            break
        if (q.mn in STORES or q.mn in ("s.s", "s.d")) and len(q.ops) == 2 and q.ops[1] == ci.ops[1]:
            return q
        if q.labels or (breg and q.dest() == breg):
            return None
    return None


def rescue(oins, cins):
    """second pass: cc -S instructions whose main word wasn't matched, against the words nobody claimed:
    a load as1 retargeted (an `and 255` folded into it), or a load as1 replaced with a copy of the
    register just stored there (store-to-load forwarding)"""
    free = [o for o in oins if o.ccs is None and o.word != 0]
    for ci in cins:
        prim = ci.tpls[-1]
        if prim.obj is not None:
            continue
        anchors = [t.obj.idx for t in ci.tpls if t.obj is not None]
        if not anchors:
            for d in (1, -1, 2, -2, 3, -3):
                k = ci.idx + d
                if 0 <= k < len(cins):
                    anchors += [t.obj.idx for t in cins[k].tpls if t.obj is not None]
                if anchors:
                    break
        if not anchors:
            continue
        center = sum(anchors) / len(anchors)
        best = None
        st = prior_store(ci, cins)
        rt = canon(ci.ops[0]) if ci.ops else None
        for o in free:
            if o.ccs is not None:
                continue
            dist = abs(o.idx - center)
            note = None
            if o.mn in prim.mn and dist <= 8 and (prim.imm is None or o.imm is None or o.mask != 0xFFFFFFFF
                                                  or prim.imm == o.imm):
                if prim.regs and isinstance(prim.regs[0], str) and o.regs and o.regs[0] != prim.regs[0]:
                    note = "as1 retargeted %s>%s" % (rname(prim.regs[0]), rname(o.regs[0]))
                else:
                    note = "as1 rewrote it"
            elif st is not None and dist <= 16 and o.dest() == rt and canon(st.ops[0]) in o.regs[1:]:
                note = "load forwarded from the store of %s (as1: %s)" % (rname(canon(st.ops[0])), o.mn)
            if note and (best is None or dist < best[0]):
                best = (dist, o, note)
        if best:
            _, o, note = best
            o.ccs = prim
            prim.obj = o
            prim.note = note


def drop_reason(ci, cins):
    if ci.mn == "nop":
        return "nop"
    if ci.mn == "move":
        return "move fwd"
    if prior_store(ci, cins) is not None:
        return "load fwd from the store"
    if ci.mn == "and" and len(ci.ops) == 3 and num(ci.ops[2]) in (255, 0xFFFF):
        src = canon(ci.ops[1])
        for p in range(ci.idx - 1, max(-1, ci.idx - 12), -1):
            if cins[p].dest() == src:
                if cins[p].mn in ("lbu", "lhu"):
                    return "mask folded into the %s above (it loads into %s)" % (cins[p].mn, rname(canon(ci.ops[0])))
                break
        return "mask dropped"
    return "dropped"


# ---------------------------------------------------------------- the C side

def read_names():
    """func_XXXXXXXX <-> C names from names.txt and undefined_syms (no ROM names)"""
    a2c, c2a = {}, {}
    p = C.path("names.txt")
    if os.path.exists(p):
        for line in open(p):
            m = re.match(r"\s*(func_[0-9A-F]{8})\s*=\s*(\w+)", line)
            if m:
                a2c[m.group(1)] = m.group(2)
                c2a[m.group(2)] = m.group(1)
    for line in open(C.path("undefined_syms.%s.txt" % C.VERSION)):
        m = re.match(r"\s*func_([0-9A-Fa-f]{8})\s*=\s*(\w+);", line)
        if m:
            a = "func_%s" % m.group(1).upper()
            a2c.setdefault(a, m.group(2))
            c2a.setdefault(m.group(2), a)
    return a2c, c2a


def def_re(name):
    return re.compile(r"^[A-Za-z_][^;{}()\n]*\b%s\s*\(([^;{}]*?)\)\s*\{" % re.escape(name), re.M)


def find_file(names, module):
    if module:
        p = C.path("src", "modules", module + ".c")
        return p if os.path.exists(p) else None
    for f in sorted(glob.glob(C.path("src", "**", "*.c"), recursive=True)):
        if os.sep + "modules" + os.sep in f:
            continue
        s = open(f, errors="replace").read()
        for n in names:
            if n and (re.search(r'INCLUDE_ASM\("[^"]*",\s*%s\)' % re.escape(n), s) or def_re(n).search(s)):
                return f
    return None


def make_command(src):
    """the build's compile command for SRC (shlex tokens), from `make -n -B`"""
    rel = os.path.relpath(src, ROOT)
    if rel.startswith(".."):
        return None
    obj = "build/" + rel[:-2] + ".o"
    try:
        r = subprocess.run(["make", "--no-print-directory", "-n", "-B", obj], capture_output=True, text=True,
                           cwd=ROOT, timeout=120)
    except Exception:  # noqa: BLE001
        return None
    txt = r.stdout.replace("\\\n", " ")
    for line in txt.splitlines():
        t = shlex.split(line)
        if "-o" in t and t[t.index("-o") + 1] == obj:
            return t
    return None


def split_command(tok):
    """(asm-processor prefix up to and including the last '--' + '-c', cc path, cflags)"""
    ccs = [i for i, x in enumerate(tok) if x.endswith("/cc")]
    if not ccs:
        return None
    if "--" in tok:
        last = len(tok) - 1 - tok[::-1].index("--")
        prefix = tok[:last + 1]
        rest = tok[last + 1:]
    else:
        prefix = None
        rest = tok[ccs[0] + 1:]
    o = rest.index("-o")
    flags = [x for x in rest[:o] if x != "-c"] + rest[o + 2:-1]
    return prefix, tok[ccs[0]], flags


def assign_lhs(text):
    if not text:
        return None
    t = text.strip()
    t = re.sub(r"^\}?\s*(?:else\s+)?(?:for|while|if|switch)\s*\(\s*", "", t)
    t = re.sub(r"^return\s+", "", t)
    m = re.match(r"^(?:(?:const|static|volatile|unsigned|signed|struct\s+\w+|[A-Za-z_]\w*)\s+\**)*"
                 r"(\**[A-Za-z_]\w*(?:\s*(?:\[[^\]]*\]|->\s*\w+|\.\s*\w+))*)\s*(?:[-+*/%&|^]|<<|>>)?=(?!=)", t)
    if m:
        return re.sub(r"\s+", "", m.group(1))
    m = re.match(r"^\(?\s*([A-Za-z_]\w*)\s*(\+\+|--)", t) or re.match(r"^(\+\+|--)\s*([A-Za-z_]\w*)", t)
    if m:
        return m.group(1) if m.group(1) not in ("++", "--") else m.group(2)
    return None


def call_args(text):
    """[(callee, [arg texts])] of the calls in a C line"""
    res = []
    for m in re.finditer(r"\b([A-Za-z_]\w*)\s*\(", text or ""):
        if m.group(1) in ("if", "while", "for", "switch", "return", "sizeof"):
            continue
        depth, k, args, cur = 1, m.end(), [], ""
        while k < len(text) and depth:
            ch = text[k]
            if ch == "(":
                depth += 1
            elif ch == ")":
                depth -= 1
                if not depth:
                    break
            if ch == "," and depth == 1:
                args.append(cur.strip())
                cur = ""
            else:
                cur += ch
            k += 1
        if cur.strip():
            args.append(cur.strip())
        res.append((m.group(1), args))
    return res


def params_of(src, cname):
    m = def_re(cname).search(src)
    if not m:
        return []
    p = m.group(1).strip()
    if p in ("", "void"):
        return []
    out = []
    for x in p.split(","):
        ids = re.findall(r"[A-Za-z_]\w*", x)
        out.append(ids[-1] if ids else "?")
    return out


def ccs_slot_values(cins, frame, params, lines):
    """{offset: [value names]} for the cc -S code"""
    vals = {}

    def add(off, v):
        lst = vals.setdefault(off, [])
        if v and v not in lst:
            lst.append(v)

    def ltext(n):
        return lines[n - 1] if n and 0 < n <= len(lines) else ""

    for ci in cins:
        if ci.sp_off is None:
            continue
        off = ci.sp_off
        reg = canon(ci.ops[0]) if ci.ops else None
        if frame is not None and off >= frame:
            k = (off - frame) // 4
            add(off, ("param " + params[k]) if k < len(params) else "arg%d" % k)
            continue
        if ci.prologue and ci.sp_kind == "store" and reg in CALLEE_SAVED:
            add(off, "saved " + rname(reg))
            continue
        calls = call_args(ltext(ci.line))
        if ci.sp_kind == "store" and 16 <= off < 48 and not ci.prologue:
            k = 4 + (off - 16) // 4
            hit = [(c, a) for c, a in calls if len(a) > k]
            if hit:
                add(off, "out arg%d of %s: %s (L%s)" % (k + 1, hit[0][0], hit[0][1][k][:20], ci.line))
                continue
        if ci.sp_kind == "addr":
            n = int(reg[1:]) if reg and reg[0] == "r" else -1
            name = None
            if not 4 <= n <= 7:
                # an address passed on the stack: sw REG, 16+4k($sp) a little later
                for q in cins[ci.idx + 1:ci.idx + 12]:
                    if q.mn == "sw" and q.sp_off is not None and canon(q.ops[0]) == reg and 16 <= q.sp_off < 48:
                        n = 4 + 4 + (q.sp_off - 16) // 4
                        break
            if 4 <= n <= 15:
                for callee, args in calls:
                    if len(args) > n - 4 and "&" in args[n - 4]:
                        name = args[n - 4]
                        break
            if not name:
                am = re.search(r"&\s*([A-Za-z_]\w*)", ltext(ci.line))
                name = "&" + am.group(1) if am else None
            add(off, (name or "&?") + " (L%s)" % ci.line)
            continue
        if ci.sp_kind == "store":
            name = assign_lhs(ltext(ci.line))
            if not name:
                for p in range(ci.idx - 1, -1, -1):
                    if cins[p].dest() == reg:
                        if cins[p].mn == "move" and p > 0:
                            name = assign_lhs(ltext(cins[p].line))
                        else:
                            name = assign_lhs(ltext(cins[p].line))
                        break
            add(off, ("%s (L%s)" % (name, ci.line)) if name else "spill %s (L%s)" % (rname(reg), ci.line))
    for ci in cins:
        if ci.sp_off is not None and ci.sp_off not in vals:
            add(ci.sp_off, "read only (L%s)" % ci.line)
    return vals


# ---------------------------------------------------------------- main

def tfirst(seq, regs_of):
    """{index: [t-regs first used there]}, [(treg, index)] in order"""
    seen = set()
    at = {}
    order = []
    for i, x in enumerate(seq):
        for r in regs_of(x):
            if r in TREGS and r not in seen:
                seen.add(r)
                at.setdefault(i, []).append(rname(r))
                order.append((rname(r), i))
    return at, order


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0], epilog=__doc__.split("\n\n", 1)[1],
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("func", help="func_XXXXXXXX, func_mNNN_XXXXX or a main-segment C name")
    ap.add_argument("--file", help="the C file (default: found from the name); a scratch copy works")
    ap.add_argument("--asm", help="the .s / INCLUDE_ASM name, when FUNC is a C name the tool can't place")
    ap.add_argument("--module", help="mNNN, for a module function not named func_mNNN_XXXXX")
    ap.add_argument("--nm", action="store_true", help="compile the whole file with -DNON_MATCHING instead of "
                    "installing only this function's body")
    ap.add_argument("--only-diff", action="store_true", help="list only the blocks where ROM and object differ")
    ap.add_argument("--summary", action="store_true", help="print only the stack map and the summary")
    ap.add_argument("--no-lines", action="store_true", help="don't print the C source lines in the listing")
    ap.add_argument("--tag", default="", help="own scratch directory build/match/ccs_side-TAG (parallel runs)")
    ap.add_argument("--keep", action="store_true", help="print the paths of the scratch .c/.s/.o files")
    args = ap.parse_args()

    a2c, c2a = read_names()
    func = args.func
    module = args.module
    mm = re.fullmatch(r"func_(m\d{3})_[0-9A-F]{5}", func)
    if mm:
        module = mm.group(1)
    if re.fullmatch(r"func_[0-9A-Fa-f]{8}", func):
        asmname = args.asm or "func_" + func[5:].upper()
        cname = a2c.get(asmname, func)
    else:
        cname = func
        asmname = args.asm or c2a.get(func) or func
    src_path = os.path.abspath(args.file) if args.file else find_file([cname, asmname], module)
    if not src_path or not os.path.exists(src_path):
        sys.exit("ccs_side: no C file found for %s (use --file)" % func)
    if module is None and os.sep + "modules" + os.sep in src_path:
        module = os.path.basename(src_path)[:-2]
    src = open(src_path).read()

    # the ROM side
    if module:
        cands = [C.path("asm", "nonmatchings", "modules", module, asmname + ".s")]
    else:
        cands = [p for p in glob.glob(C.path("asm", "nonmatchings", "**", asmname + ".s"), recursive=True)
                 if os.sep + "modules" + os.sep not in p and os.sep + "ro" + os.sep not in p]
    cands = [p for p in cands if os.path.exists(p)]
    if not cands:
        sys.exit("ccs_side: no asm/nonmatchings .s for %s (run `make extract`, or pass --asm)" % asmname)
    rom_s = cands[0]

    # the C: install the NON_MATCHING body (line numbers kept)
    text = src
    nm_used = False
    if not args.nm:
        for m in NM_RE.finditer(src):
            if m.group(4) == asmname:
                text = src[:m.start()] + "\n" + m.group(2) + "\n\n\n" + src[m.end():]
                nm_used = True
                break
    dm = def_re(cname).search(text)
    if not dm:
        if nm_used or args.nm:
            sys.exit("ccs_side: no definition of %s in the NON_MATCHING body (a C name? use FUNC = that name, "
                     "or --asm)" % cname)
        sys.exit("ccs_side: %s has no C body in %s (only INCLUDE_ASM)" % (cname, os.path.relpath(src_path, ROOT)))
    # a static only asm calls would be dropped: make the target global
    text = re.sub(r"^static\s+(?=[^\n;{}]*\b%s\s*\()" % re.escape(cname), "", text, flags=re.M)

    wd = C.workdir("ccs_side", args.tag)
    base = os.path.basename(src_path)
    cfile = os.path.join(wd, base)
    sfile = cfile[:-2] + ".s"
    ofile = os.path.join(wd, base[:-2] + ".o")
    tok = make_command(src_path) if not args.file or os.path.relpath(src_path, ROOT).startswith("src") else None
    if tok is None:
        # a scratch copy: the flags of the object it stands for
        stand = C.path("src", "modules", module + ".c") if module else find_file([cname, asmname], None)
        tok = make_command(stand) if stand else None
    if tok is None or split_command(tok) is None:
        flags = (C.GAME_CFLAGS + (["-I", "include/modules"] if module else [])) + ["-O2", "-mips1"]
        prefix = [sys.executable, C.ASM_PROC, "--asm-prelude", "include/prelude.inc", C.IDO_CC, "--",
                  "mips-linux-gnu-as"] + C.ASFLAGS + ["--"]
        cc = C.IDO_CC
        how = "flags from tools/match/common.py (make -n failed)"
    else:
        prefix, cc, flags = split_command(tok)
        how = "flags from `make -n` for %s" % next((tok[i + 1] for i, x in enumerate(tok) if x == "-o"), "?")
    extra = ["-DNON_MATCHING"] if args.nm else []
    incl = ["-I", os.path.dirname(src_path)]

    # cc -S (INCLUDE_ASM lines blanked; cfe sees plain C)
    stext = re.sub(r"^(INCLUDE_ASM|INCLUDE_RODATA|GLOBAL_ASM)\(.*\);[ \t]*$", "", text, flags=re.M)
    open(cfile, "w").write(stext)
    if os.path.exists(sfile):
        os.remove(sfile)
    absflags = []
    for k, x in enumerate(flags):
        if k and flags[k - 1] == "-I" and not os.path.isabs(x):
            x = os.path.join(ROOT, x)
        absflags.append(x)
    r = subprocess.run([os.path.join(ROOT, cc), "-S"] + absflags + extra + incl + [cfile], capture_output=True,
                       text=True, cwd=wd)
    if not os.path.exists(sfile):
        sys.exit("ccs_side: cc -S failed:\n" + (r.stdout + r.stderr)[-3000:])
    cins, cframe = parse_ccs(sfile, cname)
    if not cins:
        sys.exit("ccs_side: %s not in the cc -S output %s" % (cname, sfile))

    # our object (the build's command, asm-processor and all)
    open(cfile, "w").write(text)
    if os.path.exists(ofile):
        os.remove(ofile)
    if prefix:
        cmd = prefix + ["-c"] + flags + extra + incl + ["-o", ofile, cfile]
    else:
        cmd = [cc, "-c"] + flags + extra + incl + ["-o", ofile, cfile]
    r = subprocess.run(cmd, capture_output=True, text=True, cwd=ROOT)
    if r.returncode or not os.path.exists(ofile):
        sys.exit("ccs_side: compile failed:\n" + (r.stdout + r.stderr)[-3000:])

    dis = C.disassembler(pseudos=False)
    rins, vram = read_rom_s(rom_s, dis)
    oins, err = read_obj(ofile, cname, dis, vram)
    if err:
        sys.exit("ccs_side: " + err)

    # ROM <-> object, by opcode
    sm = difflib.SequenceMatcher(None, [x.mn for x in oins], [x.mn for x in rins], autojunk=False)
    pairs = []  # (obj idx or None, rom idx or None)
    for tag, i1, i2, j1, j2 in sm.get_opcodes():
        if tag == "equal":
            pairs += [(i1 + k, j1 + k) for k in range(i2 - i1)]
        elif tag == "replace":
            n = max(i2 - i1, j2 - j1)
            pairs += [(i1 + k if k < i2 - i1 else None, j1 + k if k < j2 - j1 else None) for k in range(n)]
        elif tag == "delete":
            pairs += [(i1 + k, None) for k in range(i2 - i1)]
        else:
            pairs += [(None, j1 + k) for k in range(j2 - j1)]
    o2r = {o: r_ for o, r_ in pairs if o is not None and r_ is not None}

    # cc -S <-> object
    oseg, osigs = obj_segments(oins)
    cseg, csigs = ccs_segments(cins)
    omap = map_segments(osigs, csigs)
    match_ccs(oins, oseg, omap, cins, cseg)
    rescue(oins, cins)

    def ostate(ci):
        hit = [t for t in ci.tpls if t.obj is not None]
        if not hit:
            return "drop"
        return "kept" if len(hit) == len(ci.tpls) else "part"

    lines = src.split("\n")

    # per-pair differences
    def pdiff(o, r_):
        if o is None:
            return "+rom"
        if r_ is None:
            return "+obj"
        m = o.mask & r_.mask
        if (o.word & m) == (r_.word & m):
            return ""
        if o.mn != r_.mn:
            return "OP"
        out = []
        for a, b in zip(o.regs, r_.regs):
            if a != b:
                out.append("R %s/%s" % (rname(a), rname(b)))
        if o.target is not None and o.target != r_.target:
            out.append("B")
        if o.imm != r_.imm and o.mask == 0xFFFFFFFF and r_.mask == 0xFFFFFFFF and o.target is None:
            out.append("I %s/%s" % (hexs(o.imm), hexs(r_.imm)))
        return " ".join(out) or "?"

    # t-register first use, each column
    c_at, c_order = tfirst(cins, lambda c: c.regs)
    o_at, o_order = tfirst(oins, lambda x: x.regs)
    r_at, r_order = tfirst(rins, lambda x: x.regs)

    # rows: pairs, with the dropped cc -S instructions after their predecessor's row
    drops_after = {}
    last_obj = None
    first_drops = []
    for ci in cins:
        st = ostate(ci)
        if st == "drop":
            if last_obj is None:
                first_drops.append(ci)
            else:
                drops_after.setdefault(last_obj, []).append(ci)
        else:
            hit = [t for t in ci.tpls if t.obj is not None]
            prim = [t for t in hit if t.primary]
            last_obj = prim[0].obj.idx if prim else max(t.obj.idx for t in hit)
    rows = [("drop", ci) for ci in first_drops]
    for o, r_ in pairs:
        rows.append(("pair", o, r_))
        if o is not None:
            rows += [("drop", ci) for ci in drops_after.get(o, [])]

    # obj basic blocks
    targets = {x.target for x in oins if x.target is not None}
    bstart = set([0]) | {t // 4 for t in targets if 0 <= t // 4 < len(oins)}
    for x in oins:
        if (x.is_branch or x.mn in ("j", "jr")) and x.idx + 2 < len(oins):
            bstart.add(x.idx + 2)

    W = 40
    blocks = []  # (header, [lines], has_diff)
    cur = None
    shown_lines = set()
    ndiff = 0
    regpairs = {}
    dropped = []
    for row in rows:
        if row[0] == "pair":
            _, oi, ri = row
            o = oins[oi] if oi is not None else None
            r_ = rins[ri] if ri is not None else None
            if o is not None and oi in bstart:
                ci0 = o.ccs.ci if o.ccs else None
                lab = ""
                s0 = omap.get(oseg[oi])
                for ci in cins:
                    if cseg[ci.idx] == s0:
                        lab = (" ccs %s" % ",".join(ci.labels)) if ci.labels else ""
                        break
                hdr = "== block +0x%03X obj / %s rom%s%s" % (
                    o.off, ("+0x%03X" % r_.off) if r_ is not None else "-", lab,
                    ("  (L%s)" % ci0.line) if ci0 and ci0.line else "")
                cur = [hdr, [], False]
                blocks.append(cur)
            if cur is None:
                cur = ["== block (start)", [], False]
                blocks.append(cur)
            d = pdiff(o, r_)
            notes = []
            if d:
                notes.append(d)
                cur[2] = True
                ndiff += 1
                if o is not None and r_ is not None and o.mn == r_.mn:
                    for a, b in zip(o.regs, r_.regs):
                        if a != b:
                            regpairs[(rname(a), rname(b))] = regpairs.get((rname(a), rname(b)), 0) + 1
            ctext = ""
            ci = None
            if o is not None and o.ccs is not None:
                t = o.ccs
                ci = t.ci
                ctext = lab_of(ci) + ("" if t.primary else "· ") + ci.text()
                if t.note:
                    notes.append(t.note)
                elif t.regs is not None:
                    fw = []
                    for k, rr in enumerate(t.regs):
                        if isinstance(rr, str) and k < len(o.regs) and o.regs[k] != rr:
                            if k == 0 and o.dest() is not None:
                                notes.append("as1 retargeted %s>%s" % (rname(rr), rname(o.regs[k])))
                            else:
                                fw.append("%s>%s" % (rname(rr), rname(o.regs[k])))
                    if fw:
                        notes.append("fwd " + ",".join(fw))
                st = ostate(ci)
                if st == "part" and t.primary:
                    miss = [sorted(x.mn)[0] for x in ci.tpls if x.obj is None]
                    notes.append("(as1 shared/dropped its %s)" % "/".join(miss))
            elif o is not None:
                ctext = ""
                notes.append("+ as1 " + ("nop" if o.word == 0 else "added"))
            first = []
            if ci is not None and o.ccs.primary and ci.idx in c_at:
                first.append("c:" + "/".join(c_at[ci.idx]))
            if o is not None and oi in o_at:
                first.append("o:" + "/".join(o_at[oi]))
            if r_ is not None and ri in r_at:
                first.append("r:" + "/".join(r_at[ri]))
            if first:
                notes.append("1st " + " ".join(first))
            if not args.no_lines and ci is not None and ci.line and ci.line not in shown_lines:
                shown_lines.add(ci.line)
                cur[1].append("%s   %5d| %s" % (" " * 2, ci.line, lines[ci.line - 1].strip()[:110]
                                              if 0 < ci.line <= len(lines) else ""))
            mk = "*" if d else ("+" if (o is not None and o.ccs is None) else " ")
            cur[1].append("%s %-*s %s %s %s" % (
                mk, W, ctext[:W],
                ("+%03X %-32s" % (o.off, o.text()[:32])) if o is not None else " " * 37,
                ("+%03X %-32s" % (r_.off, r_.text()[:32])) if r_ is not None else " " * 37,
                "; ".join(notes)))
        else:
            ci = row[1]
            why = drop_reason(ci, cins)
            dropped.append((ci, why))
            if cur is None:
                cur = ["== block (start)", [], False]
                blocks.append(cur)
            if not args.no_lines and ci.line and ci.line not in shown_lines:
                shown_lines.add(ci.line)
                cur[1].append("%s   %5d| %s" % (" " * 2, ci.line, lines[ci.line - 1].strip()[:110]
                                              if 0 < ci.line <= len(lines) else ""))
            first = ("; 1st c:" + "/".join(c_at[ci.idx])) if ci.idx in c_at else ""
            cur[1].append("- %-*s %s %s %s" % (W, (lab_of(ci) + ci.text())[:W], " " * 37, " " * 37,
                                               "as1: " + why + first))

    if not args.summary:
        print("%s: cc -S %s | object %s | ROM %s" % (cname, os.path.relpath(sfile, ROOT), os.path.relpath(
            ofile, ROOT), os.path.relpath(rom_s, ROOT)))
        print("(%s%s; %s)" % ("NON_MATCHING body installed" if nm_used else ("-DNON_MATCHING" if args.nm else
                                                                           "C as in the file"),
                              "", how))
        print("  %-*s %-37s %-37s %s" % (W, "cc -S (C line)", "object", "ROM", "marks"))
        for hdr, ls, hd in blocks:
            if args.only_diff and not hd:
                continue
            print(hdr)
            for x in ls:
                print(x.rstrip())

    # ---- stack map
    params = params_of(text, cname)
    cvals = ccs_slot_values(cins, cframe, params, lines)
    ofr, rfr = frame_of(oins), frame_of(rins)

    def slots(seq):
        d = {}
        for x in seq:
            if x.sp_off is not None:
                d.setdefault(x.sp_off, []).append(x)
        return d
    os_, rs_ = slots(oins), slots(rins)
    oval = {}
    for x in oins:
        if x.sp_off is not None and x.ccs is not None and x.ccs.ci.sp_off is not None:
            for v in cvals.get(x.ccs.ci.sp_off, []):
                if v not in oval.setdefault(x.sp_off, []):
                    oval[x.sp_off].append(v)
    for off in os_:
        if off not in oval:
            oval[off] = list(cvals.get(off, []))
        if ofr and off >= ofr and not oval[off]:
            k = (off - ofr) // 4
            oval[off] = ["param " + params[k] if k < len(params) else "arg%d" % k]
    rmap = {}
    for oi, ri in o2r.items():
        if oins[oi].sp_off is not None and rins[ri].sp_off is not None:
            rmap.setdefault(rins[ri].sp_off, set()).add(oins[oi].sp_off)
    print("-- stack slots (frame: object 0x%X, ROM 0x%X, cc -S 0x%X)" % (ofr, rfr, cframe or 0))
    print("   %-6s %-34s %-34s %-34s %s" % ("off", "value (from the C)", "object: first use", "ROM: first use", ""))
    nstack = 0
    for off in sorted(set(os_) | set(rs_), reverse=True):
        ox = os_.get(off, [None])[0]
        rx = rs_.get(off, [None])[0]
        val = ", ".join(oval.get(off, []))
        note = ""
        held = rmap.get(off)
        if held and held != {off}:
            note = "ROM holds the object's " + "/".join("0x%X" % h for h in sorted(held))
            if not val:
                val = ", ".join(", ".join(oval.get(h, [])) for h in sorted(held))
        elif ox is None:
            note = "ROM only"
            val = val or "(%s)" % ", ".join(", ".join(oval.get(h, [])) for h in sorted(held or ()))
        elif rx is None:
            note = "object only"
        elif ox.text() != rx.text() and not (ox.mask != 0xFFFFFFFF or rx.mask != 0xFFFFFFFF):
            note = "first use differs"
        if note:
            nstack += 1
        print(" %s %-6s %-34s %-34s %-34s %s" % (
            "*" if note else " ", "0x%X" % off, val[:34],
            ("+%03X %s" % (ox.off, ox.text()))[:34] if ox else "-",
            ("+%03X %s" % (rx.off, rx.text()))[:34] if rx else "-", note))

    # ---- summary
    pos = sum(1 for i in range(min(len(oins), len(rins)))
              if (oins[i].word & oins[i].mask & rins[i].mask) != (rins[i].word & oins[i].mask & rins[i].mask))
    pos += abs(len(oins) - len(rins))
    print("-- summary %s" % cname)
    print("   words: object %d, ROM %d; frame object 0x%X, ROM 0x%X; positional diffs %d; aligned rows that "
          "differ %d; stack slots that differ %d%s" % (len(oins), len(rins), ofr, rfr, pos, ndiff, nstack,
                                                     "  => MATCH" if pos == 0 else ""))
    if regpairs:
        print("   registers (object/ROM): " + ", ".join("%s/%s x%d" % (a, b, n) for (a, b), n in
                                                          sorted(regpairs.items(), key=lambda x: -x[1])))
    if ndiff:
        print("   differing rows:")
        for hdr, ls, hd in blocks:
            for x in ls:
                if x.startswith("*"):
                    print("    " + " ".join(x[1:].split())[:150])
    kinds = {}
    for ci, why in dropped:
        kinds[why] = kinds.get(why, 0) + 1
    print("   as1 removed %d of %d cc -S instructions: %s" % (len(dropped), len(cins), ", ".join(
        "%s %d" % kv for kv in sorted(kinds.items()))))
    for ci, why in dropped:
        if why != "move fwd":
            print("     %s %-34s %s | %s" % (lab_of(ci), ci.text(), why, (lines[ci.line - 1].strip()[:70]
                                                                         if ci.line and ci.line <= len(lines) else "")))
    added = [x for x in oins if x.ccs is None]
    print("   object words with no cc -S instruction: %d (%d nops)" % (len(added), sum(1 for x in added if x.word == 0)))

    def ordr(order, seq, fmt):
        return " ".join("%s(%s)" % (t, fmt(seq[i])) for t, i in order)
    print("   t-register first use:")
    print("     cc -S  " + ordr(c_order, cins, lambda c: "L%s" % c.line))
    print("     object " + ordr(o_order, oins, lambda x: "+%X" % x.off))
    print("     ROM    " + ordr(r_order, rins, lambda x: "+%X" % x.off))
    if [t for t, _ in o_order] != [t for t, _ in r_order]:
        print("     (object and ROM take the t-registers in a different order)")
    if args.keep:
        print("   scratch: %s %s %s" % (cfile, sfile, ofile))


def lab_of(ci):
    return ("L%-5s" % ci.line) if ci.line else " " * 6


def hexs(v):
    if isinstance(v, int):
        return ("-0x%X" % -v) if v < 0 else "0x%X" % v
    return str(v)


if __name__ == "__main__":
    main()
