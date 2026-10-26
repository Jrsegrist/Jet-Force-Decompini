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
| `src/asset_loading.c`, `src/asset_loading.h` | `src/asset_loading.c`, `.h` | all 7 functions (pi_init, asset_table_load, dmacopy in JFG form) |
| `src/set_rsp_segment.c` | `src/set_rsp_segment.c` | whole file |
| `src/joypad.c`, `src/joypad.h` | `src/joypad.c`, `.h` | 12 functions (input_init in JFG form; input_update as NON_MATCHING C), constants changed for JFG |
| `src/printf.c`, `src/printf.h` | `src/printf.c`, `.h`, `src/unused_string.c` (memset) | 12 functions (with debug_text_init, debug_text_print), plus the JFG character renderer |
| `src/memory.c`, `src/memory.h` | `src/memory.c`, `.h` | 15 functions (mempool_init_main, mempool_new_sub, mempool_alloc_fixed, mempool_free, mempool_free_timer, mempool_free_queue, mempool_free_queue_clear in JFG form; func_8004AA14 is the flush loop of mempool_free_timer), the pool structures |
| `src/audio.c`, `src/audio.h` | `src/audio.c`, `.h`, `src/audiosfx.h` | 34 functions (music_volume, music_channel_reset_all, music_play, music_jingle_play, music_volume_config_set, music_volume_set, sound_volume_change, sound_update_queue, sound_volume_set_relative, sound_play, sound_play_direct added) |
| `src/4BC70.c` | `src/save_data.c`, `.h` | 16 controller pak functions (get_controller_pak_file_list, delete_file, get_free_space, copy_controller_pak_data added), constants changed for JFG |
| `src/53C0.c` | `src/objects.c`, `include/structs.h` | 12 functions (object_undo_player_tumble, free_object, render_object, move_object added; object_do_player_tumble as NON_MATCHING C), Vertex |
| `src/12D50.c` | `src/tracks.c`, `.h`, `include/level_object_entries.h` | 13 functions (skydome_spawn, reset_fog, set_fog, get_fog_settings, slowly_change_fog, obj_loop_fogchanger added; get_level_segment_index_from_position as NON_MATCHING C), FogData |
| `src/69050.c` | `src/fade_transition.c`, `.h` | 5 functions |
| `src/5B700.c` | `src/weather.c`, `src/particles.c` | 8 functions (weather_update, weather_free, weather_init, rain_fog added) |
| `src/217C0.c` | `src/lights.c`, `src/objects.c` | 10 functions (light_enable/disable, lightGetLights, lights_free, lights_init, light_add_from_object_header, light_setup_light_sources, light_direction_calc, light_remove) |
| `src/25A70.c` | `src/object_functions.c` | obj_loop_rangetrigger, obj_init_animator |
| `src/44350.c` | `src/game.c`, `include/level.h` | level_header, level_type |
| `src/707F0.c` | `src/font.c`, `.h` | 20 dialogue box functions (set_text_font, dialogue_clear/close, open_dialogue_box, the colour/font/size setters, draw_text, draw_dialogue_text_pos_unused, render_dialogue_boxes, render_dialogue_box, render_fill_rectangle, render_dialogue_text, parse_string_with_number, get_text_width, s32_to_string) and the DialogueBox/DialogueTextElement structures |
| `src/59060.c` | `src/menu.c` | get_language |
| `src/boot.c` | `src/main.c` | boot, the idle thread, thread3_verify_stack |
| `src/40240.c` | `src/camera.c`, `.h` | 22 camera/viewport functions (cam_init, viewport_menu_set, copy_framebuffer_size_to_coords, viewport_scissor, viewport_reset, mtx_perspective, mtx_ortho, viewport_rsp_set, mtx_cam_push added; cam_set_fov as NON_MATCHING C) |
| `src/55130.c` | `src/textures_sprites.c`, `src/video.c`, `src/screen_asset.c`, `include/structs.h`, `include/sched.h` | 17 functions (tex_free, tex_load_sprite, sprite_free, set_animated_texture_header, rendermode_reset, fb_size, video_delta_reset, video_init, fb_init_vi, fb_update, update_colour_cycle, tex_animate_texture added; load_texture, screenimage_load, screenimage_draw as NON_MATCHING C) |
| `src/4E610.c` | `src/rcp_dkr.c`, `.h`, `include/sched.h` | 9 functions (gfxtask_run_xbus, gfxtask_wait, bgdraw_render, rdp_init, gfxtask_init, bgdraw_chequer added), DKR_OSTask |
| `src/45750.c` | `src/thread3_main.c` | 6 functions (force/safe_mark_write_save_file, thread3_main added) |
| `src/67780.c` | `src/thread0_epc.c` | 3 functions (thread0_create added) |
| `src/2610.c` | `src/audiomgr.c` | 2 functions |
| `src/3430.c` | `src/audio_spatial.c` | 8 functions (audspat_point_set_position, audspat_point_stop, audspat_play_sound_direct, audspat_play_sound_at_position, audspat_point_create, audspat_calculate_spatial_pan added), AudioPoint |
| `src/74270.c` | `src/game_text.c` | 1 function |
| `include/common.h` | `include/types.h` | `MtxF`, `stubbed_printf` |
| `include/modules/object.h` | `include/structs.h` | `ObjectTransform`, `Vec3f`/`Vec3s`, and the `Object` fields JFG keeps at DKR's offsets (with their names) |
| `include/modules/gfx.h` | `include/structs.h`, `include/f3ddkr.h` | `Vertex`, `Triangle` (without DKR's unions), `gSPPolygon`, `G_TRIN` |

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
