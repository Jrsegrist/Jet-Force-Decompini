/*
 * Runlink module 19: the in-game pause menu. It pauses the game, remembers
 * the level and character, and switches between the menu's screens (each
 * one drawn by another module: 9, 18, 37), loading the next screen while the
 * current one fades out. It also has the multiplayer version and a small
 * screen of its own that asks for confirmation before leaving the level.
 */
#include "module.h"
#include "gfx.h"

/* A level name and the world it belongs to */
typedef struct M019Level {
    /* 0x0 */ char *name;
    /* 0x4 */ s16 world;
    /* 0x6 */ s16 text;
} M019Level;

extern Gfx *D_800FF398; /* the display list being built */
extern Mtx *D_800FF39C;
extern Vertex *D_800FF3A0;
extern s32 D_800FF3C8[]; /* the buttons pressed this frame, per player */
extern s8 D_800FF3D8[];  /* the stick moves this frame, per player (x) */
extern s8 D_800FF3DC[];  /* (y) */
extern char **gMenuText;
extern u8 D_800A4FB4;
extern s16 D_800A4FB8;
extern u8 D_800A4FBC;
extern u8 D_800A4FC0;
extern u8 D_800A4FC8;
extern u8 D_800A4FD0; /* the number of players */
extern s8 D_800A4FEC;
extern s8 D_800A503C;
extern s32 D_m012_01C24;

extern s8 is_game_paused(void);
extern void set_game_paused(s32 paused);
extern s32 level_id(void);
extern s32 leveltable_world(s32 level);
extern s32 func_800468CC(void);
extern s32 music_volume(void);
extern void func_800005F8(f32 volume, s32 arg1); /* fades the music */
extern void func_8005342C(s32 module);          /* unloads a runlink module */
extern s32 runlink_module_loaded(s32 module);
extern void dialogue_close(s32 arg0);
extern void mempool_free_queue_flush(void);
extern void mempool_free_timer(s32 frames);
extern void func_80059E1C(s16 *assets); /* loads menu textures */
extern void func_80059FE8(s16 *assets);
extern void func_80059CCC(s16 *assets); /* frees them */
extern s32 get_current_viewport(void);
extern s32 cam_get_active_camera_id(void);
extern f32 cam_get_fov(void);
extern s32 cam_set_layout(s32 mode);
extern void set_active_camera(s32 camera);
extern void cam_fov_scale_set(f32 x, f32 y);
extern void cam_set_fov(f32 fov, s32 arg1);
extern void viewport_scissor(Gfx **dList);
extern void viewport_reset(Gfx **dList);
extern void mtx_perspective(Gfx **dList, Mtx **mtx);
extern void func_80054EA0(u32 *width, u32 *height); /* the frame buffer size */
extern void bgdraw_render(Gfx **dList, Mtx **mtx, s32 arg2);
extern void rendermode_reset(Gfx **dList);
extern void func_8005A0D0(s32 arg0);
extern void set_text_font(s32 font);
extern void set_text_colour(s32 red, s32 green, s32 blue, s32 alpha, s32 opacity);
extern void set_text_background_colour(s32 red, s32 green, s32 blue, s32 alpha);
extern void draw_text(Gfx **dList, s32 x, s32 y, char *text, s32 alignment);

extern void func_m005_00BF8();
extern void func_m009_00000();
extern void func_m009_003A8();
extern void func_m009_00690();
extern void func_m012_000E4();
extern void func_m012_00108();
extern void func_m012_0014C();
extern void func_m012_00158();
extern s32 func_m012_01820();
extern s32 func_m012_01830();
extern s32 func_m012_01870();
extern void func_m012_018F4();
extern void func_m018_000CC();
extern void func_m018_0015C();
extern void func_m018_00188();
extern void func_m018_01028();
extern void func_m018_01140();
extern void func_m018_0130C();
extern void func_m018_020C0();
extern void func_m018_023E8();
extern void func_m018_02464();
extern void func_m018_042D4();
extern void func_m018_043AC();
extern void func_m018_04408();
extern void func_m018_048E4();
extern void func_m018_04AF0();
extern void func_m018_05788();
extern void func_m018_05B40();
extern void func_m018_05BD8();
extern void func_m018_05C0C();
extern void func_m037_00000();
extern void func_m037_00160();
extern void func_m037_001A0();
extern void func_m038_00000();

void func_m019_001B0(s32 screen);
void func_m019_00324(s32 level, s32 arg1, s32 arg2, s32 arg3);

static M019Level sLevels[] = {
    { "Goldwood Forest", 12, 0x5C },
    { "SS Anubis", 10, 0x23 },
    { "Swamp", 7, 0x9E },
    { "Sekhmet Battleship", 8, 0x43 },
    { "Spawn Ship", 11, 0x3D },
    { "Canyon", 2, 0x51 },
    { "Ichor", 5, 0x2B },
    { "Alien Cave", 6, 0xD8 },
    { "Goldwood Caves", 13, 0x10 },
    { "Dune", 9, 0x35 },
    { "Canyon Mine", 17, 0xB3 },
    { "Asteroid", 3, 0xE1 },
    { "Mizar", 4, 0x9B },
    { "Sub-Worlds", 18, 0xE5 },
    { "General", 0, -1 },
    { "FrontEnd", 1, -1 },
    { "Multi", 14, -1 },
    { "Old", 15, -1 },
    { "Baddy Test", 16, -1 },
    { NULL, 0, 0 },
};
static s32 sUnused1180 = 0;
static s32 sMode = 1;
static s32 sLevelIndex = 0;
static s32 sUnused118C = 0;
static Gfx sDisplayList1190[] = {
    { { 0xE7000000, 0x00000000 } }, { { 0xBA001001, 0x00000000 } }, { { 0xBA000E02, 0x00000000 } },
    { { 0xB9000002, 0x00000000 } }, { { 0xB6000000, 0x00010001 } }, { { 0xFCFFFFFF, 0xFFFDF6FB } },
    { { 0xEF000C0F, 0x00504240 } }, { { 0xFA000000, 0x000000A0 } }, { { 0xF6460300, 0x000A00C0 } },
    { { 0xE7000000, 0x00000000 } }, { { 0xB8000000, 0x00000000 } },
};
static Gfx sDisplayList11E8[] = {
    { { 0xE7000000, 0x00000000 } }, { { 0xBA001001, 0x00000000 } }, { { 0xBA000E02, 0x00000000 } },
    { { 0xB9000002, 0x00000000 } }, { { 0xB6000000, 0x00010001 } }, { { 0xFC72FEE5, 0xFFFDF6FB } },
    { { 0xEF000C0F, 0x00504240 } }, { { 0xFA000000, 0x00400080 } }, { { 0xFB000000, 0x08200880 } },
    { { 0xF6460300, 0x000A00C0 } }, { { 0xE7000000, 0x00000000 } }, { { 0xB8000000, 0x00000000 } },
};
static s16 sAssets[] = { 0x65, -1 };
static s16 sAssets2[] = { 0x1F, -1 };

static s32 sLevel;
static s32 sLevel2;
static s32 sCharacter;
static s32 sCharacter2;
static s32 sUnk10;
static s32 sStage; /* 1: the next screen is loading, 2: shown */
static s32 sNewLevel;
static s32 sNewLevelArg1;
static s32 sNewLevelArg2;
static s32 sNewLevelArg3;
s32 D_m019_013D8;
static s32 sUnk2C;
static s32 sUnk30;
static s32 sUnk34;
static s32 sUnk38;
static s32 sUnk3C;
static s32 sUnk40;
static s32 sScreen;
static s32 sNextScreen;
static s8 sScreenLoaded;
static s32 sFade;
static s32 sFadeStep;
static s32 sChoice;
static s32 sDone;
static s32 sAsking;

void func_m019_00000(s32 arg0) {
    Object *player;
    s32 world;
    s32 i;
    char *name;

    if (is_game_paused() == 0) {
        player = func_8000E4D0(0);
        if (player != NULL) {
            sCharacter = ((s8 *) player->unk68)[1];
        } else {
            sCharacter = func_800468CC();
        }
        sCharacter2 = sCharacter;
        sLevel2 = sLevel = level_id();
        world = leveltable_world(sLevel);
        sLevelIndex = 0;
        i = sLevelIndex;
        while ((name = sLevels[i].name) != NULL && sLevels[i].world != world) {
            i++;
        }
        if (name != NULL) {
            sLevelIndex = i;
        }
        sUnk10 = D_800A4FC8;
        sUnk2C = 0;
        sUnk30 = 0;
        sUnk34 = 0;
        sUnk38 = 0;
        set_game_paused(1);
        D_m012_01C24 = music_volume();
        func_800005F8(0.5f, 0);
        func_m012_000E4(320, 240);
        func_80001248(0x21, 0);
        sUnk40 = 0;
        sUnk3C = 1;
        D_m019_013D8 = arg0;
        sScreen = 10;
        sNextScreen = 0;
        sStage = 2;
        func_m019_001B0(1);
        sScreenLoaded = 0;
    }
}

void func_m019_001B0(s32 screen) {
    sNextScreen = screen;
    sStage = 1;
}

s32 func_m019_001C8(void) {
    return sScreen;
}

static void free_module_38(void) {
    func_8005342C(0x26);
}

s32 func_m019_001F8(void) {
    return sCharacter;
}

static void confirm_close(void);
static void confirm_open(void);
static void confirm_update(s32 updateRate);

/* Unloads the screen's module */
static void close_screen(s32 screen) {
    switch (sScreen) {
        case 2:
            func_m009_003A8();
            func_8005342C(9);
            func_m012_00158(1);
            break;
        case 1:
            func_m018_023E8();
            break;
        case 12:
            confirm_close();
            break;
        case 3:
            func_m018_0015C();
            func_8005342C(9);
            func_m012_00158(1);
            break;
        case 4:
            func_m018_01140();
            func_m012_00158(1);
            break;
        case 6:
            func_m037_00160();
            func_8005342C(0x25);
            break;
        case 5:
            func_m018_043AC();
            func_8005342C(0x29);
            func_m012_00158(1);
            break;
        case 9:
            func_m018_05788();
            break;
        case 7:
            func_m018_05BD8();
            func_m012_00158(1);
            break;
    }
}

void func_m019_00324(s32 level, s32 arg1, s32 arg2, s32 arg3) {
    sNewLevel = level;
    sNewLevelArg1 = arg1;
    sNewLevelArg2 = arg2;
    sNewLevelArg3 = arg3;
    sNextScreen = 11;
    sStage = 1;
}

void func_m019_00360(Gfx **dList, Mtx **mtx, Vertex **vtx, s32 updateRate) {
    s32 ready;

    D_800FF398 = *dList;
    D_800FF39C = *mtx;
    D_800FF3A0 = *vtx;
    dialogue_close(1);
    if (runlink_module_loaded(5)) {
        func_m005_00BF8(0);
    }
    if (sNextScreen && sScreenLoaded) {
        switch (sNextScreen) {
            case 3:
                func_m018_000CC();
                break;
            case 2:
                func_m009_00000(1);
                break;
            case 4:
                func_m018_01028();
                break;
            case 1:
                func_m018_020C0();
                break;
            case 12:
                confirm_open();
                break;
            case 6:
                func_m037_00000(1);
                break;
            case 5:
                func_m018_042D4();
                break;
            case 9:
                func_m018_048E4();
                break;
            case 7:
                func_m018_05B40();
                break;
            case 10:
                mempool_free_queue_flush();
                mempool_free_timer(0);
                free_module_38();
                mempool_free_timer(2);
                func_m012_00108(2);
                if (runlink_module_loaded(0x12)) {
                    func_8005342C(0x12);
                }
                break;
            case 11:
                mempool_free_queue_flush();
                mempool_free_timer(0);
                free_module_38();
                mempool_free_timer(2);
                func_m012_00108(2);
                if (runlink_module_loaded(0x12)) {
                    func_8005342C(0x12);
                }
                func_m012_018F4(sNewLevel, sNewLevelArg1, sNewLevelArg2, sNewLevelArg3);
                break;
        }
        sScreen = sNextScreen;
        sNextScreen = 0;
        sStage = 2;
        sScreenLoaded = 0;
    }
    ready = sNextScreen != 0 && func_m012_01820() == 0xFF && func_m012_01830() != 0 && sStage == 1 && func_m012_01870() == 0;
    if (!ready) {
        switch (sScreen) {
            case 1:
                func_m038_00000();
                func_m018_02464(updateRate);
                break;
            case 12:
                func_m038_00000();
                confirm_update(updateRate);
                break;
            case 2:
                func_m009_00690(updateRate);
                break;
            case 3:
                func_m038_00000();
                func_m018_00188(updateRate);
                break;
            case 4:
                func_m038_00000();
                func_m018_0130C(updateRate);
                break;
            case 6:
                func_m037_001A0(updateRate);
                break;
            case 5:
                func_m038_00000();
                func_m018_04408(updateRate);
                break;
            case 9:
                func_m038_00000();
                func_m018_04AF0(updateRate);
                break;
            case 7:
                func_m038_00000();
                func_m018_05C0C(updateRate);
                break;
            case 8:
                func_m038_00000();
                func_m018_02464(updateRate);
                break;
        }
    }
    if (sNextScreen != 0) {
        if (sStage == 1) {
            if (ready) {
                mempool_free_queue_flush();
                mempool_free_timer(0);
                close_screen(sScreen);
                mempool_free_timer(2);
                sScreenLoaded = 1;
            } else {
                func_m012_0014C(4);
            }
        }
    } else {
        func_m012_0014C(-4);
    }
    *dList = D_800FF398;
    *mtx = D_800FF39C;
    *vtx = D_800FF3A0;
}

s32 func_m019_00830(void) {
    return sMode;
}

void func_m019_00840(void) {
    if (is_game_paused() == 0) {
        set_game_paused(1);
        D_m012_01C24 = music_volume();
        func_800005F8(0.5f, 0);
        func_m012_000E4(320, 240);
        func_80001248(0x21, 0);
        sUnk40 = 0;
        sUnk3C = 1;
        sScreen = 10;
        sNextScreen = 0;
        sStage = 2;
        func_m019_001B0(12);
        sScreenLoaded = 0;
    }
}

static void confirm_open(void) {
    func_80059E1C(sAssets);
    func_80059FE8(sAssets2);
    sFade = 0;
    sDone = 0;
    sFadeStep = 10;
    sChoice = 0;
    sAsking = 0;
}

static void confirm_close(void) {
    func_80059CCC(sAssets);
}

/* Asks whether to leave the level (and, after yes, whether to save) */
static void confirm_update(s32 updateRate) {
    s32 i;
    s32 up;
    s32 down;
    s32 left;
    s32 right;
    s32 confirm;
    s32 back;
    s32 viewport;
    s32 camera;
    f32 fov;
    u32 width;
    u32 height;

    up = down = left = right = confirm = back = 0;
    i = D_800A4FD0;
    while (i--) {
        confirm |= D_800FF3C8[i] & (A_BUTTON | START_BUTTON);
        back |= D_800FF3C8[i] & B_BUTTON;
        up += D_800FF3DC[i] > 0;
        down += D_800FF3DC[i] < 0;
        left += D_800FF3D8[i] < 0;
        right += D_800FF3D8[i] > 0;
    }
    sFade += updateRate * sFadeStep;
    if (sFade >= 0x100) {
        sFade = 0x1FE - sFade;
        sFadeStep = -sFadeStep;
    } else if (sFade < 0) {
        sFade = -sFade;
        sFadeStep = -sFadeStep;
    }
    viewport = get_current_viewport();
    camera = cam_get_active_camera_id();
    fov = cam_get_fov();
    cam_set_layout(0);
    set_active_camera(0);
    viewport_scissor(&D_800FF398);
    func_80054EA0(&width, &height);
    width >>= 1;
    height >>= 1;
    bgdraw_render(&D_800FF398, &D_800FF39C, 0);
    rendermode_reset(&D_800FF398);
    gDPSetFogColor(D_800FF398++, 255, 255, 255, 255);
    gSPFogFactor(D_800FF398++, 0x6400, 0x9867);
    gDPPipeSync(D_800FF398++);
    func_m038_00000();
    cam_fov_scale_set(1.0f, 1.0f);
    cam_set_fov(52.0f, 1);
    viewport_reset(&D_800FF398);
    mtx_perspective(&D_800FF398, &D_800FF39C);
    func_8005A0D0(0x1F);
    set_text_font(2);
    set_text_colour(255, 255, 255, 255, 255);
    set_text_background_colour(0, 0, 0, 0);
    if (sAsking == 0) {
        draw_text(&D_800FF398, width, 50, gMenuText[511], 4);
        if (sChoice == 0) {
            set_text_colour(0, sFade, 255, 255, 255);
        } else {
            set_text_colour(0, 255, 255, 255, 255);
        }
        draw_text(&D_800FF398, width, 100, gMenuText[512], 4);
        if (sChoice == 1) {
            set_text_colour(0, sFade, 255, 255, 255);
        } else {
            set_text_colour(0, 255, 255, 255, 255);
        }
        draw_text(&D_800FF398, width, 140, gMenuText[40], 4);
    } else {
        draw_text(&D_800FF398, width, 50, gMenuText[40], 4);
        set_text_colour(0, 255, 255, 255, 255);
        draw_text(&D_800FF398, width, 100, gMenuText[316], 4);
        if (sChoice == 0) {
            set_text_colour(0, sFade, 255, 255, 255);
        } else {
            set_text_colour(0, 255, 255, 255, 255);
        }
        draw_text(&D_800FF398, width - 25, 140, gMenuText[317], 4);
        if (sChoice == 1) {
            set_text_colour(0, sFade, 255, 255, 255);
        } else {
            set_text_colour(0, 255, 255, 255, 255);
        }
        draw_text(&D_800FF398, width + 25, 140, gMenuText[318], 4);
    }
    if (sDone == 0) {
        if (sAsking != 0) {
            if (back || (confirm && sChoice == 1)) {
                func_80001248(0x16D, 0);
                sChoice = 1;
                sAsking = 0;
            } else if (confirm) {
                func_80001248(0x16D, 0);
                sDone = 1;
                if (D_800A4FB4 != 0) {
                    D_800A4FEC = 10;
                    if (D_800A503C != 0) {
                        func_m019_00324(D_800A4FB8, D_800A4FC0, D_800A4FBC, 16);
                    } else {
                        func_m019_00324(0x11D, 3, 0, 16);
                    }
                } else {
                    D_800A4FEC = 10;
                    func_m019_00324(0, 0, 0, 8);
                }
            } else if (right || left) {
                func_80001248(0x16B, 0);
                sChoice = (sChoice + 1) & 1;
            }
        } else if (back || (confirm && sChoice == 0)) {
            func_80001248(0x16D, 0);
            sDone = 1;
            func_m019_001B0(10);
        } else if (confirm) {
            func_80001248(0x16D, 0);
            sAsking = 1;
            sChoice = 1;
        } else if (up || down) {
            func_80001248(0x16B, 0);
            sChoice = (sChoice + 1) & 1;
        }
    }
    cam_set_layout(viewport);
    set_active_camera(camera);
    viewport_scissor(&D_800FF398);
    cam_set_fov(fov, 1);
}
