*For anyone who has found this repository and wants to know what it is, build it, or help with it.*

# Jet Force Decompini

[![CI](https://github.com/Jrsegrist/Jet-Force-Decompini/actions/workflows/ci.yml/badge.svg)](https://github.com/Jrsegrist/Jet-Force-Decompini/actions/workflows/ci.yml)

A matching decompilation of Jet Force Gemini (Nintendo 64, Rare, 1999).

A *decompilation* turns the game's machine code back into C source. *Matching*
means that the C, built with the compiler the original developers used (IDO
5.3), produces the original ROM byte for byte. That is how the C is known to be
right: `make` rebuilds the ROM from source and checks its SHA-1. A native
Windows port built from that source is the later goal.

This repository contains no game data. To build it you need your own copy of
the ROM, which stays on your machine.

## Status

`make extract` and `make` rebuild a byte-identical US ROM. `make progress`
prints how much of it is built from C:

| Part | Functions in C | Bytes in C |
| --- | --- | --- |
| main segment, game code | 1,132 / 1,320 | 318,920 / 539,824 |
| main segment, libraries (libultra, n_audio, libc) | 306 / 340 | 101,280 / 104,204 |
| runlink modules (157) | 1,152 / 1,275 (102 modules all C, 51 partly) | 603,308 / 850,816 |

Of the game code's 1,132 C functions, 1,017 have a real name (DKR's, the SDK's or one of our own, see [Naming](#naming));
the other 115 keep their address names until the code shows what they do (`make progress`, the `names` line).

What that covers:

- **The main segment** is the code the console loads at boot: the engine
  (memory, assets, levels and tracks, objects, camera, lights, particles,
  text, audio, the run-time module loader) and the libraries. Its game code is
  split into C files in `src/`; a function not decompiled yet is pulled in from
  its assembly with an `INCLUDE_ASM` line. Much of the engine is shared with
  Diddy Kong Racing, and that code comes from the DKR decompilation (see
  [CREDITS.md](CREDITS.md)).
- **The libraries** are almost all C: 99 libultra objects build from
  [decompals/ultralib](https://github.com/decompals/ultralib) (`src/ultra/`),
  and the sound player and the n_audio library are C written from the
  assembly. What is left is hand-written assembly (libultra's `.s` routines,
  the RSP microcode).
- **The 157 runlink modules** hold most of the game itself: objects, enemies,
  bosses, menus and the HUD. The game loads and links them at run time; this
  project splits them into assembly and C and rebuilds them with a relocation
  linker of its own ([docs/runlink.md](docs/runlink.md)).
- Assets (models, textures, levels, sounds, text) are still binary blobs taken
  from your ROM.

[docs/layout.md](docs/layout.md) describes how the ROM and the code are laid
out, which files come from where, and what is still assembly and why.

## What you need

- Linux, or WSL on Windows, with `binutils-mips-linux-gnu`, `make`, `git`,
  `curl` and `python3` (with `venv`).
- Your own Jet Force Gemini ROM, US 1.0, big-endian (`.z64`):

| Version | File | SHA-1 |
| --- | --- | --- |
| US 1.0 (NJFE, CIC-6105, 32 MB) | `baserom.us.z64` | `493ced9008dbe932d6e91179b68e8630cf23a023` |

Put it in the repository root as `baserom.us.z64`. Git ignores it, and
everything extracted from it (`asm/`, `assets/`, `build/`) is ignored too:
none of it may be committed.

## Building

```sh
git clone --recursive https://github.com/Jrsegrist/Jet-Force-Decompini.git
cd Jet-Force-Decompini
make setup        # once: Python venv, the IDO 5.3 recompilation, m2c, submodules
                  # now copy your ROM here as baserom.us.z64
make extract      # checks the ROM's SHA-1, reads its symbol names, splits code and modules
make              # builds build/jfg.us.z64 and checks its SHA-1
make check        # checks the SHA-1 of the built ROM again
make progress     # functions and bytes built from C (tools/progress.py --files or --modules for detail)
make nonmatching  # compiles the not-yet-matching C bodies (see docs/matching.md)
```

- `make setup` installs the Python packages into `.venv/`, downloads the
  statically recompiled IDO 5.3 compiler into `tools/ido5.3/` and clones m2c
  into `tools/m2c/`. Neither is part of this repository.
- After changing the ROM split (`jfg.us.yaml`) or the symbol files, run
  `make extract` again.
- `make` does not notice changed compiler flags: delete the object to rebuild
  it.
- `make distclean` removes everything the build and `make extract` wrote.

### Names and ROM-derived data

The ROM carries the names of about 2,400 global symbols for its run-time
module linker. `make extract` reads them from your ROM (`tools/runlink_syms.py`)
into `build/syms/` so that the disassembly is readable, but none of them are
committed. Committed C refers to functions and data by address
(`func_80012345`, `D_80012345`), by the names the DKR decompilation uses, or by
names of our own (see [Naming](#naming)); `undefined_syms.us.txt` and
`names.txt` tie those to addresses, and alias files generated at build time
let them meet the ROM names in the assembly.

## Contributing

- [docs/matching.md](docs/matching.md): how to match one function, the
  tools, the compiler's habits, and what is not accepted.
- [CONTRIBUTING.md](CONTRIBUTING.md): branches, the checks to run before a
  pull request, commit style, and the rules on ROM data and outside code.
- [docs/layout.md](docs/layout.md): the ROM, the main segment, the libraries
  and their build flags.
- [docs/runlink.md](docs/runlink.md): the runlink module format, how modules
  are split and rebuilt, and how to work on a module in C.
- [tools/match/README.md](tools/match/README.md): the matching tools.

## Naming

Functions and data start with splat's address names, `func_XXXXXXXX` and
`D_XXXXXXXX` (`func_mNNN_XXXXX` / `D_mNNN_XXXXX` in runlink module NNN), and
keep them until there is evidence for a real name. A name may come only from:

1. **The DKR decompilation** (its `src/` and `include/`, CC0): the same
   function, variable, struct or field in DKR, matched by code (same
   structure, constants, callees and data), not by guess. DKR's spelling is
   kept, including the "official names" DKR records. Credited in
   [CREDITS.md](CREDITS.md).
2. **libultra / the SDK's public API**, for library code.
3. **What the code itself shows**: its behaviour, its callers and callees,
   the strings it uses, the data it touches.

A name never comes from the ROM's runlink name table (which `make extract`,
splat's assembly and m2c's output show): not the name, and not a word or a
close paraphrase of it. If the only reason to pick a word is that the ROM's
name for that function uses it, pick another word, or keep the address name.
Ordinary words that the code itself supports (`draw`, `explosion`) are fine.
A guess is not a name: when unsure, keep the address name and add a one-line
comment saying what is known. `tools/check_names.py` (see [Checks](#checks))
catches ROM names; the rest is up to the reviewer.

| what | style | examples |
| --- | --- | --- |
| DKR's functions, variables, types and fields | exactly DKR's spelling (DKR mixes snake_case and camelCase; a DKR name is never re-cased) | `mempool_alloc`, `lightGetLights`, `thread0_Main`, `gCameras` |
| libultra and SDK code | the SDK's names | `osFlashInit`, `__osPiDevMgr` |
| other functions | lower snake_case, `subsystem_verb_object` or `subsystem_object_verb`, using the prefix of the DKR-named code they belong with (`cam_`, `viewport_`, `mempool_`, `music_`, `sound_`, `audspat_`, `tex_`, `light_`, `weather_`, `rain_`, `transition_`, `dialogue_`, `input_`, `obj_` …) | `cam_shake_start`, `music_volume_fade` |
| global variables | `g` + CamelCase; a file-scope `static` defined in C `s` + CamelCase (as DKR) | `gWeatherType`, `sPlayerID` |
| types | CamelCase | `AudioPoint`, `DialogueBox` |
| struct fields | lowerCamelCase; unidentified ones stay `unkXX` / `padXX` (XX = the offset) | `segmentID`, `unk4C` |
| constants and macros | UPPER_SNAKE_CASE | `WEATHER_RAIN` |
| runlink module code | exported symbols keep `func_mNNN_XXXXX` / `D_mNNN_XXXXX` (the runlink build places them by that name); `static` functions and data follow the rules above; module-local types are prefixed with the module (`M001Particle`) | |

When a DKR name and one of our own describe related code in one file, both
stay as they are: DKR's names are never changed to fit the table, and our own
names never imitate a DKR name's casing.

Each named function gets a one-line comment on what it does, ending with its
address, e.g. `/* Stops the shakes of the four cameras (0x800422C4). */`.

**The rename map.** [`names.txt`](names.txt) holds every rename as
`old = new` (fields as `field Type.old = new`, renamed by hand: the compiler
reports a use that was missed). A name replaced later is edited on its address
line and keeps a `renamed previous = new` line. `tools/rename.py` applies the
map to the whole tree (C, headers, `undefined_syms`, docs) and is safe to
re-run; INCLUDE_ASM lines, `symbol_addrs.us.txt` and the yaml keep the address
names, because splat names the generated assembly by address.

```sh
tools/rename.py                    # apply names.txt (e.g. on another branch, before a merge)
tools/rename.py --check            # no old name left, fields renamed (also run by the CI)
tools/rename.py --field-uses       # every .old/->old use of a renamed field's old name, to check by hand
tools/rename.py --which func_80012345   # old <-> new, for notes and old reports
```

The assembly keeps the old names. At build time `tools/rename.py --aliases`
writes `build/syms/names_aliases.us.txt` from names.txt and the C objects
(`old = new;` where C defines the symbol, `PROVIDE(new = old);` where the
assembly does); the linker and the runlink build read it, and the matching
tools read names.txt, so a rename changes nothing they do. Names never change
code: after a rename the ROM, the `NON_MATCHING` objects and every check come
out the same.

## Checks

CI ([.github/workflows/ci.yml](.github/workflows/ci.yml)) runs on every push
and pull request to `main`. GitHub has no ROM, so it runs what can be checked
without one (`tools/ci_norom.sh`; run it yourself in a clean checkout, with no
ROM and no `make extract`, after `make setup`):

- no tracked file is ROM data or extracted from it: no ROM, binaries,
  assembly, disassembly or long number tables (`tools/ci_rom_free.py`);
- the Python tools compile and pass pyflakes, the matching tools start
  (`--help`), and the tools' self-tests pass (`tools/ci_test_tools.py`:
  made-up runlink modules are built, split and rebuilt byte for byte;
  `runlink_syms.py`, `runlink_diff.py`, `progress.py` and `rename.py` on
  synthetic files);
- `names.txt` is applied everywhere: no old name is left
  (`tools/rename.py --check`);
- every C file in `src/` compiles with IDO 5.3 at the project's flags, with
  placeholder assembly in place of each `INCLUDE_ASM`
  (`tools/ci_stub_asm.py`), and so do the `NON_MATCHING` bodies
  (`make nonmatching`).

Two checks need the ROM and are yours to run before you push:

```sh
make check                                                   # byte-identical ROM
tools/check_names.py --dkr ~/path/to/dkr --since 827f28f     # no ROM-only names, in files or history
```

`check_names.py` compares every tracked file, every untracked file git does
not ignore, and every commit message and added line with the names
`make extract` read from your ROM, and fails on any that the DKR
decompilation (a clone of it, `--dkr`) and ultralib do not have too. It prints
the names it finds on your terminal only: never paste them anywhere.
`--since 827f28f` skips one older commit whose hit the project has accepted.
The full list of checks for a pull request is in
[CONTRIBUTING.md](CONTRIBUTING.md).

## Credits

- The [Diddy Kong Racing decompilation](https://github.com/DavidSM64/Diddy-Kong-Racing)
  (CC0 1.0), whose C is the basis of the shared engine code here.
- [decompals/ultralib](https://github.com/decompals/ultralib) for libultra,
  used as a git submodule and only `#include`d (it has no licence file, so
  nothing is copied into this repository).
- [ido-static-recomp](https://github.com/decompals/ido-static-recomp) (IDO 5.3,
  fetched as a release by `make setup`; no licence file, not redistributed),
  [splat](https://github.com/ethteck/splat),
  [spimdisasm](https://github.com/Decompollaborate/spimdisasm) and
  [rabbitizer](https://github.com/Decompollaborate/rabbitizer) (MIT),
  [asm-processor](https://github.com/simonlindholm/asm-processor) and
  [asm-differ](https://github.com/simonlindholm/asm-differ) (Unlicense, git
  submodules), [decomp-permuter](https://github.com/simonlindholm/decomp-permuter)
  (MIT, git submodule, a matching tool only), [m2c](https://github.com/matt-kempster/m2c) (GPL-3.0, fetched by
  `make setup` and only run as a tool).

Details are in [CREDITS.md](CREDITS.md).

## Licence

MIT, see [LICENSE](LICENSE). Code adapted from the DKR decompilation is CC0
upstream and credited in [CREDITS.md](CREDITS.md). The ROM, and anything
extracted from it, is not part of this repository and is not covered by the
licence.
