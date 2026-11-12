/*
 * Runlink module 41: the front end's character display. It draws the model of
 * the selected character (a menu item's object) with a textured frame around
 * its bounding box, slides it towards its place, and shows four counting
 * numbers and a scrolling string of digits next to it, with a flickering light
 * and its sound.
 */
#include "common.h"
#include "object.h"
#include "gfx.h"

/* The menu items (D_800FF780, size 0x20; the main segment's MenuItem) */
typedef struct M041MenuItem {
    /* 0x00 */ s16 unk0;
    /* 0x02 */ s16 unk2;
    /* 0x04 */ s16 unk4;
    /* 0x06 */ s16 unk6;
    /* 0x08 */ f32 unk8;
    /* 0x0C */ f32 unkC;
    /* 0x10 */ f32 unk10;
    /* 0x14 */ f32 unk14;
    /* 0x18 */ f32 unk18;
    /* 0x1C */ s8 unk1C;
    /* 0x1D */ s8 unk1D;
    /* 0x1E */ s8 unk1E;
    /* 0x1F */ s8 unk1F;
} M041MenuItem;

/* A model instance (the object's modelInstances entries) */
typedef struct M041Instance {
    /* 0x00 */ void *model;
    /* 0x04 */ u8 pad4[0xA - 0x4];
    /* 0x0A */ s8 unkA;
    /* 0x0B */ s8 matrixIndex;
    /* 0x0C */ u8 padC[0x10 - 0xC];
    /* 0x10 */ void *matrices[2];
    /* 0x18 */ u8 pad18[0x68 - 0x18];
    /* 0x68 */ f32 *bounds;
} M041Instance;

/* The hub player's state (module 135's player object's unk68) */
typedef struct M041HubPlayer {
    /* 0x00 */ s32 flags;
    /* 0x04 */ u8 pad4[0x28 - 0x4];
    /* 0x28 */ f32 unk28;
    /* 0x2C */ u8 pad2C[0x51 - 0x2C];
    /* 0x51 */ s8 index;
} M041HubPlayer;

typedef struct M041Texture {
    /* 0x00 */ u8 pad0[0x12];
    /* 0x12 */ u16 frameCount;
} M041Texture;

extern Gfx *D_800FF398; /* the display list being built */
extern Mtx *D_800FF39C;
extern Vertex *D_800FF3A0;
extern void *D_800FF3F8[]; /* the menu items' loaded assets */
extern M041MenuItem D_800FF780[];

extern s32 fb_mode(void);
extern s32 func_80048E2C(s32 min, s32 max);                     /* DKR get_random_number */
extern void func_80001248(s32 sound, s32 *handle);               /* DKR amSndPlay */
extern void func_8003D21C(M041Instance *inst, void *model, Object *obj, f32 arg3, f32 arg4);
extern void func_800221F0(Object *obj, M041Instance *inst, ObjectShading *shading, void *matrix);
extern void func_80055C00(Gfx **dList, void *tex, s32 flags, s32 texOffset);
extern void func_80055B80(Gfx **dList); /* resets the render mode */
extern void func_8005A0D0(s32 item);
extern void func_8000DF00(Object *obj, s32 updateRate);
extern Object *func_m135_0205C(void);
extern void func_m058_00618(s32 updateRate);
extern void sound_stop(s32 handle);
extern void func_800013FC(s32 sound, s32 handle, u8 volume);
extern void zbuffer_clear_rect(Gfx **dList, s32 width, s32 height, s32 x0, s32 y0, s32 x1, s32 y1);
extern void func_80040E64(Gfx **dList, Mtx **mtx);
extern s32 check_fadeout_transition(void);
extern void s32_to_decimal_string(char *buf, s32 value, s32 arg2); /* writes a number */
extern void debug_text_add(s16 x, s16 y, char *text);       /* draws a text */

/*
 * NON_MATCHING, the whole module (its .rodata needs both functions in C). Both
 * functions have the ROM's length, frame and instructions; what differs is register
 * allocation: in func_m041_00000 IDO gives obj the last saved register (fp) where the
 * ROM has s2 (frame in fp), shifting s2-s7; in func_m041_008B4 the angle loop's
 * counter and IDO's remainder count swap a1/a2, and the t-registers after it differ.
 */
#ifdef NON_MATCHING
static Triangle sTriangles[] = {
    { 0x40, 0, 1, 3, { { 0, 0x7E0 }, { 0x3E0, 0x7E0 }, { 0, 0 } } },
    { 0x40, 1, 4, 3, { { 0x3E0, 0x7E0 }, { 0x3E0, 0 }, { 0, 0 } } },
    { 0x40, 1, 2, 4, { { 0, 0x7E0 }, { 0x3E0, 0x7E0 }, { 0, 0 } } },
    { 0x40, 2, 5, 4, { { 0x3E0, 0x7E0 }, { 0x3E0, 0 }, { 0, 0 } } },
    { 0x40, 3, 4, 6, { { 0, 0x7E0 }, { 0x3E0, 0x7E0 }, { 0, 0 } } },
    { 0x40, 4, 7, 6, { { 0x3E0, 0x7E0 }, { 0x3E0, 0 }, { 0, 0 } } },
    { 0x40, 4, 5, 7, { { 0, 0x7E0 }, { 0x3E0, 0x7E0 }, { 0, 0 } } },
    { 0x40, 5, 8, 7, { { 0x3E0, 0x7E0 }, { 0x3E0, 0 }, { 0, 0 } } },
    { 0x40, 0, 2, 6, { { 0, 0 }, { 0, 0 }, { 0, 0 } } },
    { 0x40, 2, 8, 6, { { 0, 0 }, { 0, 0 }, { 0, 0 } } },
};
static u8 sTextures[] = { 0x20, 0x24, 0x28, 0 }; /* the first frame texture per character */
static u16 sCountSpeeds[] = { 1, 10, 100, 25 };
static s16 sCountPos[2][4][2] = {
    { { 240, 60 }, { 240, 103 }, { 286, 60 }, { 286, 103 } },
    { { 210, 60 }, { 210, 98 }, { 250, 60 }, { 250, 98 } },
};
static char *sStrings[] = { "01011101111000100", "010111011110101" };
static s32 sStringLengths[] = { 16, 14 };
static s32 sStringTimer = 6;
static s16 sState = 4;
static s16 sFlickerTimer = 60;
static s32 sFrame = 0;
static s32 sSound = 0;
static f32 sLastSpeed = 0.05f;
static s32 sSlideSound = 0;

static u16 sCounts[4];
static s16 sTimer;
static s16 sNextState;
static s16 sLight;
static s16 sLightFade;
static Object *sHub;
static M041HubPlayer *sHubPlayer;

void func_m041_00000(s32 arg0, s32 frame) {
    Vertex *verts;
    Object *obj;
    M041Instance *inst;
    void **model;
    f32 scale;
    s32 midY;
    s32 midX;
    s32 midZ;
    s32 visible;

    verts = D_800FF3A0;
    obj = D_800FF3F8[(arg0 == 0 ? &D_800FF780[11] : &D_800FF780[49])->unk6];
    inst = ((M041Instance **) obj->modelInstances)[obj->modelIndex];
    model = inst->model;
    if (fb_mode() & 1) {
        scale = 0.8f;
    } else {
        scale = 1.0f;
    }
    if (inst->unkA != 0) {
        func_8003D21C(inst, model, obj, scale, 0.0f);
        func_800221F0(obj, inst, obj->shading, inst->matrices[inst->matrixIndex]);
        inst->unkA = 0;
    }
    visible = obj->unk28 < 0.955f;
    if (visible) {
        Vertex *v;
        Triangle *tri;
        s32 i;
        u8 n;

        midY = inst->bounds[1] + (inst->bounds[7] - inst->bounds[1]) / (f32) 2;
        midX = inst->bounds[0] + (inst->bounds[3] - inst->bounds[0]) / (f32) 2;
        midZ = inst->bounds[2] + (inst->bounds[5] - inst->bounds[2]) / (f32) 2;
        D_800FF3A0->x = inst->bounds[0];
        D_800FF3A0->y = inst->bounds[1];
        D_800FF3A0->z = inst->bounds[2];
        D_800FF3A0++;
        D_800FF3A0->x = midX;
        D_800FF3A0->y = inst->bounds[1];
        D_800FF3A0->z = midZ;
        D_800FF3A0++;
        D_800FF3A0->x = inst->bounds[3];
        D_800FF3A0->y = inst->bounds[4];
        D_800FF3A0->z = inst->bounds[5];
        D_800FF3A0++;
        D_800FF3A0->x = inst->bounds[0];
        D_800FF3A0->y = midY;
        D_800FF3A0->z = inst->bounds[2];
        D_800FF3A0++;
        D_800FF3A0->x = midX;
        D_800FF3A0->y = midY;
        D_800FF3A0->z = midZ;
        D_800FF3A0++;
        D_800FF3A0->x = inst->bounds[3];
        D_800FF3A0->y = midY;
        D_800FF3A0->z = inst->bounds[5];
        D_800FF3A0++;
        D_800FF3A0->x = inst->bounds[0];
        D_800FF3A0->y = inst->bounds[7];
        D_800FF3A0->z = inst->bounds[2];
        D_800FF3A0++;
        D_800FF3A0->x = midX;
        D_800FF3A0->y = inst->bounds[7];
        D_800FF3A0->z = midZ;
        D_800FF3A0++;
        D_800FF3A0->x = inst->bounds[9];
        D_800FF3A0->y = inst->bounds[10];
        D_800FF3A0->z = inst->bounds[11];
        D_800FF3A0++;
        for (v = verts; v != D_800FF3A0; v++) {
            v->r = 255;
            v->g = 255;
            v->b = 255;
            v->a = 255;
        }
        gSPVertexJFG(D_800FF398++, (u32) verts + 0x80000000, 10, 0);
        tri = sTriangles;
        if (arg0 == 0) {
            n = sTextures[obj->modelIndex];
            model = &D_800FF3F8[n];
            for (i = 0; i < 4; i++) {
                func_80055C00(&D_800FF398, *model++, 0, frame << 8);
                gSPPolygon(D_800FF398++, (u32) tri + 0x80000000, 2, 1);
                tri += 2;
            }
        } else {
            obj = D_800FF3F8[D_800FF780[49].unk6 - 3];
            for (i = 0; i < 4; i++) {
                func_80055C00(&D_800FF398, obj, 0, i << 16);
                gSPPolygon(D_800FF398++, (u32) tri + 0x80000000, 2, 1);
                tri += 2;
            }
        }
        func_80055B80(&D_800FF398);
    }
    if (arg0 == 0) {
        func_8005A0D0(11);
    } else {
        func_8005A0D0(49);
    }
    if (visible) {
        func_80055C00(&D_800FF398, NULL, 0, 0);
        gDPSetCycleType(D_800FF398++, G_CYC_1CYCLE);
        gDPSetCombine(D_800FF398++, 0x767EEC, 0xFFFDF6FB);
        gDPSetOtherMode(D_800FF398++, 0x082C0F, 0x504240);
        gDPSetEnvColor(D_800FF398++, 0, 0, 0, sLight >> 2);
        gDPSetPrimColor(D_800FF398++, 0, 0, 0, 0x1A, 0, sLight);
        gSPVertexJFG(D_800FF398++, (u32) verts + 0x80000000, 10, 0);
        gSPPolygon(D_800FF398++, (u32) &sTriangles[8] + 0x80000000, 2, 1);
        func_80055B80(&D_800FF398);
    }
}

void func_m041_008B4(s32 arg0, s32 arg1, s32 updateRate) {
    M041MenuItem *item;
    M041Instance *inst;
    Object *obj;
    f32 target;
    f32 diff;
    f32 speed;
    s32 d;
    s32 angle;
    s32 index;
    f32 y;
    s32 i;
    s32 j;
    s32 delta;
    s32 x;
    s32 first;
    char *s;
    char *p;
    M041Texture *tex;
    s32 vol;
    s32 r;
    s32 show;
    s32 blink;
    char buf[16];

    index = arg0 == 0 ? 11 : 49;
    item = &D_800FF780[index];
    obj = D_800FF3F8[item->unk6];
    if (sState == 4 && obj->unk28 < 0.005f) {
        sCounts[0] = 0;
        sCounts[1] = 0;
        sCounts[2] = 0;
        sCounts[3] = 0;
        sTimer = 0;
        sState = 1;
        sNextState = 3;
    } else if (obj->unk28 > 0.005f) {
        sState = 4;
    }
    if (sState != 4) {
        func_8000DF00(obj, updateRate);
    }
    inst = ((M041Instance **) obj->modelInstances)[obj->modelIndex];
    inst->unkA = updateRate;
    if (arg0 == 0) {
        if (sHub == NULL) {
            sHub = func_m135_0205C();
        }
        if (sHub == NULL) {
            return;
        }
        sHubPlayer = sHub->unk68;
        if (sHubPlayer == NULL) {
            return;
        }
        if (sHubPlayer->unk28 <= 0.5f || (sHubPlayer->flags & 4)) {
            target = 240.0f;
        } else {
            target = 100.0f;
        }
        diff = item->unkC - target;
        if (diff < 0.0f) {
            diff = -diff;
        }
        if ((sHubPlayer->flags & 2) && diff < 1.0f && target == 100.0f) {
            for (i = 0; i < updateRate; i++) {
                if (item->unk0 < 0) {
                    item->unk0 += 0x8000;
                }
                angle = 0x8000;
                d = angle - item->unk0;
                if (d > 0x1600) {
                    d = 0x1600;
                }
                item->unk0 += d >> 4;
            }
            d = angle - item->unk0;
            if (d < 0x800) {
                speed = -0.05f;
            } else {
                speed = 0.05f;
            }
        } else {
            item->unk0 += updateRate * 352;
            speed = 0.05f;
        }
        for (i = 0; i < updateRate; i++) {
            item->unkC += (target - item->unkC) * 0.076f;
        }
        if (item->unkC > 230.0f) {
            obj->modelIndex = sHubPlayer->index;
            if (sHubPlayer->flags & 4) {
                func_m058_00618(updateRate);
            }
        }
    } else {
        item->unk0 = 0;
        if (arg1 != 0) {
            speed = -0.05f;
        } else {
            obj->unk28 = 1.0f;
            speed = 0.05f;
        }
    }
    func_8001138C(obj, speed, updateRate);
    if (speed != sLastSpeed) {
        if (sSlideSound != 0) {
            sound_stop(sSlideSound);
        }
        if (speed == 0.05f) {
            func_80001248(0xCE, &sSlideSound);
        } else {
            func_80001248(0xCD, &sSlideSound);
        }
    }
    if (obj->unk28 > 0.005f) {
        if (item->unkC < 110.0f || arg0 != 0) {
            sLight = 255;
            sFrame = 0;
            sLightFade = 3;
        }
    }
    sLastSpeed = speed;
    if (arg0 == 0) {
        s32 x0;
        s32 x1;
        f32 y;

        x0 = item->unkC - 56.0f + 160.0f;
        x1 = item->unkC + 45.0f + 160.0f;
        y = 120.0f - item->unk10;
        zbuffer_clear_rect(&D_800FF398, 320, 240, x0, y - 142.0f, x1, y + 47.0f);
    }
    func_80040E64(&D_800FF398, &D_800FF39C);
    func_m041_00000(arg0, sFrame);
    if (obj->unk28 < 0.995f) {
        if (sState != 4) {
            sTimer += updateRate;
            show = TRUE;
            blink = FALSE;
            delta = 0;
            switch (sState) {
                case 0:
                    show = FALSE;
                    break;
                case 1:
                    if (sTimer > 120) {
                        sTimer = 0;
                        sState = sNextState;
                        sCounts[0] = 0;
                        sCounts[1] = 0;
                        sCounts[2] = 0;
                        sCounts[3] = 0;
                    } else {
                        blink = TRUE;
                    }
                    break;
                case 2:
                    delta = -0x40;
                    if (sTimer > 60) {
                        sTimer = 0;
                        sState = 1;
                        sNextState = 3;
                    }
                    break;
                case 3:
                    delta = 0x40;
                    if (sTimer > 60) {
                        sTimer = 0;
                        sState = 1;
                        sNextState = 2;
                    }
                    break;
            }
            if (arg0 == 0) {
                tex = D_800FF3F8[sTextures[obj->modelIndex]];
                sFrame += delta * updateRate;
                if (sFrame < 0) {
                    sFrame = 0;
                }
                if (sFrame >= tex->frameCount) {
                    sFrame = tex->frameCount - 1;
                }
            }
            if (check_fadeout_transition() != 0 || arg0 != 0) {
                show = FALSE;
            }
            if (show) {
                for (i = 0; i < 4; i++) {
                    s32_to_decimal_string(buf, sCounts[i] * 100, 0);
                    x = sCountPos[arg0][i][0];
                    if (i >= 2) {
                        for (p = &buf[1]; *p != '\0'; p++) {
                            x -= 4;
                        }
                    }
                    debug_text_add(x, sCountPos[arg0][i][1], buf);
                    if (blink) {
                        sCounts[i] += sCountSpeeds[i] * updateRate;
                    }
                    if (sCounts[i] > 30000) {
                        sCounts[i] = 0;
                    }
                }
                if (arg0 == 0) {
                    debug_text_add(240, 137, sStrings[0]);
                } else {
                    debug_text_add(211, 134, sStrings[1]);
                }
                if (blink) {
                    sStringTimer -= updateRate;
                    while (sStringTimer < 0) {
                        s = sStrings[arg0];
                        first = s[0];
                        p = s + 1;
                        for (j = 0; j < sStringLengths[arg0]; j++) {
                            *s++ = *p++;
                        }
                        *s = first;
                        sStringTimer += 6;
                    }
                }
            }
        }
        if (sLight > 0) {
            if (sSound == 0) {
                func_80001248(0xCF, &sSound);
            }
            vol = sLight >> 1;
            if (vol > 127) {
                vol = 127;
            }
            func_800013FC(0xCF, sSound, vol);
        } else if (sSound != 0) {
            sound_stop(sSound);
        }
        if (sFlickerTimer < updateRate) {
            r = func_80048E2C(0x80, 0xFF);
            if (sLight < r) {
                sLight = r;
                sLightFade = func_80048E2C(1, 8);
            }
            sFlickerTimer = func_80048E2C(0xA0, 0xFF);
        } else {
            sFlickerTimer -= updateRate;
        }
        if (sLightFade * updateRate < sLight) {
            sLight -= sLightFade * updateRate;
        } else {
            sLight = 0;
        }
    } else if (sSound != 0) {
        sound_stop(sSound);
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m041/ro", func_m041_00000);

INCLUDE_ASM("asm/nonmatchings/modules/m041/ro", func_m041_008B4);

INCLUDE_ASM("asm/nonmatchings/modules/m041", data);

INCLUDE_ASM("asm/nonmatchings/modules/m041/ro", rodata);

INCLUDE_ASM("asm/nonmatchings/modules/m041", bss);
#endif
