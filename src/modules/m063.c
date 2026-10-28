/*
 * Runlink module 63: the start screen of the front end. The game's title
 * fades in over a 3D logo that pulses in and out, with a blinking
 * "PRESS START"; pressing A or Start brings up three choices (one player,
 * multiplayer, options), picked with the stick, which then start the game,
 * the multiplayer setup or the options menu.
 */
#include "module.h"
#include "gfx.h"

/* An image drawn by texrect_draw / texrect_draw_scaled (a NULL texture ends a list) */
typedef struct M063Image {
    /* 0x0 */ void *texture;
    /* 0x4 */ s32 unk4;
    /* 0x8 */ s16 xOffset;
    /* 0xA */ s16 yOffset;
} M063Image;

/* One of the choices */
typedef struct M063Option {
    /* 0x0 */ s16 x;
    /* 0x2 */ s16 y;
    /* 0x4 */ M063Image *image;
    /* 0x8 */ char *text;
} M063Option;

/* What func_800416CC draws the logo with (laid out like the start of an object) */
typedef struct M063Sprite {
    /* 0x00 */ ObjectTransform trans;
    /* 0x18 */ u8 pad18[0x28 - 0x18];
    /* 0x28 */ f32 unk28;
    /* 0x2C */ u8 pad2C[0x38 - 0x2C];
} M063Sprite;

extern Gfx *D_800FF398; /* the display list being built */
extern Mtx *D_800FF39C;
extern Vertex *D_800FF3A0;
extern void *D_800FF3F8[]; /* the loaded menu textures, by asset number */
extern s32 D_800FF3C8[];   /* the buttons pressed this frame, per player */
extern s8 D_800FF3D8[];    /* the stick moves this frame, per player */
extern s8 D_800FF1B0[];    /* which players take part (multiplayer) */
extern s32 D_800FB10C;     /* the number of controllers */
extern u8 D_800A505C;
extern u8 D_800A5060; /* the choice */
extern u8 D_800A4FC4;
extern u8 D_800A4FC8;
extern u8 D_800A4FCC;
extern u8 D_800A4FD0;

extern void bgdraw_fillcolour(s32 r, s32 g, s32 b);
extern void input_assign_players(void);
extern void music_play(s32 seq);
extern void music_volume_set(s32 volume);
extern void func_800005F8(f32 volume, s32 arg1); /* fades the music */
extern s32 func_80047C0C(void);
extern void func_800476F0(s32 arg0);
extern void func_800487B8(void);
extern void func_80047608(s32 arg0, s32 arg1, s32 arg2);
extern void func_8004665C(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5);
extern void func_80059E1C(s16 *assets); /* loads menu textures */
extern void func_80059CCC(s16 *assets); /* frees them */
extern s32 func_80055180(void);
extern void mtx_ortho(Gfx **dList, Mtx **mtx);
extern void func_80055C00(Gfx **dList, void *tex, s32 arg2, s32 arg3);
extern void rendermode_reset(Gfx **dList);
extern void func_80056000(s32 arg0);
extern ObjectTransform *cam_get_cameras(void);
extern void func_800416CC(Gfx **dList, Mtx **mtx, Vertex **vtx, M063Sprite *sprite, void *tex, s32 arg5, s32 alpha);
extern void func_8004EDBC(Gfx **dList, M063Image *image, s32 x, s32 y, u8 r, u8 g, u8 b, s32 alpha); /* DKR texrect_draw */
extern void func_8004F02C(Gfx **dList, M063Image *image, f32 x, f32 y, f32 xScale, f32 yScale, u32 colour,
                          s32 flags); /* DKR texrect_draw_scaled */
extern void set_text_font(s32 font);
extern void set_text_background_colour(s32 red, s32 green, s32 blue, s32 alpha);
extern void set_text_colour(s32 red, s32 green, s32 blue, s32 alpha, s32 opacity);
extern void draw_text(Gfx **dList, s32 x, s32 y, char *text, s32 alignment);

static char sPressStart[] = "PRESS START";
static char sNewGame[] = "NEW GAME";
static char sContinue[] = "CONTINUE EXISTING GAME";
static s16 sOtherAssets[] = { 0x80, 0x180, 0x280, 0x380, 0x480, 0x580, -1 };
static Object *sObject = NULL;
static s16 sAssets[] = { 0x66, 0x67, 0x68, 0x69, 0x93, 0x94, -1 };
static M063Image sTitleImage[] = { { NULL, -22, -22, 0 }, { NULL, 0, 0, 0 } };
static M063Image sPlayImage[] = { { NULL, -22, -22, 0 }, { NULL, 0, 0, 0 } };
static M063Image sMultiImage[] = { { NULL, -22, -22, 0 }, { NULL, 0, 0, 0 } };
static M063Image sOptionsImage[] = { { NULL, -22, -22, 0 }, { NULL, 0, 0, 0 } };
static M063Option *sOptions = NULL;
static Vertex sBackVertices[] = {
    { -160, 120, 1, 0, 0, 0, 255 }, { 0, 120, 1, 0, 0, 0, 255 },  { 160, 120, 1, 0, 0, 0, 255 },
    { -160, 0, 1, 0, 0, 0, 255 },   { 0, 0, 1, 0, 0, 0, 255 },    { 160, 0, 1, 0, 0, 0, 255 },
    { -160, -120, 1, 0, 0, 0, 255 }, { 0, -120, 1, 0, 0, 0, 255 }, { 160, -120, 1, 0, 0, 0, 255 },
};
static s32 sUnusedD74 = 0;
static Triangle sBackTriangles[] = {
    { 0, 0, 3, 1, { { 0x9E0, 0x760 }, { 0x9E0, 0 }, { 0, 0x760 } } },
    { 0, 1, 3, 4, { { 0, 0x760 }, { 0x9E0, 0 }, { 0, 0 } } },
    { 0, 1, 4, 2, { { 0, 0x760 }, { 0, 0 }, { 0x9E0, 0x760 } } },
    { 0, 2, 4, 5, { { 0x9E0, 0x760 }, { 0, 0 }, { 0x9E0, 0 } } },
    { 0, 3, 6, 4, { { 0x9E0, 0 }, { 0x9E0, 0x760 }, { 0, 0 } } },
    { 0, 4, 6, 7, { { 0, 0 }, { 0x9E0, 0x760 }, { 0, 0x760 } } },
    { 0, 4, 7, 5, { { 0, 0 }, { 0, 0x760 }, { 0x9E0, 0 } } },
    { 0, 5, 7, 8, { { 0x9E0, 0 }, { 0, 0x760 }, { 0x9E0, 0x760 } } },
};
static Vertex sQuadVertices[] = {
    { 0, 0, 0, 255, 64, 0, 160 }, { 0, 0, 0, 255, 64, 0, 160 }, { 0, 0, 0, 255, 64, 0, 160 },
    { 0, 0, 0, 255, 64, 0, 160 }, { 0, 0, 0, 255, 64, 0, 160 }, { 0, 0, 0, 255, 64, 0, 160 },
};
static s32 sUnusedE34 = 0;
static Triangle sQuadTriangles[] = {
    { 0x40, 0, 1, 2, { { 0, 0 }, { 0, 0 }, { 0, 0 } } },
    { 0x40, 3, 4, 5, { { 0, 0 }, { 0, 0 }, { 0, 0 } } },
};
static char sPlayText[] = "PLAY";
static char sMultiText[] = "MULTI";
static char sOptionsText[] = "OPTIONS";
static M063Option sOptionTable[] = {
    { 88, 172, sPlayImage, sPlayText },
    { 160, 172, sMultiImage, sMultiText },
    { 232, 172, sOptionsImage, sOptionsText },
};

static s32 sLastOption;
static s32 sPressed;
static s32 sUnused8;
static s32 sStage;
static s32 sTextMode;
static s32 sAngle;
static s32 sFadeIn;
static s32 sTextFade;
static s32 sPulseTimer;
static s32 sUnused24;
static s32 sPulse;
static s32 sPulseStep;

void func_m063_00000(void) {
    s32 i;

    bgdraw_fillcolour(0, 0, 0);
    input_assign_players();
    for (i = 0; i < 4; i++) {
        D_800FF1B0[i] = 0;
    }
    sLastOption = 2;
    sOptions = sOptionTable;
    sStage = 0;
    sTextMode = 3;
    sPressed = 0;
    D_800A5060 = 0;
    sAngle = 0xC000;
    sTextFade = 0;
    sPulse = 0;
    sPulseStep = 4;
    sPulseTimer = 360;
    func_80059E1C(sAssets);
    sTitleImage[0].texture = D_800FF3F8[0x66];
    sPlayImage[0].texture = D_800FF3F8[0x68];
    sMultiImage[0].texture = D_800FF3F8[0x69];
    sOptionsImage[0].texture = D_800FF3F8[0x67];
    music_play(0xB);
    music_volume_set(0x7F);
    D_800A4FC4 = 0;
    D_800A4FC8 = 0;
    D_800A4FD0 = 1;
    func_800476F0(func_80047C0C());
}

void func_m063_00140(s32 updateRate) {
    s32 pressed;
    s32 backPressed;
    s32 i; /* also the alpha of the title */
    s32 colour;
    s32 hiRes;
    s32 textY;
    M063Sprite sprite;

    func_80047C0C();
    hiRes = func_80055180() & 1;
    if (hiRes) {
        textY = 0xB0;
    } else {
        textY = 0xAC;
    }
    pressed = 0;
    backPressed = 0;
    if (D_800A505C == 0) {
        i = 1;
        while (i--) {
            pressed |= D_800FF3C8[i] & (A_BUTTON | START_BUTTON);
            backPressed |= D_800FF3C8[i] & B_BUTTON;
        }
        func_800487B8();
    }
    switch (sStage) {
        case 0:
            if (pressed) {
                sPressed = 1;
            }
            if (sPressed) {
                sFadeIn += updateRate * 8;
                if (sFadeIn > 0xFF) {
                    sFadeIn = 0xFF;
                }
                if (sFadeIn >= 0xC0) {
                    sTextFade += updateRate * 16;
                    sTextMode = 1;
                    if (sTextFade >= 0xFF) {
                        sTextFade = 0xFF;
                        sStage = 2;
                    }
                }
            } else {
                sFadeIn += updateRate;
                if (sFadeIn > 0xFF) {
                    sFadeIn = 0xFF;
                }
                if (sFadeIn >= 0xC0) {
                    sTextFade += updateRate * 2;
                    sTextMode = 0;
                    if (sTextFade >= 0xFF) {
                        sTextFade = 0xFF;
                        sStage = 1;
                    }
                }
            }
            break;
        case 1:
            sAngle += updateRate << 10;
            if (sPulseTimer != 0) {
                sPulseTimer -= updateRate;
                if (sPulseTimer <= 0) {
                    sPulseTimer = 0;
                }
            } else {
                sPulse += sPulseStep * updateRate;
                if (sPulse >= 0xFF) {
                    sPulseTimer = 360;
                    sPulse = 0xFF;
                    sPulseStep = -4;
                } else if (sPulse <= 0) {
                    sPulseTimer = 360;
                    sPulse = 0;
                    sPulseStep = 4;
                }
            }
            if (pressed) {
                sStage = 2;
                sTextMode = 1;
            }
            break;
        case 2:
            if (pressed) {
                if (D_800A5060 == 1) {
                    i = 1;
                    while (i--) {
                        if (D_800FF3C8[i] & (A_BUTTON | START_BUTTON)) {
                            D_800FF1B0[i] = 1;
                        }
                    }
                    sStage = 3;
                } else if (sLastOption == D_800A5060) {
                    func_8004665C(0, 0, 0, 4, 1, 0);
                    func_800005F8(1.0f, 0);
                    sStage = 4;
                } else {
                    D_800A4FC4 = 0;
                    D_800A4FD0 = 1;
                    D_800A4FCC = 1;
                    func_80047608(0x18, 0, 0);
                    sStage = 4;
                }
                func_80001248(0x16D, 0);
            } else if (D_800A505C == 0) {
                i = 1;
                while (i--) {
                    if (D_800FF3D8[i] > 0) {
                        if (D_800A5060 < sLastOption) {
                            D_800A5060++;
                            func_80001248(0x16B, 0);
                        } else {
                            func_80001248(0x16E, 0);
                        }
                        if (D_800A5060 == 1 && D_800FB10C < 2) {
                            D_800A5060 = 2;
                        }
                    } else if (D_800FF3D8[i] < 0) {
                        if (D_800A5060 > 0) {
                            D_800A5060--;
                            func_80001248(0x16B, 0);
                        } else {
                            func_80001248(0x16E, 0);
                        }
                    }
                }
                if (D_800A5060 == 1 && D_800FB10C < 2) {
                    D_800A5060 = 0;
                }
            }
            break;
        case 3:
            sFadeIn = 0;
            if (sFadeIn <= 0) {
                D_800A4FD0 = 0;
                i = 4;
                while (i--) {
                    if (D_800FF1B0[i] == 1) {
                        D_800A4FD0++;
                    }
                }
                D_800A4FC4 = 1;
                D_800A4FC8 = 0;
                D_800A4FCC = 1;
                func_8004665C(0xA1, 0, 0, 6, 1, 0);
                sStage = 4;
            }
            break;
        case 4:
            break;
    }
    mtx_ortho(&D_800FF398, &D_800FF39C);
    func_80055C00(&D_800FF398, D_800FF3F8[0x93], 4, 0);
    gDPSetOtherMode(D_800FF398++, 0x082C0F, 0x504340);
    gDPSetPrimColor(D_800FF398++, 0, 0, 255, 255, 255, 255);
    gSPVertexJFG(D_800FF398++, OS_K0_TO_PHYSICAL(sBackVertices), 10, 0);
    gSPPolygon(D_800FF398++, OS_K0_TO_PHYSICAL(sBackTriangles), 8, 1);
    rendermode_reset(&D_800FF398);
    set_text_background_colour(0, 0, 0, 0);
    set_text_font(2);
    switch (sTextMode) {
        case 0:
            if (sPulse != 0xFF) {
                if (!hiRes) {
                    func_80056000(0);
                }
                sprite.trans.rotation.y_rotation = 0;
                sprite.trans.rotation.x_rotation = 0;
                sprite.trans.rotation.z_rotation = -cam_get_cameras()->rotation.z_rotation;
                sprite.trans.x_position = -4.0f;
                sprite.trans.y_position = 100 - textY;
                sprite.trans.z_position = 1.0f;
                sprite.trans.scale = 1.0f;
                i = ((0xFF - sPulse) * sTextFade) / 255;
                sprite.unk28 = 0.0f;
                gDPPipeSync(D_800FF398++);
                gDPSetPrimColor(D_800FF398++, 0, 0, 255, 255, 255, i);
                func_800416CC(&D_800FF398, &D_800FF39C, &D_800FF3A0, &sprite, D_800FF3F8[0x94], 0, i);
                gDPPipeSync(D_800FF398++);
                gDPSetPrimColor(D_800FF398++, 0, 0, 255, 255, 255, 255);
                func_80056000(1);
            }
            if (sPulse != 0) {
                i = (sPulse * sTextFade) / 255;
                if (!hiRes) {
                    func_8004EDBC(&D_800FF398, sTitleImage, 160, textY, 255, 255, 255, i);
                } else {
                    func_8004F02C(&D_800FF398, sTitleImage, 160.0f, textY, 0.75f, 1.0f, i | ~0xFF, 0);
                }
                set_text_colour(255, 128, 128, 255, (((s32) (func_80049C34(sAngle) * 127.0f) + 128) * i) / 255);
                draw_text(&D_800FF398, 160, textY + 52, sPressStart, 12);
            }
            break;
        case 1:
            i = sLastOption + 1;
            while (i--) {
                if (i == D_800A5060) {
                    colour = 0xFF;
                } else if (i == 1 && D_800FB10C < 2) {
                    colour = 0x40; /* multiplayer needs two controllers */
                } else {
                    colour = 0x80;
                }
                if (!hiRes) {
                    func_8004EDBC(&D_800FF398, sOptions[i].image, sOptions[i].x, sOptions[i].y, colour, colour,
                                  colour, sTextFade);
                } else {
                    func_8004F02C(&D_800FF398, sOptions[i].image, sOptions[i].x, sOptions[i].y, 0.75f, 1.0f,
                                  (colour << 24) | (colour << 16) | (colour << 8) | sTextFade, 0);
                }
                set_text_colour(colour, colour, 255, 255, sTextFade);
                draw_text(&D_800FF398, sOptions[i].x, sOptions[i].y + 52, sOptions[i].text, 12);
            }
            break;
        case 3:
            break;
    }
}

void func_m063_00C14(void) {
    func_80059CCC(sAssets);
    if (sObject != NULL) {
        func_800079B8(sObject);
        sObject = NULL;
    }
}
