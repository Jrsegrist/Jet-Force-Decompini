# Jet Force Decompini

[![CI](https://github.com/Jrsegrist/Jet-Force-Decompini/actions/workflows/ci.yml/badge.svg)](https://github.com/Jrsegrist/Jet-Force-Decompini/actions/workflows/ci.yml)

A matching decompilation of Jet Force Gemini (Nintendo 64, Rare, 1999): C
source that compiles back to a byte-identical ROM. The later goal is a native
Windows port built from that source.

This repository contains no game data. You need your own ROM.

## Status

`make extract` and `make` rebuild a byte-identical US ROM. `make progress`
prints how much of it is built from C:

| Part | Functions in C | Bytes in C |
| --- | --- | --- |
| main segment, game code | 750 / 1,320 | 98,088 / 539,928 |
| main segment, libraries (libultra, n_audio, libc) | 81 / 341 | 12,428 / 102,408 |
| runlink modules (157) | 0 / 1,385 (counted by returns) | 0 / 851,744 |

- The game code in the main segment is split into C files (`src/`), one
  `INCLUDE_ASM` per function that is not decompiled yet. Code shared with
  Diddy Kong Racing comes from the DKR decompilation (see
  [CREDITS.md](CREDITS.md)); the rest is written from the assembly and named
  by address or, where the code shows what it does, by a name of our own
  ([Naming](#naming)).
- 57 libultra objects build from [decompals/ultralib](https://github.com/decompals/ultralib)
  (`src/ultra/`); the rest of the library region is still assembly
  (hand-written `.s` routines, n_audio, and objects from SDK revisions
  ultralib does not model).
- The 157 run-time linked modules are split into assembly and rebuilt by a
  relocation linker of our own (`tools/runlink_build.py`); see
  [docs/runlink.md](docs/runlink.md).
- Assets and audio are still binary blobs.

## ROM

| Version | File | SHA-1 |
| --- | --- | --- |
| US 1.0 (NJFE, CIC-6105, 32 MB) | `baserom.us.z64` | `493ced9008dbe932d6e91179b68e8630cf23a023` |

Put the big-endian `.z64` ROM in the repository root as `baserom.us.z64`.

## Building

You need Linux or WSL with `binutils-mips-linux-gnu`, `make`, `git`, `curl` and
`python3` (with `venv`).

```sh
git submodule update --init   # or clone with --recursive
make setup      # Python venv, IDO 5.3 recomp, m2c, submodules
make extract    # checks the ROM, recovers symbol names, splits code and modules
make            # builds build/jfg.us.z64 and checks its SHA-1
make check      # checks the SHA-1 of the built ROM again
make progress   # functions and bytes built from C
```

After changing the ROM split (`jfg.us.yaml`) or symbol files, run
`make extract` again. `make` does not notice changed compiler flags: delete the
object to rebuild it.

### Names and ROM-derived data

The ROM carries the names of about 2,400 global symbols for its run-time
module linker. `make extract` reads them from your ROM (`tools/runlink_syms.py`)
into `build/syms/` so that the disassembly is readable, but none of them are
committed. Committed C refers to functions and data by address
(`func_80012345`, `D_80012345`), by the names the DKR decompilation uses, or by
names of our own (see [Naming](#naming)); `undefined_syms.us.txt` and
`names.txt` tie those to addresses, and alias files generated at build time
let them meet the ROM names in the assembly.

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
A guess is not a name: when unsure, keep the address name and add a one-line
comment saying what is known. `tools/check_names.py` (see [Checks](#checks))
catches ROM names; the rest is up to the reviewer.

| what | style | examples |
| --- | --- | --- |
| DKR's functions, variables, types and fields | exactly DKR's spelling (DKR mixes snake_case and camelCase; a DKR name is never re-cased) | `mempool_alloc`, `lightGetLights`, `thread0_Main`, `gCameras` |
| libultra and SDK code | the SDK's names | `osFlashInit`, `__osPiDevMgr` |
| other functions | lower snake_case, `subsystem_verb_object` or `subsystem_object_verb`, using the prefix of the DKR-named code they belong with (`cam_`, `viewport_`, `mempool_`, `music_`, `sound_`, `audspat_`, `tex_`, `light_`, `weather_`, `rain_`, `transition_`, `dialogue_`, `input_`, `obj_` …) | `rain_density_get` |
| global variables | `g` + CamelCase; a file-scope `static` defined in C `s` + CamelCase (as DKR) | `gWeatherType`, `sPlayerID` |
| types | CamelCase | `AudioPoint`, `DialogueBox` |
| struct fields | lowerCamelCase; unidentified ones stay `unkXX` / `padXX` (XX = the offset) | `segmentID`, `unk4C` |
| constants and macros | UPPER_SNAKE_CASE | `WEATHER_RAIN` |
| runlink module code | exported symbols keep `func_mNNN_XXXXX` / `D_mNNN_XXXXX` (the runlink build places them by that name); `static` functions and data follow the rules above; module-local types are prefixed with the module (`M001Particle`) | |

When a DKR name and one of our own describe related code in one file, both
stay as they are: DKR's names are never changed to fit the table, and our own
names never imitate a DKR name's casing.

Each named function gets a one-line comment on what it does, ending with its
address, e.g. `/* Returns the rain density, 0.0 to 1.0 (0x8005BFC4). */`.

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
tools/rename.py --which func_80012345   # old <-> new, for notes and old reports
```

The assembly keeps the old names. At build time `tools/rename.py --aliases`
writes `build/syms/names_aliases.us.txt` from names.txt and the C objects
(`old = new;` where C defines the symbol, `PROVIDE(new = old);` where the
assembly does); the linker and the runlink build read it, and the matching
tools read names.txt, so a rename changes nothing they do. Names never change
code: after a rename the ROM, the `NON_MATCHING` objects and every check come
out the same.

### Working on a function

- The function's assembly is `asm/nonmatchings/<file>/func_XXXXXXXX.s`.
- `tools/m2c/m2c.py` gives a first draft in C.
- Replace the `INCLUDE_ASM` line with C, build, and compare with
  `.venv/bin/python3 tools/asm-differ/diff.py -mw <function>` (it uses the
  map file) or just `make check`.
- `tools/match/` has faster helpers: a probe that compares a whole C file
  with the ROM without linking (main segment and modules), variant runners,
  and scans for ultralib and DKR code; see
  [tools/match/README.md](tools/match/README.md).
- A function whose C is right but does not match yet keeps that C behind
  `#ifdef NON_MATCHING`, with its `INCLUDE_ASM` in the `#else` branch and a
  `NON_MATCHING:` comment saying what still differs. The normal build uses the
  assembly; `make nonmatching` compiles the C bodies (into `build_nm/`, not
  linked) so that they keep building. These functions count as assembly in
  `make progress`.

```c
#ifdef NON_MATCHING
void func_80012345(void) {
    ...
}
#else
INCLUDE_ASM("asm/nonmatchings/12D50", func_80012345);
#endif
```

## Checks

CI ([.github/workflows/ci.yml](.github/workflows/ci.yml)) runs on every push
and pull request to `main`. GitHub has no ROM, so it runs what can be checked
without one (`tools/ci_norom.sh`; run it yourself in a clean checkout after
`make setup`):

- no tracked file is ROM data or extracted from it: no ROM, binaries,
  assembly, disassembly or long number tables (`tools/ci_rom_free.py`);
- the Python tools compile and pass pyflakes, and their self-tests pass
  (`tools/ci_test_tools.py`: made-up runlink modules are built, split and
  rebuilt byte for byte; `runlink_syms.py`, `runlink_diff.py`,
  `progress.py` and `rename.py` on synthetic files);
- `names.txt` is applied everywhere: no old name is left
  (`tools/rename.py --check`);
- every C file in `src/` compiles with IDO 5.3 at the project's flags, with
  placeholder assembly in place of each `INCLUDE_ASM`
  (`tools/ci_stub_asm.py`), and so do the `NON_MATCHING` bodies.

Two checks need the ROM and are yours to run before you push:

```sh
make check                                   # byte-identical ROM
tools/check_names.py --dkr ~/path/to/dkr     # no ROM-only names, in files or history
```

`check_names.py` compares every tracked file, commit message and added line
with the names `make extract` read from your ROM, and fails on any that the
DKR decompilation (a clone of it, `--dkr`) and ultralib do not have too.

## Compiler

- Game code: IDO 5.3 with `-O2 -mips1`.
- libultra: SDK 2.0I with Nintendo's flags (os and io at -O1, the rest -O3,
  -mips2); a few objects are the 2.0J form. The n_audio library is not in
  ultralib and stays assembly for now.
- Hand-written assembly: the entry point, the exception/interrupt code and
  maths at 0x49700, the inflate routines at 0x737F0, and parts of 0x74A80.

## ROM layout (US 1.0)

| ROM | Contents |
| --- | --- |
| 0x0000000 | header |
| 0x0000040 | IPL3 (CIC-6105) |
| 0x0001000 | main code segment, uncompressed, loaded to 0x80000400. The entry point clears bss 0x800B0B50-0x801059A0, sets sp and calls the boot function at 0x8003F500. Text 0x80000450-0x800A0660, .data to 0x800A7480, .rodata to 0x800B0B50 |
| 0x00B1750 | asset table (DKR format): u32 count (71), then count+1 u32 offsets relative to 0xB1880 |
| 0x00B1880 | 71 asset files to 0x1ECF220 (textures, models, levels, audio banks, text). Many are Rare "rzip" (5-byte header: little-endian u32 size + 1 byte, then raw deflate) |
| 0x1ECF220 | runlink: main's relocations against module symbols |
| 0x1ED0270 | runlink: global symbol table |
| 0x1ED2780 | runlink: module table, 157 entries |
| 0x1ED3B20 | runlink: the 157 module images |
| 0x1FEB040 | runlink: name offsets, one per symbol |
| 0x1FED550 | runlink: symbol names |

The main segment holds the engine (audio, memory, tracks and levels, lights,
camera, the object core, fonts, particles, rzip, the runlink loader at about
0x80052800) and the libraries. Game objects, enemies, menus and the front end
live in the runlink modules.

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
  submodules), [m2c](https://github.com/matt-kempster/m2c) (GPL-3.0, fetched by
  `make setup` and only run as a tool).

Details are in [CREDITS.md](CREDITS.md).

## Licence

MIT, see [LICENSE](LICENSE). Code adapted from the DKR decompilation is CC0
upstream.
