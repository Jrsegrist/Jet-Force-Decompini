*For contributors who want to match a function: turn one function's assembly into C that IDO 5.3 compiles to exactly the same instructions.*

# Matching a function

This guide takes one function from assembly to matching C: where to find it,
which tools help, what IDO's output tells you about the original source, and
what the project does not accept. Set the project up first (README,
"Building"): `make setup`, your ROM as `baserom.us.z64`, `make extract`,
`make`.

Words used here:

- **Matching**: the compiled C is the same, word for word, as the ROM's code.
  Code that does the same thing but compiles differently does not match.
- **IDO**: IDO 5.3, the SGI C compiler the game was built with. The game code
  and the modules are built `-O2 -mips1`; the libraries use other flags
  ([layout.md](layout.md), "Build flags").
- **`INCLUDE_ASM`**: a line in a C file that puts a function in as assembly.
  Matching a function means replacing its `INCLUDE_ASM` line with C.
- **Frame**, **slot**: the stack space a function reserves (`addiu sp, sp, -N`)
  and the word in it that holds one value.
- **Saved register** (`s0`-`s7`, `fp`): a register the function must preserve,
  used for values that live across calls. **Temporaries** (`t0`-`t9`, `v0`,
  `v1`, `a0`-`a3`) are not preserved.

## 1. Pick a function

- `.venv/bin/python3 tools/progress.py --files` (and `--modules`) shows what is
  left per file.
- A function's assembly is `asm/nonmatchings/<file>/func_XXXXXXXX.s`; a module
  function's is `asm/nonmatchings/modules/mNNN/func_mNNN_XXXXX.s` (or the
  `ro/` copy, which carries the function's own .rodata; see
  [runlink.md](runlink.md)).
- Small, fresh functions match fastest: a function nobody has tried usually
  matches in 10-20 attempts. Old `NON_MATCHING` leftovers are mostly hard
  register-allocation problems.
- Skip what cannot be C, or not yet (layout.md, "What is still assembly"):
  functions splat marks as hand-written, and statics that only assembly
  calls (IDO drops a static function nothing in C calls, so it must wait for
  its callers).
- **Check whether it is library code or DKR code first.** In the library
  range, `tools/match/ulgame.py` compiles ultralib (SDK 2.0I to 2.0L, -O1 to
  -O3, -mips1 and -mips2) and looks for whole objects at the remaining
  function starts. The game's libraries also have `-g` and `-O2 -g3` objects
  (layout.md), which its range does not cover. For engine code,
  `tools/match/dkrscan.py --dkr PATH` finds the DKR functions that JFG has
  exactly; for the rest, find the DKR twin by code shape, callers, data and
  constants. A DKR function is usually a JFG variant:
  start from DKR's code (its `src/` and `include/` only, never its
  `libultra/`), keep its names and declaration order, and change what the
  assembly shows (offsets, constants, dropped or added checks). Record it in
  [CREDITS.md](../CREDITS.md).

Nothing the build or the tools generate shows a name from your ROM's runlink
table any more (the assembly names everything by address; `make check` runs
`tools/check_rom_names.py` to make sure). Don't look those names up, and
never use them, not even as a research aid (see "Naming" below).

## 2. Write a first draft

[m2c](https://github.com/matt-kempster/m2c) (in `tools/m2c/` after `make
setup`) turns a function's assembly into rough C:

```sh
.venv/bin/python3 tools/m2c/m2c.py asm/nonmatchings/12D50/func_80012345.s
```

For a module function with a jump table, pass the module's `rodata.s` too.
m2c is GPL-licensed and its output is only a draft: rewrite it in the
project's style, with the types and structs from the headers, keeping the
address names it prints (`func_XXXXXXXX`, `D_XXXXXXXX`) or using a name the
project already has.

Put the C in place of the `INCLUDE_ASM` line. Things the C needs:

- Calls and data by address name, or by the names in `names.txt` and
  `undefined_syms.us.txt`. A main-segment function you define under a real
  name needs `func_XXXXXXXX = name;` in `undefined_syms.us.txt`, so that the
  assembly that calls it still links; data the C refers to needs
  `D_XXXXXXXX = 0xXXXXXXXX;` unless the assembly defines it.
- A float constant is an `extern f32 D_...;` until the file's .rodata is in C
  (then it is a plain literal); a string is `extern char D_...[]`.
- A call into a runlink module from the main segment goes through the
  dangling-jump trap at 0x80054530: declare `void func_80054530();` with no
  prototype.
- Modules have their own rules (exported vs `static` symbols, sections in C);
  see [runlink.md](runlink.md), "Modules in C".

## 3. Compare with the ROM

The tools are in `tools/match/` ([README](../tools/match/README.md); each has
`--help`). Run them with `.venv/bin/python3`.

| Tool | Use |
| --- | --- |
| `probe.py FILE.c` | compiles one main-segment C file with the game flags and compares each C function with the ROM, word by word, relocated fields masked. `--show` disassembles the functions that differ (C on the left, ROM on the right). It lists the address of every symbol the object uses (`SYMBOL MISMATCH` means a wrong extern) and warns when the file gets .data or .rodata of its own. Extra compiler flags go after the file (`probe.py src/naudio_8B330.c -O1 -g2 -mips2` for `-g` code). |
| `modprobe.py mNNN FILE.c` | the same for a module: each function's text, then the module's .data and .rodata. `-v` disassembles differences (ROM on the left, C on the right). |
| `var.py` | probes many variants of one function at once, from a variants file (whole definitions, or `old \|\|\| new` edits). |
| `buildvar.py` | like `var.py`, but a full `make` and `make check` per variant. |
| `permdecl.py` | writes a variants file with every order of a set of declarations. |
| `perm.py` | sets up [decomp-permuter](https://github.com/simonlindholm/decomp-permuter) for one function (see below). |
| asm-differ | `.venv/bin/python3 tools/asm-differ/diff.py -mw FUNC` diffs one function of the built ROM against the original, rebuilding on change; module functions work too (`tools/runlink_diff.py`). |

The probe is fast but blind to some things. It masks the targets of calls and
the `%hi`/`%lo` halves of addresses, so a call to the wrong function, or a
global at the wrong offset, still "matches". A function that ends off a
16-byte boundary before an `INCLUDE_ASM` can probe clean and still break the
link. **The full build is the only proof**: `make` must end with the SHA-1
check passing.

On a file in `src/`, the probe skips every function that still has an
`INCLUDE_ASM` line, including `NON_MATCHING` ones. To probe a `NON_MATCHING`
body, use a scratch copy of the file without the `#else` branch, and pass
`-DNON_MATCHING`. The probe finds a function in the ROM by its address
name or by its name in `names.txt`; while you work on a function under a new
name that is in neither, give it its address name (`func_XXXXXXXX`).

## 4. When it does not match yet: `NON_MATCHING`

C that is right but does not compile to the same code yet is kept, with the
assembly as the build's copy:

```c
#ifdef NON_MATCHING
/* NON_MATCHING: the loop counter is in a1 instead of a0 (4 words) */
void func_80012345(void) {
    ...
}
#else
INCLUDE_ASM("asm/nonmatchings/12D50", func_80012345);
#endif
```

- The comment says, in one line, what still differs.
- It counts as assembly in `make progress`.
- `make nonmatching` compiles every file with `-DNON_MATCHING` (objects in
  `build_nm/`, not linked), and CI runs it, so a header change that breaks a
  body shows up at once. A body must compile.
- Statics that wait for an unmatched caller can be staged the same way:
  matching C under `NON_MATCHING`, switched on when the caller matches.

## 5. The permuter

decomp-permuter (MIT, a git submodule at `tools/permuter`) rewrites a
function at random and keeps the versions whose code is closer to the
target. `perm.py` builds its workspace with this project's compiler command:

```sh
.venv/bin/python3 tools/match/perm.py import src/7A7D0.c player_spheres_push_out
timeout 1800 .venv/bin/python3 tools/permuter/permuter.py build/perm/player_spheres_push_out -j4 --best-only
.venv/bin/python3 tools/match/perm.py best build/perm/player_spheres_push_out
```

- Seed it with your best C. Use `-j4` at most, run it in the background with
  a time limit, and stop it after 20-30 minutes without progress.
- Its default score ignores stack offsets and the frame size: use
  `--stack-diffs` for frame problems. A function already under a C name needs
  `--asm func_XXXXXXXX` on import, to find its `INCLUDE_ASM` file.
- **A lower score, or zero, is not a match.** Read every candidate, check
  that it does the same thing as before, write it back in the project's
  style, and confirm with the probe and the full build. Its candidates are
  often finished by a small hand sweep of statement or declaration order.
- `build/perm/` holds names from your ROM: never commit or paste it.

## 6. How IDO thinks: the common levers

Matching is mostly working out how the original source was written from the
shape of the code IDO made of it. These are the patterns that come up most at
`-O2`. None of them changes what the function does.

### The stack frame

- **Every local declared at the top of a function gets a stack word, in
  declaration order, the first one highest**, even one that lives in a
  register or is never used. The frame grows in 8-byte steps. So the offset
  a value is spilled to tells you where its variable was declared.
- Values that live across a call but are not variables (temporaries, a
  variable's second life), and variables declared inside a block, get words
  below the declared ones.
- **Frame too big**: the original wrote some values inline instead of in
  named locals (IDO reuses a repeated subexpression within one statement), or
  declared some variables inside blocks.
- **Frame too small**: the original had an unused local; add
  `UNUSED s32 pad;` at the right position. DKR's code often has these.
- Swap two declarations to swap their slots. Arrays declared last sit
  lowest. When several locals move, script the orders (`permdecl.py`).

### Registers

- IDO numbers registers in statement order, not store order. Write
  conversions and loads in the order the target uses its registers.
- A value kept in a saved register through the whole function is a local
  variable, often one reused for something else: a parameter reused for a
  later value, a loop counter reused as an accumulator. One saved register
  holding two values that never overlap is one variable.
- A pointer computed before the first call gets a saved register first
  (`s = &e->sub;` at the top).
- **Empty `if`s.** An empty `if (x) {}` on a value that is set anyway emits
  no code but changes how IDO weighs each value's claim on a register. They
  are the main lever for register swaps that nothing else moves; try them at
  each position, and in pairs. Only on values that are always set (see
  "Fakes" below).
- "Every temporary one register higher" can mean a body wrapped in
  `if (x != NULL)` where the original returned early, or a missing local.

### Types and constants

- `1.0` (a double) passed to an `f32` parameter is a different constant from
  `1.0f`; `x * (f32) 0` keeps a multiply that `x * 0.0f` folds away;
  `/ (f32) 8` keeps a divide that `/ 8.0f` turns into `* 0.125`.
- IDO shares a constant between two uses only when both have the same
  signedness: a `u32` mask's 1 and a signed compare with 1 load separately.
- A byte parameter shows as `andi aN, 0xFF` at entry. A field stored with
  `sb -1` is `s8`, not `u8`. `lw; sll; srl` reads a bitfield.
- A function that returns a value (even one the caller ignores) stops IDO
  reusing `v0` after the call. A callee declared with the wrong return type
  changes its callers' code: check prototypes against definitions first.
- A call with no prototype in scope promotes `float` arguments to `double`,
  and passes only what the call gives. A function called with fewer
  arguments in some places than others had no prototype: declare it `()`.

### Statement order and lines

- **IDO schedules by source line.** Joining two statements on one line, or
  splitting one, can change the code: `for (i = 0; i < 4; i++) a[i] = 0;` on
  one line differs from the same loop over two lines.
- Plain stores move freely, but a load through one pointer does not move
  above a store through another: write first the statement whose load the
  target does first.
- When only the order of a few stores differs, try every order (a script
  does 120 orders of five in minutes).
- Operand order matters: `a + b` against `b + a` can swap `addu` or `add.s`
  operands; a no-op cast, a different spelling (`list = first + list`), or
  writing a sum without an intermediate variable can fix it.

### Control flow

- `for` loops come out rotated and sometimes unrolled. `while (i--)`,
  `do {} while`, a bound read from memory, or `while (TRUE) { ...; if (++j ==
  8) break; }` give other shapes; a `count` variable becomes `!=`, a bound in
  the condition stays `slt`.
- Switch bodies are laid out in source order; a `default:` written first
  comes right after the compare chain. IDO only emits a jump table when
  enough cases are listed.
- Branch polarity decides which arm IDO puts in the delay slot:
  `if (x >= limit) return;` and `if (x < limit) { ... }` differ. Else-if
  chains, early returns and a single `return result;` at the end all give
  different code.

### Globals and data

- A separate `lui` for each store means separate symbols; one shared base
  means one struct or an array. A shared `lui` across `sym`, `sym+4`, `sym+8`
  means the data was defined in the original file: an `extern` costs a
  `lui` per access, so it waits until that section is in C.
- A pointer local set to a global's address (`T *p = &gThing;`) gives every
  access its own `%hi`/`%lo`; indexing the global directly lets IDO share
  one.
- `static const` floats and `extern` floats load differently from literals:
  a function with float literals needs its file's .rodata in C.

## 7. Fakes

A *fake match* is C that matches only because it contains something the
original programmer would not have written. Some are accepted, many are not.

Accepted, because they cannot change behaviour:

- empty `if`s on values that are always set (`if (obj) {}` where `obj` is
  never NULL there);
- unused locals, `if (0) {}` and dead assignments of the kind DKR's own code
  has;
- a read of an uninitialised value that the original also does, with a
  comment saying so.

Rejected:

- anything that **reads a value that was never set**, even if no code comes
  of it: `if (effect) {}` on an uninitialised local, `x * 0` on one. That is
  undefined behaviour;
- anything that **changes behaviour**: a changed compare, a store moved out of
  a loop, a loop counter changed inside the loop (`k = k - 3`), a shift by -1,
  `i < (i = 4)`;
- a prototype changed outside the function to make it match, unless every
  caller still matches;
- anything that gets around the name check (below).

When the only way to match is one of these, keep the function as
`NON_MATCHING` and say what is left.

## 8. Naming

A function or variable keeps its address name until there is evidence for a
real one. The full rules and style are in the README ("Naming"); in short, a
name may come only from:

1. DKR's `src/` and `include/` (CC0), for a function that is the same as a
   DKR one;
2. ultralib or the SDK, for library code;
3. a plain description of what the code does, worked out from the C, its
   callers and callees.

**Never use a name that only the ROM's runlink name table has**, and never a
near copy of one (a changed letter, reordered words, an added prefix), and
don't use them to find or pair functions either. No generated file shows
them. If your own description comes out close to one, choose a clearly
different one.
Ordinary words the code supports (`draw`, `update`) are fine. Never quote a
ROM-only name anywhere: code, comments, docs or commit messages.

Renames go in [`names.txt`](../names.txt) (`func_XXXXXXXX = name`, fields as
`field Type.unkXX = name`), and `tools/rename.py` applies them to the tree.
`tools/rename.py --check` fails if an old name is left anywhere;
`--field-uses` lists the uses of a renamed field's old name to check by hand.

`tools/check_names.py --dkr PATH_TO_DKR --since 827f28f` enforces the rule. It
reads the runlink names from your ROM, then every word of every tracked or
new file, every commit message and every added line, and fails on any word
that is a ROM name that neither DKR's source nor ultralib also has. It prints
where (file:line, commit), never the name.
Run it before every commit you push. Never work around it (splitting a string
in two, changing a letter): a game string that holds such a name stays in
assembly.

## 9. Finish

1. Replace the `INCLUDE_ASM` (or drop the `NON_MATCHING` wrapper).
2. `make` and `make check`: the ROM must be byte-identical.
3. `make nonmatching`, `tools/rename.py --check`, `tools/check_names.py`,
   `python3 tools/ci_rom_free.py`.
4. Credit any DKR code in CREDITS.md.
5. Commit (see [CONTRIBUTING.md](../CONTRIBUTING.md)).
