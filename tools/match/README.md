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
