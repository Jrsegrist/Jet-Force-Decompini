# Jet Force Gemini Decomped

A matching decompilation of Jet Force Gemini (Nintendo 64, Rare, 1999): C
source that compiles back to a byte-identical ROM. The later goal is a native
Windows port built from that source.

This repository contains no game data. You need your own ROM.

## Status

`make extract` and `make` rebuild a byte-identical US ROM. `make progress`
prints how much of it is built from C:

| Part | Functions in C | Bytes in C |
| --- | --- | --- |
| main segment, game code | 438 / 1,319 | 33,560 / 539,980 |
| main segment, libraries (libultra, n_audio, libc) | 81 / 341 | 12,428 / 102,408 |
| runlink modules (157) | 0 / 1,385 (counted by returns) | 0 / 851,744 |

- The game code in the main segment is split into C files (`src/`), one
  `INCLUDE_ASM` per function that is not decompiled yet. Code shared with
  Diddy Kong Racing comes from the DKR decompilation (see
  [CREDITS.md](CREDITS.md)); the rest is written from the assembly and named
  by address.
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
(`func_80012345`, `D_80012345`) or by the names the DKR decompilation uses;
`undefined_syms.us.txt` ties those to addresses, and an alias file generated at
extract time lets them meet the ROM names in the assembly.

### Working on a function

- The function's assembly is `asm/nonmatchings/<file>/func_XXXXXXXX.s`.
- `tools/m2c/m2c.py` gives a first draft in C.
- Replace the `INCLUDE_ASM` line with C, build, and compare with
  `.venv/bin/python3 tools/asm-differ/diff.py -mw <function>` (it uses the
  map file) or just `make check`.

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
