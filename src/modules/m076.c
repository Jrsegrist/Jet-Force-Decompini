/*
 * Runlink module 76: a flat panel of light with an optional glow box behind
 * it. The panel is a rectangle of the setup's size (scaled), drawn as two
 * triangles; when the panel is big enough the glow extends its corners away
 * from a point, drawn as two strips of eight triangles whose texture scrolls.
 */
#include "module.h"
#include "gfx.h"

typedef struct M076Setup {
    LevelObjectEntryCommon common;
    /* 0x0A */ u16 width;
    /* 0x0C */ u16 height;
    /* 0x0E */ s16 focusX;
    /* 0x10 */ s16 focusY;
    /* 0x12 */ s16 focusZ;
    /* 0x14 */ u8 glowMin;
    /* 0x15 */ u8 glowMax;
    /* 0x16 */ u8 r;
    /* 0x17 */ u8 g;
    /* 0x18 */ u8 b;
    /* 0x19 */ u8 a;
    /* 0x1A */ u16 glowLength;
    /* 0x1C */ u8 scaleX;
    /* 0x1D */ u8 scaleY;
    /* 0x1E */ u8 flags;
    /* 0x1F */ s8 rollAngle;
    /* 0x20 */ s16 pitch;
    /* 0x22 */ s16 yaw;
} M076Setup;

typedef struct M076State {
    /* 0x00 */ Vertex verts[2][8];
    /* 0xA0 */ Vec3f focus;
    /* 0xAC */ f32 width;
    /* 0xB0 */ f32 height;
    /* 0xB4 */ f32 glowLength;
    /* 0xB8 */ f32 scaleX;
    /* 0xBC */ f32 scaleY;
    /* 0xC0 */ f32 glowMin;
    /* 0xC4 */ f32 glowMax;
    /* 0xC8 */ u8 r;
    /* 0xC9 */ u8 g;
    /* 0xCA */ u8 b;
    /* 0xCB */ u8 a;
    /* 0xCC */ u8 flags; /* 1: panel, 2: glow, 4: back glow */
    /* 0xCD */ u8 buffer;
    /* 0xCE */ u16 drawFlags;
} M076State;

extern s8 D_800A0CE0;
extern f32 func_80097140(f32 x); /* sqrtf */
extern void func_80041CA0(Gfx **dl, s32 arg1, Object *obj, f32 arg3, f32 arg4);
extern void func_80041DC0(Gfx **dl);
extern void func_80055C00(Gfx **dl, void *tex, s32 flags, s32 offset); /* sets a texture */

s32 D_m076_00D10 = 76;

static s16 sScroll = 0;

/* The panel */
static Triangle sPanelTris[2] = {
    { 0x40, 0, 2, 1, { { 0, 0 }, { 0, 0 }, { 0, 0 } } },
    { 0x40, 0, 3, 2, { { 0, 0 }, { 0, 0 }, { 0, 0 } } },
};

/* The glow, front and back */
static Triangle sGlowTris[8] = {
    { 0x00, 0, 5, 4, { { 0, 0 }, { 0x3E0, 0 }, { 0x3E0, 0 } } },
    { 0x00, 0, 1, 5, { { 0, 0 }, { 0, 0 }, { 0x3E0, 0 } } },
    { 0x00, 1, 6, 5, { { 0, 0 }, { 0x3E0, 0 }, { 0x3E0, 0 } } },
    { 0x00, 1, 2, 6, { { 0, 0 }, { 0, 0 }, { 0x3E0, 0 } } },
    { 0x00, 2, 7, 6, { { 0, 0 }, { 0x3E0, 0 }, { 0x3E0, 0 } } },
    { 0x00, 2, 3, 7, { { 0, 0 }, { 0, 0 }, { 0x3E0, 0 } } },
    { 0x00, 3, 4, 7, { { 0, 0 }, { 0x3E0, 0 }, { 0x3E0, 0 } } },
    { 0x00, 3, 0, 4, { { 0, 0 }, { 0, 0 }, { 0x3E0, 0 } } },
};
static s16 sGlowV[8][3] = {
    { 0, 0x400, 0 }, { 0, 0x400, 0x400 }, { 0, 0x400, 0 }, { 0, 0x400, 0x400 },
    { 0, 0x400, 0 }, { 0, 0x400, 0x400 }, { 0, 0x400, 0 }, { 0, 0x400, 0x400 },
};
static Triangle sBackGlowTris[8] = {
    { 0x00, 0, 4, 5, { { 0, 0 }, { 0x3E0, 0 }, { 0x3E0, 0 } } },
    { 0x00, 0, 5, 1, { { 0, 0 }, { 0x3E0, 0 }, { 0, 0 } } },
    { 0x00, 1, 5, 6, { { 0, 0 }, { 0x3E0, 0 }, { 0x3E0, 0 } } },
    { 0x00, 1, 6, 2, { { 0, 0 }, { 0x3E0, 0 }, { 0, 0 } } },
    { 0x00, 2, 6, 7, { { 0, 0 }, { 0x3E0, 0 }, { 0x3E0, 0 } } },
    { 0x00, 2, 7, 3, { { 0, 0 }, { 0x3E0, 0 }, { 0, 0 } } },
    { 0x00, 3, 7, 4, { { 0, 0 }, { 0x3E0, 0 }, { 0x3E0, 0 } } },
    { 0x00, 3, 4, 0, { { 0, 0 }, { 0x3E0, 0 }, { 0, 0 } } },
};
static s16 sBackGlowV[8][3] = {
    { 0, 0, 0x400 }, { 0, 0x400, 0x400 }, { 0, 0, 0x400 }, { 0, 0x400, 0x400 },
    { 0, 0, 0x400 }, { 0, 0x400, 0x400 }, { 0, 0, 0x400 }, { 0, 0x400, 0x400 },
};

void func_m076_002A8(Object *obj);

void func_m076_00000(Object *obj, M076Setup *setup, s32 keepState) {
    M076State *s;
    s32 i;
    s32 k;
    Vertex *v;

    obj->trans.rotation.y_rotation = setup->yaw;
    obj->trans.rotation.x_rotation = setup->pitch;
    obj->trans.rotation.z_rotation = setup->rollAngle << 6;
    obj->trans.scale = 0.1f;
    s = obj->unk68;
    s->focus.x = setup->focusX * 10.0f;
    s->focus.y = setup->focusY * 10.0f;
    s->focus.z = setup->focusZ * 10.0f;
    s->width = setup->width * 10.0f;
    s->height = setup->height * 10.0f;
    s->glowLength = setup->glowLength * 10.0f;
    s->scaleX = setup->scaleX * 0.01f;
    s->scaleY = setup->scaleY * 0.01f;
    s->glowMin = setup->glowMin * 0.01f;
    s->glowMax = setup->glowMax * 0.01f;
    s->r = setup->r;
    s->g = setup->g;
    s->b = setup->b;
    s->a = setup->a;
    s->flags = setup->flags;
    for (k = 0; k < 2; k++) {
        for (i = 0, v = s->verts[k]; i < 8; i++) {
            if (i < 4) {
                v->r = 0xFF;
                v->g = 0xFF;
                v->b = 0xFF;
                v->a = setup->a;
            } else {
                v->r = setup->r;
                v->g = setup->g;
                v->b = setup->b;
                v->a = 0;
            }
            v++;
        }
    }
    if (!keepState) {
        s->buffer = 0;
        s->drawFlags = 0;
    }
    func_m076_002A8(obj);
}

void func_m076_002A8(Object *obj) {
    s32 i;
    f32 halfW;
    f32 halfH;
    f32 f;
    s32 n;
    f32 len;
    f32 dx;
    f32 dy;
    f32 dz;
    f32 unused;
    Vec3f *p;
    Vec3f pts[8];
    M076State *s;
    Vertex *v;

    runlink_unload_timer_set(D_m076_00D10, 2, 2);
    s = obj->unk68;
    halfW = s->width * s->scaleX * 0.5f;
    s->buffer ^= 1;
    s->drawFlags = 0;
    halfH = s->height * s->scaleY * 0.5f;
    if (halfW != 0.0f && halfH != 0.0f) {
        D_800A0CE0 = 1;
        pts[0].x = -halfW;
        pts[0].y = halfH;
        pts[0].z = 0.0f;
        pts[1].x = halfW;
        pts[1].y = halfH;
        pts[1].z = 0.0f;
        pts[2].x = halfW;
        pts[2].y = -halfH;
        pts[2].z = 0.0f;
        pts[3].x = -halfW;
        pts[3].y = -halfH;
        pts[3].z = 0.0f;
        s->drawFlags |= s->flags & 1;
        if (s->flags & 2) {
            f = s->scaleX * s->scaleY;
            if (s->glowMin <= f) {
                f = (f - s->glowMin) / (s->glowMax - s->glowMin);
                if (f > 1.0f) {
                    f = 1.0f;
                }
                len = s->glowLength * f;
                for (i = 0, p = pts; i < 4; i++, p++) {
                    dx = p->x - s->focus.x;
                    dy = p->y - s->focus.y;
                    dz = p->z - s->focus.z;
                    f = dx * dx + dy * dy + dz * dz;
                    if (f != 0.0f) {
                        f = len / func_80097140(f);
                    }
                    p[4].x = p->x + dx * f;
                    p[4].y = p->y + dy * f;
                    p[4].z = p->z + dz * f;
                }
                s->drawFlags |= s->flags & 6;
            }
        }
        n = 4;
        if (s->drawFlags & 2) {
            n = 8;
        }
        v = s->verts[s->buffer];
        for (i = 0, p = pts; i < n; i++, v++, p++) {
            v->x = p->x;
            v->y = p->y;
            v->z = p->z;
            if (OBJ_STATE(obj, s32)[0]) {
                if (i >= 4) {
                    v->r = s->r;
                    v->g = s->g;
                    v->b = s->b;
                } else {
                    v->a = s->a;
                }
            }
        }
    }
    OBJ_STATE(obj, s32)[0] = 0;
}

void func_m076_0096C(s32 updateRate) {
    s16 k;
    s16 i;
    Triangle *tri;
    s16 *v;

    sScroll = (sScroll + updateRate * 2) & 0x3FF;
    for (k = 0; k < 3; k++) {
        if (k == 0) {
            tri = sGlowTris;
            v = sGlowV[0];
        } else {
            tri = sBackGlowTris;
            v = sBackGlowV[0];
        }
        i = 8;
        while (i--) {
            tri->uv[0][1] = v[0] + sScroll;
            tri->uv[1][1] = v[1] + sScroll;
            tri->uv[2][1] = v[2] + sScroll;
            v += 3;
            tri++;
        }
    }
}

void func_m076_00A34(Gfx **dl, s32 arg1, Object *obj) {
    M076State *s;

    runlink_unload_timer_set(D_m076_00D10, 2, 2);
    s = obj->unk68;
    if (s->flags != 0) {
        func_80041CA0(dl, arg1, obj, 1.0f, 0.0f);
        gDPPipeSync((*dl)++);
        gDPSetEnvColor((*dl)++, 255, 255, 255, 255);
        if (s->drawFlags & 1) {
            func_80055C00(dl, NULL, 0x17, 0);
            gDPSetPrimColor((*dl)++, 0, 0, 255, 255, 255, 0xD0);
            gSPVertexJFG((*dl)++, OS_K0_TO_PHYSICAL(s->verts[s->buffer]), 5, 0);
            gSPPolygon((*dl)++, OS_K0_TO_PHYSICAL(sPanelTris), 2, 0);
            gDPPipeSync((*dl)++);
        }
        if (s->drawFlags & 6) {
            func_80055C00(dl, *obj->modelInstances, 0x17, 0);
            gDPSetPrimColor((*dl)++, 0, 0, 255, 255, 255, 255);
            gSPVertexJFG((*dl)++, OS_K0_TO_PHYSICAL(s->verts[s->buffer]), 9, 0);
            if (s->drawFlags & 4) {
                gSPPolygon((*dl)++, OS_K0_TO_PHYSICAL(sBackGlowTris), 8, 1);
            }
            if (s->drawFlags & 2) {
                gSPPolygon((*dl)++, OS_K0_TO_PHYSICAL(sGlowTris), 8, 1);
            }
            gDPPipeSync((*dl)++);
        }
        gDPSetPrimColor((*dl)++, 0, 0, 255, 255, 255, 255);
        func_80041DC0(dl);
    }
}
