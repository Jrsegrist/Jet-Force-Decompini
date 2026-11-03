# Matching tools

Helpers for matching functions against the ROM. They need your ROM
(`baserom.us.z64`) and `make extract`, find the repository from their own
location (any checkout or worktree), and write scratch files under
`build/match/`. Run them with the venv's Python (`.venv/bin/python3`, which
has rabbitizer), from any directory. Each has `--help`.

Some print names read from your ROM (runlink names): never commit or paste
their output.

| Tool | What it does | Example |
|---|---|---|
| `probe.py` | Compiles one main-segment C file with the game flags and compares each function with the ROM (relocations masked); lists referenced symbols' ROM addresses and warns on own .data/.rodata. | `probe.py src/gzip.c` (`--show` disassembles diffs; extra cc flags after the file) |
| `modprobe.py` | The same for runlink modules: text per function, then .data and .rodata. | `modprobe.py m020 src/modules/m020.c -v` |
| `var.py` | Probes many variants of one function (full definitions or `old \|\|\| new` replacement sets) in a scratch copy. | `var.py src/gzip.c func_80001234 vars.txt --stop` |
| `buildvar.py` | Like var.py, but a full `make` + `make check` per variant (catches the %hi/%lo and jal targets the probe masks); restores the file. | `buildvar.py src/gzip.c func_80001234 vars.txt 0x80001234 0x80001300` |
| `permdecl.py` | Writes a variants file with every order of a set of declarations (slot `@1@` of a `=== TEMPLATE` block). | `permdecl.py tmpl.txt vars.txt 's32 i; s32 j; f32 x'` |
| `ulgame.py` | Compiles ultralib (2.0I/J/K/L x -O1/2/3 x -mips1/2) and finds whole objects at the start of remaining asm functions. | `ulgame.py --only io/ --opt 2 --mips 1` |
| `dkrscan.py` | Compiles DKR's src/*.c and finds its functions in JFG's main segment (exact, masked). | `dkrscan.py --dkr ~/jfg_refs/dkr --only math_util` |
| `near.py` | After dkrscan: how close each DKR function with an official name is to its still-asm JFG address. | `near.py --max 50` |
| `perm.py` | Makes a decomp-permuter workspace for one function (game or module flags), and shows the best result and its diff. | `perm.py import src/7A7D0.c func_8007CCF0` |

`common.py` holds the shared paths and flags (keep `GAME_CFLAGS` in step
with the Makefile).

## Variants files (var.py, buildvar.py)

```
=== PRE
old text --- new text          (applied to the whole file first)
=== base
=== swap
    s32 i; ||| s32 j;\n    s32 i;
=== full
void func_80001234(void) {
    ...
}
```

An empty block is the function as it is; a block of `old ||| new` lines
edits the current definition (`\n` = newline); anything else is a whole
replacement definition (with any declarations it needs before it).

## The permuter (perm.py)

[decomp-permuter](https://github.com/simonlindholm/decomp-permuter) (MIT) is
a git submodule at `tools/permuter`, pinned like asm-differ. It rewrites a
function at random (temporaries, statement and declaration order, casts,
operand order ...) and keeps the rewrites whose code is closer to the target.
It needs `toml` (in requirements.txt; `make setup` installs it), and
optionally `Levenshtein`.

`perm.py` makes the permuter's workspace for one function with this
project's compile command (IDO 5.3 `-O2 -mips1` and `GAME_CFLAGS` from
common.py; `-I include/modules` for `src/modules/*.c`):

```
.venv/bin/python3 tools/match/perm.py import src/7A7D0.c func_8007CCF0
.venv/bin/python3 tools/permuter/permuter.py build/perm/func_8007CCF0 -j4 --best-only
.venv/bin/python3 tools/match/perm.py best build/perm/func_8007CCF0
.venv/bin/python3 tools/match/perm.py diff build/perm/func_8007CCF0 build/perm/func_8007CCF0/output-0-1/source.c
```

- `import FILE FUNC` preprocesses FILE with IDO's own `cc -E` (game flags,
  `-DNON_MATCHING`, so a NON_MATCHING body is what it permutes), keeps FUNC's
  body and turns every other function into a prototype (statics lose
  `static`), and assembles FUNC's `.s` (from its INCLUDE_ASM line) as
  `target.o`. Names the assembly spells differently from the C at the same
  address (runlink names, `rl_mNNN_XXXXX` for `func_mNNN_XXXXX`) are renamed
  to the C's, so a score of 0 is reachable. `--asm NAME` for a static whose
  INCLUDE_ASM name differs, `--replace DEF.c` to start from a draft
  definition instead of the file's, `--tag` to keep several workspaces.
- The workspace is `build/perm/FUNC[-TAG]/` (base.c, target.s/.o,
  compile.sh, settings.toml with `compiler_type = "ido"`); the permuter
  writes `output-SCORE-N/` there. build/ is gitignored: target.s and the
  outputs hold names from your ROM, so never commit or paste them.
- Seed it with the best C you have. When you know which choices matter, put
  PERM macros in base.c (`PERM_GENERAL(a, b)`, `PERM_LINESWAP(...)`,
  `PERM_RANDOMIZE(...)` around the part to vary; see the permuter's README).
- Run with `-j4` at most, and stop a run after 20-30 minutes without
  progress.
- **A lower score is not a match.** The permuter can produce code that
  scores lower and means something else (a dropped store, a changed
  condition). Read every candidate, check it means the same as before, write
  it back into the real file in the project's style, and confirm with
  `probe.py`/`modprobe.py` and the full build (`make`, `make check`).

Notes from using it (round 8):

- The default score ignores stack offsets and the frame size: a 0 can still
  differ in the frame. Use `--stack-diffs` for frame/slot problems, and
  always check with the probe.
- Its candidates are written in the permuter's own style (`new_var`,
  statements joined onto one line, `if (x) {}`). Read what changed and find
  the plain spelling that keeps the match: IDO schedules by source line, so
  a joined line can be a real lever (a one-line `for (...) a[i] = 0;`).
- `perm.py diff` with a candidate shows what is left; small hand sweeps
  (statement order, declaration order, pads) finished several of its near
  misses.
