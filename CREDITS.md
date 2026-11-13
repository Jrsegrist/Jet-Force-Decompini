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
| `src/memory.c`, `src/memory.h` | `src/memory.c`, `.h` | 15 functions (mempool_init_main, mempool_new_sub, mempool_alloc_fixed, mempool_free, mempool_free_timer, mempool_free_queue, mempool_free_queue_clear in JFG form; mempool_free_queue_flush is the flush loop of mempool_free_timer), the pool structures |
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
| `src/modules/m025.c` | `src/audio.c` | the audio set-up (audio_init, sound_seqplayer_init) in JFG's form, ALSeqpConfig |
| `src/modules/m034.c` | `src/objects.c` | allocate_object_pools, decrypt_magic_codes in JFG's form |
| `src/modules/m035.c` | `src/font.h`, `src/font.c` | DialogueBoxBackground, DialogueTextElement; the window set-up of load_fonts |
| `src/modules/m036.c` | `src/thread3_main.c` | alloc_displaylist_heap, default_alloc_displaylist_heap and their size tables in JFG's form |
| `src/modules/m044.c` | `src/audio_spatial.c`, `.h` | the audio line code (AudioLine in JFG's order, audspat_reset and audspat_update_all's audio line parts, audspat_line_add_vertex, audspat_distance_to_segment, audspat_line_validate) |
| `src/modules/m057.c` | `include/f3ddkr.h`, `include/structs.h` | gDkrDmaDisplayList; the TextureHeader fields it uses |
| `src/modules/m049.c` | `include/structs.h` | the TextureHeader fields it uses |
| `include/common.h` | `include/types.h` | `MtxF`, `stubbed_printf` |
| `include/modules/object.h` | `include/structs.h` | `ObjectTransform`, `Vec3f`/`Vec3s`, and the `Object` fields JFG keeps at DKR's offsets (with their names) |
| `include/modules/gfx.h` | `include/structs.h`, `include/f3ddkr.h` | `Vertex`, `Triangle` (without DKR's unions), `gSPPolygon`, `G_TRIN` |

Added in round 4 (JFG variants of DKR's code, DKR's `src/` and `include/` only):

| File here | From DKR | What |
| --- | --- | --- |
| `src/5B700.c` | `src/weather.c`, `src/particles.c`, `include/structs.h` | rain_init, free_rain_memory, rain_sound, rain_lightning, emitter_init_with_pos, move_particle_basic, move_particle_basic_parent, move_particle_attached_to_parent, move_particle_with_acceleration, move_particle_forward (named in naming phase 1), the Particle/ParticleEmitter/ParticleBehaviour layouts |
| `src/69050.c` | `src/fade_transition.c` | transition_render_fullscreen, transition_render_barndoor_hor/vert/diag, transition_render_blank (named in naming phase 1) |
| `src/4BC70.c` | `src/save_data.c` | get_file_type |
| `src/432D0.c` | `include/structs.h` | ObjectTransform |
| `src/memory.c` | `src/memory.c` | mempool_init, mempool_alloc_safe, mempool_alloc, mempool_slot_find, mempool_slot_assign; mempool_slot_clear as NON_MATCHING C |
| `src/audio.c` | `src/audio.c`, `.h` | music_sequence_stop, MusicData |
| `src/2610.c` | `src/audiomgr.c` | __amMain, __amDMA, __clearAudioDMA (named in naming phase 1), AMDMABuffer and the DMA state |
| `src/25A70.c` | `src/object_functions.c` | obj_init_torch_mist (the radius scaling of obj_init_torch_mist/func_80024F40) |
| `src/3430.c` | `src/audio_spatial.c` | audspat_init |
| `src/12D50.c` | `src/tracks.c` | get_inside_segment_count_xz |
| `src/40240.c` | `src/camera.c` | cam_set_fov now matching, set_viewport_properties, mtx_ortho's copy loop (func_80042134, NON_MATCHING) |
| `src/3C470.c` | `src/textures_sprites.c` | the ASSETCACHE_ID/PTR cache layout |
| `src/67780.c` | `src/thread0_epc.c` | thread0_Main, the thread search of write_epc_data_to_cpak |
| `src/74270.c` | `src/game_text.c` | init_dialogue_text |

Only DKR's `src/` and `include/` are used. DKR's `libultra/` directory holds
SGI's proprietary SDK source, not DKR's CC0 work, and nothing here comes
from it.

Function and variable names in these files are DKR's, including the
"official names" DKR records in its comments. Many of these functions differ
in JFG (dropped checks, other struct offsets, extra arguments); the comments
mark those with "JFG". Functions named `func_XXXXXXXX` were written from the
assembly and owe nothing to DKR beyond the structure names they use.

Names taken from DKR in naming phase 1 (`names.txt`): the same function,
variable or field in DKR, matched by its code, callers and data. The code
itself was already in C; only the names come from DKR.

| From DKR | Names |
| --- | --- |
| `src/audiomgr.c` | __amMain, __amDMA, __amDmaNew, __clearAudioDMA, gAudioSched, audioStack, dmaState, dmaBuffs, audFrameCt, nextDMA, audDMAMessageQ, audDMAIOMesgBuf |
| `src/audio.c` | gSeqSoundTable, gMusicPlaying, gMusicSliderVolume |
| `src/audio_spatial.c` | audspat_init |
| `src/camera.c` | cam_shake_off, cam_shake_on, get_distance_to_active_camera, cam_set_layout, set_active_camera, set_viewport_properties, mtx_pop, get_distance_to_camera, gNoCamShake, gPerspectiveMatrixF, perspNorm, gViewProjMatrixF, gCurrentMVPMatrixF |
| `src/fade_transition.c` | transition_render_fullscreen, transition_render_barndoor_hor/vert/diag, transition_render_blank, enable/disable_new_screen_transitions, check_fadeout_transition, transition_end, gTransitionsDisabled, gTransitionFadeIn, dTransitionFadeSettings, dTransitionShapeSettings |
| `src/font.c` | fontConvertString, gFontConversionTable |
| `src/game.c` | leveltable_type, leveltable_world, level_music_start, level_id, level_name, level_free, gNumberOfLevelHeaders, gGlobalLevelTable, gMapId |
| `src/game_text.c` | init_dialogue_text, free_message_box, set_subtitles, gCurrentMessageText, gTextTableEntries, gDialogueAlpha, gTextAlphaVelocity, gDialogueXPos1/YPos1/XPos2/YPos2, gSubtitleSetting |
| `src/lights.c` | light_toggle |
| `src/menu.c` | get_all_save_files_ptr, set_language, gSavefileData, gMenuText |
| `src/object_functions.c` | obj_init_torch_mist, obj_loop_skycontrol |
| `src/objects.c`, `include/structs.h` | obj_init_emitter, clear_object_pointers; Object.numActiveEmitters, Object.modelInstances |
| `src/particles.c` | obj_disable_emitter, move_particle_basic, move_particle_basic_parent, move_particle_attached_to_parent, move_particle_with_acceleration, move_particle_forward |
| `src/rcp_dkr.c` | dRdpInit |
| `src/save_data.c` | rumble_start, rumble_stop, erase_save_file, calculate_eeprom_settings_checksum, read_eeprom_settings, write_eeprom_settings (JFG keeps the block in flash), get_file_type, gRumble |
| `src/thread0_epc.c` | get_lockup_status, gEPCMesgQueue, gEPCMesgBuf, gEPCPIQueue, gEPCPIBuf |
| `src/thread3_main.c` | default_alloc_displaylist_heap (module 36), get_settings, is_game_paused, get_ingame_map_id, mark_read_save_file, mark_read_all_save_files, mark_save_file_to_erase, mark_read/write_eeprom_settings, gDisplayLists, gCurrDisplayList, gMatrixHeap, gGameCurrMatrix, gVertexHeap, gGameCurrVertexList, gTriangleHeap, gGameCurrTriList, gSPTaskNum, gPrevPlayerCount, gCurrNumF3dCmdsPerPlayer, gCurrNumHudMatPerPlayer, gCurrNumHudTrisPerPlayer, gCurrNumHudVertsPerPlayer, gSettingsPtr, gIsPaused, gPlayableMapId |
| `src/tracks.c` | get_inside_segment_count_xz |
| `src/video.c` | fb_mode, fb_memcpy, gVideoModeIndex, gVideoLastFramebuffer, gVideoDepthBuffer |
| `src/weather.c` | rain_init, rain_lightning, rain_sound, gLightningFrequencyStep/Target, gRainOpacity/Step/Target, gRainHiddenTimer, gLightningTimer, gThunderTimer, gRainSplashDelay, gRainSplashGfx, gRainVertexFlip, gWeatherAssetTable, gThunderTransition |

`osScGetCmdQ` and `osScGetInterruptQ` (`src/50300.c`) are the N64 SDK
scheduler's public names. Every other name in `names.txt` is our own,
written from what the code does.

### Round 5, main segment

From the Diddy Kong Racing decompilation (CC0), adapted to JFG:

| DKR file | Functions |
| --- | --- |
| `src/audio.c` | music_sequence_init; initOsc's JFG form (NON_MATCHING, func_80003E40) |
| `src/audio_spatial.c` | audspat_reset, audspat_point_stop_by_index |
| `src/audiomgr.c` | amCreateAudioMgr (JFG's form); __amHandleDoneMsg (in the notes, not committed: needs a .data split) |
| `src/camera.c` | camera_reset (the layout of cam_reset), copy_viewports_to_stack |
| `src/game.c` | level_global_init; gTempAssetTable, gLevelNames, gNumberOfWorlds, gRaceTypeCountTable |
| `src/game_text.c` | render_subtitles, process_subtitles, set_current_text |
| `src/menu.c` | load_menu_text, menu_assetgroup_free, menu_assetgroup_load (and their loop for menu_assetgroup_reset) |
| `src/object_functions.c` | obj_loop_animator (JFG's form, and the template of obj_loop_texture_animator) |
| `src/object_models.c` | allocate_object_model_pools, free_3d_model, free_model_data |
| `src/objects.c` | try_free_object_header, gParticlePtrList_flush, render_misc_model, render_object_parts, get_first_active_object, init_object_shading, obj_tex_animate, load_object_header; obj_init_emitter's, track_spawn_objects' and obj_dist_racer's JFG forms (NON_MATCHING) |
| `src/particles.c` | obj_enable_emitter, obj_spawn_particle |
| `src/textures_sprites.c` | tex_init_textures |
| `src/thread3_main.c` | init_game |
| `src/transition.c` | transition_begin, transition_update, transition_render |
| `src/tracks.c` | track_tex_anim, initialise_player_viewport_vars, free_track, update_fog, apply_fog, compute_scene_camera_transform_matrix, shadow_render |
| `src/weather.c` | rain_set, rain_update |
| `include/f3ddkr.h`, `include/structs.h` | gDkrDmaDisplayList, gSPPolygon (local copies); Vertex, Triangle, TriangleBatchInfo, TextureHeader, ObjHeaderParticleEntry |

The scheduler in `src/50300.c` (Rare's variant of the SDK's) is written from
the assembly with our own structures. Its function names are ultralib's
(`osCreateScheduler`, `osScAddClient`, `osScRemoveClient`, `__scMain`,
`__scHandleRetrace`, `__scHandleRSP`, `__scHandleRDP`, `__scTaskReady`,
`__scTaskComplete`, `__scAppendList`, `__scExec`, `__scYield`,
`__scSchedule`); `__scHandleAudioReady`, `rcp_hang_bisect` and
`rcp_hang_find` are our own.

### Round 6, main segment

From the Diddy Kong Racing decompilation (CC0), adapted to JFG:

| DKR file | Functions |
| --- | --- |
| `src/tracks.c` | skydome_render, block_visible, func_8002A31C (as view_planes_update) |
| `src/lights.c` | light_add_from_level_object_entry |
| `src/camera.c` | render_ortho_triangle_image; render_sprite_billboard (structure only) |

### Round 7, main segment

From the Diddy Kong Racing decompilation (CC0), adapted to JFG:

| DKR file | Functions |
| --- | --- |
| `src/weather.c` | snow_render (with `include/f3ddkr.h`'s gSPPolygon and the Vertex layout of `include/structs.h`); snow_vertices (structure only, NON_MATCHING attempts not committed) |
| `src/particles.c` | obj_trigger_emitter |
| `src/camera.c` | func_80067D3C (as func_80040AA4) |
| `src/lights.c` | light_update (structure only) |
| `src/tracks.c` | render_level_segment (func_80014EBC, NON_MATCHING body) |
| `src/textures_sprites.c` | sprite_init_frame (func_80056D24, NON_MATCHING body) |
| `src/tracks.c` | init_track, generate_track, func_8002C954, func_8002C71C and LOCAL_OFFSET_TO_RAM_ADDRESS, in module 24 (`src/modules/m024.c`, JFG's forms) |

### Round 9 (perm9), main segment

From the Diddy Kong Racing decompilation (CC0), adapted to JFG:

| DKR file | Functions |
| --- | --- |
| `src/tracks.c` | render_level_segment (func_80014EBC) now matches; its round-7 NON_MATCHING body finished with the permuter |

## Tools and libraries

| What | Licence | How it is used |
| --- | --- | --- |
| [decompals/ultralib](https://github.com/decompals/ultralib) (libultra) | no licence file | git submodule `lib/ultralib`; `src/ultra/*` and the flash library in `src/846F0.c` (five functions) only `#include` its sources, nothing is copied; the other flash functions there, an earlier form, are written from the assembly |
| [decompals/ido-static-recomp](https://github.com/decompals/ido-static-recomp) (IDO 5.3) | no licence file | release binaries fetched by `make setup` into `tools/ido5.3/` (git-ignored) |
| [asm-processor](https://github.com/simonlindholm/asm-processor) | Unlicense | git submodule `tools/asm-processor`; `include/prelude.inc` is based on its prelude |
| [asm-differ](https://github.com/simonlindholm/asm-differ) | Unlicense | git submodule `tools/asm-differ` |
| [decomp-permuter](https://github.com/simonlindholm/decomp-permuter) | MIT | git submodule `tools/permuter` (a matching tool; tools/match/perm.py sets up its workspaces) |
| [splat](https://github.com/ethteck/splat), [spimdisasm](https://github.com/Decompollaborate/spimdisasm), [rabbitizer](https://github.com/Decompollaborate/rabbitizer) | MIT | installed from PyPI by `make setup` |
| [m2c](https://github.com/matt-kempster/m2c) | GPL-3.0 | cloned by `make setup` into `tools/m2c/` (git-ignored) and only run as a tool; none of its code is in this repository |

Not used: the unlicensed Jet Force Gemini projects by Ryan-Myers and Drahsid,
and GoldenEye's libultra sources. Nothing in this repository comes from them.

### Round 10 (perm10), main segment

From the Diddy Kong Racing decompilation (CC0), adapted to JFG:

| DKR file | Functions |
| --- | --- |
| `src/tracks.c` | get_level_segment_waves, rewritten as JFG's func_8001B078 (the planes under a point in every segment there, sorted by height; DKR's pointer loop and sort kept); with it track_nearest_triangle (round 9) drops its NON_MATCHING |
| `src/screen_asset.c` | screenimage_load (screenLoad): its earlier NON_MATCHING body now matches (one word for the header and the compressed data, colour tag 0xFFFF) |

### Round 11 (perm11), main segment

From the Diddy Kong Racing decompilation (CC0), adapted to JFG:

| DKR file | Functions |
| --- | --- |
| `src/objects.c` | track_spawn_objects: its JFG form (NON_MATCHING until now) matches (DKR's locals without the settings pad, the map pointer re-read after the store, `index == 1` for the list test) |

### Round 13 (main13), main segment

From the Diddy Kong Racing decompilation (CC0), adapted to JFG:

| DKR file | Functions |
| --- | --- |
| `src/tracks.c` | func_8002DE30, as JFG's obj_shading_from_ground at 0x8001FFE0 (object shading from the triangle under it; 16-bit cell masks, the rate a variable; DKR's `if (blockId) {}` fakematch kept) |
| `src/objects.c` | sort_objects_by_dist (JFG 0x8000E278: pointer walk, JFG's loop counts); obj_spawn_attachment (JFG 0x8000774C: built in a scratch buffer, then copied) (helper ma13h1) |
| `src/rcp_dkr.c` | bgdraw_texture (JFG 0x8004E4E8: the size from `fb_size(&w, &h)`, unsigned coordinates) (helper ma13h3) |
| `src/tracks.c` | render_scene, as the NON_MATCHING body of JFG's func_800121C0 (JFG level header fields) (helper ma13h3) |

### Round 14 (mods14), modules

From the Diddy Kong Racing decompilation (CC0), adapted to JFG:

| DKR file | Functions |
| --- | --- |
| `src/tracks.c` | track_init_collision, as the NON_MATCHING body of JFG's func_m024_00CE4 (5.0f for 10.0f, the edge flags, the 0x880 batch skip, a third-vertex test, the plane count returned) (helper m14h2) |
| `include/structs.h` | CollisionFacetPlanes (m024's segment triangle planes) (helper m14h2) |

### Round 14 (main14), main segment

From the Diddy Kong Racing decompilation (CC0), adapted to JFG:

| DKR file | Functions |
| --- | --- |
| `src/fade_transition.c` | transition_init_shape |
| `src/particles.c` | setup_particle_position, particle_update; setup_particle_velocity (NON_MATCHING) |
| `src/weather.c` | weather_reset (NON_MATCHING, as func_8005AD90); snow_vertices (NON_MATCHING) |
| `src/objects.c` | obj_update |
| `src/object_functions.c` | obj_loop_exit (helper ma14h1) |
| `src/tracks.c` | trackbg_render_flashy; func_8002FF6C (NON_MATCHING) (helper ma14h1) |
| `src/thread3_main.c` | main_game_loop, structure only (helper ma14h1) |
| `src/textures_sprites.c` | material_set (NON_MATCHING) (helper ma14h1) |
| `src/audiomgr.c` | __amHandleDoneMsg; __amHandleFrameMsg (NON_MATCHING, JFG's three output buffers and high quality mode) |
| `src/game.c` | level_load, structure only (NON_MATCHING) |

### Round 15 (main15), main segment

From the Diddy Kong Racing decompilation (CC0), adapted to JFG:

| DKR file | Functions |
| --- | --- |
| `src/audio_spatial.c` | audspat_update_all (JFG: points only, the pan from the world position, grouped points); the "OUT OF AUDIO POINTS" report in audspat_point_create |
| `src/audiomgr.c` | the three stubbed reports of __amHandleDoneMsg, __amDMA and __clearAudioDMA |
| `src/font.c` | render_text_string (JP version, JFG variant); fontCreateDisplayList (NON_MATCHING) (helper ma15h1) |
| `src/rcp_dkr.c` | texrect_draw_scaled, as the NON_MATCHING body of JFG's func_8004F02C |
| `src/tracks.c` | check_if_in_draw_range, as the NON_MATCHING body of JFG's func_80015ED8 |
| `src/main.c` | the stack check of the idle thread, with its stubbed report (boot.c) |
| `src/camera.c` | the stubbed reports of cam_set_layout, set_active_camera, set_ortho_matrix_height, render_ortho_triangle_image, mtx_cam_push |
| `src/textures_sprites.c` | the textures file's debug strings (560B0.c, split from 55130) |

### Round 16 (main16), main segment

From the Diddy Kong Racing decompilation (CC0), adapted to JFG:

| DKR file | Functions |
| --- | --- |
| `src/printf.c` | vsprintf (JFG: floats read by pointer, f32 arithmetic) |
| `src/objects.c` | func_80014090, as JFG's object_uv_offsets_update (0x8000DF00); object_do_player_tumble now matches in DKR's statement order |
| `src/save_data.c` | init_controller_paks (0x8004C2BC) |
| `src/memory.c` | the memory pools' .bss, now defined in C |

### Round 17 (main17), main segment

From the Diddy Kong Racing decompilation (CC0), adapted to JFG:

| DKR file | Functions |
| --- | --- |
| `src/object_models.c` | object_model_init (JFG-extended: asset instances, model-id remaps, display lists per id range); model_instance_init (JFG-extended: matrices, vertex buffers, point and slot tables) (helper ma17h1) |
| `src/tracks.c` | func_8002E904 and func_8002F440, as the NON_MATCHING bodies of JFG's func_8001E8B8 and func_8001FA24 (helper ma17h1) |

### Round 20 (modsA20), modules m003-m041

From the Diddy Kong Racing decompilation (CC0), adapted to JFG:

| JFG file | DKR file | Used |
| --- | --- | --- |
| `src/modules/m012.c` | `include/f3ddkr.h` | gDkrDmaDisplayList (local copy, length as `<< 3`) (helper mA20h1) |

### Round 20 (main20), main segment

From the Diddy Kong Racing decompilation (CC0), adapted to JFG:

| DKR file | Functions |
| --- | --- |
| `src/audiosfx.c` | the sound player as JFG's `src/audiosfx.c` (0x84E00): sndp_init_player, sndp_voice_handler, sndp_remove_events, sndp_get_state_counts, sndp_allocate, sndp_deallocate, sndp_play_with_priority (as JFG's sndp_play), sndp_end, sndp_apply_pitch_slide, sndp_set_priority, sndp_get_state, sndp_stop, sndp_stop_with_flags, sndp_stop_all, sndp_stop_all_retrigger, sndp_stop_all_looped, sndp_set_param, sndp_get_group_volume, sndp_set_group_volume, sndp_set_global_volume, sndp_get_global_volume, sndp_set_active_sound_limit (JFG: built -g -mips2, debug prints kept, larger sound states) |
| `src/menu.c` | menu_element_render, as the NON_MATCHING body of JFG's func_8005A0D0 (helper mn20h1) |

Written from the game's assembly, not taken from any source: alLink and alUnlink
(`src/ultra/audio/sl.c`) and __alCSeqNextDelta (`src/naudio_8F500.c`) come out the
same as ultralib's versions, because they are the same compiled library code. JFG's
objects hold only part of the matching ultralib files, so they cannot be included
from ultralib the way the other libultra objects are.

### Round 21 (libother21), main-segment libraries

No DKR code. The controller pak, rumble pak, EPI and PI DMA objects are
ultralib's 2.0J sources built `-O2 -g3`, included from `lib/ultralib` by the
`src/ultra/io` wrappers (Makefile `UL_O2G3_OBJS`); nothing is copied.

Written from the game's assembly, not taken from any source: osMotorStop and
osMotorStart (`src/motor_stop_start.c`). The game's 2.0J rumble pak object has
only `__osMotorAccess` (2.0J makes the two names macros), so the game carries
the two calls as functions of its own (ultralib's `motor.c` has the two only
in their older, different 2.0I form).

Also written from the assembly: libultra objects in an earlier form than any
ultralib builds, so they cannot be included. Functions in them that come out
the same as ultralib's do so because they are the same library code:

| JFG file | Functions | Earlier form | Same as ultralib's |
| --- | --- | --- | --- |
| `src/979F0.c` | osAiSetNextBuffer | hardware bug test `(end & 0x3FFF) == 0x2000` | none |
| `src/99B40.c` | _VirtualToPhysicalTask, osSpTaskLoad, osSpTaskStartGo | no loadable-microcode check for a yielded task | _VirtualToPhysicalTask, osSpTaskStartGo |
| `src/98550.c` | osContInit, __osContGetInitData, __osPackRequestData | start-up wait computed from osClockRate at run time; PIF RAM cleared as 16 words | __osContGetInitData |
| `src/98910.c` | osContStartReadData, osContGetReadData, __osPackReadData | PIF RAM cleared as 16 words; osContStartReadData fills it with 0xFF before the read | osContGetReadData |
