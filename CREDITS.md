# Credits and third-party material

## Diddy Kong Racing decompilation

Jet Force Gemini shares Rare's engine with Diddy Kong Racing, and much of its
main code segment compiles from the same source. The
[Diddy Kong Racing decompilation](https://github.com/DavidSM64/Diddy-Kong-Racing)
(CC0 1.0 Universal, used at commit `a5eb080`) is the basis for these files.
CC0 places no conditions on reuse; the credit is given because it is due.

| File here | From DKR | What |
| --- | --- | --- |
| `src/gzip.c`, `src/gzip.h` | `src/gzip.c`, `src/gzip.h` | whole file |
| `src/asset_loading.c`, `src/asset_loading.h` | `src/asset_loading.c`, `.h` | 4 of 7 functions |
| `src/set_rsp_segment.c` | `src/set_rsp_segment.c` | whole file |
| `src/joypad.c`, `src/joypad.h` | `src/joypad.c`, `.h` | 11 functions, constants changed for JFG |
| `src/printf.c`, `src/printf.h` | `src/printf.c`, `.h`, `src/unused_string.c` (memset) | 10 functions, plus the JFG character renderer |
| `src/memory.c`, `src/memory.h` | `src/memory.c`, `.h` | 7 functions, the pool structures |
| `src/audio.c`, `src/audio.h` | `src/audio.c`, `.h`, `src/audiosfx.h` | 23 functions |
| `src/4BC70.c` | `src/save_data.c`, `.h` | 12 controller pak functions, constants changed for JFG |
| `src/53C0.c` | `src/objects.c` | 8 functions |
| `src/12D50.c` | `src/tracks.c` | 7 functions |
| `src/69050.c` | `src/fade_transition.c`, `.h` | 5 functions |
| `src/5B700.c` | `src/weather.c`, `src/particles.c` | 4 functions |
| `src/217C0.c` | `src/lights.c` | 1 function |
| `src/boot.c` | `src/main.c` | boot and the idle thread |
| `src/40240.c` | `src/camera.c`, `.h` | 10 camera/viewport functions |
| `src/55130.c` | `src/textures_sprites.c`, `include/structs.h` | 5 functions |
| `src/4E610.c` | `src/rcp_dkr.c` | 3 functions |
| `src/45750.c` | `src/thread3_main.c` | 3 functions |
| `src/67780.c` | `src/thread0_epc.c` | 2 functions |
| `src/2610.c` | `src/audiomgr.c` | 2 functions |
| `src/3430.c` | `src/audio_spatial.c` | 2 functions |
| `src/74270.c` | `src/game_text.c` | 1 function |
| `include/common.h` | `include/types.h` | `MtxF`, `stubbed_printf` |

Function and variable names in these files are DKR's, including the
"official names" DKR records in its comments.

## Tools and libraries

| What | Licence | How it is used |
| --- | --- | --- |
| [decompals/ultralib](https://github.com/decompals/ultralib) (libultra) | no licence file | git submodule `lib/ultralib`; `src/ultra/*` only `#include` its sources, nothing is copied |
| [decompals/ido-static-recomp](https://github.com/decompals/ido-static-recomp) (IDO 5.3) | no licence file | release binaries fetched by `make setup` into `tools/ido5.3/` (git-ignored) |
| [asm-processor](https://github.com/simonlindholm/asm-processor) | Unlicense | git submodule `tools/asm-processor`; `include/prelude.inc` is based on its prelude |
| [asm-differ](https://github.com/simonlindholm/asm-differ) | Unlicense | git submodule `tools/asm-differ` |
| [splat](https://github.com/ethteck/splat), [spimdisasm](https://github.com/Decompollaborate/spimdisasm), [rabbitizer](https://github.com/Decompollaborate/rabbitizer) | MIT | installed from PyPI by `make setup` |
| [m2c](https://github.com/matt-kempster/m2c) | GPL-3.0 | cloned by `make setup` into `tools/m2c/` (git-ignored) and only run as a tool; none of its code is in this repository |

Not used: the unlicensed Jet Force Gemini projects by Ryan-Myers and Drahsid,
and GoldenEye's libultra sources. Nothing in this repository comes from them.
