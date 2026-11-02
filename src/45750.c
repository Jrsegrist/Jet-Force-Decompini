#include "common.h"
#include "joypad.h"

/* From the Diddy Kong Racing decompilation (CC0 1.0); see CREDITS.md. */
extern OSMesgQueue gNMIMesgQueue;
extern s32 gGameMode;
extern s32 gNMIMesgBuf;          // Official Name: resetPressed;
extern s32 sControllerStatus;
extern s32 gSaveDataFlags; // Official Name: load_save_flags

/* JFG's save flag bits (DKR has them elsewhere) */
#define SAVE_DATA_FLAG_WRITE_SAVE_FILE_NUMBER_BITS 0x38
#define SAVE_DATA_FLAG_WRITE_SAVE_DATA 0x40000000
#define GAMEMODE_INGAME 0

extern s32 D_800A331C;
extern s32 D_800A3320;
extern s16 D_800A3260;
extern s16 gPlayableMapId;
extern s32 sBootDelayTimer;
s32 leveltable_world(s32 level);
s32 drm_check_rdram_a(void);
s32 drm_check_rdram_b(void);
void init_game(void);
void func_8004665C(s32 arg0, s32 level, s32 arg2, s32 arg3, s32 arg4, s32 arg5);
void check_reset_and_halt(void);
void func_80044FAC(void);
void thread3_verify_stack(void);
s32 input_update(s32 saveDataFlags, s32 updateRate);

/**
 * Main looping function for the main thread.
 * JFG: hangs on a PAL console, runs two boot checks, and starts at a level
 * instead of checking for a reset in the loop.
 * Official Name: mainThread
 */
void thread3_main(UNUSED void *unused) {
    u32 *mem;
    if (osTvType == OS_TV_PAL) {
        // Clear memory downwards from the end of the first 4 MB until it crashes.
        mem = (u32 *) 0x80400000;
        while (1) {
            *--mem = 0;
        }
    }
    D_800A331C = drm_check_rdram_a();
    D_800A3320 = drm_check_rdram_b();
    init_game();
    gSaveDataFlags = input_update(gSaveDataFlags, 0);
    sBootDelayTimer = 0;
    gGameMode = 6;
    func_8004665C(0, D_800A3260, 0, 0, 1, 0);
    while (1) {
        check_reset_and_halt();
        func_80044FAC();
        thread3_verify_stack();
    }
}

extern s32 D_800A3290;
void audioStopThread(void);
void fb_clear_and_unblank(void);
void rumble_stop_all(s32 arg0);
void func_8004B56C(s32 updateRate);
s32 is_reset_pressed(void);
void __osSpSetStatus(u32 data);

/* JFG: on reset, stop the audio, wait a little, stop the RSP/RDP and the rumble paks, then hang (0x80044C20). */
void check_reset_and_halt(void) {
    if (is_reset_pressed() != 0) {
        audioStopThread();
        if (D_800A3290 != 0) {
            osSetTime(0);
            while (osGetTime() < 0x22ADD7) {
            }
        }
        __osSpSetStatus(0xAAAA82);
        osDpSetStatus(0x1D6);
        fb_clear_and_unblank();
        rumble_stop_all(1);
        func_8004B56C(2);
        func_8004B56C(2);
        func_8004B56C(2);
        while (1) {
        }
    }
}

/* A MIPS instruction word, R-type and I-type views */
typedef union MipsInstr {
    struct {
        u32 op : 6;
        u32 rs : 5;
        u32 rt : 5;
        u32 rd : 5;
        u32 sa : 5;
        u32 funct : 6;
    } r;
    struct {
        u32 op : 6;
        u32 rs : 5;
        u32 rt : 5;
        u32 imm : 16;
    } i;
} MipsInstr;

extern MipsInstr *D_800A3360[5];

/* JFG: in each of five code areas, rewrite the first `addiu rt, rs, 0x666` of 64 words into `or rt, ra, zero`
 * and flush the caches for it. */
void code_patch_addiu_666(void) {
    s32 i = 5;
    MipsInstr *instr;
    s32 j;

    while (i--) {
        instr = D_800A3360[i];
        j = 0x40;
        while (j--) {
            if (instr->i.op == 9 && instr->i.imm == 0x666) {
                instr->r.op = 0;
                instr->r.rs = 31;
                instr->r.rd = instr->r.rt;
                instr->r.rt = 0;
                instr->r.sa = 0;
                instr->r.funct = 0x25;
                osWritebackDCache(instr, 4);
                osInvalICache(instr, 4);
                break;
            }
            instr++;
        }
    }
}

typedef struct OSSched OSSched;
void osCreateScheduler(OSSched *sc, void *stack, OSPri priority, u8 mode, u8 numFields);
extern OSSched D_800FB440;    // the scheduler
extern u8 D_800FD748[];       // the scheduler thread's stack top
extern void *D_800A3324;
extern s8 D_800FD7BC;
extern s32 D_800A3530;        // set while the game is booting (see asset_loading.c)
void mempool_init_main(void);
void gzip_init(void);
void video_init(OSSched *sc);
void pi_init(void);
void gfxtask_init(OSSched *sc);
void func_800539E0(void);
void func_8005342C(s32 module);
void *mempool_alloc_safe(s32 size, u32 colourTag);
void func_80054530();

/**
 * JFG: set up the scheduler, memory, video, loading and graphics, checking for a reset between the steps.
 * DKR: init_game
 */
void init_game(void) {
    s32 viMode;

    if (osTvType == OS_TV_PAL) {
        viMode = OS_VI_PAL_LPN1;
    } else if (osTvType == OS_TV_NTSC) {
        viMode = OS_VI_NTSC_LPN1;
    } else if (osTvType == OS_TV_MPAL) {
        viMode = OS_VI_MPAL_LPN1;
    }
    osCreateScheduler(&D_800FB440, D_800FD748, 13, viMode, 1);
    code_patch_addiu_666();
    mempool_init_main();
    gzip_init();
    D_800A3324 = mempool_alloc_safe(0x40, 0x7F7F7FFF);
    D_800FD7BC = 0;
    D_800A3530 = 1;
    video_init(&D_800FB440);
    check_reset_and_halt();
    pi_init();
    check_reset_and_halt();
    gfxtask_init(&D_800FB440);
    check_reset_and_halt();
    func_800539E0();
    check_reset_and_halt();
    D_800A3290 = 1;
    func_80054530();
    check_reset_and_halt();
    func_8005342C(0x24);
    D_800A3530 = 0;
}

extern s32 D_800A328C;
extern s16 D_800FD7A4;
extern s16 D_800A3280;
extern u8 gCurrDisplayList;
void func_80054530();

void func_80044F24(s32 step) {
    s32 alpha;

    if (step < D_800A328C) {
        D_800A328C -= step;
        alpha = D_800A328C * 4;
        if (alpha >= 0x100) {
            alpha = 0xFF;
        }
        func_80054530(&gCurrDisplayList, D_800FD7A4, leveltable_world(gPlayableMapId), D_800A3280, alpha);
    } else {
        D_800A328C = 0;
    }
}

INCLUDE_ASM("asm/nonmatchings/45750", func_80044FAC);

/* JFG: a screen rectangle whose visibility is tested against the depth buffer */
typedef struct ScreenRect {
    /* 0x00 */ s16 x;
    /* 0x02 */ s16 y;
    /* 0x04 */ u8 pad4;
    /* 0x05 */ s8 width;
    /* 0x06 */ s8 height;
    /* 0x07 */ s8 visible;
} ScreenRect;

extern s32 D_800A3378;
extern ScreenRect D_800FD750[8];
extern u16 *gVideoDepthBuffer;
void fb_size(s32 *width, s32 *height);

/* JFG: queue the square of half-size `size` around (x, y), clipped to the screen; returns its index or -1. */
s32 screen_rect_queue(s32 x, s32 y, s32 size) {
    s32 index;
    s32 x1;
    s32 y1;
    s32 x2;
    s32 y2;
    s32 width;
    s32 height;

    fb_size(&width, &height);
    index = -1;
    if (D_800A3378 < 8) {
        x2 = x + size;
        y2 = y + size;
        if (x2 >= 0 && y2 >= 0) {
            x1 = x - size;
            y1 = y - size;
            if (x1 < width && y1 < height) {
                if (x1 < 0) {
                    x1 = 0;
                }
                if (y1 < 0) {
                    y1 = 0;
                }
                if (x2 >= width) {
                    x2 = width - 1;
                }
                if (y2 >= height) {
                    y2 = height - 1;
                }
                index = D_800A3378;
                D_800A3378++;
                D_800FD750[index].x = x1;
                D_800FD750[index].y = y1;
                D_800FD750[index].width = x2 - x1 + 1;
                D_800FD750[index].height = y2 - y1 + 1;
            }
        }
    }
    return index;
}

/* JFG: for each queued rectangle, visible = no depth-buffer pixel in it is at the far value (0xFFFC). */
void screen_rects_test_depth(void) {
    ScreenRect *rect;
    s32 i;
    s32 visible;
    u16 *pixels;
    s32 width;
    s32 height;
    s32 h;
    s32 w;

    fb_size(&width, &height);
    rect = D_800FD750;
    i = 8;
    while (i--) {
        visible = 1;
        if (D_800A3378 > 0) {
            D_800A3378--;
            pixels = gVideoDepthBuffer + (rect->y * width + rect->x);
            h = rect->height;
            while (h--) {
                w = rect->width;
                while (w--) {
                    if ((pixels[w] & 0xFFFC) == 0xFFFC) {
                        visible = 0;
                        goto done;
                    }
                }
                pixels += width;
            }
        }
    done:
        rect->visible = visible;
        rect++;
    }
}

extern s8 D_800FD757[];

s32 func_80045A14(s32 index) {
    if (index < 0 || index >= 8) {
        return 1;
    }
    return D_800FD757[index * 8];
}

extern s32 D_801008B4;
s32 check_viewport_background_flag(s32 viewPortIndex);
void copy_viewport_frame_size_to_coords(s32 viewPortIndex, s32 *x1, s32 *y1, s32 *x2, s32 *y2);
s32 check_fadeout_transition(void);
s32 runlink_module_loaded(s32 module);
f32 func_8005BFC4(void);
void func_8006F8E4(void);
void func_8006E188(void);
void func_8006D0D8(void);
/* The module trap called with float arguments in a3 and on the stack, and for a result; needs
   `func_80054530_iiiff = 0x80054530;` and `func_80054530_ret = 0x80054530;` in undefined_syms.us.txt. */
void func_80054530_iiiff(u16 *fb, s32 width, s32 height, f32 arg3, f32 arg4);
s32 func_80054530_ret(void);

/* JFG: after the frame: module 5's pass over the framebuffer rows (from the second frame on, while the height
 * keeps), the other module hooks, then one of two finish calls. */
void frame_post_process(u16 *fb, UNUSED s32 arg1) {
    s32 width;
    s32 height;
    s32 x1;
    s32 y1;
    s32 x2;
    s32 y2;

    fb_size(&width, &height);
    if (check_viewport_background_flag(0)) {
        copy_viewport_frame_size_to_coords(0, &x1, &y1, &x2, &y2);
        fb = &fb[y1 * width];
        height = y2 - y1 + 1;
    }
    if (!check_fadeout_transition() && runlink_module_loaded(5) && func_80054530_ret()) {
        if (D_801008B4 > 0 && (u32) height >= D_801008B4) {
            func_80054530_iiiff(fb, width, D_801008B4, 256.0f, func_8005BFC4());
        }
        D_801008B4 = height;
    }
    func_8006F8E4();
    if (runlink_module_loaded(2) && !check_fadeout_transition()) {
        func_80054530();
    }
    if (!check_fadeout_transition()) {
        func_8006E188();
    } else {
        func_8006D0D8();
    }
}

extern s16 D_800FD794;
extern s16 D_800FD796;
extern s16 D_800FD798;
extern s16 D_800FD79A;
extern s16 D_800FD79C;
extern s16 D_800FD79E;
extern s16 D_800FD7A0;
extern s16 D_800FD7A2;
extern s16 D_800A3230;
extern s16 D_800A3234;
extern s16 D_800A3238;

void func_80045B98(s32 a0, s32 a1, s32 a2, s32 a3, s32 a4, s32 a5, s32 a6, s32 a7, s32 time) {
    D_800FD794 = a0;
    D_800FD796 = a1;
    D_800FD798 = a2;
    D_800FD79A = a3;
    D_800FD79C = a4;
    D_800FD79E = a5;
    D_800FD7A0 = a6;
    D_800FD7A2 = a7;
    if (time < 0) {
        D_800A3234 = -time;
        D_800A3238 = 1;
    } else {
        D_800A3234 = time;
        D_800A3238 = 0;
    }
    D_800A3230 = D_800A3234;
}

extern s16 D_800A3220;
extern s16 D_800A3224;
extern s16 D_800A3228;
extern s16 D_800A322C;
extern u8 D_800A321C;

/* JFG: step the transition of four values from D_800FD79C.. towards D_800FD794.. (see func_80045B98);
 * D_800A321C: still running, or finished going forwards. */
void transition_values_update(s32 updateRate) {
    if (D_800A3230) {
        D_800A3220 = D_800FD79C;
        D_800A3224 = D_800FD79E;
        D_800A3228 = D_800FD7A0;
        D_800A322C = D_800FD7A2;
        D_800A3230 -= updateRate;
        if (D_800A3230 > 0) {
            updateRate = (D_800A3230 << 16) / D_800A3234; // the fraction left, 16.16
            D_800A3220 += ((D_800FD794 - D_800FD79C) * updateRate) >> 16;
            D_800A3224 += ((D_800FD796 - D_800FD79E) * updateRate) >> 16;
            D_800A3228 += ((D_800FD798 - D_800FD7A0) * updateRate) >> 16;
            D_800A322C += ((D_800FD79A - D_800FD7A2) * updateRate) >> 16;
        } else {
            D_800A3230 = 0;
        }
    }
    D_800A321C = D_800A3230 || !D_800A3238;
}

s16 func_80045DC0(void) {
    return D_800A3230;
}

extern s16 D_800A3220;
extern s16 D_800A3224;
extern s16 D_800A3228;
extern s16 D_800A322C;

void func_80045DD0(s32 *x1, s32 *y1, s32 *x2, s32 *y2) {
    *x1 = D_800A3220;
    *y1 = D_800A3224;
    *x2 = D_800A3228;
    *y2 = D_800A322C;
}

extern s8 D_800A4FB0;
s32 get_language(void);
void set_language(s32 language);
s32 options_screen_mode_get(void);
void func_8005A834(s32 arg0);
void func_8005A8C4(void);
s8 options_screen_offset_get(void);
void options_screen_offset_set(s32 adjust);
u8 func_8005A98C(void);
void func_8005A99C(s32 mode);
u16 func_8005A9E4(void);
void func_8005A9F4(s32 volume);
u16 options_music_volume_get(void);
void options_music_volume_set(s32 volume);
void func_80047608(s32 arg0, s32 level, s32 arg2);

#ifdef NON_MATCHING
/* NON_MATCHING: in the START loop the target puts the andi before i++ and ORs as (start, held) */
/* JFG: the boot screen; START on any controller sets D_800A4FB0, after 10 frames the settings are applied. */
void func_80045E04(UNUSED s32 updateRate) {
    s32 i;
    s32 start;

    for (i = 0, start = 0; i < 4; i++) {
        start = start | (input_held(i) & START_BUTTON);
    }
    if (start) {
        D_800A4FB0 = 1;
    }
    if (++sBootDelayTimer >= 10) {
        set_language(get_language());
        func_8005A834(options_screen_mode_get());
        options_screen_offset_set(options_screen_offset_get());
        func_8005A99C(func_8005A98C());
        func_8005A9F4(func_8005A9E4());
        options_music_volume_set(options_music_volume_get());
        func_8005A8C4();
        func_80047608(2, 0, 0);
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/45750", func_80045E04);
#endif

extern s32 D_800F6F54;
extern s8 gIsPaused;
extern u8 D_800A4FC8;
extern u8 D_800A3208;
extern u8 D_800A4FC4;
extern u8 D_800A4FD0;
extern u8 D_800A4FB4;
extern s32 D_800A3328;
extern s32 D_800A38FC;
extern s32 Time;
extern s16 D_800A32BC;
extern s32 D_800FD7C4;
extern s32 D_800A32B8;
extern Gfx *gfx;
extern u8 vtx[];
extern u8 gGameCurrMatrix[];
extern u8 gGameCurrTriList[];
extern char D_800AC574[];
extern char D_800AC580[];
extern char D_800AC58C[];
extern char D_800AC598[];
extern s8 D_800A3294;
struct LevelHeader *level_header(void);
void debug_trace_add(Gfx *dList, char *file, s32 line);
void objects_set_attachment_byte(s32 updateRate);
s32 func_80011C2C(void);
void func_80077614(s32 updateRate);
void func_80008FF0(s32 updateRate);
void func_80063358(s32 updateRate);
void *cam_get_active_camera(void);
void func_80079890(s32 arg0, void *camera);
void func_80012150(s32 updateRate);
void rumble_stop_all(s32 arg0);
void objects_update_special(s32 updateRate);
void func_80008FE8(void);
void gParticlePtrList_flush(void);
void func_800121C0(Gfx **dList, void *mtx, void *vtx, void *arg3, s32 updateRate);
void process_subtitles(s32 updateRate);
void splitscreen_dividers_draw(Gfx **dList);
/* The module trap called with three floats; needs `func_80054530_fff = 0x80054530;` in undefined_syms.us.txt. */
void func_80054530_fff(f32 x, f32 y, f32 z, s32 updateRate);

#ifdef NON_MATCHING
/* NON_MATCHING: the target keeps the held-buttons OR in a stack slot (only $s0/$s1 used, frame 0x48) and tests
   i < numPlayers with slt; mine gives it $s1 and $s2 */
/* JFG: one frame of the game: the module hooks, the players' buttons (START pauses), the world update unless
 * paused, then the drawing passes. */
void func_80045F00(s32 updateRate) {
    s32 i;
    s32 held;
    u32 pressed;
    s32 numPlayers;
    s32 startPlayer;
    s32 v;
    u8 *header;

    header = (u8 *) level_header();
    D_800F6F54 = 1;
    if (runlink_module_loaded(5) && func_80054530_ret() && !gIsPaused) {
        func_80054530_fff(0.0f, 0.0f, -2.0f, updateRate);
    }
    held = 0;
    pressed = 0;
    startPlayer = -1;
    if (D_800A4FC8 && D_800A3208) {
        numPlayers = 2;
    } else if (D_800A4FC4) {
        numPlayers = D_800A4FD0;
    } else {
        numPlayers = 1;
    }
    for (i = 0; i < numPlayers; i++) {
        held |= input_held(i);
        pressed |= input_pressed(i);
        if (pressed & START_BUTTON) {
            startPlayer = i;
        }
    }
    debug_trace_add(gfx, D_800AC574, 0x62E);
    objects_set_attachment_byte(updateRate);
    v = func_80011C2C();
    D_800A38FC = 1;
    if (gIsPaused == 0 && D_800A3328 == 0 && v == 0) {
        Time += updateRate;
        func_80077614(updateRate);
        if (runlink_module_loaded(3)) {
            func_80054530(updateRate);
        }
        func_80008FF0(updateRate);
        func_80063358(updateRate);
        func_80079890(0, cam_get_active_camera());
        func_80012150(updateRate);
        if ((s8) header[0x6C] != 1 && (s8) header[0x6C] != 2 && (pressed & START_BUTTON) && D_800A32BC == 0 &&
            D_800A3294 == 0 && D_800FD7C4 == 0) {
            rumble_stop_all(1);
            if (!D_800A4FC4 && !D_800A4FB4) {
                if (D_800A3208) {
                    func_80054530(startPlayer);
                } else {
                    func_80054530(startPlayer);
                }
            } else {
                func_80054530();
            }
            D_800A38FC = 0;
        }
    } else if (gIsPaused == 0 && D_800A3328 == 0 && v != 0) {
        objects_update_special(updateRate);
        D_800A38FC = 0;
    } else {
        func_80008FE8();
        D_800A38FC = 0;
    }
    if (D_800A32BC > 0) {
        D_800A32BC -= updateRate;
        if (D_800A32BC < 0) {
            D_800A32BC = 0;
        }
    }
    gParticlePtrList_flush();
    debug_trace_add(gfx, D_800AC580, 0x683);
    func_800121C0(&gfx, gGameCurrMatrix, vtx, gGameCurrTriList, updateRate);
    debug_trace_add(gfx, D_800AC58C, 0x6C0);
    process_subtitles(updateRate);
    if (runlink_module_loaded(7)) {
        func_80054530(&gfx, updateRate);
    }
    if (D_800A32B8) {
        func_80054530(&gfx, gGameCurrMatrix, vtx, D_800A32B8);
    }
    splitscreen_dividers_draw(&gfx);
    debug_trace_add(gfx, D_800AC598, 0x6DC);
}
#else
INCLUDE_ASM("asm/nonmatchings/45750", func_80045F00);
#endif

void func_80046334(s32 *level, s32 *a, s32 *b) {
    s32 value = *level;
    if (value == 0xF5 || value == 0xFA || value == 0x11D || value == 0x16B || value == 0x196) {
        *level = 0xF5;
        *a = 0;
        *b = 0;
    }
}

/* JFG: one character's data in the game state (0x76 bytes) */
typedef struct CharacterData {
    /* 0x00 */ s16 unk0;
    /* 0x02 */ s16 unk2;
    /* 0x04 */ u16 unk4_hi : 11;
    /* 0x04 */ u16 unk4_lo : 5;
    /* 0x06 */ s16 unk6;
    /* 0x08 */ s16 unk8;
    /* 0x0A */ u16 weapons; // one bit per weapon
    /* 0x0C */ s16 unkC[4];
    /* 0x14 */ s16 unk14[16]; // per weapon
    /* 0x34 */ s16 unk34[16]; // per weapon
    /* 0x54 */ u8 unk54[16];  // per weapon
    /* 0x64 */ s16 unk64;
    /* 0x66 */ u8 items[8]; // one bit per item, highest bit first
    /* 0x6E */ u8 pad6E;
    /* 0x6F */ u8 unk6F;
    /* 0x70 */ u8 unk70;
    /* 0x71 */ s8 unk71;
    /* 0x72 */ u8 pad72[2];
    /* 0x74 */ u16 unk74_hi : 11;
    /* 0x74 */ u16 unk74_lo : 5;
} CharacterData;

typedef struct GameStatsEntry {
    /* 0x0 */ u8 unk0;
    /* 0x1 */ u8 unk1;
    /* 0x2 */ s16 unk2;
    /* 0x4 */ s32 unk4;
} GameStatsEntry;

typedef struct GameStats {
    /* 0x000 */ u8 unk0;
    /* 0x001 */ u8 unk1;
    /* 0x002 */ s8 unk2;
    /* 0x003 */ u8 pad3;
    /* 0x004 */ u8 unk4;
    /* 0x005 */ u8 unk5;
    /* 0x006 */ u8 unk6;
    /* 0x007 */ u8 pad7;
    /* 0x008 */ u8 unk8[32];
    /* 0x028 */ u8 unk28[8];
    /* 0x030 */ u8 pad30[0x5C - 0x30]; // the game flags start here (see settings_flag_get)
    /* 0x05C */ u16 unk5C[64];
    /* 0x0DC */ u16 unkDC[64];
    /* 0x15C */ CharacterData chars[4];
    /* 0x334 */ GameStatsEntry unk334[40];
    /* 0x474 */ u32 shots[16];
    /* 0x4B4 */ u32 hits[16];
    /* 0x4F4 */ u16 kills[16];
    /* 0x514 */ s32 unk514;
    /* 0x518 */ s32 unk518;
    /* 0x51C */ u8 pad51C[0x530 - 0x51C];
    /* 0x530 */ s16 unk530;
    /* 0x532 */ u8 pad532[2];
    /* 0x534 */ u8 unk534[4];
} GameStats;

extern s8 D_800A3294;
extern s16 gPlayableMapId;
extern s16 D_800A3240;
extern s16 D_800A3244;
extern s16 D_800A3248;
extern s16 D_800A324C;
extern s16 D_800A3250;
extern s16 D_800A3254;
extern s16 D_800A3258;
extern s16 D_800A3264;
extern s16 D_800A3268;
extern s16 D_800A3270;
extern s16 D_800A3274;
extern s16 D_800A3278;
extern s16 D_800A327C;
extern s8 D_800A3284;
extern s8 D_800A3288;
extern GameStats *gSettingsPtr;
extern s8 D_800A32A0;
extern u8 D_800A3304[];
s32 func_80046550(s16 level, s16 *newLevel, s16 *arg2, s16 *arg3, s16 *arg4);
void transition_begin(void *arg0);

/* JFG: once (until D_800A3294 is cleared): pick the next level: a fixed one for some levels, the current
 * D_800FD7A4 setup (counting D_800A3280 down while it lasts) or the saved one (D_800A3270..7C); then the
 * character and the module hooks. */
void level_next_select(void) {
    if (D_800A3294 == 0) {
        if (!func_80046550(gPlayableMapId, &D_800A3250, &D_800A3254, &D_800A3258, &D_800A3268)) {
            if (D_800FD7A4 == 0 || D_800FD7A4 == 0xFF || D_800A3280 > 0) {
                if (D_800FD7A4 != 0 && D_800FD7A4 != 0xFF) {
                    D_800A3280--;
                    D_800A3284 = 1;
                }
                D_800A3250 = gPlayableMapId;
                D_800A3254 = D_800A3248;
                D_800A3258 = D_800A3244;
                D_800A3268 = D_800FD7A4;
            } else {
                D_800A3250 = D_800A3270;
                D_800A3254 = D_800A3274;
                D_800A3258 = D_800A3278;
                D_800A3268 = D_800A327C;
                D_800A3288 = 1;
            }
        }
        D_800A3260 = D_800A3240;
        D_800A3264 = D_800A324C;
        if (D_800A3288) {
            if (gSettingsPtr->chars[D_800A3260 & 3].unk4_lo == 1) {
                gSettingsPtr->chars[D_800A3260 & 3].unk4_lo = 0;
            }
        }
        if (!check_fadeout_transition()) {
            D_800A32A0 = 0;
            transition_begin(D_800A3304);
        }
        D_800A3294 = 30;
    }
}

/* JFG: where some levels send the player (level, two zeros and D_800FD7A4); FALSE for the others. */
s32 func_80046550(s16 level, s16 *newLevel, s16 *arg2, s16 *arg3, s16 *arg4) {
    switch (level) {
        case 0xD2:
            *newLevel = 0xF9;
            *arg2 = 0;
            *arg3 = 0;
            *arg4 = D_800FD7A4;
            return TRUE;
        case 0xB5:
            *newLevel = 0x13;
            *arg2 = 0;
            *arg3 = 0;
            *arg4 = D_800FD7A4;
            return TRUE;
        case 0xC3:
            *newLevel = 0xC2;
            *arg2 = 0;
            *arg3 = 0;
            *arg4 = D_800FD7A4;
            return TRUE;
        case 0x182:
            *newLevel = 0x18B;
            *arg2 = 0;
            *arg3 = 0;
            *arg4 = D_800FD7A4;
            return TRUE;
        case 0xE9:
            *newLevel = 0x18C;
            *arg2 = 0;
            *arg3 = 0;
            *arg4 = D_800FD7A4;
            return TRUE;
    }
    return FALSE;
}

extern s8 D_800A329C;
extern u8 D_800A32FC[];
extern f32 D_800AC624;
s32 game_flag_get(s32 flag);
void func_8003B6BC(void);
s16 leveltable_region(void);
void set_game_paused(s32 arg0);
u8 music_current_sequence(void);
u8 leveltable_tune(s32 level);
s8 is_game_paused(void);
void music_volume_fade(f32 arg0, s32 arg1);
void rumble_stop_all(s32 arg0);
void rumble_allow_set(s32 arg0);

#ifdef NON_MATCHING
/* NON_MATCHING: temporaries only: the target stores the level through $a0 (reloaded from its home) and numbers
   the later temporaries one higher; the music compare has its operands the other way round */
/* JFG: go to a level (with a few swaps by game flag): store it as the next one, mark the module 5 fade, start the
 * transition and the level's music if it changes, and stop all rumble. */
void func_8004665C(s32 level, s32 arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5) {
    u8 seq;

    if (D_800A3294 && level == D_800A3250 && arg1 == D_800A3260 && arg3 == D_800A3264) {
        return;
    }
    if (D_800FD7C4 == 0) {
        if (level == 0x11E && game_flag_get(0x5E)) {
            level = 0x130;
        } else if (level == 0xC2 && game_flag_get(0x4D)) {
            level = 0x179;
        } else if (level == 0x14C) {
            func_8003B6BC();
        }
    }
    D_800A3250 = level;
    D_800A3254 = 0;
    D_800A3258 = arg2;
    D_800A3260 = arg1;
    D_800A3264 = arg3;
    D_800A3268 = leveltable_region();
    if ((arg5 & 1) && D_800FD7A4 != 0 && D_800FD7A4 != 0xFF) {
        if ((D_800A3268 != D_800FD7A4 && D_800A3268 != 0xFF) || arg3 == 0x11) {
            D_800A329C = 1;
            func_80054530();
            set_game_paused(1);
        }
    }
    if (D_800A3294 > 0) {
        return;
    }
    D_800A3294 = 10;
    if (!check_fadeout_transition()) {
        if (!D_800A329C) {
            if (arg4) {
                D_800A32A0 = 1;
                transition_begin(D_800A32FC);
            } else {
                D_800A32A0 = 0;
                transition_begin(D_800A3304);
            }
        }
        D_800A3294 = 30;
    }
    seq = music_current_sequence();
    if (leveltable_tune(D_800A3250) != seq && !is_game_paused()) {
        music_volume_fade(D_800AC624, 0);
    }
    rumble_stop_all(1);
    rumble_allow_set(0);
}
#else
INCLUDE_ASM("asm/nonmatchings/45750", func_8004665C);
#endif

extern s16 D_800A3254;

void func_800468A4(s32 arg0) {
    D_800A3254 = arg0;
}

extern s16 D_800A3248;

s16 func_800468B0(void) {
    return D_800A3248;
}

extern s16 D_800A325C;

void func_800468C0(s32 arg0) {
    D_800A325C = arg0;
}

extern s16 D_800A3260;

s16 func_800468CC(void) {
    return D_800A3260;
}

extern s16 D_800A3250;

s16 func_800468DC(void) {
    return D_800A3250;
}

INCLUDE_ASM("asm/nonmatchings/45750", func_800468EC);

extern u8 D_800A326C;

void func_80047570(void) {
    D_800A326C = 0x1;
}

/**
 *  Sets the current game mode.
 *  Official Name: mainSetMode?
 */
void set_game_mode(s32 changeTo) {
    gGameMode = changeTo;
}

extern s32 D_800FD7C8;
extern s8 D_800FD7BC;

/* JFG: a level and the flags needed to start it */
typedef struct LevelUnlock {
    s16 level;
    u16 flags;
} LevelUnlock;
extern LevelUnlock D_800A337C[];
extern u8 D_800FF391;

/* Starts the first level of the unlock list whose required flags are all set (0x8004758C). */
void load_first_unlocked_level(void) {
    LevelUnlock *entry;

    gGameMode = 1;
    D_800FD7C8 = 1;
    entry = D_800A337C;
    while ((D_800FF391 & entry->flags) != entry->flags) {
        entry++;
    }
    func_8004665C(entry->level, 0, 0, 3, 1, 0);
}

void func_80047608(s32 arg0, s32 level, UNUSED s32 arg2) {
    gGameMode = 1;
    D_800FD7C8 = 1;
    if (D_800FD7BC == 0) {
        D_800FD7BC = 1;
    }
    func_8004665C(level, D_800A3260, 0, arg0, 1, 0);
}

extern GameStats *gSettingsPtr;

void func_80047674(void) {
    gSettingsPtr->unk1 = gSettingsPtr->chars[gSettingsPtr->unk0 & 3].unk4_hi;
    gSettingsPtr->unk2 = leveltable_world(gSettingsPtr->unk1);
}

extern s16 D_800A3314[4];
void func_80047AE0(CharacterData *chr, s32 level);
void func_80047EFC(s32 arg0, s32 arg1);

/* JFG: clear a game's settings and statistics and reset its characters. */
void game_settings_reset(GameStats *game) {
    s32 i;
    GameStatsEntry *entry;

    game->unk0 = 0;
    for (i = 0; i < 64; i++) {
        game->unk5C[i] = 0;
        game->unkDC[i] = 0;
    }
    entry = game->unk334;
    for (i = 0; i < 40; i++) {
        entry->unk0 = 0;
        entry->unk1 = 0;
        entry->unk4 = 0;
        entry->unk2 = 0;
        entry++;
    }
    game->unk4 = 0;
    game->unk5 = 0;
    game->unk6 = 50;
    game->unk530 = 0;
    game->unk514 = 0;
    game->unk518 = 0;
    for (i = 0; i < 16; i++) {
        game->shots[i] = 0;
        game->hits[i] = 0;
        game->kills[i] = 0;
    }
    i = 0;
    while (i < 32) {
        game->unk8[i] = 0;
        i++;
    }
    i = 0;
    while (i < 8) {
        game->unk28[i] = 0;
        i++;
    }
    i = 0;
    while (i < 0x8C) {
        ((u8 *) game)[0x30 + i] = 0;
        i++;
    }
    func_80047EFC(1, 1);
    for (i = 0; i < 4; i++) {
        func_80047AE0(&game->chars[i], D_800A3314[i]);
    }
    game->chars[0].unk64 = 0;
    game->chars[1].unk64 = 0x400;
    game->chars[2].unk64 = 0;
    game->unk534[0] = 0;
    game->unk534[1] = 0;
    game->unk534[2] = 0;
    game->unk534[3] = 0;
}

/* Counts a kill with a weapon in the game statistics (0x80047868). */
void stats_kill_add(s32 weapon) {
    if (weapon >= 0 && weapon < 16 && gSettingsPtr->kills[weapon] < 0xFFFF) {
        gSettingsPtr->kills[weapon]++;
    }
}

/* Counts a shot with a weapon in the game statistics (0x800478A4). */
void stats_shot_add(s32 weapon) {
    if (weapon >= 0 && weapon < 16 && gSettingsPtr->shots[weapon] < 0xFFFFFFFF) {
        gSettingsPtr->shots[weapon]++;
    }
}

/* Counts a hit with a weapon in the game statistics (0x800478E0). */
void stats_hit_add(s32 weapon) {
    if (weapon >= 0 && weapon < 16 && gSettingsPtr->hits[weapon] < 0xFFFFFFFF) {
        gSettingsPtr->hits[weapon]++;
    }
}

extern u8 D_800FEFD0[4];
extern CharacterData D_800FEFD8[4];
void func_80047AE0(CharacterData *chr, s32 arg1);

void func_8004791C(void) {
    s32 i;
    for (i = 0; i < 4; i++) {
        func_80047AE0(&D_800FEFD8[i], 0);
        D_800FEFD0[i] = 0;
    }
}

/* JFG: one player's data in multiplayer (0xA0 bytes) */
typedef struct MultiPlayer {
    /* 0x00 */ s16 unk0[4];
    /* 0x08 */ s16 unk8[4];
    /* 0x10 */ s16 unk10;
    /* 0x12 */ u8 unk12;
    /* 0x13 */ u8 unk13;
    /* 0x14 */ u8 pad14;
    /* 0x15 */ u8 unk15;
    /* 0x16 */ CharacterData chr;
    /* 0x8C */ s8 unk8C;
    /* 0x8D */ u8 pad8D[3];
    /* 0x90 */ s32 unk90;
    /* 0x94 */ s16 unk94[4];
    /* 0x9C */ s16 unk9C;
    /* 0x9E */ u8 pad9E[2];
} MultiPlayer;
extern MultiPlayer D_800FED50[4];
extern u8 D_800A4FC4;
extern u8 D_800A4FD0;
extern u8 D_800A4FE0;

/* The unk94 and unk9C fields of D_800FED50[0..3], referenced as their own symbols. */
extern s16 D_800FEDE4, D_800FEDE6, D_800FEDE8, D_800FEDEA, D_800FEDEC;
extern s16 D_800FEE84, D_800FEE86, D_800FEE88, D_800FEE8A, D_800FEE8C;
extern s16 D_800FEF24, D_800FEF26, D_800FEF28, D_800FEF2A, D_800FEF2C;
extern s16 D_800FEFC4, D_800FEFC6, D_800FEFC8, D_800FEFCA, D_800FEFCC;

void func_8004797C(void) {
    D_800FEDEC = 0;
    D_800FEDE4 = 0;
    D_800FEDE6 = 0;
    D_800FEDE8 = 0;
    D_800FEDEA = 0;
    D_800FEE8C = 0;
    D_800FEE84 = 0;
    D_800FEE86 = 0;
    D_800FEE88 = 0;
    D_800FEE8A = 0;
    D_800FEF2C = 0;
    D_800FEF24 = 0;
    D_800FEF26 = 0;
    D_800FEF28 = 0;
    D_800FEF2A = 0;
    D_800FEFCC = 0;
    D_800FEFC4 = 0;
    D_800FEFC6 = 0;
    D_800FEFC8 = 0;
    D_800FEFCA = 0;
}

void func_80047A20(void) {
    s32 i;
    s32 j;
    s32 n;
    u8 *src;
    u8 *dst;
    MultiPlayer *mp;
    CharacterData *chr;
    u8 *id;

    for (i = 0; i < 4; i++) {
        mp = &D_800FED50[i];
        chr = &D_800FEFD8[i];
        id = &D_800FEFD0[i];
        for (j = 0; j < 4; j++) {
            D_800FED50[i].unk0[j] = 0;
            D_800FED50[i].unk8[j] = 0;
        }
        mp->unk10 = 0;
        mp->unk13 = D_800A4FE0;
        mp->unk8C = i;
        mp->unk90 = 0;
        mp->unk15 = 0;
        mp->unk12 = *id;
        src = (u8 *) chr;
        dst = (u8 *) &mp->chr;
        n = sizeof(CharacterData);
        while (n--) {
            *dst++ = *src++;
        }
    }
}

u8 *weapon_info_get(s32 index);

#ifdef NON_MATCHING
/* NON_MATCHING: the target loads the item loop's bound (8) before zeroing the counter */
/* JFG: reset a character's data for a level: the default weapon only, each weapon's values from its info, no
 * items. */
void func_80047AE0(CharacterData *chr, s32 level) {
    s32 i;
    u8 *info;

    chr->unk0 = 0x2200;
    chr->unk2 = 6;
    chr->unk4_hi = level;
    chr->unk74_hi = level;
    chr->unk4_lo = 0;
    for (i = 0; i < 16; i++) {
        info = weapon_info_get(i);
        chr->unk14[i] = 0;
        chr->unk34[i] = *(s16 *) (info + 0x24);
        chr->unk54[i] = info[3];
    }
    info = weapon_info_get(0);
    chr->unk6F = 1;
    chr->unk70 = 0;
    chr->weapons = 1 << chr->unk70;
    chr->unk14[0] = *(s16 *) (info + 0x28);
    chr->unkC[0] = 0;
    chr->unkC[1] = 0;
    chr->unkC[2] = 0;
    chr->unkC[3] = 0;
    i = 0;
    while (i < 8) {
        chr->items[i] = 0;
        i++;
    }
    chr->unk6 = 0;
    chr->unk8 = 0;
    chr->unk74_lo = 0;
}
#else
INCLUDE_ASM("asm/nonmatchings/45750", func_80047AE0);
#endif

/* Returns the game settings (the save data being played) (0x80047C0C). */
GameStats *get_settings(void) {
    return gSettingsPtr;
}

#ifdef NON_MATCHING
/* NON_MATCHING: the target schedules `character & 3` into the first delay slot (mine: the lui of gSettingsPtr) */
u8 *func_80047C1C(s32 player, s32 character) {
    if (D_800A4FC4 != 0 && D_800A4FD0 >= 2) {
        if (player < 0 || player >= D_800A4FD0) {
            player = 0;
        }
        return (u8 *) &D_800FED50[player].chr;
    }
    return (u8 *) &gSettingsPtr->chars[character & 3];
}
#else
INCLUDE_ASM("asm/nonmatchings/45750", func_80047C1C);
#endif

u8 *func_80047C1C(s32 player, s32 character);
extern s32 gCharItemsChanged; // JFG: characters whose items changed (one bit each)

/* JFG: give a player's character an item (0x80047C9C). */
void character_item_give(s32 player, s32 character, s32 item) {
    u8 *chr = func_80047C1C(player, character);

    if (chr != NULL) {
        (chr + 0x66)[item >> 3] |= 0x80 >> (item & 7);
        gCharItemsChanged |= 1 << (character & 3);
    }
}

/* JFG: take an item from a player's character (0x80047D0C). */
void character_item_take(s32 player, s32 character, s32 item) {
    u8 *chr = func_80047C1C(player, character);

    if (chr != NULL) {
        (chr + 0x66)[item >> 3] &= ~(0x80 >> (item & 7));
        gCharItemsChanged |= 1 << (character & 3);
    }
}

/* JFG: whether a player's character has an item (a bit array at 0x66, highest bit first) (0x80047D80). */
s32 character_has_item(s32 player, s32 character, s32 item) {
    UNUSED s32 pad;
    u8 *chr;
    s32 has = 0;

    chr = func_80047C1C(player, character);
    if (chr != NULL) {
        if ((chr + 0x66)[item >> 3] & (0x80 >> (item & 7))) {
            has = 1;
        }
    }
    return has;
}

#ifdef NON_MATCHING
/* NON_MATCHING: the target computes each `!= 0` into a fresh t-register and copies it to v0 (mine: a0) */
/* JFG: whether any of player 0's first three characters has an item. */
s32 func_80047DD8(s32 item) {
    s32 ret;
    ret = character_has_item(0, 0, item) != 0;
    if (!ret) {
        ret = character_has_item(0, 1, item) != 0;
        if (!ret) {
            ret = character_has_item(0, 2, item) != 0;
        }
    }
    return ret;
}
#else
INCLUDE_ASM("asm/nonmatchings/45750", func_80047DD8);
#endif

/* JFG: whether a player's character has a weapon (a bit mask at 0xA) (0x80047E44). */
s32 character_has_weapon(s32 player, s32 character, s32 weapon) {
    u8 *chr = func_80047C1C(player, character);

    if (chr != NULL) {
        if (*(u16 *) (chr + 0xA) & (1 << weapon)) {
            return TRUE;
        }
    }
    return FALSE;
}

/* JFG: read one of the game's 0x480 flags (a bit array at 0x30, lowest bit first) (0x80047E8C). */
s32 settings_flag_get(GameStats *game, s32 flag) {
    if (flag >= 0 && flag < 0x480) {
        if (((u8 *) game)[(flag >> 3) + 0x30] & (1 << (flag & 7))) {
            return TRUE;
        }
    }
    return FALSE;
}

/* Returns one of the 0x480 game flags of the current settings (0x80047ED0). */
s32 game_flag_get(s32 flag) {
    return settings_flag_get(gSettingsPtr, flag);
}

/* JFG: 0x80 bytes of unlock bits (D_800FF37C and D_800FF391 are its 0x64 and 0x79) */
typedef struct UnlockBits {
    /* 0x00 */ u8 pad0[0x64];
    /* 0x64 */ s32 unk64;
    /* 0x68 */ u16 unk68;
    /* 0x6A */ u8 unk6A;
    /* 0x6B */ u8 unk6B;
    /* 0x6C */ u8 pad6C[0x79 - 0x6C];
    /* 0x79 */ u8 unk79;
    /* 0x7A */ u8 pad7A[6];
} UnlockBits;
extern UnlockBits D_800FF318;
extern s32 D_800FF37C;
void mark_write_eeprom_settings(void);

#ifdef NON_MATCHING
/* NON_MATCHING: the two word loops get other registers (target: src $v1, dst $a0, count $v0) and some
   temporaries are renumbered; the frame needs a fourth word above copy (pad) */
/* JFG: set or clear one of the game's 0x480 flags (0x5A and 0x5B are always set while D_800A331C is 0); setting
 * one also sets the unlock bits it stands for, and mark_write_eeprom_settings runs if any of them changed. */
void func_80047EFC(s32 flag, s32 set) {
    s32 *src;
    s32 *dst;
    s32 i;
    UNUSED s32 pad;
    s32 copy[32];

    if ((flag == 0x5B || flag == 0x5A) && D_800A331C == 0) {
        set = TRUE;
    }
    if (flag >= 0 && flag < 0x480) {
        if (set) {
            ((u8 *) gSettingsPtr)[(flag >> 3) + 0x30] |= 1 << (flag & 7);
        } else {
            ((u8 *) gSettingsPtr)[(flag >> 3) + 0x30] &= ~(1 << (flag & 7));
        }
        if (set) {
            src = (s32 *) &D_800FF318;
            dst = copy;
            i = 32;
            while (i--) {
                *dst++ = *src++;
            }
            src = (s32 *) &D_800FF318;
            if (flag >= 0 && flag < 20) {
                if (flag < 4) {
                    D_800FF391 = D_800FF318.unk79 | (1 << flag);
                }
                D_800FF37C = D_800FF318.unk64 | (1 << flag);
            } else if (flag >= 0x42 && flag < 0x47) {
                D_800FF318.unk6A |= 1 << (flag + 30);
            } else if (flag >= 0x49 && flag < 0x4D) {
                D_800FF318.unk6B |= 1 << (flag + 23);
            } else if (flag >= 0x52 && flag < 0x58) {
                D_800FF318.unk68 |= 1 << (flag + 14);
            } else if (flag == 0x5F) {
                D_800FF318.unk79 |= 0x40;
                D_800FF318.unk64 |= 0x70;
            } else if (flag == 0x5D) {
                D_800FF318.unk79 |= 0x80;
            }
            dst = copy;
            i = 32;
            while (i--) {
                if (*src++ != *dst++) {
                    mark_write_eeprom_settings();
                    return;
                }
            }
        }
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/45750", func_80047EFC);
#endif

typedef struct LevelHeader {
    /* 0x000 */ u8 pad0[0x103];
    /* 0x103 */ u8 region;
} LevelHeader;
LevelHeader *level_header(void);

void func_80048130(s32 bit, s32 mode) {
    LevelHeader *header = level_header();
    s32 mask = 1 << (bit & 0xF);

    if (!(gSettingsPtr->unk5C[header->region] & mask) && !(gSettingsPtr->unkDC[header->region] & mask)) {
        switch (mode) {
            case 1:
                gSettingsPtr->unk5C[header->region] |= mask;
                gSettingsPtr->unk4++;
                gSettingsPtr->unk6--;
                break;
            case 2:
                gSettingsPtr->unkDC[header->region] |= mask;
                gSettingsPtr->unk5++;
                gSettingsPtr->unk6--;
                break;
        }
    }
}

s32 func_8004822C(s32 bit) {
    LevelHeader *header = level_header();
    s32 mask = 1 << (bit & 0xF);
    if (gSettingsPtr->unk5C[header->region] & mask) {
        return 1;
    }
    if (gSettingsPtr->unkDC[header->region] & mask) {
        return 2;
    }
    return 0;
}

void func_800482A0(s32 player, s32 arg1, s32 arg2) {
    if (arg1 >= 0) {
        CharacterData *chr = &gSettingsPtr->chars[player & 3];
        gSettingsPtr->unk1 = arg1;
        chr->unk4_hi = arg1;
        chr->unk71 = arg2;
    }
    gSettingsPtr->unk2 = arg2;
}

extern s8 gIsPaused;

/* Returns the pause state (nonzero while paused) (0x800482F8). */
s8 is_game_paused(void) {
    return gIsPaused;
}

s32 runlink_module_loaded(s32 module);
void func_8005342C(s32 module);
void func_80000450(s32 mode);
extern s8 gIsPaused;

/* Sets the pause state and changes the sound volume to match (0x80048308). */
void set_game_paused(s32 mode) {
    gIsPaused = mode;
    if (gIsPaused == 0) {
        if (runlink_module_loaded(0x13) != 0) {
            func_8005342C(0x13);
        }
        func_80000450(0);
        return;
    }
    func_80000450(1);
}

/**
 * Sets and returns (nonzero) the message set when pressing the reset button.
 * Official name: mainResetPressed
 */
s32 is_reset_pressed(void) {
    if (gNMIMesgBuf == 0) {
        gNMIMesgBuf = (s32) ((osRecvMesg(&gNMIMesgQueue, NULL, OS_MESG_NOBLOCK) + 1) != 0);
    }
    return gNMIMesgBuf;
}

/* Returns the current level (map) number (0x800483B8). */
s16 get_ingame_map_id(void) {
    return gPlayableMapId;
}

/* Marks a save file to be read (0x800483C8). */
void mark_read_save_file(s32 saveFileIndex) {
    gSaveDataFlags &= ~7;
    gSaveDataFlags |= (0x80000000 | (saveFileIndex & 7));
}

extern s32 gSavefileData;

/* Marks all save files to be read into the given buffer (0x800483F8). */
void mark_read_all_save_files(s32 games) {
    gSavefileData = games;
    gSaveDataFlags |= 0x8000000;
}

extern s32 gSavefileData;

/* Returns the buffer all save files are read into (0x8004841C). */
s32 get_all_save_files_ptr(void) {
    return gSavefileData;
}

/**
 * Forcefully marks a flag to write a save file.
 * JFG: the file number is three bits at bit 3.
 * Official Name: mainSaveGame
 */
void force_mark_write_save_file(s32 saveFileIndex) {
    gSaveDataFlags &= ~SAVE_DATA_FLAG_WRITE_SAVE_FILE_NUMBER_BITS;
    gSaveDataFlags |= (SAVE_DATA_FLAG_WRITE_SAVE_DATA | ((saveFileIndex & 7) << 3));
}

/**
 * Marks a flag to write a save file, as long as the game is in its main mode.
 * JFG: no tracks mode check.
 * Official Name: mainSaveGame2
 */
void safe_mark_write_save_file(s32 saveFileIndex) {
    if (gGameMode == GAMEMODE_INGAME) {
        gSaveDataFlags &= ~SAVE_DATA_FLAG_WRITE_SAVE_FILE_NUMBER_BITS;
        gSaveDataFlags |= (SAVE_DATA_FLAG_WRITE_SAVE_DATA | ((saveFileIndex & 7) << 3));
    }
}

/* Marks a save file to be erased (0x800484A4). */
void mark_save_file_to_erase(s32 saveFileIndex) {
    gSaveDataFlags = ((saveFileIndex & 7) << 6) | 0x20000000;
}

extern s32 gSaveDataFlags;

/* Marks all save files to be erased (0x800484C0). */
void mark_all_save_files_to_erase(void) {
    gSaveDataFlags = 0x10000000;
}

extern s32 gSaveDataFlags;

/* Marks the whole save flash to be erased (0x800484D0). */
void mark_erase_all_save_data(void) {
    gSaveDataFlags = 0x1000000;
}

/* Marks the settings to be read (0x800484E0). */
void mark_read_eeprom_settings(void) {
    gSaveDataFlags |= 0x4000000;
}

/* Marks the settings to be written (0x800484FC). */
void mark_write_eeprom_settings(void) {
    gSaveDataFlags |= 0x2000000;
}

extern s16 D_800A32BC;

void func_80048518(s32 arg0) {
    D_800A32BC = arg0;
}

/**
 * Returns TRUE if the game doesn't detect any controllers.
 * Official name: mainDemoOnly
 */
s32 is_controller_missing(void) {
    if (sControllerStatus == CONTROLLER_MISSING) {
        return TRUE;
    } else {
        return FALSE;
    }
}

extern u8 D_800A4FCC;

u8 func_80048548(void) {
    return D_800A4FCC;
}

s32 leveltable_type(s32 level);
extern u8 D_800A4FB4;

u8 func_80048558(void) {
    s32 type = leveltable_type(gPlayableMapId);
    if (type == 1 || type == 2) {
        return 0;
    }
    if (D_800A4FC4 == 0 && D_800A4FB4 == 0) {
        return 1;
    }
    return D_800A4FD0;
}

extern s16 D_800A3240;

s32 func_800485C8(s32 player) {
    if (D_800A4FD0 < 2 || (D_800A4FC4 == 0 && D_800A4FB4 == 0)) {
        return D_800A3240;
    }
    return D_800FED50[player & 3].unk12;
}

extern s16 D_800A3268;
extern s8 *D_800FD7B8;
extern s8 D_800A3298;
void objflags_clear_all(void);
void objflags_select_region(void);

/* JFG: clear the current settings' region bits, counters and weapon statistics. */
void settings_clear_stats(void) {
    s32 i;

    for (i = 0; i < 64; i++) {
        gSettingsPtr->unk5C[i] = 0;
        gSettingsPtr->unkDC[i] = 0;
    }
    gSettingsPtr->unk4 = 0;
    gSettingsPtr->unk5 = 0;
    gSettingsPtr->unk6 = D_800FD7B8[D_800A3268];
    gSettingsPtr->unk530 = 0;
    gSettingsPtr->unk514 = 0;
    for (i = 0; i < 16; i++) {
        gSettingsPtr->shots[i] = 0;
        gSettingsPtr->hits[i] = 0;
        gSettingsPtr->kills[i] = 0;
    }
    objflags_clear_all();
    if (D_800A3298 != 0) {
        objflags_select_region();
    }
}

extern s32 D_800A3398;
extern s32 D_800A339C[];
extern s32 D_800A32D0;

#ifdef NON_MATCHING
/* NON_MATCHING: registers: the target keeps the pressed buttons in v1 and &D_800A3398 in v0 */
/* JFG: a button sequence on controller 1 (zero-terminated list D_800A339C) toggles D_800A32D0. */
void func_800487B8(void) {
    s32 pressed = input_pressed(0);

    if (pressed != 0) {
        if (pressed == D_800A339C[D_800A3398]) {
            if (D_800A339C[++D_800A3398] == 0) {
                D_800A3398 = 0;
                D_800A32D0 ^= 1;
            }
        } else {
            D_800A3398 = 0;
        }
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/45750", func_800487B8);
#endif

INCLUDE_ASM("asm/nonmatchings/45750", func_8004883C);
