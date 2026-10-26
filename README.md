# Jet Force Gemini decompilation

A matching decompilation of Jet Force Gemini (Nintendo 64, Rare, 1999): C source
that compiles back to a byte-identical ROM. The later goal is a native PC port
built from that source.

This repository contains no game data. You need your own ROM.

## Status

- `make extract` + `make` rebuild a byte-identical US ROM from disassembly
  (no C yet).
- The main code segment is split into 167 assembly files by splat.
- The runlink modules (most of the game logic), assets and audio are
  still binary blobs.

## ROM

| Version | File | SHA-1 |
| --- | --- | --- |
| US 1.0 (NJFE, CIC-6105, 32 MB) | `baserom.us.z64` | `493ced9008dbe932d6e91179b68e8630cf23a023` |

Put the big-endian `.z64` ROM in the repository root as `baserom.us.z64`.

## Building

You need Linux or WSL with `binutils-mips-linux-gnu`, `make` and `python3` (with `venv`).

```sh
make setup      # .venv with splat64 (requirements.txt)
make extract    # checks the ROM, recovers symbol names, splits with splat
make            # builds build/jfg.us.z64 and checks its SHA-1
```

`make extract` runs `tools/runlink_syms.py`. The ROM carries the names of
about 2,400 global symbols for its run-time module linker. The script reads
them from your ROM into `build/syms/` and splat uses them for the main
segment. No ROM-derived file is committed.

## ROM layout (US 1.0)

| ROM | Contents |
| --- | --- |
| 0x0000000 | header |
| 0x0000040 | IPL3 (CIC-6105) |
| 0x0001000 | main code segment, uncompressed, loaded to 0x80000400. Entry point 0x80000400 clears bss 0x800B0B50-0x801059A0, sets sp, jumps to `boot` (0x8003F500). Text 0x80000450-0x800A0660, .data to 0x800A7480, .rodata to 0x800B0B50 |
| 0x00B1750 | asset table (DKR format): u32 count (71), then count+1 u32 offsets relative to 0xB1880 |
| 0x00B1880 | 71 asset files to 0x1ECF220 (textures, models, levels, audio banks, text). Many are Rare "rzip" (5-byte header: little-endian u32 size + 1 byte, then raw deflate) |
| 0x1ECF220 | runlink: main's relocations against module symbols (u32 count, then 8-byte entries) |
| 0x1ED0270 | runlink: global symbol table, 2,372 u32 (module << 20 \| offset; modules 4093/4094/4095 are main's rodata/data/bss) |
| 0x1ED2780 | runlink: module table, 157 entries of 0x20 bytes (ROM offset, text/data/bss sizes, relocation sizes, init/fini offsets) |
| 0x1ED3B20 | runlink: 157 relocatable code modules (0xCFF20 bytes of text), uncompressed |
| 0x1FEB040 | runlink: name offsets, one per symbol |
| 0x1FED550 | runlink: symbol name strings |

The main segment holds the engine (audio manager, memory, track/level,
lights, camera, objects core, font, particles, rzip/inflate, libultra and the
n_audio library). Game objects, enemies, menus and the front end live in the
runlink modules. The loader (`runlink*` functions, about 0x800527F0 in main)
allocates each module, reads it from ROM, and patches its relocations.

## Compiler

- Game code: IDO 5.3 with `-O2 -mips1`. The code has load-delay `nop`s, no
  branch-likely, and DKR's C for small functions compiles to identical bytes.
  A few files are hand-written assembly or `-mips2`.
- libultra: `-mips2` objects at the SDK's usual levels, close to SDK 2.0I.

## Licence

To be decided by the project owner. MIT is intended; see LICENSE.
