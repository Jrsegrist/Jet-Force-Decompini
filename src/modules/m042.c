/*
 * Runlink module 42: the front end's save file screen and its on-screen
 * keyboard (choosing, copying and erasing a save file, and entering a name).
 */
#include "module.h"
#include "gfx.h"

/* One key of the keyboard (in .data) */
typedef struct M042Key {
    /* 0x0 */ s8 model; /* the key object's model */
    /* 0x1 */ s8 joint; /* the keyboard model's joint the key sits on */
    /* 0x2 */ u8 ch;    /* the character (1 = delete, 2 = end) */
    /* 0x3 */ u8 pad3;
    /* 0x4 */ f32 lift; /* how far the key is raised */
} M042Key;

/* A menu item (D_800FF780, size 0x20; the main segment's MenuItem) */
typedef struct M042MenuItem {
    /* 0x00 */ s16 unk0;
    /* 0x02 */ s16 unk2;
    /* 0x04 */ s16 unk4;
    /* 0x06 */ s16 unk6;
    /* 0x08 */ f32 unk8;
    /* 0x0C */ f32 x;
    /* 0x10 */ f32 y;
    /* 0x14 */ f32 z;
    /* 0x18 */ u8 pad18[0x20 - 0x18];
} M042MenuItem;

/* A save file (and the current settings, which have the same layout) */
typedef struct M042Save {
    /* 0x000 */ u8 pad0[3];
    /* 0x003 */ s8 slot; /* -1: empty */
    /* 0x004 */ u8 pad4[0x518 - 0x4];
    /* 0x518 */ u32 time; /* the time played, in frames */
    /* 0x51C */ char name[0x538 - 0x51C];
} M042Save;

/* A model instance (the object's modelInstances entries) */
typedef struct M042Instance {
    /* 0x00 */ void *model;
    /* 0x04 */ u8 pad4[0xA - 0x4];
    /* 0x0A */ s8 unkA;
    /* 0x0B */ s8 matrixIndex;
    /* 0x0C */ u8 padC[0x10 - 0xC];
    /* 0x10 */ MtxF *matrices[2]; /* the joints' matrices */
} M042Instance;

extern Gfx *D_800FF398; /* the display list being built */
extern Mtx *D_800FF39C;
extern s8 D_800FF3D8[]; /* the stick moves this frame, per player (x) */
extern s8 D_800FF3DC[]; /* (y) */
extern Object *D_800FF3F8[]; /* the menu's loaded assets */
extern M042MenuItem D_800FF780[];
extern char **gMenuText;
extern s32 D_800A51CC; /* the text opacity */
extern u8 D_800A4FC8;
extern s32 D_800FB10C; /* the number of controllers */
extern u8 sControllerConnected[];

extern void menu_assetgroup_load(s16 *assets); /* loads menu textures */
extern void menu_assetgroup_reset(s16 *assets);
extern void menu_assetgroup_free(s16 *assets); /* frees menu textures */
extern void mark_read_all_save_files(void *buf);
extern void music_play(s32 seq);
extern void func_m038_00000();
extern void set_text_font(s32 font);
extern void set_text_colour(s32 red, s32 green, s32 blue, s32 alpha, s32 opacity);
extern void draw_text(Gfx **dList, s32 x, s32 y, char *text, s32 alignment);
extern void rendermode_reset(Gfx **dList);
extern void set_active_camera(s32 camera);
extern void viewport_apply(Gfx **dList, s32 arg1);
extern void cam_set_fov(f32 fov, s32 arg1);
extern void cam_fov_scale_set(f32 x, f32 y);
extern void mtx_perspective(Gfx **dList, Mtx **mtx);
extern void func_8005A0D0(s32 item); /* draws a menu item */
extern void func_8003D21C(M042Instance *inst, void *model, Object *obj, f32 arg3, f32 arg4);
extern s32 input_pressed(s32 player); /* the buttons pressed */
extern void load_first_unlocked_level(void);
extern s32 settings_flag_get(M042Save *save, s32 flag);
extern M042Save *get_settings(void);
extern void game_settings_reset(M042Save *settings);
extern void force_mark_write_save_file(s32 file);
extern void mark_save_file_to_erase(s32 file);
extern void func_8004665C(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5);
extern void func_8005AAE8(s32 arg0);
extern void func_80049BA0(MtxF *mtx); /* identity */
extern void func_80049A30(s16 angle, MtxF *mtx);
extern void func_80048B60(MtxF *mtx, f32 x, f32 y, f32 z, f32 *ox, f32 *oy, f32 *oz); /* DKR mathMtxXFMF */

static s16 sAssets[] = { 149, 150, 151, -1 };
static s16 sImages[] = { 51, 52, 31, -1 };
static s32 sLoaded = FALSE;
static s32 sPlayer = 0;
static s32 sCursorKey = 0; /* the key under the cursor */
/* The keys: their model, their joint and the character */
static M042Key sKeys[] = {
    { 0x1A, 0x0C, '1', 0, 0.0f }, { 0x1B, 0x0B, '2', 0, 0.0f }, { 0x1C, 0x0A, '3', 0, 0.0f },
    { 0x1D, 0x09, '4', 0, 0.0f }, { 0x1E, 0x08, '5', 0, 0.0f }, { 0x1F, 0x07, '6', 0, 0.0f },
    { 0x20, 0x06, '7', 0, 0.0f }, { 0x21, 0x05, '8', 0, 0.0f }, { 0x22, 0x04, '9', 0, 0.0f },
    { 0x23, 0x03, '0', 0, 0.0f }, { 0x00, 0x16, 'A', 0, 0.0f }, { 0x01, 0x15, 'B', 0, 0.0f },
    { 0x02, 0x14, 'C', 0, 0.0f }, { 0x03, 0x13, 'D', 0, 0.0f }, { 0x04, 0x12, 'E', 0, 0.0f },
    { 0x05, 0x11, 'F', 0, 0.0f }, { 0x06, 0x10, 'G', 0, 0.0f }, { 0x07, 0x0F, 'H', 0, 0.0f },
    { 0x08, 0x0E, 'I', 0, 0.0f }, { 0x09, 0x0D, 'J', 0, 0.0f }, { 0x0A, 0x20, 'K', 0, 0.0f },
    { 0x0B, 0x1F, 'L', 0, 0.0f }, { 0x0C, 0x1E, 'M', 0, 0.0f }, { 0x0D, 0x1D, 'N', 0, 0.0f },
    { 0x0E, 0x1C, 'O', 0, 0.0f }, { 0x0F, 0x1B, 'P', 0, 0.0f }, { 0x10, 0x1A, 'Q', 0, 0.0f },
    { 0x11, 0x19, 'R', 0, 0.0f }, { 0x12, 0x18, 'S', 0, 0.0f }, { 0x13, 0x17, 'T', 0, 0.0f },
    { 0x14, 0x2A, 'U', 0, 0.0f }, { 0x15, 0x29, 'V', 0, 0.0f }, { 0x16, 0x28, 'W', 0, 0.0f },
    { 0x17, 0x27, 'X', 0, 0.0f }, { 0x18, 0x26, 'Y', 0, 0.0f }, { 0x19, 0x25, 'Z', 0, 0.0f },
    { 0x26, 0x24, '-', 0, 0.0f }, { 0x27, 0x23, ' ', 0, 0.0f }, { 0x28, 0x22, '.', 0, 0.0f },
    { 0x25, 0x21, 1, 0, 0.0f },   { 0x24, 0x2B, 2, 0, 0.0f },
};
#define KEY_COUNT 41
static s32 sUnk2E04[] = { 800, 1000 };
static s16 sPulse = 0; /* a pulsing text opacity */
static s8 sPulseStep = 8;
static s32 sUnk2E14[] = { 0, 0, 0 };
static s32 sUnk2E20[] = { 2, 4, 1, 8, 1, 4, 2, 8, 0x100, 0x400, 0x200, 0x800 };

static s32 sTimer;
static s32 sUnk4;
static s32 sList[8]; /* a command list (func_m042_02C80 returns it) */
static s16 *sListEnd;
static s32 sUnk2C;
static char sName[0x18];
static char *sCursor;
static s16 sAngle; /* the keyboard's angle */
static s8 sFile; /* the save file chosen */
static s8 sSel;  /* the selection */
static s8 sCopyFrom;
static s8 sUnk51;
static s16 sUnk52;
static s32 sUnk54;
static s32 sWave[14]; /* the keys' wave phases, as the keyboard appears */
static s8 sState;
static s8 sCopying;
static s8 sErase;
static s8 sNameEmpty;
static f32 sTitleY;
static M042Save sSaves[6];
static s16 sFade;
static s16 sUnk1FEA;
static s32 sUnk1FEC;
static s8 sPairs[6][2]; /* the menu options shown: model (-1: none) and whether it is available */
static s32 sUnk1FFC;

void func_m042_00000(void) {
    s32 i;

    if (sLoaded == 0) {
        sLoaded = 1;
        menu_assetgroup_load(sAssets);
        menu_assetgroup_reset(sImages);
        D_800FF780[31].unk6 = 0x97;
        D_800FF780[51].unk0 = -0x8000;
        sAngle = 0x7FF0;
        D_800FF780[52].unk0 = -0x8000;
        for (i = 0; i < KEY_COUNT; i++) {
            sKeys[i].lift = 0.0f;
        }
        sCursor = sName;
        *sCursor = '\0';
        sState = 0;
        sTitleY = -20.0f;
        sFile = 0;
        sSel = 0;
        sCopyFrom = 0;
        sFade = 0;
        mark_read_all_save_files(sSaves);
        for (i = 0; i < 6; i++) {
            sPairs[i][0] = -1;
            sPairs[i][1] = 0;
        }
        music_play(2);
        sTimer = 0;
    }
}

void func_m042_00150(void) {
    menu_assetgroup_free(sAssets);
    sLoaded = 0;
}

#ifdef NON_MATCHING
/* NON_MATCHING: same instructions and frame, but t-register numbers differ from the key loop's
 * joint load (0x282C) to the end; its .rodata stays asm until it matches. */
static void func_m042_02C8C(s32 arg0);

void func_m042_00178(s32 updateRate) {
    s32 left;
    s32 right;
    s32 up;
    s32 down;
    s32 empty;
    s32 i;
    s32 len;
    s32 phase;
    s32 row;
    s32 hours;
    s32 mins;
    f32 t;
    Object *keyObj;
    M042Instance *inst;
    f32 target;
    f32 saved;
    MtxF mtx;
    f32 x;
    f32 y;
    f32 z;
    MtxF *joints;
    M042Key *key;
    M042Save *save;
    s8 *pair;
    s32 moved;
    char buf[44];
    u8 *dst;
    u8 *src;
    u8 *dst2;
    s32 n;
    M042MenuItem *item;

    moved = FALSE;
    item = &D_800FF780[52];
    if (sLoaded != 0) {
        if (sTimer < 30) {
            sTimer += updateRate;
        }
        func_m038_00000();
        set_text_font(2);
        set_text_colour(0, 0, 255, 255, sFade);
        rendermode_reset(&D_800FF398);
        gMoveWd(D_800FF398++, G_MW_NUMLIGHT, G_MWO_NUMLIGHT, 0);
        gSPClearGeometryMode(D_800FF398++, 0x1001);
        gDPSetFogColor(D_800FF398++, 255, 255, 255, 255);
        gSPFogFactor(D_800FF398++, 0x6400, 0x9867);
        gDPPipeSync(D_800FF398++);
        set_active_camera(0);
        viewport_apply(&D_800FF398, 0);
        cam_set_fov(52.0f, 1);
        sListEnd = (s16 *) sList;
        *sListEnd = 0x2006;
        *++sListEnd = sAngle;
        *++sListEnd = 0x1000;
        sListEnd++;
        down = 0;
        up = 0;
        right = 0;
        left = 0;
        if (D_800FF3D8[sPlayer] < -32) {
            left = 1;
        } else if (D_800FF3D8[sPlayer] > 32) {
            right = 1;
        }
        if (D_800FF3DC[sPlayer] < -32) {
            down = 1;
        } else if (D_800FF3DC[sPlayer] > 32) {
            up = 1;
        }
        mtx_perspective(&D_800FF398, &D_800FF39C);
        cam_fov_scale_set(1.0f, 1.0f);
        cam_set_fov(52.0f, 1);
        mtx_perspective(&D_800FF398, &D_800FF39C);
        func_8005A0D0(0x1F);
        keyObj = D_800FF3F8[149];
        inst = ((M042Instance **) keyObj->modelInstances)[0];
        inst->unkA = updateRate;
        func_8003D21C(inst, inst->model, keyObj, 1.0f, 0.0f);
        inst->unkA = 0;
        func_8005A0D0(0x33);
        sPulse += sPulseStep * updateRate;
        if (sPulse >= 0x100) {
            sPulse = 0x1FE - sPulse;
            sPulseStep = -sPulseStep;
        } else if (sPulse < 0) {
            sPulse = -sPulse;
            sPulseStep = -sPulseStep;
        }
        empty = 0;
        for (i = 0; i < 6; i++) {
            if (sSaves[i].slot == -1) {
                empty++;
            }
        }
        switch (sState) {
            case 0:
                for (i = 0; i < updateRate; i++) {
                    sAngle += (0x8000 - sAngle) >> 4;
                }
                if (sAngle > 0x7A00) {
                    sFade += updateRate * 4;
                    if (sFade > 0xFF) {
                        sFade = 0xFF;
                    }
                    pair = sPairs[0];
                    for (i = 0; i < 6; i++) {
                        pair[0] = i + 0x2C;
                        pair[1] = 1;
                        pair += 2;
                    }
                    if (right) {
                        sSel++;
                        if (sSel >= 6) {
                            sSel = 0;
                        }
                        moved = TRUE;
                    }
                    if (left) {
                        sSel--;
                        if (sSel < 0) {
                            sSel = 5;
                        }
                        moved = TRUE;
                    }
                    if (input_pressed(sPlayer) & 0x8000) {
                        sState = 3;
                        func_80001248(0x16C, 0);
                        mins = 0;
                        for (i = 0; i < 14; i++) {
                            sWave[i] = mins;
                            mins -= 0x1000;
                        }
                    }
                    for (i = 0; i < updateRate; i++) {
                        sTitleY += (22.0f - sTitleY) * 0.25f;
                    }
                    gDPPipeSync(D_800FF398++);
                    sFile = sSel;
                    sprintf(buf, "%s %d", gMenuText[448], sFile + 1);
                    draw_text(&D_800FF398, 160, 104, buf, 4);
                    if (sSaves[sFile].slot == -1) {
                        draw_text(&D_800FF398, 160, 119, gMenuText[449], 4);
                    } else {
                        draw_text(&D_800FF398, 160, 119, sSaves[sFile].name, 4);
                        t = sSaves[sFile].time / 60.0f;
                        hours = (s32) t / 3600;
                        t -= hours * 3600.0f;
                        mins = (s32) t / 60;
                        if (hours >= 1000) {
                            hours = 999;
                            mins = 59;
                        }
                        sprintf(buf, "%s %02d:%02d", gMenuText[456], hours, mins);
                        draw_text(&D_800FF398, 160, 134, buf, 4);
                    }
                    if ((input_pressed(sPlayer) & 0x4000) && sTimer >= 30) {
                        func_80001248(0x16C, 0);
                        load_first_unlocked_level();
                        sState = 9;
                    }
                }
                func_m042_02C8C(updateRate);
                break;
            case 3:
                for (i = 0; i < updateRate; i++) {
                    sTitleY += (-20.0f - sTitleY) * 0.25f;
                }
                if (sTitleY < -19.95f) {
                    if (sSaves[sFile].slot == -1) {
                        sState = 2;
                        sCopying = 0;
                    } else {
                        sState = 4;
                        sSel = 0;
                    }
                }
                break;
            case 4:
                for (i = 0; i < updateRate; i++) {
                    sAngle += (0x8000 - sAngle) >> 4;
                }
                if (sAngle > 0x7A00) {
                    for (i = 0; i < updateRate; i++) {
                        sTitleY += (22.0f - sTitleY) * 0.25f;
                    }
                    sFade += updateRate * 4;
                    if (sFade > 0xFF) {
                        sFade = 0xFF;
                    }
                    sPairs[0][0] = 0x29;
                    sPairs[1][0] = 0x2A;
                    sPairs[2][0] = 0x2B;
                    sPairs[3][0] = -1;
                    sPairs[4][0] = -1;
                    sPairs[5][0] = -1;
                    sPairs[0][1] = 1;
                    sPairs[2][1] = 1;
                    if (settings_flag_get(&sSaves[sFile], 3) && D_800FB10C >= 2) {
                        sPairs[3][0] = 0x32;
                        sPairs[3][1] = 1;
                    }
                    if (empty == 0) {
                        sPairs[1][1] = 0;
                    } else {
                        sPairs[1][1] = 1;
                    }
                    if (settings_flag_get(&sSaves[sFile], 3) && sControllerConnected[1] != 0) {
                        if (right) {
                            switch (sSel) {
                                case 0:
                                    sSel = 3;
                                    break;
                                case 3:
                                    sSel = 1;
                                    break;
                                default:
                                    sSel++;
                                    if (sSel >= 3) {
                                        sSel = 0;
                                    }
                                    break;
                            }
                            if (sSel == 1 && empty == 0) {
                                sSel = 2;
                            }
                            moved = TRUE;
                        }
                        if (left) {
                            switch (sSel) {
                                case 3:
                                    sSel = 0;
                                    break;
                                case 1:
                                    sSel = 3;
                                    break;
                                case 2:
                                    if (empty == 0) {
                                        sSel = 3;
                                    } else {
                                        sSel = 1;
                                    }
                                    break;
                                case 0:
                                    sSel = 2;
                                    break;
                            }
                            moved = TRUE;
                        }
                    } else {
                        if (sSel == 3) {
                            sSel = 0;
                        }
                        if (right) {
                            sSel++;
                            if (sSel >= 3) {
                                sSel = 0;
                            }
                            if (sSel == 1 && empty == 0) {
                                sSel = 2;
                            }
                            moved = TRUE;
                        }
                        if (left) {
                            sSel--;
                            if (sSel < 0) {
                                sSel = 2;
                            }
                            moved = TRUE;
                            if (sSel == 1 && empty == 0) {
                                sSel = 0;
                            }
                        }
                    }
                    switch (sSel) {
                        case 0:
                            sprintf(buf, "%s %d", gMenuText[513], sFile + 1);
                            break;
                        case 1:
                            sprintf(buf, "%s %d", gMenuText[451], sFile + 1);
                            break;
                        case 2:
                            sprintf(buf, "%s %d", gMenuText[450], sFile + 1);
                            break;
                        case 3:
                            sprintf(buf, "%s %d", gMenuText[514], sFile + 1);
                            break;
                    }
                    draw_text(&D_800FF398, 160, 104, buf, 4);
                    draw_text(&D_800FF398, 160, 119, sSaves[sFile].name, 4);
                    if (input_pressed(sPlayer) & 0x8000) {
                        func_80001248(0x16C, 0);
                        switch (sSel) {
                            case 0:
                            case 3:
                                func_80001248(0x16C, 0);
                                dst = (u8 *) get_settings();
                                src = (u8 *) &sSaves[sFile];
                                for (n = sizeof(M042Save); n != 0; n--) {
                                    *dst++ = *src++;
                                }
                                if (sSel == 0) {
                                    D_800A4FC8 = 0;
                                } else {
                                    D_800A4FC8 = 1;
                                }
                                func_8004665C(0x7F, 0, 0, 5, 1, 0);
                                func_8005AAE8(1);
                                sState = 9;
                                break;
                            case 1:
                                sCopyFrom = sFile;
                                sSel = 0;
                                while (sSaves[sSel].slot != -1) {
                                    sSel++;
                                    if (sSel >= 6) {
                                        sSel = 0;
                                    }
                                }
                                sState = 5;
                                break;
                            case 2:
                                sErase = 0;
                                sState = 8;
                                break;
                        }
                    }
                    if (input_pressed(sPlayer) & 0x4000) {
                        func_80001248(0x16C, 0);
                        sState = 7;
                    }
                    if (sAngle > 0x7A00) {
                        for (i = 0; i < updateRate; i++) {
                            sTitleY += (22.0f - sTitleY) * 0.25f;
                        }
                    }
                }
                break;
            case 8:
                sFade += updateRate * 4;
                if (sFade > 0xFF) {
                    sFade = 0xFF;
                }
                sprintf(buf, "%s %d", gMenuText[450], sFile + 1);
                draw_text(&D_800FF398, 160, 104, buf, 4);
                draw_text(&D_800FF398, 160, 119, sSaves[sFile].name, 4);
                if (sErase != 0) {
                    set_text_colour(0, 0, 255, 255, (sPulse * sFade) >> 8);
                } else {
                    set_text_colour(0, 0, 255, 255, sFade);
                }
                draw_text(&D_800FF398, 125, 134, gMenuText[43], 4);
                if (sErase == 0) {
                    set_text_colour(0, 0, 255, 255, (sPulse * sFade) >> 8);
                } else {
                    set_text_colour(0, 0, 255, 255, sFade);
                }
                draw_text(&D_800FF398, 195, 134, gMenuText[35], 4);
                if (left || right) {
                    sErase ^= 1;
                }
                if (input_pressed(sPlayer) & 0x8000) {
                    func_80001248(0x16C, 0);
                    if (sErase != 0) {
                        mark_save_file_to_erase(sFile);
                        sSaves[sFile].slot = -1;
                    }
                    sState = 7;
                }
                if (input_pressed(sPlayer) & 0x4000) {
                    func_80001248(0x16C, 0);
                    sState = 7;
                }
                break;
            case 7:
                for (i = 0; i < updateRate; i++) {
                    sTitleY += (-20.0f - sTitleY) * 0.25f;
                }
                if (sTitleY < -19.95f) {
                    sSel = sFile;
                    sState = 0;
                }
                break;
            case 5:
                sFade += updateRate * 4;
                if (sFade > 0xFF) {
                    sFade = 0xFF;
                }
                if (sPairs[0][0] != 0x2C) {
                    for (i = 0; i < updateRate; i++) {
                        sTitleY += (-20.0f - sTitleY) * 0.25f;
                    }
                    if (sTitleY < -19.95f) {
                        for (i = 0, pair = sPairs[0]; i < 6; i++, pair += 2) {
                            pair[0] = i + 0x2C;
                            if (sSaves[i].slot == -1) {
                                pair[1] = 1;
                            } else {
                                pair[1] = 0;
                            }
                        }
                    }
                } else {
                    for (i = 0; i < updateRate; i++) {
                        sTitleY += (22.0f - sTitleY) * 0.25f;
                    }
                    sprintf(buf, "%s %d", gMenuText[451], sCopyFrom + 1);
                    draw_text(&D_800FF398, 160, 104, buf, 4);
                    draw_text(&D_800FF398, 160, 119, sSaves[sFile].name, 4);
                    sprintf(buf, "%s %d", gMenuText[452], sSel + 1);
                    draw_text(&D_800FF398, 160, 134, buf, 4);
                    if (right) {
                        sSel++;
                        if (sSel >= 6) {
                            sSel = 0;
                        }
                        while (sSaves[sSel].slot != -1) {
                            sSel++;
                            if (sSel >= 6) {
                                sSel = 0;
                            }
                        }
                        moved = TRUE;
                    }
                    if (left) {
                        sSel--;
                        moved = TRUE;
                        if (sSel < 0) {
                            sSel = 5;
                        }
                        while (sSaves[sSel].slot != -1) {
                            sSel--;
                            if (sSel < 0) {
                                sSel = 5;
                            }
                        }
                    }
                    if (input_pressed(sPlayer) & 0x8000) {
                        func_80001248(0x16C, 0);
                        sState = 2;
                        sCopying = 1;
                    } else if (input_pressed(sPlayer) & 0x4000) {
                        func_80001248(0x16C, 0);
                        sState = 6;
                    }
                }
                break;
            case 6:
                for (i = 0; i < updateRate; i++) {
                    sTitleY += (-20.0f - sTitleY) * 0.25f;
                }
                if (sTitleY < -19.95f) {
                    if (empty > 0) {
                        sState = 4;
                    } else {
                        sState = 0;
                    }
                    sSel = 0;
                }
                break;
            case 2:
                sFade = 0;
                sNameEmpty = 1;
                pair = sPairs[0];
                for (i = 0; i < 6; i++) {
                    *pair = -1;
                    pair += 2;
                }
                for (i = 0; i < updateRate; i++) {
                    sAngle += (0x1800 - sAngle) >> 4;
                }
                for (i = 0; i < 14; i++) {
                    sWave[i] += updateRate << 11;
                    if (sWave[i] > 0x1C000) {
                        sWave[i] = 0x1C000;
                    }
                }
                if (sWave[13] == 0x1C000) {
                    sCursor = sName;
                    *sCursor = '\0';
                    sState = 1;
                }
                break;
            case 1:
                if (left) {
                    sCursorKey--;
                    if (sCursorKey < 0) {
                        sCursorKey += KEY_COUNT;
                    }
                    moved = TRUE;
                }
                if (right) {
                    sCursorKey++;
                    if (sCursorKey >= KEY_COUNT) {
                        sCursorKey -= KEY_COUNT;
                    }
                    moved = TRUE;
                }
                if (up) {
                    moved = TRUE;
                    switch (sCursorKey) {
                        case 4:
                        case 5:
                            sCursorKey = 40;
                            break;
                        case 40:
                            sCursorKey = 35;
                            break;
                        default:
                            sCursorKey -= 10;
                            if (sCursorKey < 0) {
                                sCursorKey += 40;
                            }
                            break;
                    }
                }
                if (down) {
                    moved = TRUE;
                    switch (sCursorKey) {
                        case 34:
                        case 35:
                            sCursorKey = 40;
                            break;
                        case 40:
                            sCursorKey = 4;
                            break;
                        default:
                            sCursorKey += 10;
                            if (sCursorKey > 40) {
                                sCursorKey -= 40;
                            }
                            break;
                    }
                }
                len = sCursor - sName;
                if (input_pressed(sPlayer) & 0x8000) {
                    sNameEmpty = 0;
                    key = &sKeys[sCursorKey];
                    key->lift = 0.0f;
                    switch (key->ch) {
                        case 1:
                            if (len > 0) {
                                *--sCursor = '\0';
                                func_80001248(0x16C, 0);
                            } else {
                                func_80001248(0x16E, 0);
                            }
                            break;
                        case 2:
                            if (sCopying == 0) {
                                save = get_settings();
                                game_settings_reset(save);
                                sprintf(save->name, "%s", sName);
                                force_mark_write_save_file(sFile);
                                func_80001248(0x16C, 0);
                                func_8004665C(0x142, 0, 0, 0, 1, 0);
                                sState = 9;
                            } else {
                                save = get_settings();
                                dst = (u8 *) &sSaves[sSel];
                                dst2 = (u8 *) save;
                                src = (u8 *) &sSaves[sCopyFrom];
                                for (n = sizeof(M042Save); n != 0; n--) {
                                    *dst++ = *src;
                                    *dst2++ = *src++;
                                }
                                sprintf(sSaves[sSel].name, "%s", sName);
                                sprintf(save->name, "%s", sName);
                                sSaves[sSel].slot = sSel;
                                save->slot = sSel;
                                force_mark_write_save_file(sSel);
                                sSel = sCopyFrom;
                                sState = 0;
                            }
                            break;
                        default:
                            if (len < 16) {
                                *sCursor = key->ch;
                                *++sCursor = '\0';
                                func_80001248(0x16C, 0);
                            } else {
                                func_80001248(0x16E, 0);
                            }
                            break;
                    }
                }
                for (i = 0; i < updateRate; i++) {
                    sAngle += (0x1800 - sAngle) >> 4;
                }
                if ((input_pressed(sPlayer) & 0x4000) && sState == 1) {
                    sSel = sFile;
                    sState = 0;
                }
                break;
        }
        if (sAngle < 0x7A00) {
            D_800FF780[52].unk2 = sAngle;
            keyObj = D_800FF3F8[150];
            gDPSetFogColor(D_800FF398++, 0, 40, 40, 164);
            gSPFogFactor(D_800FF398++, 0x280, -0x180);
            key = sKeys;
            row = 0;
            for (i = 0; i < KEY_COUNT; i++, key++) {
                keyObj->modelIndex = key->model;
                if (i == sCursorKey && sState == 1) {
                    target = 8.0f;
                    gSPFogFactor(D_800FF398++, 0x19, 0x19);
                } else {
                    target = 0.0f;
                }
                if (sState == 2) {
                    if (i == 10 || i == 20 || i == 30) {
                        row++;
                    }
                    if (i == 40) {
                        phase = sWave[7];
                    } else {
                        phase = sWave[(i % 10) + row];
                    }
                    if (phase < 0) {
                        key->lift = 8.0f;
                    } else {
                        key->lift = func_80049C34(phase) * 8.0f + 8.0f;
                    }
                } else {
                    for (phase = 0; phase < updateRate; phase++) {
                        key->lift += (target - key->lift) * 0.25f;
                    }
                }
                func_80049BA0(&mtx);
                func_80049A30(-sAngle, &mtx);
                func_80048B60(&mtx, 0.0f, 0.0f, key->lift, &x, &y, &z);
                joints = inst->matrices[inst->matrixIndex] + key->joint;
                item->x = joints[0][3][0] + x;
                item->y = joints[0][3][1] + y;
                item->z = joints[0][3][2] + z;
                func_8005A0D0(0x34);
                if (i == sCursorKey && sState == 1) {
                    gSPFogFactor(D_800FF398++, 0x280, -0x180);
                }
            }
        }
        func_80049BA0(&mtx);
        func_80049A30(-sAngle, &mtx);
        func_80048B60(&mtx, 0, sTitleY, 0.0f, &x, &y, &z);
        joints = inst->matrices[inst->matrixIndex] + 2;
        D_800FF780[52].x = joints[0][3][0] + x;
        D_800FF780[52].y = joints[0][3][1] + y + 9.0f;
        D_800FF780[52].z = joints[0][3][2] + z;
        D_800FF780[52].unk2 = 0x8000 - sAngle;
        saved = D_800FF780[52].unk8;
        D_800FF780[52].unk8 = 0.38f;
        keyObj = D_800FF3F8[150];
        gDPSetFogColor(D_800FF398++, 0, 40, 40, 164);
        pair = sPairs[0];
        if (*pair == 0x2C) {
            item->unk0 = 0;
        }
        for (i = 0; i < 6; i++, pair += 2) {
            if (pair[0] != -1) {
                keyObj->modelIndex = pair[0];
                if (pair[1] != 0) {
                    D_800A51CC = 255;
                } else {
                    D_800A51CC = 64;
                }
                if (i == sSel) {
                    gSPFogFactor(D_800FF398++, 0x19, 0x19);
                } else {
                    gSPFogFactor(D_800FF398++, 0x280, -0x180);
                }
                func_8005A0D0(0x34);
            }
        }
        D_800A51CC = 255;
        item->unk8 = saved;
        item->unk0 = -0x8000;
        set_text_colour(0, 0, 255, 255, 255);
        switch (sState) {
            case 1:
            case 2:
                if (sNameEmpty != 0) {
                    draw_text(&D_800FF398, 160, 48, gMenuText[453], 4);
                } else {
                    draw_text(&D_800FF398, 160, 48, sName, 4);
                }
                break;
            case 0:
                draw_text(&D_800FF398, 160, 48, gMenuText[454], 4);
                break;
            case 4:
                draw_text(&D_800FF398, 160, 48, gMenuText[455], 4);
                break;
            case 8:
                draw_text(&D_800FF398, 160, 48, gMenuText[450], 4);
                break;
            case 5:
                draw_text(&D_800FF398, 160, 48, gMenuText[451], 4);
                break;
        }
        if (moved) {
            func_80001248(0x16B, 0);
        }
        cam_set_fov(52.0f, 1);
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m042/ro", func_m042_00178);
#endif

s32 *func_m042_02C80(void) {
    return sList;
}

/* Does nothing (called by 00178) */
#ifdef NON_MATCHING
static
#endif
void func_m042_02C8C(UNUSED s32 arg0) {
}

#ifndef NON_MATCHING
/* (.rodata: C, and the assembly functions' own from ro/) */
#endif
