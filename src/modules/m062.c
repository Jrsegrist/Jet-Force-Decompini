/*
 * Runlink module 62: the multiplayer mode select of the front end (the
 * choice of game mode and its settings).
 */
#include "module.h"

extern s32 *gMenuText; /* the settings, read here as words */
extern void menu_assetgroup_load(s16 *assets); /* loads menu textures */
extern void menu_assetgroup_reset(s16 *assets);
extern void menu_assetgroup_free(s16 *assets); /* frees menu textures */
extern void music_play(s32 seq);

#define SETTING(offset) (gMenuText[(offset) / 4])

/* A game mode that needs a bit of D_800FF318's mode masks */
typedef struct M062Mode {
    /* 0x0 */ u16 mask;
    /* 0x2 */ s16 mode;
    /* 0x4 */ s32 arg;
} M062Mode;

typedef struct M062Rule {
    /* 0x0 */ u16 mask;
    /* 0x2 */ s16 mode;
} M062Rule;

typedef struct M062TeamMode {
    /* 0x0 */ u16 mask;
    /* 0x2 */ s16 mode;
    /* 0x4 */ u8 flag;
} M062TeamMode;

/* The render mode for the page's images (SetCombine, SetOtherMode) */
static u32 sRenderMode[] = { 0xFC71FEE3, 0xFFFEFB7D, 0xEF000C0F, 0x00504240 };
static s16 sAssets2[] = { 0x43, 0x44, -1, 0 };
static s16 sImages[] = { 0x1F, 0x27, 0x28, 0x2D, -1, 0 };
static s16 sAssets[] = { 0x65, 0x7C, 0x81, -1 };
static s8 sBoxHeights[] = { 16, 16, 16, 16, 16, 16, 16, 7, 7 }; /* per page: the box's half height */
static s32 sPage = 8; /* the setting being changed */
static s8 sLightIds[8] = { 6, 0, 1, 7, 2, 8, 4, 10 };
static s8 sCursor[8] = { 0 };
static s8 sTable4C[8] ={ 0, 1, 3, 2, 0, 1, 0, 0 };
static s8 sRowIds[8] = { 2, 7, 3, 8, 4, 9, 5, 10 };
static s8 sTeamCursor[4] = { 5, 4, 4, 5 };
static s8 sRuleCursor[4] = { 1, 3, 2, 0 };
static s8 sTeamCursor2[4] = { 4, 6, 0, 0 };
static M062Rule sRules[8] = {
    { 0, 0x6E }, { 0, 0x91 }, { 0, 0xC8 }, { 0, 0x164 }, { 1, 0xDD }, { 2, 0x90 }, { 4, 0x163 }, { 8, 0x10E },
};
static s16 sRuleTexts[4] = { 0x149, 0x14E, 0x14C, 0 };
static s32 sRuleKinds[3] = { 0x10, 0x20, 0x30 };
static M062TeamMode sTeamModes[4] = { { 1, 0x10C, 0 }, { 2, 0x102, 1 }, { 4, 0x18D, 1 }, { 0x10, 0xCA, 0 } };
static s16 sTeamTexts[2] = { 0x155, 0x149 };
static M062Mode sModes[] = { { 1, 0xFB, 0x1518 }, { 2, 0x16C, 0x189C }, { 4, 0x198, 0xE10 } };
static s16 sTimeLimits[] = { -1, 40, 60, 80, 100, 0 };
static u32 sTablesDC[] = { 0xFFFF0001, 0x00030005, 0x000A0000, 0x090C0E00 };
/* The texts of each mode's rows (-1: no row) */
static s16 sRowTexts[3][8] = {
    { 0x140, 0x149, 0x209, 0x150, 0x144, -1, 0x145, 0x17A },
    { 0x140, 0x156, 0x144, 0x158, 0x20A, -1, 0, 0x17A },
    { 0x140, -1, 0x144, -1, 0x15F, -1, 0x165, 0x17A },
};
static s16 sAngle = 0;
static u32 sTail[4] = { 0 };

static s32 sUnk0;
static s16 sUnk4;
static s16 sUnk6;
static f32 sUnk8;
static s32 sUnkC;
static s32 sUnk10;
static s32 sUnk14;
static s32 sUnk18;
static s32 sUnk1C;
static u32 sUnk20;
static u32 sUnk24;
static s32 sUnk28;
static s32 sUnk2C;
static s32 sUnk30;
static s32 sUnk34;
static s32 sUnk38;
static s32 sUnk3C;
static s32 sUnk40;
static s32 sUnk44;
static s32 sUnk48;
static s32 sUnk4C;
/* Copies of settings */
static s32 sSaved50[4];
static s32 sSaved60[4];
static s32 sSaved70[8];
static s32 sSaved90[4];
static s32 sSavedA0[2];
static s32 sSavedA8[4];
static s32 sSavedB8[2];
static s32 sSavedC0[2];
static s32 sSavedC8[3];

void func_m062_00000(void) {
    menu_assetgroup_load(sAssets);
    menu_assetgroup_reset(sImages);
    menu_assetgroup_load(sAssets2);
    sSaved50[0] = SETTING(0x5A0);
    sSaved50[1] = SETTING(0x5A4);
    sSaved50[2] = SETTING(0x5A8);
    sSaved60[0] = SETTING(0x504);
    sSaved60[1] = SETTING(0x508);
    sSaved60[2] = SETTING(0x50C);
    sSaved70[0] = SETTING(0x5AC);
    sSaved70[1] = SETTING(0x5B0);
    sSaved70[2] = SETTING(0x5B4);
    sSaved70[3] = SETTING(0x6FC);
    sSaved70[4] = SETTING(0x5B8);
    sSaved70[5] = SETTING(0x5BC);
    sSaved70[6] = SETTING(0x6F8);
    sSaved70[7] = SETTING(0x6F4);
    sSaved90[0] = SETTING(0x518);
    sSaved90[1] = SETTING(0x51C);
    sSaved90[2] = SETTING(0x520);
    sSavedA0[0] = SETTING(0x544);
    sSavedA0[1] = SETTING(0x548);
    sSavedA8[0] = SETTING(0x54C);
    sSavedA8[1] = SETTING(0x550);
    sSavedA8[2] = SETTING(0x82C);
    sSavedA8[3] = SETTING(0x830);
    sSavedB8[0] = SETTING(0x554);
    sSavedB8[1] = SETTING(0x504);
    sSavedC0[0] = SETTING(0x280);
    sSavedC0[1] = SETTING(0x5AC);
    sSavedC8[0] = SETTING(0x544);
    sSavedC8[1] = SETTING(0x598);
    sSavedC8[2] = SETTING(0x59C);
    sUnkC = 0;
    sUnk10 = -1;
    sUnk4 = 0x36;
    sUnk8 = 7.0f;
    sUnk4C = 0;
    music_play(0x10);
    sUnk18 = 0;
}

/* DKR's gDkrDmaDisplayList (f3ddkr.h): runs a display list of numberOfCommands commands */
#define G_DMADL 0x07
#define gDkrDmaDisplayList(pkt, address, numberOfCommands)                         \
    {                                                                              \
        Gfx *_g = (Gfx *) (pkt);                                                   \
                                                                                   \
        _g->words.w0 = (_SHIFTL(G_DMADL, 24, 8) | _SHIFTL((numberOfCommands), 16, 8) | \
                        _SHIFTL((numberOfCommands * 8), 0, 16));                   \
        _g->words.w1 = (unsigned int) (address);                                   \
    }

/* The page's 3D model (the box of rows), as used here: its vertices, lights and vertex batches */
typedef struct M062Vertex {
    /* 0x0 */ u8 pad0[6];
    /* 0x6 */ u8 r;
    /* 0x7 */ u8 g;
    /* 0x8 */ u8 b;
    /* 0x9 */ u8 a;
} M062Vertex;

typedef struct M062Light {
    /* 0x0 */ s16 a;
    /* 0x2 */ s16 b;
    /* 0x4 */ u32 id;
} M062Light;

typedef struct M062Batch {
    /* 0x00 */ u8 pad0;
    /* 0x01 */ s8 kind;
    /* 0x02 */ u8 pad2[4];
    /* 0x06 */ s16 start; /* the first vertex (the next batch's start ends it) */
    /* 0x08 */ u8 pad8[8];
} M062Batch;

typedef struct M062LightDef {
    /* 0x00 */ u8 pad0[0x16];
    /* 0x16 */ s16 scale;
} M062LightDef;

typedef struct M062LightSlot {
    /* 0x0 */ M062LightDef *def;
    /* 0x4 */ s32 unk4;
} M062LightSlot;

typedef struct M062Model {
    /* 0x00 */ u8 pad0[0x16];
    /* 0x16 */ s16 numBatches;
    /* 0x18 */ M062LightSlot *lights;
    /* 0x1C */ u8 pad1C[0x24 - 0x1C];
    /* 0x24 */ M062Batch *batches;
    /* 0x28 */ u8 pad28[4];
    /* 0x2C */ u8 numLights;
} M062Model;

typedef struct M062Instance {
    /* 0x00 */ M062Model *model;
    /* 0x04 */ u8 pad4[6];
    /* 0x0A */ s8 updateRate;
    /* 0x0B */ s8 buffer;
    /* 0x0C */ u8 padC[0xC];
    /* 0x18 */ M062Vertex *vertices[2];
    /* 0x20 */ u8 pad20[0x54];
    /* 0x74 */ M062Light *lights;
    /* 0x78 */ s16 *lightOut[2];
} M062Instance;

typedef struct M062MenuItem {
    /* 0x00 */ u8 pad0[0x10];
    /* 0x10 */ f32 unk10;
    /* 0x14 */ u8 pad14[0xC];
} M062MenuItem;

typedef struct M062Player {
    /* 0x00 */ u8 pad0[0x12];
    /* 0x12 */ u8 unk12;
    /* 0x13 */ u8 pad13[0xA0 - 0x13];
} M062Player;

extern s8 D_800A500C;   /* the game mode picked */
extern u8 D_800FF318[]; /* [0x6B]: the game modes that can be picked */
extern s16 D_800A4FE8;
extern s32 D_800A4FDC;
extern s8 D_800A5010;   /* the time limit picked */
extern s8 D_800A5018;
extern s8 D_800A5020;
extern s8 D_800A5058;
extern s16 D_800FF1B8[4];
extern s32 gMenuTimer;
extern s8 D_800A4FD8;
extern s8 D_800A4FEC;
extern Gfx *D_800FF398;
extern Mtx *D_800FF39C;
extern Object *D_800FF3F8[];
extern M062MenuItem D_800FF780[];
extern s32 D_800FF3C8[]; /* the buttons pressed this frame, per player */
extern s32 D_800FF3B8[];
extern s8 D_800FF3D8[]; /* the stick moves this frame, per player */
extern s8 D_800FF3DC[];
extern u8 D_800A4FD0; /* how many players joined */
extern u8 D_800A506C; /* the page: 0, 1 or 2 */
extern s8 D_800A4FF4;
extern s8 D_800A4FF8;
extern s8 D_800A4FFC;
extern s8 D_800A5000;
extern s8 D_800A5004;
extern s8 D_800A5008;
extern u8 D_800A5064;
extern s8 D_800A4FE0;
extern s16 D_800A4FE4;
extern s8 D_800A4FF0;
extern s8 D_800A5024;
extern s8 D_800A5028;
extern s8 D_800A502C;
extern s8 D_800A5030;
extern s8 D_800A5034;
extern s8 D_800A5038;
extern s8 D_800A503C;
extern s8 D_800A4FD4;
extern M062Player D_800FED50[];
extern void func_8006FCA8(Gfx **dl, s32 textX, s32 textY, char *text, s32 flags);
extern s32 func_800639E8(char *buf, char *fmt, ...); /* sprintf */
extern void music_volume_fade(f32 volume, s32 arg1);
extern void func_80047A20(void);
extern void func_800468C0(s32 arg0);
extern void func_8004665C(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5);
extern void func_800468A4(s32 arg0);
extern void func_80047580(s32 arg0);
extern void fb_size(u32 *width, u32 *height);
extern void func_8004DFC4(Gfx **dl, Mtx **mtx, s32 arg2); /* draws the background */
extern void func_80055B80(Gfx **dl);                     /* resets the render settings */
extern void set_active_camera(s32 camera);
extern void cam_fov_scale_set(f32 x, f32 y);
extern void func_8003F7F4(f32 fov, s32 arg1); /* sets the field of view */
extern void viewport_apply(Gfx **dl, s32 arg1);
extern void func_80040E64(Gfx **dl, Mtx **mtx);
extern void func_8005A0D0(s32 item); /* draws a menu item */
extern void func_80055C00(Gfx **dl, void *tex, s32 flags, s32 arg3);
extern void func_8006FC54(s32 font);
extern void func_8006FC68(s32 r, s32 g, s32 b, s32 a, s32 opacity);
extern void load_first_unlocked_level(void);
extern void viewport_scissor(Gfx **dl);
extern void func_m038_00000(void);

#define MODE_MASK (*(u16 *) &D_800FF318[0x68])
static void func_m062_01158(s32 updateRate);
static void func_m062_01A48(s32 updateRate);
static void func_m062_0228C(s32 updateRate);

/* Draws the mode select, reads the players' input and runs the page picked */
void func_m062_001E4(s32 updateRate) {
    f32 target;
    s32 id;
    s32 i;
    s32 j;
    s32 k;
    s32 kind;
    s32 colour;
    s32 n;
    M062Vertex *v;
    s32 left;
    s32 top;
    s32 right;
    s32 bottom;
    M062Light *light;
    s16 *out;
    M062LightDef *def;
    UNUSED s32 pad;
    M062Batch *batch;
    M062Instance *inst;
    M062Model *model;
    UNUSED s32 pad2;
    Object *obj;

    if (sUnk18 < 30) {
        sUnk18 += updateRate;
    }
    target = sBoxHeights[sPage];
    for (i = 0; i < updateRate; i++) {
        sUnk8 += (target - sUnk8) * 0.25f;
    }
    fb_size(&sUnk20, &sUnk24);
    sUnk20 >>= 1;
    sUnk24 >>= 1;
    func_8004DFC4(&D_800FF398, &D_800FF39C, 0);
    func_80055B80(&D_800FF398);
    gDPSetFogColor(D_800FF398++, 255, 255, 255, 255);
    gSPFogFactor(D_800FF398++, 0x6400, 0x9867);
    gDPPipeSync(D_800FF398++);
    func_m038_00000();
    set_active_camera(0);
    cam_fov_scale_set(1.0f, 1.0f);
    func_8003F7F4(52.0f, 1);
    viewport_apply(&D_800FF398, 0);
    left = sUnk20 - sUnk4;
    top = sUnk24 - (s32) sUnk8;
    right = sUnk4 + sUnk20;
    bottom = (s32) sUnk8 + sUnk24;
    obj = D_800FF3F8[0x81];
    inst = *obj->modelInstances;
    model = inst->model;
    inst->updateRate = updateRate;
    func_8001138C(obj, 0.01f, updateRate);
    sUnk28 = sUnk2C = sUnk34 = sUnk38 = sUnk3C = sUnk40 = sUnk44 = sUnk48 = 0;
    if (D_800A5058 == 0 && obj->unk28 >= 1.0f) {
        for (i = 0; i < D_800A4FD0; i++) {
            sUnk28 |= D_800FF3C8[i] & 0x9000;
            sUnk2C |= D_800FF3C8[i] & 0x4000;
            sUnk34 += D_800FF3B8[i] & 8;
            sUnk38 += D_800FF3B8[i] & 4;
            sUnk3C += D_800FF3DC[i] > 0;
            sUnk40 += D_800FF3DC[i] < 0;
            sUnk48 += D_800FF3D8[i] > 0;
            sUnk44 += D_800FF3D8[i] < 0;
        }
    }
    if (obj->unk28 < 0.756f) {
        sCursor[6] = 0;
        sCursor[7] = 0;
        if (obj->unk28 < 0.672f) {
            sCursor[4] = 0;
            sCursor[5] = 0;
            if (obj->unk28 < 0.589f) {
                sCursor[2] = 0;
                sCursor[3] = 0;
                if (obj->unk28 < 0.507f) {
                    sCursor[0] = 0;
                    sCursor[1] = 0;
                }
            }
        }
    }
    for (i = 0; i < 8; i++) {
        light = inst->lights;
        out = inst->lightOut[inst->buffer];
        for (j = 0; j < model->numLights; j++, light++) {
            id = light->id & 0xFF;
            def = model->lights[id].def;
            if (id == sLightIds[i]) {
                light->a = sCursor[i] << 8;
            }
            *out++ = (light->a >> 8) * def->scale;
            if (light->b >= 0) {
                *out++ = (light->b >> 8) * def->scale;
            }
        }
    }
    batch = model->batches;
    for (k = 0; k < model->numBatches; k++, batch++) {
        kind = batch->kind;
        if (kind == sRowIds[sUnkC]) {
            colour = func_80049C34(sAngle) * 55.0f + 200.0f;
            sAngle += updateRate << 10;
        } else {
            colour = 0x60;
        }
        if (kind != 6 && kind != 1) {
            n = batch[1].start - batch->start;
            v = &inst->vertices[inst->buffer][batch->start];
            while (n--) {
                v->r = colour;
                v->g = colour;
                v->b = colour;
                v++;
            }
        }
    }
    func_80040E64(&D_800FF398, &D_800FF39C);
    D_800FF780[40].unk10 = -sUnk8;
    D_800FF780[39].unk10 = sUnk8;
    func_8005A0D0(0x2D);
    func_8005A0D0(0x1F);
    func_8005A0D0(0x27);
    func_8005A0D0(0x28);
    func_80055C00(&D_800FF398, NULL, 0, 0);
    gDPSetEnvColor(D_800FF398++, 0, 0, 0, 160);
    gDPSetPrimColor(D_800FF398++, 0, 0, 40, 40, 40, 0);
    gDkrDmaDisplayList(D_800FF398++, OS_K0_TO_PHYSICAL(sRenderMode), 2);
    gDPFillRectangle(D_800FF398++, left, top, right, bottom);
    gDPSetScissor(D_800FF398++, G_SC_NON_INTERLACE, left, top, right, bottom);
    sUnk14 = sUnk24 - (s32) sUnk8 + 3;
    func_8006FC54(2);
    func_8006FC68(0, 255, 255, 255, 255);
    gDPPipeSync(D_800FF398++);
    func_8006FCA8(&D_800FF398, sUnk20, sUnk14, (char *) gMenuText[sRowTexts[D_800A506C][sUnkC]], 4);
    sUnk14 += 15;
    switch (sPage) {
        case 8: /* picking a row */
            if (sUnk28) {
                sPage = sUnkC;
                func_80001248(0x16D, 0);
                if (sUnkC != 7) {
                    sUnk28 = 0;
                }
            } else if (sUnk2C && !sUnk4C && sUnk18 >= 30) {
                func_80001248(0x16D, 0);
                load_first_unlocked_level();
                sUnk4C = 1;
            } else {
                sUnk30 = sUnkC;
                if (sUnk40) {
                    if (sUnkC == 6) {
                        sUnkC = 1;
                    } else {
                        sUnkC += 2;
                    }
                }
                if (sUnk3C) {
                    if (sUnkC == 1) {
                        sUnkC = 6;
                    } else {
                        sUnkC -= 2;
                    }
                }
                if (sUnk44) {
                    sUnkC--;
                }
                if (sUnk48) {
                    sUnkC++;
                }
                if (sUnkC < 0) {
                    sUnkC = 7;
                }
                if (sUnkC >= 8) {
                    sUnkC = 0;
                }
                /* skip the rows the mode doesn't have */
                while (sRowTexts[D_800A506C][sUnkC] == -1) {
                    if (sUnk3C) {
                        if (sUnkC == 1) {
                            sUnkC = 6;
                        } else {
                            sUnkC -= 2;
                        }
                    } else if (sUnk40) {
                        if (sUnkC == 6) {
                            sUnkC = 1;
                        } else {
                            sUnkC += 2;
                        }
                    } else if (sUnk44) {
                        sUnkC--;
                    } else if (sUnk48) {
                        sUnkC++;
                    }
                }
                if (sUnk30 != sUnkC) {
                    func_80001248(0x16B, 0);
                }
            }
            break;
        default: /* changing a row's setting: row 0 (the mode) here, the others in the mode's page */
            if (sPage == 0) {
                if (sUnk44) {
                    if (D_800A506C == 0) {
                        if (D_800FF318[0x6B]) {
                            D_800A506C = 2;
                            func_80001248(0x16C, 0);
                        } else if (D_800FF318[0x6A]) {
                            D_800A506C = 1;
                            func_80001248(0x16C, 0);
                        }
                    } else if (D_800A506C == 1) {
                        D_800A506C = 0;
                        func_80001248(0x16C, 0);
                    } else if (D_800FF318[0x6A]) {
                        D_800A506C = 1;
                        func_80001248(0x16C, 0);
                    } else {
                        D_800A506C = 0;
                        func_80001248(0x16C, 0);
                    }
                }
                if (sUnk48) {
                    if (D_800A506C == 0) {
                        if (D_800FF318[0x6A]) {
                            D_800A506C = 1;
                            func_80001248(0x16C, 0);
                        } else if (D_800FF318[0x6B]) {
                            D_800A506C = 2;
                            func_80001248(0x16C, 0);
                        }
                    } else if (D_800A506C == 2) {
                        D_800A506C = 0;
                        func_80001248(0x16C, 0);
                    } else if (D_800FF318[0x6B]) {
                        D_800A506C = 2;
                        func_80001248(0x16C, 0);
                    } else {
                        D_800A506C = 0;
                        func_80001248(0x16C, 0);
                    }
                }
                func_8006FCA8(&D_800FF398, sUnk20, sUnk14, (char *) sSaved50[D_800A506C], 4);
                if (sUnk2C) {
                    sPage = 8;
                }
            }
            break;
    }
    if (obj->unk28 >= 0.507f) {
        sCursor[0] = D_800A506C;
    }
    switch (D_800A506C) {
        case 0:
            func_m062_01158(updateRate);
            break;
        case 1:
            func_m062_01A48(updateRate);
            break;
        case 2:
            func_m062_0228C(updateRate);
            break;
    }
    viewport_scissor(&D_800FF398);
}

/* The first mode's settings page */
static void func_m062_01158(UNUSED s32 updateRate) {
    UNUSED s32 pad;
    s32 old;
    char buf[0x100];

    while (sRules[D_800A5004].mask != 0 && !(MODE_MASK & sRules[D_800A5004].mask)) {
        D_800A5004++;
        if (D_800A5004 >= 8) {
            D_800A5004 = 0;
        }
    }
    D_800A4FE8 = sRules[D_800A5004].mode;
    switch (sPage) {
        case 2:
            if (sUnk44 && D_800A4FF4 > 0) {
                func_80001248(0x16C, 0);
                D_800A4FF4--;
            }
            if (sUnk48 && D_800A4FF4 < 2) {
                func_80001248(0x16C, 0);
                D_800A4FF4++;
            }
            func_8006FCA8(&D_800FF398, sUnk20, sUnk14, (char *) sSaved60[D_800A4FF4], 4);
            break;
        case 4:
            old = D_800A5004;
            if (sUnk44) {
                do {
                    D_800A5004--;
                    if (D_800A5004 < 0) {
                        D_800A5004 = 7;
                    }
                } while (sRules[D_800A5004].mask != 0 && !(MODE_MASK & sRules[D_800A5004].mask));
            } else if (sUnk48) {
                do {
                    D_800A5004++;
                    if (D_800A5004 >= 8) {
                        D_800A5004 = 0;
                    }
                } while (sRules[D_800A5004].mask != 0 && !(MODE_MASK & sRules[D_800A5004].mask));
            }
            if (old != D_800A5004) {
                func_80001248(0x16C, 0);
            }
            D_800A4FE8 = sRules[D_800A5004].mode;
            func_8006FCA8(&D_800FF398, sUnk20, sUnk14, (char *) sSaved70[D_800A5004], 4);
            break;
        case 6:
            if (sUnk44 && D_800A5008 > 0) {
                func_80001248(0x16C, 0);
                D_800A5008--;
            }
            if (sUnk48 && D_800A5008 < 2) {
                func_80001248(0x16C, 0);
                D_800A5008++;
            }
            func_8006FCA8(&D_800FF398, sUnk20, sUnk14, (char *) sSaved90[D_800A5008], 4);
            break;
        case 1:
            switch (D_800A4FF4) {
                case 0:
                    if (sUnk44 && D_800A4FF8 >= 2) {
                        func_80001248(0x16C, 0);
                        D_800A4FF8--;
                    }
                    if (sUnk48 && D_800A4FF8 < 29) {
                        func_80001248(0x16C, 0);
                        D_800A4FF8++;
                    }
                    if (D_800A4FF8 == 1) {
                        func_800639E8(buf, "%2d %s", D_800A4FF8, SETTING(0x52C));
                    } else {
                        func_800639E8(buf, "%2d %s", D_800A4FF8, SETTING(0x528));
                    }
                    break;
                case 1:
                    if (sUnk44 && D_800A5000 >= 2) {
                        func_80001248(0x16C, 0);
                        D_800A5000--;
                    }
                    if (sUnk48 && D_800A5000 < 20) {
                        func_80001248(0x16C, 0);
                        D_800A5000++;
                    }
                    if (D_800A5000 == 1) {
                        func_800639E8(buf, "%2d %s", D_800A5000, SETTING(0x53C));
                    } else {
                        func_800639E8(buf, "%2d %s", D_800A5000, SETTING(0x538));
                    }
                    break;
                case 2:
                    if (sUnk44 && D_800A4FFC >= 2) {
                        func_80001248(0x16C, 0);
                        D_800A4FFC--;
                    }
                    if (sUnk48 && D_800A4FFC < 9) {
                        func_80001248(0x16C, 0);
                        D_800A4FFC++;
                    }
                    if (D_800A4FFC == 1) {
                        func_800639E8(buf, "%2d %s", D_800A4FFC, SETTING(0x534));
                    } else {
                        func_800639E8(buf, "%2d %s", D_800A4FFC, SETTING(0x530));
                    }
                    break;
            }
            func_8006FCA8(&D_800FF398, sUnk20, sUnk14, buf, 4);
            break;
        case 3:
            if (sUnk44 && D_800A5064 > 0) {
                func_80001248(0x16C, 0);
                D_800A5064--;
            }
            if (sUnk48 && D_800A5064 < 1) {
                func_80001248(0x16C, 0);
                D_800A5064++;
            }
            func_8006FCA8(&D_800FF398, sUnk20, sUnk14, (char *) sSavedA0[D_800A5064], 4);
            break;
        case 7:
            if (sUnk28) {
                D_800A5058 = 1;
                music_volume_fade(0.5f, 0);
                for (old = 0; old < 4; old++) {
                    D_800FF1B8[old] = 0;
                }
                gMenuTimer = 0;
                D_800A4FEC = 0;
                func_80047A20();
                func_800468C0(D_800A4FD0);
                func_8004665C(D_800A4FE8, 0, 0, 16, 1, 0);
                func_80047580(0);
            }
            break;
    }
    D_800A4FD8 = sRuleKinds[D_800A4FF4];
    D_800A4FDC = D_800A4FF8 * 3600;
    D_800A4FE0 = D_800A4FFC;
    D_800A4FE4 = D_800A5000;
    D_800A4FF0 = D_800A5008;
    sRowTexts[0][1] = sRuleTexts[D_800A4FF4];
    sCursor[2] = D_800A4FF4 + 1;
    sCursor[4] = 1;
    sCursor[6] = D_800A5008 + 1;
    sCursor[1] = sRuleCursor[D_800A4FF4];
    sCursor[5] = 0;
    sCursor[3] = 1;
    if (sUnk2C || sUnk28) {
        func_80001248(0x16D, 0);
        sPage = 8;
    }
}

/* The second mode's settings page */
static void func_m062_01A48(UNUSED s32 updateRate) {
    s32 mask;
    s32 old;
    char buf[0x100];

    mask = D_800FF318[0x6A]; while (!(sTeamModes[D_800A5030].mask & mask)) {
        D_800A5030++;
        if (D_800A5030 >= 4) {
            D_800A5030 = 0;
        }
    }
    D_800A4FE8 = sTeamModes[D_800A5030].mode;
    D_800A503C = sTeamModes[D_800A5030].flag;
    switch (sPage) {
        case 2:
            old = D_800A5030;
            if (sUnk44) {
                do {
                    D_800A5030--;
                    if (D_800A5030 < 0) {
                        D_800A5030 = 3;
                    }
                } while (!(sTeamModes[D_800A5030].mask & mask));
            } else if (sUnk48) {
                do {
                    D_800A5030++;
                    if (D_800A5030 >= 4) {
                        D_800A5030 = 0;
                    }
                } while (!(sTeamModes[D_800A5030].mask & mask));
            }
            if (old != D_800A5030) {
                func_80001248(0x16C, 0);
            }
            D_800A4FE8 = sTeamModes[D_800A5030].mode;
            D_800A503C = sTeamModes[D_800A5030].flag;
            func_8006FCA8(&D_800FF398, sUnk20, sUnk14, (char *) sSavedA8[D_800A5030], 4);
            break;
        case 4:
            if (sUnk44) {
                D_800A5024--;
                func_80001248(0x16C, 0);
            }
            if (sUnk48) {
                D_800A5024++;
                func_80001248(0x16C, 0);
            }
            if (D_800A5024 < 0) {
                D_800A5024 = 1;
            }
            if (D_800A5024 >= 2) {
                D_800A5024 = 0;
            }
            func_8006FCA8(&D_800FF398, sUnk20, sUnk14, (char *) sSavedB8[D_800A5024], 4);
            break;
        case 6:
            switch (D_800A5024) {
                case 0:
                    if (sUnk44) {
                        D_800A502C--;
                        func_80001248(0x16C, 0);
                    }
                    if (sUnk48) {
                        D_800A502C++;
                        func_80001248(0x16C, 0);
                    }
                    if (D_800A502C < 1) {
                        D_800A502C = 50;
                    }
                    if (D_800A502C >= 51) {
                        D_800A502C = 1;
                    }
                    func_800639E8(buf, "%s: %2d", SETTING(0x554), D_800A502C);
                    break;
                case 1:
                    if (sUnk44) {
                        D_800A5028--;
                        func_80001248(0x16C, 0);
                    }
                    if (sUnk48) {
                        D_800A5028++;
                        func_80001248(0x16C, 0);
                    }
                    if (D_800A5028 < 1) {
                        D_800A5028 = 60;
                    }
                    if (D_800A5028 >= 61) {
                        D_800A5028 = 1;
                    }
                    if (D_800A5028 == 1) {
                        func_800639E8(buf, "%2d %s", D_800A5028, SETTING(0x52C));
                    } else {
                        func_800639E8(buf, "%2d %s", D_800A5028, SETTING(0x528));
                    }
                    break;
            }
            func_8006FCA8(&D_800FF398, sUnk20, sUnk14, buf, 4);
            break;
        case 1:
            if (sUnk44) {
                D_800A5034--;
                func_80001248(0x16C, 0);
            }
            if (sUnk48) {
                D_800A5034++;
                func_80001248(0x16C, 0);
            }
            if (D_800A5034 < 0) {
                D_800A5034 = 7;
            }
            if (D_800A5034 >= 8) {
                D_800A5034 = 0;
            }
            func_8006FCA8(&D_800FF398, sUnk20, sUnk14, (char *) gMenuText[0x55C / 4 + D_800A5034], 4);
            break;
        case 3:
            if (sUnk44) {
                D_800A5038--;
                func_80001248(0x16C, 0);
            }
            if (sUnk48) {
                D_800A5038++;
                func_80001248(0x16C, 0);
            }
            if (D_800A5038 < 0) {
                D_800A5038 = 10;
            }
            if (D_800A5038 >= 11) {
                D_800A5038 = 0;
            }
            func_800639E8(buf, "%s: %2d", SETTING(0x560), D_800A5038);
            func_8006FCA8(&D_800FF398, sUnk20, sUnk14, buf, 4);
            break;
        case 7:
            if (sUnk28) {
                func_80047A20();
                D_800A5058 = 1;
                music_volume_fade(0.5f, 0);
                for (old = 0; old < 4; old++) {
                    D_800FF1B8[old] = 0;
                    D_800FED50[old].unk12 = 3;
                }
                gMenuTimer = 0;
                D_800A4FD8 = 4;
                D_800A4FEC = 0;
                if (D_800A503C) {
                    func_800468C0(1);
                } else {
                    func_800468C0(D_800A4FD0);
                }
                D_800A4FD4 = 0;
                func_8004665C(D_800A4FE8, 0, 0, 16, 1, 0);
                func_800468A4(0);
                func_80047580(0);
            }
            break;
    }
    sRowTexts[1][6] = sTeamTexts[D_800A5024];
    sCursor[2] = sTeamCursor[D_800A5030];
    sCursor[4] = D_800A5024 + 2;
    sCursor[6] = sTeamCursor2[D_800A5024];
    sCursor[1] = 5;
    sCursor[3] = 2;
    sCursor[5] = 0;
    if (sUnk2C || sUnk28) {
        func_80001248(0x16D, 0);
        sPage = 8;
    }
}

/* Changes the setting being picked with the stick and draws it; the third button starts the game (called from 001E4) */
static void func_m062_0228C(UNUSED s32 arg0) {
    s32 mask;
    s32 old;
    char buf[0x100];

    /* on one line: IDO then schedules the mask load as the ROM does */
    mask = D_800FF318[0x6B]; while (!(sModes[D_800A500C].mask & mask)) {
        D_800A500C++;
        if (D_800A500C >= 3) {
            D_800A500C = 0;
        }
    }
    D_800A4FE8 = sModes[D_800A500C].mode;
    D_800A4FDC = sModes[D_800A500C].arg;
    switch (sPage) {
        case 2:
            old = D_800A500C;
            if (sUnk44) {
                do {
                    D_800A500C--;
                    if (D_800A500C < 0) {
                        D_800A500C = 2;
                    }
                } while (!(sModes[D_800A500C].mask & mask));
            } else if (sUnk48) {
                do {
                    D_800A500C++;
                    if (D_800A500C >= 3) {
                        D_800A500C = 0;
                    }
                } while (!(sModes[D_800A500C].mask & mask));
            }
            if (old != D_800A500C) {
                func_80001248(0x16C, 0);
            }
            D_800A4FE8 = sModes[D_800A500C].mode;
            D_800A4FDC = sModes[D_800A500C].arg;
            func_8006FCA8(&D_800FF398, sUnk20, sUnk14, (char *) sSavedC0[D_800A500C], 4);
            break;
        case 4:
            if (sUnk44) {
                D_800A5010--;
                func_80001248(0x16C, 0);
            }
            if (sUnk48) {
                D_800A5010++;
                func_80001248(0x16C, 0);
            }
            if (D_800A5010 < 0) {
                D_800A5010 = 4;
            }
            if (D_800A5010 >= 5) {
                D_800A5010 = 0;
            }
            D_800A5018 = sTimeLimits[D_800A5010];
            if (D_800A5010 == 0) {
                func_800639E8(buf, "%s %s", SETTING(0x580), SETTING(0x584));
            } else {
                func_800639E8(buf, "%3d %s", D_800A5018, SETTING(0x584));
            }
            func_8006FCA8(&D_800FF398, sUnk20, sUnk14, buf, 4);
            break;
        case 6:
            if (sUnk44) {
                D_800A5020--;
                func_80001248(0x16C, 0);
            }
            if (sUnk48) {
                D_800A5020++;
                func_80001248(0x16C, 0);
            }
            if (D_800A5020 < 0) {
                D_800A5020 = 2;
            }
            if (D_800A5020 >= 3) {
                D_800A5020 = 0;
            }
            func_8006FCA8(&D_800FF398, sUnk20, sUnk14, (char *) sSavedC8[D_800A5020], 4);
            break;
        case 7:
            if (sUnk28) {
                D_800A5058 = 1;
                music_volume_fade(0.5f, 0);
                for (old = 0; old < 4; old++) {
                    D_800FF1B8[old] = 0;
                }
                gMenuTimer = 0;
                D_800A4FD8 = 6;
                D_800A4FEC = 0;
                func_80047A20();
                func_800468C0(1);
                func_8004665C(D_800A4FE8, 0, 0, 16, 1, 0);
                func_800468A4(0);
                func_80047580(0);
            }
            break;
    }
    sCursor[2] = 6;
    sCursor[4] = 4;
    sCursor[6] = 7;
    sCursor[1] = 0;
    sCursor[3] = 0;
    sCursor[5] = 0;
    if (sUnk2C || sUnk28) {
        func_80001248(0x16D, 0);
        sPage = 8;
    }
}

void func_m062_02784(void) {
    menu_assetgroup_free(sAssets);
    menu_assetgroup_free(sAssets2);
}
