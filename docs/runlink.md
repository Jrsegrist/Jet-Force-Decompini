# Runlink: Jet Force Gemini's run-time linked modules

Most of Jet Force Gemini's game code (objects, enemies, menus, the front end)
is not in the main code segment. It lives in 157 relocatable modules that the
game loads from ROM when it needs them and links in place, patching each
relocated instruction or pointer against the global symbol table. This page
describes the format as far as the US 1.0 ROM shows it, and how this project
splits and rebuilds it.

Addresses below are ROM offsets (US 1.0) unless they start with `0x80`.

## Tables

| ROM | Size | Contents |
| --- | --- | --- |
| 0x1ECF220 | 4 + 8n | main imports: u32 count (520), then relocation entries for sites in the main segment that refer to module symbols |
| 0x1ED0270 | 4 x 2372 | global symbol table |
| 0x1ED2780 | 0x20 x 157 | module table |
| 0x1ED3B20 | 0x117518 + 8 | module images, back to back, then zero padding to 16 bytes |
| 0x1FEB040 | 4 x 2372 | name offsets into the string pool, one per symbol |
| 0x1FED550 | | NUL-terminated symbol names |

### Global symbol table

One u32 per symbol: `module << 20 | offset`.

- module 1..157: a module, offset from the start of that module's image
- module 0: main segment text, offset from 0x80000450 (the start of main's text)
- module 4093 and 4094: main's .rodata and .data, offset from 0x800A0660
  (the end of main's text)
- module 4095: main's .bss, offset from 0x800B0B50

Symbol `i`'s name is at `names + nameoffs[i]`. Some addresses have several
names.

### Module table entry (0x20 bytes, big-endian)

| Offset | Type | Field |
| --- | --- | --- |
| 0x00 | u32 | 0 in ROM; the loader keeps the module's load address here |
| 0x04 | u32 | offset of the image from 0x1ED3B20 |
| 0x08 | u32 | .text size |
| 0x0C | u32 | .data size |
| 0x10 | u32 | .bss size (allocated by the loader, not stored) |
| 0x14 | u16 | size in bytes of relocation table 1 |
| 0x16 | u16 | size in bytes of relocation table 2 |
| 0x18 | s32 | offset of an initialisation function in .text, or -1 |
| 0x1C | s32 | offset of a shutdown function in .text, or -1 |

### Module image

`.text`, then `.data`, then relocation table 1, then relocation table 2. The
.bss follows .data in memory. Images are not padded between modules.

### Relocation entries (8 bytes)

    u32 word
    u32 offset << 8 | type << 4 | kind

`offset` is the patched location, counted from the start of the module image
(for the main imports table: from the start of main's text). `type` is the
ELF MIPS relocation type:

| type | ELF name | patched field |
| --- | --- | --- |
| 2 | R_MIPS_32 | whole word |
| 4 | R_MIPS_26 | jump target (low 26 bits) |
| 5 | R_MIPS_HI16 | upper half, rounded for the following %lo |
| 6 | R_MIPS_LO16 | lower half |

`kind` says what `word` means:

| kind | word | target |
| --- | --- | --- |
| 0 | global symbol index | that symbol's address. If its module is not loaded, jumps go to a trap function and data references to a dummy location (0x800FEAD4) |
| 1 | module offset | this module's base + word. For R_MIPS_32 the word already in memory is added |
| 2 | a symbol index (unused) | R_MIPS_26 only: the instruction already holds the target as an offset into this module; the loader adds the module base |
| 3 | global symbol index | only 3 entries, all R_MIPS_32 in the main imports table: resolved like kind 0, but the site address is computed from a different base pointer (0x800FEAC0 rather than 0x800FEABC) |

The addend lives in the instruction or word, as in an ELF REL object: a
`%hi`/`%lo` pair together hold the offset from the target, a jump's field holds
`target_offset >> 2` (kind 2) or 0.

Table 1 holds the references to symbols outside the module (kind 0; main or
other modules). Table 2 holds everything inside the module: kind 0 for the
module's own global symbols, kind 1 for section offsets and local symbols
(the word is the section start for references through a section symbol, as
IDO emits for statics, or the symbol's own offset), and kind 2 jumps.

The entries are not sorted by offset in 72 of the 157 modules. They are in the
order of the module's original relocatable object (IDO's relocation order,
including how it places each %hi before its %lo).

### Loader

The loader in main resolves one entry's target at 0x80052874, applies a
relocation at 0x80052A00 (R_MIPS_26 keeps the top 6 bits, R_MIPS_HI16 rounds
for the sign of the low half, then writes back the data and instruction
caches) and processes one entry at 0x80052AB8. A %hi entry is applied
together with the %lo entry that follows it.

## Splitting and rebuilding

`make extract` runs `tools/runlink_split.py`, which writes every module as
`asm/modules/mNNN.s`: the disassembled .text, the .data as words and the .bss
as space, with each relocated field written symbolically:

- `name` (with the addend from the instruction) for kind 0, the runlink
  symbol's own name. Symbols a module exports are labels in it.
- `rl_mNNN_XXXX` for kind 1: a global label at module offset XXXX.
- `__rlk2_<index>` for kind 2: an undefined marker symbol carrying the
  unused index; the jump field is written as the marker plus the target offset.

These assemble to ordinary ELF objects whose sections are byte for byte the
module's text and data. `tools/runlink_build.py` then plays the original
linker: it maps each ELF relocation back to a runlink entry (undefined symbol:
table 1, kind 0; `__rlk2_`: kind 2; `rl_mNNN_` or a section symbol: kind 1;
another symbol defined in the module: table 2, kind 0), takes the
initialisation and shutdown offsets from the symbols `__rl_init` and
`__rl_fini`, and writes the module images and the module table. The build
links those in place of the extracted blobs, and the ROM's SHA-1 checks the
result.

Because gas orders relocations by offset, the splitter also writes the
original entry order to `build/runlink/order/mNNN.txt`, and the builder
follows it where present. Without it 85 modules come out identical and 72
differ only in the order of their entries. A module compiled from C with IDO
would carry its own order in its object.

Everything the splitter writes comes from the ROM and stays out of git.

## Not covered yet

- The main imports table (0x1ECF220) and the symbol and name tables are still
  copied from the ROM as they are.
- The bss of each module and the module names (the table has none; modules
  are numbered 1..157 here).
