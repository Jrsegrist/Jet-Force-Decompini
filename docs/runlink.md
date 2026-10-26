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
| 2 | a symbol index (unused) | R_MIPS_26 only: the instruction already holds the target as an offset into this module; the loader adds the module base. The word is always the word of the kind-0 entry before it (1090 of 1090 entries), so the original linker evidently left a variable unchanged |
| 3 | global symbol index | only 3 entries, all R_MIPS_32 in the main imports table: resolved like kind 0, but the site address is computed from a different base pointer (0x800FEAC0 rather than 0x800FEABC) |

The addend lives in the instruction or word, as in an ELF REL object: a
`%hi`/`%lo` pair together hold the offset from the target, a jump's field holds
`target_offset >> 2` (kind 2) or 0.

Table 1 holds the references to symbols outside the module (kind 0; main or
other modules). Table 2 holds everything inside the module: kind 0 for the
module's own global symbols, kind 1 for static data and code, and kind 2
jumps to static functions. A kind-1 word is always the start of a section
(.text, .data, .rodata or .bss) with the offset into it in the instruction:
IDO refers to static symbols through section symbols, and each module is a
single IDO object.

The entries are not sorted by offset in 72 of the 157 modules. They are in the
order of the module's original relocatable object (IDO's relocation order,
including how it places each %hi before its %lo). A module compiled from C
reproduces that order by itself (module 45 does).

### What the modules were built from

Each module is one object compiled with the game's flags, IDO 5.3 -O2
-mips1: no branch-likely instruction anywhere, load-delay nops, and matching
C compiles with the main game's CFLAGS unchanged. The global symbol table
holds exactly the objects' global symbols, so a module's own function or
variable is exported if and only if it was not `static`. The module's data
block is the object's .data followed by its .rodata, each 16-byte aligned,
and .bss follows. A module's shutdown function (`fini`), where it has one,
can be a separate small function that just calls the module's cleanup
(module 1).

### Loader

The loader in main resolves one entry's target at 0x80052874, applies a
relocation at 0x80052A00 (R_MIPS_26 keeps the top 6 bits, R_MIPS_HI16 rounds
for the sign of the low half, then writes back the data and instruction
caches) and processes one entry at 0x80052AB8. A %hi entry is applied
together with the %lo entry that follows it.

## Splitting and rebuilding

`make extract` runs `tools/runlink_split.py`, which writes every module as
`asm/modules/mNNN.s`: the disassembled .text, the .data as words and the .bss
as space, with each relocated field written symbolically. All names are by
address, so that C written from the assembly never contains a name from the
ROM's name table (those appear only in comments):

| name | stands for |
| --- | --- |
| `func_XXXXXXXX`, `D_XXXXXXXX` | main-segment code, data or bss at that address (kind 0) |
| `func_mNNN_XXXXX`, `D_mNNN_XXXXX` | a symbol exported by module NNN at offset XXXXX (kind 0) |
| `rl_mNNN_XXXXX` | module NNN's own code or data at offset XXXXX, referred to as a module offset (kind 1, or kind 2 for a jump); placed at the section start that the word gives, with the offset into the section in the instruction |

When two runlink symbols share an address (5 cases) and a reference uses the
second one, the reference keeps the runlink name.

It also splits each module for work in C, into
`asm/nonmatchings/modules/mNNN/`: one `func_mNNN_XXXXX.s` per function (for
INCLUDE_ASM), `data.s`, `rodata.s` and `bss.s` for the module's sections,
and `functions.txt` listing the functions. Function starts are the module's
exported symbols, jump targets, init and fini, and every point after a
`jr $ra` that no earlier branch or jump-table case of the function reaches
past (1275 functions). The .data/.rodata boundary is the one kind-1 word
inside the data block that is not its start.

Switch jump tables (85 in 41 modules) are found from their `jr $reg`: the
register comes from a `lw` through a `%lo` into the data block, and the table
is the run of words there that point into .text, as long as the `sltiu`
bound before it. In the per-function files the table is read as
`%hi`/`%lo(jtbl_mNNN_XXXXX)`, a local label in `rodata.s` (or `data.s` in a
module without .data), whose entries are the `.LXXXXX` labels of the cases,
as m2c expects (pass `rodata.s` to m2c with the function). gas turns these
references into section-relative relocations, which give the same runlink
entries as the section-start `rl_` names of `asm/modules/mNNN.s`. A jump
table can only point into assembly through its labels, so a function with
one in C needs the module's .rodata in C too (and vice versa).

`tools/runlink_build.py` then plays the original linker. For each module it
takes one ELF object, either assembled from `asm/modules/mNNN.s` or compiled
from `src/modules/mNNN.c` (see below), lays out .text, .data, .rodata and
.bss, and maps each ELF relocation to a runlink entry:

| relocation against | entry |
| --- | --- |
| a section symbol, a local symbol, `rl_mNNN_XXXXX`, or this module's own unexported `func_`/`D_mNNN_` name | table 2, kind 1, word = module offset of the section, symbol or name; for R_MIPS_26 kind 2, with the target offset written into the jump field and the word copied from the previous kind-0 entry |
| `func_XXXXXXXX`/`D_XXXXXXXX`, `func_mNNN_`/`D_mNNN_` of an exported symbol, a runlink name, a name that `undefined_syms.us.txt` gives an address or an address name, or a global defined in the object | kind 0, word = the runlink symbol index; table 2 if the symbol is this module's own, else table 1 |

It takes the initialisation and shutdown offsets from the symbols `__rl_init`
and `__rl_fini`, and writes the module images, the module table and
`build/runlink/modules.map` (every function's module, offset and size). The
build links the images and table in place of the extracted blobs, and the
ROM's SHA-1 checks the result.

Because gas (and asm-processor) order relocations by offset, the splitter
also writes the original entry order to `build/runlink/order/mNNN.txt`, and
the builder sorts each table by it. Without it 85 modules come out identical
and 72 differ only in the order of their entries. A module that is all C
does not need it; a module that still has INCLUDE_ASM functions does.

Everything the splitter writes comes from the ROM and stays out of git.

## Modules in C

`modules.mk` builds a module from `src/modules/mNNN.c` when that file
exists, and from the assembly otherwise. The C is compiled like game code
(IDO 5.3 -O2 -mips1, the same CFLAGS, asm-processor) and the object goes
straight to `tools/runlink_build.py`: it is not linked, and no addresses are
fixed, so references to main and to other modules stay relocations.

Working on a module:

1. Copy the function list from `asm/nonmatchings/modules/mNNN/functions.txt`
   into `src/modules/mNNN.c` as
   `INCLUDE_ASM("asm/nonmatchings/modules/mNNN", func_mNNN_XXXXX);` lines,
   plus `INCLUDE_ASM(..., data);`, `rodata` and `bss` for the sections the
   module has (the files that exist). `make` must still pass: the module
   is now an asm-processor object. Order matters: functions in offset order.
2. Replace INCLUDE_ASM lines with C one at a time. m2c works on the
   per-function files.
   - Exported functions and variables (those that have a runlink name, shown
     in a comment in the asm) must be global, named `func_mNNN_XXXXX` or
     `D_mNNN_XXXXX` (or a DKR name). Everything else must be `static`: a
     global that the module does not export is an error.
   - Call main-segment functions and use main data by address names
     (`func_80048E2C`, `D_800A3324`) or by names that `undefined_syms.us.txt`
     maps to them; other modules' exports by `func_mNNN_XXXXX`.
   - Data: either keep `INCLUDE_ASM(..., data)` (and `rodata`, `bss`) or
     define the section in C, all of it, as `static` variables in address
     order. Assembly functions refer to it by `rl_mNNN_XXXXX` names, which
     the builder resolves by offset, so C variables can have any name. C
     functions should use C-defined data: IDO then refers to it relative to
     the section, as the original did.
   - Mark the init and fini functions with `#pragma weak __rl_init = func`
     and `#pragma weak __rl_fini = func`.
3. Check: `make` (the SHA-1), or diff one function with asm-differ:
   `python3 tools/asm-differ/diff.py -mw func_mNNN_XXXXX` compares the built
   module image with the ROM's at that function (tools/runlink_diff.py,
   called from diff_settings.py); `-o` compares the C object with the
   assembly object instead (shows symbols; static data shows up as `.data+N`
   on one side and `rl_mNNN_XXXXX+N` on the other; use `-s` to stop at the
   return).
4. `make progress` counts module functions in C per module (`tools/progress.py
   --modules` lists them).

Examples: `src/modules/m078.c`, `m045.c` and `m020.c` (whole modules;
m045's relocation table is one of the 72 out-of-order ones), `m119.c` (an
exported variable), `m108.c` (a call into another module), `m094.c` (a jump
table in C), `m038.c` (a display list with C data), `m060.c` (C function,
assembly data), `m118.c` (C next to INCLUDE_ASM functions, the rest under
NON_MATCHING), and `src/modules/m001.c` (C data and five C functions, among
them a static one and the fini, next to three INCLUDE_ASM functions and the
INCLUDE_ASM .rodata).

Equivalent C that does not match yet is kept under `#ifdef NON_MATCHING`
with the INCLUDE_ASM lines in the `#else` branch (m089, m100, m110, m118,
m138, m145).

Shared headers (`-I include/modules`): `module.h` (the module ABI and the
main-segment functions modules share, including module 6's message queue
and module 10's spark), `object.h` (the game object as behaviours see it:
DKR's transform and the fields JFG keeps at DKR's offsets, the level entry
start, `OBJ_STATE(obj, T)` for the state a behaviour keeps at 0x88) and
`gfx.h` (the game's vertex and triangle formats and their commands). Older
module files declare their own types; new ones include `module.h`.

Things that came up while matching module code (the main game's IDO -O2
lessons apply too):

- Unused parameters of non-leaf functions are still stored to their home
  slots; unexplained `& 0xFF` on byte fields, odd temporaries and unused
  stack slots (m020, m108) are part of the original source; a module's .data
  can be several small static variables rather than one struct (m001).
- IDO gives every named local a stack slot, in reverse declaration order
  (the first declared gets the highest address), even one that lives in a
  register or is never used. The frame size and where spilled values and
  arrays sit therefore tell how many locals the original declared and in
  which order: m157's array offset, m088's two unused locals, m118's frames.
- A call that passes fewer arguments than other calls to the same function
  means no prototype was in scope; the registers it does not set keep
  whatever they held (m074, m094: the animation calls pass only the
  sequence). Those functions are declared `()` in `module.h`.
- A constant that IDO keeps in one register through the whole function is a
  local variable (m074's `one`); an address that each case of a switch
  recomputes and spills is a local pointer (m148's `turn`).
- Types change register choice: a callee that returns a value, even one the
  caller ignores, keeps IDO from reusing v0 right after the call (module
  10's spark add returns its spark), and IDO shares a constant only between
  expressions of the same signedness (the particle mask at 0x84 is u32, so
  its 1 and a signed compare with 1 load separately: m093, m097). A loop
  bound kept as `slt` rather than turned into `!=`, or `sltu`, tells the
  counter's and bound's types too (m089).
- Plain stores are scheduled freely, but loads from the setup struct are
  not moved above stores through another pointer: write the statement whose
  load the original hoists first (m153, m092). An if/else's branch
  polarity decides which arm IDO speculates into the delay slot (m097).
- `a + b` on floats can come out with the operands swapped when `a` is a
  variable; writing the sum without the intermediate variable keeps the
  original order (m145).
- `switch` bodies are laid out in source order; a `default:` written first
  comes right after the compare chain (m123).
- A static function that only assembly calls is dropped by IDO; keep it as
  INCLUDE_ASM until its callers are C (m118). The other way round, C calls a
  static function that is still INCLUDE_ASM by its module offset name,
  `rl_mNNN_XXXXX` (m110). Likewise a module's .rodata comes from all C or
  all INCLUDE_ASM, so the functions that use it (jump tables, float
  constants) go to C together.
- Debug prints compiled out with `stubbed_printf` still leave their format
  strings in the module's data (m089's "GVP:" string).
- Module data can stay assembly while its functions are C: C refers to it as
  `extern` `D_mNNN_XXXXX` (the builder resolves the module's own offset), as
  m060 does for its object table.

## Module families

Most modules fall into a few kinds with the same shape, and modules of one
kind share calls into main and module 3. A matched member is the template
for its siblings. (Kinds are named here from what the code does.)

| kind | modules | shape |
| --- | --- | --- |
| object behaviours | 63, e.g. 20, 74, 78, 84, 88, 92, 93, 100, 103, 108, 123, 128, 134 | an init `(Object *, level entry)` and a control `(Object *, s32 updateRate)` per behaviour, often several pairs per module; state in the object at 0x88 or in `obj->unk68`; most call `func_80079D1C` (hit box) from init |
| switches and targets | 93, 94, 118 | a behaviour pair whose control runs one of six actions on a game flag or a hit (the same six-way jump table: `func_80079BB8`, `func_80079BA8`, `func_800767EC`, `func_8007688C`, `func_80076928`, `func_800772C4`) |
| pickups | 43, 72, 104, 118, 132 | a behaviour pair that checks the player's distance and gives an item, a flag and a message |
| enemy behaviours | 3, 138-152, 154, 155 | module 3 is the enemy AI (groups, patrol paths, firing); 138-152 are enemy kinds whose movement and facing functions module 3 calls, with the enemy record in `obj->unk68` (m148, m150); their attack decisions share one shape (`func_m003_0CAA4` then `func_m003_04260`: m141, m145, m148), and m141 and m145 share their up-and-down movement code |
| bosses | 50, 51, 52 | a control function and eight helpers; 50 and 52 are the same source (the code differs only in data offsets and one constant), 51 shares four helper functions |
| weapons | 15, 16, 17 | a large control function, a firing check and twenty-odd helpers of matching sizes |
| front end | 6, 9, 12, 13, 18, 37, 40, 42, 57-59, 61-63 | screens with init / per-frame / cleanup functions, and the HUD (6, 13, 14) |
| effect libraries | 1, 2, 8, 10 | a pool of particles with add / update / draw / free functions that other modules call (module 10's spark is in `module.h`) |
| fog and lighting | 23, 39 | init / update / fog / fade getters |
| copy-protection checks | 45, 71, 98, 111, 133, 144, 156 | tiny 6105 CIC checks (all C); 71/98 and 111/133 are pairs that differ only in constants |

Identical pairs found by comparing the code: 50/52, 71/98, 111/133 (and 74/103
have the same function sizes but different code).

## Not covered yet

- The main imports table (0x1ECF220) and the symbol and name tables are still
  copied from the ROM as they are.
- The module names (the table has none; modules are numbered 1..157 here).
- Two symbols have odd module numbers (3430 and 4092); they are kept by name.
