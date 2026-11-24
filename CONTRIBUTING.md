*For anyone preparing a change or a pull request: how to branch, what to check before you open the PR, how to write commits, and the rules on ROM data and outside code.*

# Contributing

Thank you for helping. Every change must keep the build byte-identical to the
original ROM, keep the repository free of ROM data, and follow the rules on
names and outside code below. CI checks part of this; the rest needs your ROM
and is yours to run.

New to the project? Read the [README](README.md) to build it, then
[docs/matching.md](docs/matching.md) for how to match a function and
[docs/layout.md](docs/layout.md) for where things are.

## Branches and pull requests

- Fork the repository and work on a branch made from `main`. Pull requests go
  to `main`.
- Keep a branch to one subject: one file or module, one kind of change (a
  batch of names, a `.rodata` move, a tool fix). Small pull requests are
  reviewed faster.
- Bring your branch up to date with `main` before you open the PR, then run
  `tools/rename.py` once more: names added on `main` may rename symbols your
  new code still calls by their old names, and a call left with no prototype
  in scope can change the code (a `float` argument becomes a `double`).
  Then build and check again.
- In the PR description, say what changed (which functions match, which
  names were added and from where) and that the checks below pass.

## Checks before a pull request

Run all of these from the repository root, after `make extract`:

```sh
make check                       # the ROM is byte-identical (make builds it first); no ROM names generated
make nonmatching                 # every NON_MATCHING body still compiles
tools/rename.py --check          # names.txt is applied everywhere: no old name left
tools/rename.py --field-uses     # after renaming struct fields: check each listed use by hand
tools/check_names.py --dkr ~/path/to/dkr --since 827f28f   # no ROM-only names in files or history
python3 tools/ci_rom_free.py     # no ROM data in tracked files
```

- `check_names.py` needs a clone of the
  [DKR decompilation](https://github.com/DavidSM64/Diddy-Kong-Racing) for
  `--dkr`, and your ROM. It prints only where a ROM-only name is (file:line
  or commit), never the name. `make check` also runs
  `tools/check_rom_names.py` (no generated file under `asm/`, `build/` or
  `src/` holds a name from the ROM's name table) when the DKR clone is at
  `DKR_DIR` (default `~/jfg_refs/dkr`).
- If you changed anything outside `src/`, `include/`, `names.txt` and the
  docs (a tool, the Makefile, the yaml), also run the ROM-free CI checks the
  way CI does, in a fresh clone with no ROM and no `make extract`:

  ```sh
  git clone --recursive <your fork> /tmp/jfg-norom && cd /tmp/jfg-norom
  git checkout <your branch>
  make setup
  bash tools/ci_norom.sh
  ```

  `make check` cannot see a broken tool self-test; `ci_norom.sh` can.
- Once before you open the PR, run `make distclean && make extract && make`,
  so that nothing depends on stale files.
- Never stage a symbolic link, `.venv/`, `tools/ido5.3/`, `tools/m2c/`, or
  anything under `build/`. Add files by name (`git add src/12D50.c`), not
  with `git add -A`.

CI ([.github/workflows/ci.yml](.github/workflows/ci.yml)) runs the ROM-free
checks on every pull request: `ci_rom_free.py`, the Python tools and their
self-tests, `rename.py --check`, a compile of every C file with placeholder
assembly, and `make nonmatching`. It cannot run `make check` or
`check_names.py`, because it has no ROM.

## Commits

- One logical change per commit, and every commit builds and passes
  `make check` on its own.
- The subject line names the file or module first, then what happened, in
  the present tense:

  ```
  2BAA0: player_zone_track_update matches
  m026: func_m026_00518 matches; m026 all C
  5B700: func_8005C188 NON_MATCHING body 71 -> 69 words
  53C0: .rodata in C from 0xAB000
  names: 20 functions in 12D50 and 2BAA0
  docs/runlink.md: per-function .rodata
  ```

- Add a body when the reason is not obvious: what the match needed, what a
  `NON_MATCHING` body still lacks.
- Commit messages are checked too: they must not contain a ROM-only name.

## ROM data

The repository holds no game data, and nothing extracted from the ROM.

- No ROM, no assets, no binaries, no `.s` files (the assembly is extracted
  from your ROM by `make extract`), no disassembly, no linker script from
  splat, and no long tables of numbers copied from the ROM.
  `tools/ci_rom_free.py` checks this.
- The game's own strings in matched C (credits text, menu text) are fine.
- **ROM-only names stay out.** The ROM carries about 2,400 symbol names for
  its module linker. They are read from your ROM at extract time so that the
  assembly is readable, but none may be committed: not in code, comments,
  docs or commit messages, and not as a near copy (a changed letter,
  reordered words). See [docs/matching.md](docs/matching.md), "Naming".
- A game string that contains such a name stays in assembly, never in C: in
  the `.s` of the assembly function that reads it (that function stays
  assembly), or in a generic `rodata` block, or through an `INCLUDE_RODATA`
  line. Never split or alter the string to get past the check. Such strings
  have so far been handled this way in `2BAA0`, `53C0`, `5B700`, `12D50` and
  `66B80`; for any other file, ask in an issue before moving its `.rodata`.

## Outside code

- **Diddy Kong Racing.** Only the DKR decompilation's `src/` and `include/`
  are CC0 and may be used. Its `libultra/` directory is SGI's proprietary SDK
  source: never use it. Record every DKR function, structure or file you use
  in [CREDITS.md](CREDITS.md).
- **libultra.** [ultralib](https://github.com/decompals/ultralib) has no
  licence file: `#include` its sources (the `src/ultra/` wrappers), never copy
  them. Where the game's object holds only part of an ultralib file, or a form
  no ultralib version builds, write those functions from the game's assembly
  yourself, even if the result is the same as ultralib's code, and say so in
  CREDITS.md. Never copy ultralib's text.
- **m2c** is GPL-licensed: run it as a tool, and treat its output as a draft
  to rewrite, not as code to paste.
- **Never** open or copy the unlicensed Jet Force Gemini projects by
  Ryan-Myers and Drahsid, GoldenEye's leaked libultra, or any GPL code.
- **Tools.** Check the licence of any outside tool before adding it, and add
  it as a pinned git submodule, never as a copied-in tree.

## Names

Names come only from DKR's `src/` and `include/`, from ultralib or the SDK
for library code, or from a plain description of what the code does. The
rules and the style are in the README's [Naming](README.md#naming) section;
renames go in [names.txt](names.txt) and are applied with `tools/rename.py`.
When unsure, keep the address name.
