/*
 * Runlink module 73: the engine glows of a moving craft. A table gives, per
 * object, glow flares to place at points of the craft's model; they fade in,
 * pulse, and are rendered sorted by distance from the camera.
 */
#include "module.h"
#include "gfx.h"

/* One glow of a craft (level data in this module) */
typedef struct M073GlowDef {
    /* 0x0 */ s8 point; /* the model's attachment point */
    /* 0x1 */ u8 unk1;
    /* 0x2 */ s16 unk2;
    /* 0x4 */ s16 unk4;
    /* 0x6 */ s16 unk6;
    /* 0x8 */ s16 unk8;
    /* 0xA */ s16 unkA;
    /* 0xC */ f32 unkC;
} M073GlowDef;

/* The glows of one kind of object */
typedef struct M073GlowSet {
    /* 0x0 */ s16 objectID; /* -1 ends the table */
    /* 0x2 */ s16 count;
    /* 0x4 */ M073GlowDef *defs;
} M073GlowSet;

/* A glow flare (what func_80068E58 makes) */
typedef struct M073Flare {
    /* 0x00 */ u8 pad0[0x20];
    /* 0x20 */ s16 size;
} M073Flare;

/* The object's glows (obj->unk68 points at 16 of these) */
typedef struct M073Glow {
    /* 0x0 */ M073Flare *flare;
    /* 0x4 */ u8 alpha;
    /* 0x5 */ u8 alphaFrom;
    /* 0x6 */ u8 alphaTo;
    /* 0x7 */ u8 fadeTimer;
    /* 0x8 */ u8 fadeLength;
    /* 0x9 */ u8 phase;
    /* 0xA */ u8 point;
    /* 0xB */ u8 matrix; /* the point's matrix in the model instance */
} M073Glow;

/* The craft's model */
typedef struct M073Model {
    /* 0x00 */ u8 pad0[0x1C];
    /* 0x1C */ Vertex *vertices;
    /* 0x20 */ u8 pad20[0x30 - 0x20];
    /* 0x30 */ u16 (*points)[2]; /* attachment points: vertex, matrix */
    /* 0x34 */ u8 pad34[0x4E - 0x34];
    /* 0x4E */ s8 unk4E; /* 0: the points must be transformed */
} M073Model;

/* What obj->unk6C points at */
typedef struct M073ModelInstance {
    /* 0x00 */ M073Model *model;
    /* 0x04 */ u8 pad4[0xB - 0x4];
    /* 0x0B */ s8 matrixSet;
    /* 0x0C */ u8 padC[0x10 - 0xC];
    /* 0x10 */ Mtx *matrices[22];
    /* 0x68 */ Vec3f *points; /* the attachment points, transformed */
} M073ModelInstance;

/* Kept in the object at 0x88 */
typedef struct M073State {
    /* 0x0 */ u32 soundHandle;
} M073State;

extern M073Flare *func_80068E58(s32 x, s32 y, s32 z, s32 arg3, s32 arg4, f32 arg5, f32 arg6, s32 arg7, s32 arg8,
                                 s32 arg9);
extern s32 func_8004966C(s16 angle);
extern void func_800694CC(M073Flare *flare, s16 angle);
extern void func_8006999C(M073Flare *flare, s16 size, u8 alpha);
extern void func_80026FD4(Object *obj, s32 updateRate);
extern void func_80068DF8(M073Flare *flare);
extern void func_80003614(s32 handle); /* stops a positioned sound */
extern void func_800496B0(s32 arg0, Object *obj, Vec3f *in, Vec3f *out);
extern f32 get_distance_to_camera(f32 x, f32 y, f32 z); /* distance from the camera */
extern void func_80041CA0(Gfx **dl, s32 arg1, Object *obj, f32 scale, f32 arg4);
extern void func_80069BE8(Gfx **dl, M073Flare *flare, s32 arg2, s32 alpha);

static M073GlowDef D_m073_00760[2] = {
    { 0, 0, 0x11, 0x12, 0, 0, 750, 35.0f },
    { 1, 0, 0x11, 0x12, 0, 0, 750, 35.0f },
};
static M073GlowDef D_m073_00780[2] = {
    { 0, 0, 0x11, 0x12, 0, 0, 750, 45.0f },
    { 1, 0, 0x11, 0x12, 0, 0, 750, 45.0f },
};
static M073GlowDef D_m073_007A0[2] = {
    { 0, 0, 0x11, 0x12, 0, 0, 750, 35.0f },
    { 1, 0, 0x11, 0x12, 0, 0, 750, 35.0f },
};
static M073GlowDef D_m073_007C0[2] = {
    { 0, 0, 0x11, 0x12, 0, 0, 750, 35.0f },
    { 1, 0, 0x11, 0x12, 0, 0, 750, 35.0f },
};
static M073GlowDef D_m073_007E0[13] = {
    { 0, 0, 0x11, 0x12, 0, -0x4000, 300, 25.0f },  { 1, 0, 0x11, 0x12, 0, -0x4000, 300, 25.0f },
    { 2, 0, 0x11, 0x12, 0, -0x4000, 300, 25.0f },  { 3, 0, 0x11, 0x12, 0, -0x4000, 300, 25.0f },
    { 4, 0, 0x11, 0x12, 0, -0x4000, 300, 25.0f },  { 5, 0, 0x11, 0x12, 0, -0x4000, 300, 25.0f },
    { 6, 0, 0x11, 0x12, 0, -0x4000, 300, 25.0f },  { 7, 0, 0x11, 0x12, 0, -0x4000, 300, 25.0f },
    { 8, 0, 0x11, 0x12, 0, -0x4000, 300, 25.0f },  { 9, 0, 0x11, 0x12, 0, -0x4000, 300, 25.0f },
    { 10, 0, 0x11, 0x12, 0, -0x4000, 300, 25.0f }, { 11, 0, 0x11, 0x12, 0, -0x4000, 300, 25.0f },
    { 12, 0x10, 0x11, 0x12, 0, 0, 800, 50.0f },
};
static M073GlowDef D_m073_008B0[16] = {
    { 0, 0, 0x11, 0x12, 0, -0x4000, 300, 25.0f },  { 1, 0, 0x11, 0x12, 0, -0x4000, 300, 25.0f },
    { 2, 0, 0x11, 0x12, 0, -0x4000, 300, 25.0f },  { 3, 0, 0x11, 0x12, 0, -0x4000, 300, 25.0f },
    { 4, 0, 0x11, 0x12, 0, -0x4000, 300, 25.0f },  { 5, 0, 0x11, 0x12, 0, -0x4000, 300, 25.0f },
    { 6, 0, 0x11, 0x12, 0, -0x4000, 300, 25.0f },  { 7, 0, 0x11, 0x12, 0, -0x4000, 300, 25.0f },
    { 8, 0, 0x11, 0x12, 0, -0x4000, 300, 25.0f },  { 9, 0, 0x11, 0x12, 0, -0x4000, 300, 25.0f },
    { 10, 0, 0x11, 0x12, 0, -0x4000, 300, 25.0f }, { 11, 0, 0x11, 0x12, 0, -0x4000, 300, 25.0f },
    { 12, 0, 0x11, 0x12, 0, -0x4000, 300, 25.0f }, { 13, 0, 0x11, 0x12, 0, -0x4000, 300, 25.0f },
    { 14, 0, 0x11, 0x12, 0, -0x4000, 300, 25.0f }, { 15, 0, 0x11, 0x12, 0, -0x4000, 300, 25.0f },
};
static M073GlowDef D_m073_009B0[2] = {
    { 0, 0, 0x11, 0x12, 0, -0x4000, 750, 35.0f },
    { 1, 0, 0x11, 0x12, 0, -0x4000, 750, 35.0f },
};
static M073GlowDef D_m073_009D0[4] = {
    { 1, 8, 0x10, 0x10, 0, -0x4000, 50, 20.0f },
    { 2, 8, 0x10, 0x10, 0, -0x4000, 50, 20.0f },
    { 3, 8, 0x10, 0x10, 0, -0x4000, 50, 20.0f },
    { 4, 8, 0x10, 0x10, 0, -0x4000, 50, 20.0f },
};
static M073GlowDef D_m073_00A10[2] = {
    { 0, 0, 0x11, 0x12, -0x8000, 0, 410, 42.0f },
    { 1, 0, 0x11, 0x12, -0x8000, 0, 410, 42.0f },
};
static M073GlowDef D_m073_00A30[5] = {
    { 0, 0, 0x11, 0x12, -0x8000, 0, 300, 25.0f },  { 1, 0, 0x11, 0x12, -0x8000, 0, 300, 25.0f },
    { 2, 0, 0x11, 0x12, 0, -0x4000, 300, 25.0f },  { 3, 0, 0x11, 0x12, 0, -0x4000, 300, 25.0f },
    { 4, 0, 0x11, 0x12, 0, -0x4000, 300, 25.0f },
};
static M073GlowDef D_m073_00A80[1] = {
    { 0, 0, 0x11, 0x12, 0, 0, 300, 35.0f },
};
static M073GlowDef D_m073_00A90[1] = {
    { 0, 0, 0x11, 0x12, 0, -0x4000, 180, 25.0f },
};
static M073GlowDef D_m073_00AA0[2] = {
    { 1, 0, 0x11, 0x12, 0, -0x4000, 180, 16.0f },
    { 2, 0, 0x11, 0x12, 0, -0x4000, 180, 16.0f },
};

static M073GlowSet D_m073_00AC0[] = {
    { 0x106, 2, D_m073_00760 },  { 0x8A, 2, D_m073_00780 },   { 0x10A, 2, D_m073_007A0 },
    { 0x2AB, 13, D_m073_007E0 }, { 0x2B2, 16, D_m073_008B0 }, { 0x2A8, 2, D_m073_009B0 },
    { 0x2AE, 4, D_m073_009D0 },  { 0x2BE, 2, D_m073_00A10 },  { 0x2CB, 2, D_m073_007C0 },
    { 0x2D3, 5, D_m073_00A30 },  { 0x237, 1, D_m073_00A80 },  { 0x287, 1, D_m073_00A90 },
    { 0x325, 2, D_m073_00AA0 },  { -1, 0, NULL },
};

void func_m073_00000(Object *obj, LevelObjectEntryCommon *setup) {
    M073Glow *glows;
    M073Glow *glow;
    M073GlowSet *set;
    M073GlowDef *def;
    s32 count;
    M073Model *model;
    Vertex *v;
    M073ModelInstance *inst;

    glows = obj->unk68;
    set = D_m073_00AC0;
    count = 0;
    def = NULL;
    for (; def == NULL && set->objectID >= 0; set++) {
        if (set->objectID == obj->objectID) {
            count = set->count;
            def = set->defs;
        }
    }
    inst = obj->modelInstances[obj->modelIndex];
    model = inst->model;
    if (def != NULL && model != NULL) {
        glow = glows;
        while (count--) {
            v = &model->vertices[model->points[def->point][0]];
            glow->flare = func_80068E58(v->x, v->y, v->z, def->unk6, def->unk8, def->unkC, def->unkA, def->unk1,
                                         def->unk2, def->unk4);
            glow->phase = func_80048E2C(0, 255);
            glow->point = def->point;
            glow->matrix = model->points[def->point][1];
            glow++;
            def++;
        }
    }
    if (obj->behaviorId == 0x96) {
        func_80079D1C(obj, -1, 0, 8, 0, 0, 0, 20.0f, 20.0f, -1.0f, 1.0f, 0x11);
    }
}

void func_m073_00208(Object *obj, s32 updateRate) {
    M073Glow *glow;
    M073Flare *flare;
    s32 size;
    s32 i;

    glow = obj->unk68;
    i = 16;
    while (i--) {
        flare = glow->flare;
        if (flare != NULL) {
            if (glow->fadeTimer < glow->fadeLength) {
                glow->fadeTimer += updateRate;
                if (glow->fadeTimer >= glow->fadeLength) {
                    glow->fadeTimer = glow->fadeLength;
                }
                glow->alpha = glow->alphaFrom + (glow->alphaTo - glow->alphaFrom) * glow->fadeTimer / glow->fadeLength;
            }
            glow->phase += updateRate * 16;
            size = (((func_8004966C(glow->phase << 8) * flare->size) >> 20) + flare->size) * glow->alpha >> 8;
            func_800694CC(flare, glow->phase << 8);
            func_8006999C(flare, size, glow->alpha);
        }
        glow++;
    }
    func_80026FD4(obj, updateRate);
}

void func_m073_0037C(Object *obj) {
    M073Glow *glow;
    s32 i;

    glow = obj->unk68;
    i = 16;
    while (i--) {
        if (glow->flare != NULL) {
            func_80068DF8(glow->flare);
        }
        glow++;
    }
    if (OBJ_STATE(obj, M073State)->soundHandle != 0) {
        func_80003614(OBJ_STATE(obj, M073State)->soundHandle);
    }
}

void func_m073_003F4(Gfx **dl, s32 arg1, Object *obj) {
    s16 i;
    s16 j;
    s16 n;
    s16 left;
    s16 swap;
    s16 unused; /* never used, but takes stack space */
    s16 order[16];
    f32 x;
    f32 y;
    f32 z;
    Vec3f v;
    Mtx *matrices[16];
    f32 dist[16];
    f32 scale;
    M073Glow *glows[16];
    M073ModelInstance *inst;
    M073Glow *glow;
    Vec3f *point;
    M073Model *model;

    inst = obj->modelInstances[obj->modelIndex];
    glow = obj->unk68;
    model = inst->model;
    n = 0;
    for (left = 16; left-- && glow->flare != NULL; glow++) {
        if (glow->alpha != 0) {
            point = &inst->points[glow->point];
            if (model->unk4E == 0) {
                func_800496B0(1, obj, point, &v);
                scale = obj->trans.scale;
                x = v.x * scale + obj->trans.x_position;
                y = v.y * scale + obj->trans.y_position;
                z = v.z * scale + obj->trans.z_position;
            } else {
                x = point->x;
                y = point->y;
                z = point->z;
            }
            order[n] = n;
            dist[n] = get_distance_to_camera(x, y, z);
            glows[n] = glow;
            /* the & 0xFF (a no-op) is needed to match */
            matrices[n] = &inst->matrices[inst->matrixSet][glow->matrix & 0xFF];
            n++;
        }
    }
    if (n > 0) {
        for (i = n - 1; i > 0; i--) {
            for (j = 0; j < i; j++) {
                if (dist[order[j + 1]] < dist[order[j]]) {
                    swap = order[j];
                    order[j] = order[j + 1];
                    order[j + 1] = swap;
                }
            }
        }
        if (model->unk4E == 0) {
            func_80041CA0(dl, arg1, obj, 1.0f, 0.0f);
            for (i = 0; i < n; i++) {
                func_80069BE8(dl, glows[order[i]]->flare, 6, glows[order[i]]->alpha);
            }
        } else {
            for (i = 0; i < n; i++) {
                gSPMatrix((*dl)++, OS_PHYSICAL_TO_K0(matrices[order[i]]), 0x81);
                func_80069BE8(dl, glows[order[i]]->flare, 6, glows[order[i]]->alpha);
            }
        }
        gMoveWd((*dl)++, 0xA, 0, 0);
    }
}
