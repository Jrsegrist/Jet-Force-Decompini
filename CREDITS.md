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
| `src/asset_loading.c`, `src/asset_loading.h` | `src/asset_loading.c`, `.h` | 6 of 7 functions (pi_init, asset_table_load in JFG form) |
| `src/set_rsp_segment.c` | `src/set_rsp_segment.c` | whole file |
| `src/joypad.c`, `src/joypad.h` | `src/joypad.c`, `.h` | 11 functions, constants changed for JFG |
| `src/printf.c`, `src/printf.h` | `src/printf.c`, `.h`, `src/unused_string.c` (memset) | 11 functions (with debug_text_init), plus the JFG character renderer |
| `src/memory.c`, `src/memory.h` | `src/memory.c`, `.h` | 12 functions (mempool_init_main, mempool_new_sub, mempool_free, mempool_free_timer, mempool_free_queue in JFG form), the pool structures |
| `src/audio.c`, `src/audio.h` | `src/audio.c`, `.h`, `src/audiosfx.h` | 27 functions (music_volume, music_channel_reset_all, music_play, music_jingle_play added) |
| `src/4BC70.c` | `src/save_data.c`, `.h` | 12 controller pak functions, constants changed for JFG |
| `src/53C0.c` | `src/objects.c` | 9 functions (object_undo_player_tumble added) |
| `src/12D50.c` | `src/tracks.c` | 7 functions |
| `src/69050.c` | `src/fade_transition.c`, `.h` | 5 functions |
| `src/5B700.c` | `src/weather.c`, `src/particles.c` | 6 functions (weather_update, weather_free added) |
| `src/217C0.c` | `src/lights.c`, `src/objects.c` | 6 functions (light_enable/disable, lightGetLights, lights_free, light_setup_light_sources) |
| `src/25A70.c` | `src/object_functions.c` | obj_loop_rangetrigger, obj_init_animator |
| `src/44350.c` | `src/game.c`, `include/level.h` | level_header, level_type |
| `src/707F0.c` | `src/font.c`, `.h` | 13 dialogue box functions (set_text_font, dialogue_clear/close, open_dialogue_box, the colour/font/size setters, draw_text, s32_to_string) and the DialogueBox structures |
| `src/59060.c` | `src/menu.c` | get_language |
| `src/boot.c` | `src/main.c` | boot, the idle thread, thread3_verify_stack |
| `src/40240.c` | `src/camera.c`, `.h` | 13 camera/viewport functions |
| `src/55130.c` | `src/textures_sprites.c`, `src/video.c`, `include/structs.h` | 11 functions (tex_free, sprite_free, set_animated_texture_header, rendermode_reset, fb_size, video_delta_reset added) |
| `src/4E610.c` | `src/rcp_dkr.c` | 3 functions |
| `src/45750.c` | `src/thread3_main.c` | 5 functions (force/safe_mark_write_save_file added) |
| `src/67780.c` | `src/thread0_epc.c` | 3 functions (thread0_create added) |
| `src/2610.c` | `src/audiomgr.c` | 2 functions |
| `src/3430.c` | `src/audio_spatial.c` | 4 functions (audspat_point_set_position, audspat_point_stop added) |
| `src/74270.c` | `src/game_text.c` | 1 function |
| `include/common.h` | `include/types.h` | `MtxF`, `stubbed_printf` |

Function and variable names in these files are DKR's, including the
"official names" DKR records in its comments. Many of these functions differ
in JFG (dropped checks, other struct offsets, extra arguments); the comments
mark those with "JFG". Functions named `func_XXXXXXXX` were written from the
assembly and owe nothing to DKR beyond the structure names they use.

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
