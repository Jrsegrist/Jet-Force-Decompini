*For contributors and readers who want to know how the Jet Force Gemini ROM and this project's code are laid out: what each part is, how it is built, and where it came from.*

# ROM and code layout (US 1.0)

All addresses are for the US 1.0 ROM. ROM offsets are written `0x1000`;
addresses in the console's memory (RAM, "VRAM" in splat's terms) start with
`0x80`. Files in `src/` are named after the ROM offset where they start
(`src/2BAA0.c` begins at ROM 0x2BAA0), or by their DKR or libultra name
when it is known (`src/gzip.c`, `src/ultra/os/sendmesg.c`).

## The ROM

| ROM | Contents |
| --- | --- |
| 0x0000000 | header |
| 0x0000040 | IPL3, the boot code for the CIC-6105 security chip |
| 0x0001000 | the main code segment, uncompressed, loaded to 0x80000400 |
| 0x00B1750 | asset table (DKR's format): u32 count (71), then count+1 u32 offsets relative to 0xB1880 |
| 0x00B1880 | 71 asset files to 0x1ECF220 (textures, models, levels, audio banks, text). Many are Rare "rzip" files (a 5-byte header: little-endian u32 size + 1 byte, then raw deflate data) |
| 0x1ECF220 | runlink: the main segment's relocations against module symbols |
| 0x1ED0270 | runlink: global symbol table |
| 0x1ED2780 | runlink: module table, 157 entries |
| 0x1ED3B20 | runlink: the 157 module images |
| 0x1FEB040 | runlink: name offsets, one per symbol |
| 0x1FED550 | runlink: symbol names |

The split is described in [`jfg.us.yaml`](../jfg.us.yaml), which
[splat](https://github.com/ethteck/splat) reads at `make extract`. The assets
are copied from your ROM as binary files (`assets/`, not in git); none of
them is decoded yet.

## The main segment

IPL3 copies the main segment to 0x80000400 and jumps to it. The entry point
(`asm/entry.s`) clears the .bss, 0x800B0B50-0x801059A0, sets up the stack
and calls the boot function at 0x8003F500 (`src/boot.c`).

| RAM | Section |
| --- | --- |
| 0x80000400-0x80000450 | the entry point |
| 0x80000450-0x800A0660 | .text: the game code, then the libraries |
| 0x800A0660-0x800B0B50 | .data, then .rodata |
| 0x800B0B50-0x801059A0 | .bss (zeroed at boot, not stored in the ROM) |

The runlink loader counts main-segment code from 0x80000450 and calls the
main segment "module 0" (see [runlink.md](runlink.md)).

`make progress` splits the main segment at RAM 0x80084200 (ROM 0x84E00):
below that is **game code**, above it the **libraries**. A few library
objects sit below the line and are counted with the game code: the PI
manager (`src/ultra/io/pimgr.c` at 0x72F30), the flash library
(`src/846F0.c`), `src/74A70.c`, `src/765D0.c`, ultralib's `mtxxfmf` at
0x49760, and libultra's exception handler inside the assembly at 0x74A80.

### Sections and objects

Each original object file (one compiled .c or assembled .s) is one entry in
the yaml's `main` subsegments, and one file here. An object's .data, .rodata
and .bss come later in the segment, in their own subsegments:

- `[addr, c]` or `[addr, c, NAME]`: the text of `src/ADDR.c` or
  `src/NAME.c`.
- `[addr, .rodata, NAME]` (and `.data`, `.bss`): that file's own section,
  defined in its C. A file's functions that are still assembly keep their
  floats, strings and jump tables in their own `.s` file, so a file's
  .rodata can move to C before all its functions match.
- `[addr, rodata]`, `[addr, data]`, `bss_XXXXXXXX`: a block that is still
  assembly. C refers to its contents as `extern`s by address
  (`extern f32 D_800AB120;`).
- `[addr, asm]`: an object that is all assembly, kept as `asm/XXXXX.s`.

Game strings that contain names known only from the ROM stay in assembly
(see [CONTRIBUTING.md](../CONTRIBUTING.md)): either in the `.s` of the
assembly function that reads them, in a generic `rodata` block, or pulled
into a C file with an `INCLUDE_RODATA` line (`src/2BAA0.c`,
`src/66B80.c`).

## Build flags

All code is built with IDO 5.3, the SGI compiler Nintendo's SDK used, run
through its static recompilation for Linux. The game's C files are compiled
through [asm-processor](https://github.com/simonlindholm/asm-processor),
which lets a C file include functions that are still assembly
(`INCLUDE_ASM`) and puts them in their places in the object.

The optimisation level and instruction set differ between parts of the
segment, and a function only matches at its own flags:

| Code | Flags | Where it is set |
| --- | --- | --- |
| game code (`src/*.c`) and the runlink modules (`src/modules/*.c`, with `-I include/modules`) | `-O2 -mips1` | `OPT_FLAGS` and `MIPSISET` in the Makefile; `modules.mk` |
| the sound player (`src/audiosfx.c`) and the n_audio library (`src/naudio_*.c`) | `-g -mips2`: unoptimised, with debug information | target-specific `OPT_FLAGS`/`MIPSISET` in the Makefile |
| ultralib audio objects linked into n_audio (`cspsetvol`, `heap`, `sl` …) | `-g -mips2` | `UL_G_OBJS` |
| ultralib os and io objects (SDK 2.0I) | `-O1 -mips2` | `UL_SDK_OPT_FLAGS` |
| other ultralib objects (gu, mgu, libc) | `-O3 -mips2` | `UL_SDK_OPT_FLAGS` |
| ultralib PI/VI managers and two PI helpers (`pimgr`, `vimgr`, `pirawdma`, `pigetcmdq`) | `-O2 -mips2`; `pimgr` is `-mips1` | `UL_O2_OBJS`, `UL_MIPS1_OBJS` |
| ultralib's long long helpers (`ll`, `llcvt`) | `-O1 -mips3 -32` | `UL_MIPSISET` |
| ultralib controller pak, rumble pak, EPI and PI DMA objects, and a few other PI objects (SDK 2.0J) | `-O2 -g3 -mips2`: optimised, but scheduled as with debug information | `UL_O2G3_OBJS` |
| libultra code written from the assembly: `motor_stop_start`, `98120` | `-O2 -g3 -mips2` | `UL_O2G3_SRCS` |
| libultra code written from the assembly: `979F0`, `99B40`, `98550`, `98910`, `99000`, `9B780` | `-O1 -mips2` | `UL_OLD_SRCS` |
| `osSyncPrintf` (`src/9B310.c`) | `-O2 -mips2` | a target-specific `MIPSISET` |
| the libc formatter `_Printf` / `_Putfld` (`src/9CA20.c`) | `-O3 -mips2` | its own rule (asm-processor cannot pass `-O3`, so the file has no `INCLUDE_ASM` and is compiled directly) |
| the flash library (`src/846F0.c`), `74A70`, `765D0` | the game flags, `-O2 -mips1` | |

The ultralib wrappers (`src/ultra/`) build with `UL_CFLAGS`, which selects SDK
2.0I (`BUILD_VERSION=VERSION_I`); a wrapper for a 2.0J object redefines
`BUILD_VERSION` before its `#include`.

`make` does not notice a changed flag: delete the object to rebuild it. The
matching tools take their flags from `tools/match/common.py`, which must stay
in step with the Makefile.

## Where the code comes from

### Diddy Kong Racing

Jet Force Gemini was built on the engine Rare used for Diddy Kong Racing, and
much of the main segment compiles from the same source. The
[DKR decompilation](https://github.com/DavidSM64/Diddy-Kong-Racing) (CC0) is
the starting point for that code, adapted to JFG's version of each function
(other struct offsets, dropped or added checks). Only DKR's `src/` and
`include/` are used.

| File here | DKR source |
| --- | --- |
| `src/gzip.c`, `src/asset_loading.c`, `src/set_rsp_segment.c`, `src/joypad.c`, `src/printf.c`, `src/memory.c`, `src/audio.c`, `src/audiosfx.c` | the files of the same name |
| `src/boot.c` | `main.c` |
| `src/2610.c` | `audiomgr.c` |
| `src/3430.c` | `audio_spatial.c` |
| `src/12D50.c` | `tracks.c` |
| `src/217C0.c` | `lights.c` |
| `src/25A70.c` | `object_functions.c` |
| `src/40240.c` | `camera.c` |
| `src/44350.c` | `game.c` |
| `src/45750.c` | `thread3_main.c` |
| `src/4BC70.c` | `save_data.c` |
| `src/4E610.c` | `rcp_dkr.c` |
| `src/53C0.c` | `objects.c` |
| `src/55130.c`, `src/560B0.c` | `textures_sprites.c`, `video.c`, `screen_asset.c` |
| `src/59060.c` | `menu.c` |
| `src/5B700.c` | `weather.c` |
| `src/5DA60.c` | `particles.c` |
| `src/67780.c` | `thread0_epc.c` |
| `src/69050.c` | `fade_transition.c` |
| `src/707F0.c` | `font.c` |
| `src/74270.c` | `game_text.c` |
| modules 24, 25, 34, 35, 36, 44 | `tracks.c`, `audio.c`, `objects.c`, `font.c`, `thread3_main.c`, `audio_spatial.c` |

Most of these files also hold functions that DKR does not have, written from
the assembly. [CREDITS.md](../CREDITS.md) lists every DKR function and
structure used, file by file. The rest of the main segment is written from the
assembly.

### libultra (ultralib)

The N64 SDK's operating system library, libultra, is built from
[decompals/ultralib](https://github.com/decompals/ultralib), a git submodule
at `lib/ultralib`. ultralib has no licence file, so nothing is copied from it:

- `src/ultra/<dir>/<file>.c` (99 objects) are one-line wrappers that
  `#include` ultralib's source, sometimes with a macro for a small difference.
- Where the game's object holds only part of an ultralib file, or an older or
  game-specific form that no ultralib version builds, the functions are
  written from the game's assembly instead (`src/979F0.c`, `src/98120.c`,
  `src/98550.c`, `src/98910.c`, `src/99000.c`, `src/99B40.c`, `src/9B310.c`,
  `src/9B780.c`, `src/9CA20.c`, `src/74A70.c`, `src/765D0.c`,
  `src/motor_stop_start.c`, and parts of `src/846F0.c` and
  `src/ultra/audio/sl.c`). CREDITS.md says how each differs from ultralib's.

### The sound player and n_audio

The sound player (`src/audiosfx.c`) is DKR's. The n_audio library (the SDK's
alternative audio library, `src/naudio_*.c`) is written from the assembly;
both were built unoptimised with debug information (`-g`), so their C follows
the assembly almost line for line.

## What is still assembly, and why

### Assembly objects (`asm/*.s`)

The yaml's `[addr, asm]` entries are objects with no C file at all:

| ROM | What | Why |
| --- | --- | --- |
| 0x1000 | the entry point | hand-written |
| 0x49700 | three small routines that turn interrupts off and on in the CPU's status register, under a flag | hand-written (they use `mfc0`/`mtc0`, which C cannot express) |
| 0x49800 | the maths library: matrices, trigonometry, random numbers | hand-written (DKR keeps its maths library as assembly too, `src/hasm/ido/math_util.s`) |
| 0x737F0 | the inflate routines of the rzip decompressor (`gzip_inflate_block` and its helpers) | hand-written, as in DKR (`src/hasm/gzip_asm.s`) |
| 0x74A80 | five game routines, then libultra's exception handler and thread dispatcher (ultralib's `exceptasm.s`) | hand-written code and libultra assembly |
| 0x974C0-0x9D710 (18 small objects) | libultra's assembly routines: `osGetCount`, `sqrtf`, the cache operations (`osInvalDCache`, `osInvalICache`, `osWritebackDCache`, `osWritebackDCacheAll`), `bzero`, `bcopy`, `bcmp`, `osSetIntMask`, `__osDisableInt`/`__osRestoreInt`, the TLB, status, FPU and timer register helpers | libultra's own `.s` files; they are taken from the ROM rather than assembled from ultralib's sources |
| 0x9E610 | the audio microcode (`aspMain`) | RSP microcode: code for the RCP's signal processor, not for the CPU, so there is no C for it |
| 0x9FE50 | `rspboot` and the graphics microcode (F3DDKR, XBUS) | RSP microcode, as above |

splat decodes some words of the two microcode objects as CPU functions,
which is why they appear in `make progress` as library functions in
assembly.

### `INCLUDE_ASM` functions in C files

A C file can still pull in single functions from
`asm/nonmatchings/<file>/`. Those are:

- **hand-written functions** in otherwise compiled files: five in
  `src/235E0.c`, four in `src/1DA10.c`, the float routines of `src/4A9C0.c`
  (they use odd-numbered float registers for single-precision values, which
  IDO never does), and `src/69040.c`'s two-instruction function that returns
  the stack pointer. splat marks the instructions IDO never emits with a
  `handwritten instruction` comment.
- **functions kept in assembly on purpose**: those that read a string
  holding a ROM-only name (see above).
- **`NON_MATCHING` functions**: the C is written and compiles, but does not
  give the same code yet. The C is kept behind `#ifdef NON_MATCHING` with the
  `INCLUDE_ASM` in the `#else` branch, and `make nonmatching` keeps it
  building (see [matching.md](matching.md)).
- **functions not attempted yet**, many of them large, or statics that only
  assembly calls.

`.venv/bin/python3 tools/progress.py --files` lists every object with its
functions and bytes in C.

## The runlink modules

The 157 modules at the end of the ROM hold most of the game: objects,
enemies, bosses, menus and the HUD. Each is one object built with the game
flags. `make extract` splits each into `asm/modules/mNNN.s` and per-function
files under `asm/nonmatchings/modules/mNNN/`; `src/modules/mNNN.c` replaces
the assembly when it exists, and `tools/runlink_build.py` relinks the modules
in the ROM's format. [runlink.md](runlink.md) describes the format, the
module families and how to work on a module.

## Generated files

None of these are in git; `make distclean` removes them.

| Path | Written by | Holds |
| --- | --- | --- |
| `asm/` | `make extract` (splat, `tools/runlink_split.py`) | the disassembly: `asm/*.s` objects, `asm/nonmatchings/<file>/<func>.s` per function, `asm/modules/` |
| `assets/` | `make extract` | the binary blobs |
| `jfg.us.ld` | `make extract` (splat) | the linker script |
| `build/syms/` | `make extract`, `make` | the main segment's runlink symbols by address (`runlink_syms.us.txt`), the module list, splat's undefined symbols, the name aliases, and the build-internal `runlink_aliases.us.txt` (licensed names only; not for reading) |
| `build/` | `make` | objects, `jfg.us.elf`, `jfg.us.map`, `jfg.us.z64`, `build/runlink/` (module images, symbol list, module map) |
| `build_nm/` | `make nonmatching` | the `NON_MATCHING` objects (not linked) |
| `build/match/`, `build/perm/` | the matching tools | scratch output |
