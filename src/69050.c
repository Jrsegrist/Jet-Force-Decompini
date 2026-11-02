/*
 * Level effects and the screen transitions (fades). Functions in C here are
 * from the Diddy Kong Racing decompilation (CC0 1.0), src/fade_transition.c;
 * see CREDITS.md.
 */
#include "common.h"
#include "modules/gfx.h"

/* DKR src/fade_transition.h and include/structs.h */
#define FADE_FLAG_OUT (1 << 7) // Transition fading out
#define TRIN_DISABLE_TEXTURE 0

enum TransitionStatus {
    TRANSITION_NONE,
    TRANSITION_ACTIVE
};

typedef struct FadeTransition {
    /* 0x00 */ u8 type; // top 2 bits also act as flags
    /* 0x01 */ u8 red;
    /* 0x02 */ u8 green;
    /* 0x03 */ u8 blue;
    /* 0x04 */ u16 duration;
    /* 0x06 */ u16 endTimer;
} FadeTransition;

extern Vertex *sTransitionVtx[2];
extern Triangle *sTransitionTris[2];
extern Gfx dTransitionShapeSettings[];
void rendermode_reset(Gfx **dList);
extern f32 *gTransitionNextVtx;
extern f32 *gTransitionVertexTarget;
extern f32 *gTransitionVtxStep;
extern f32 gTransitionOpacityVel;
extern f32 sTransitionOpacity;
extern s32 gLastFadeBlue;
extern s32 gLastFadeBlueStep;
extern s32 gLastFadeGreen;
extern s32 gLastFadeGreenStep;
extern s32 gLastFadeRed;
extern s32 gLastFadeRedStep;
extern s32 gTransitionInvert;
extern s32 gTransitionVertexCount;
extern s32 sTransitionStatus;
extern s32 sTransitionTaskNum;
extern u16 gTransitionEndTimer;
extern u16 sTransitionFadeDuration;
extern u16 sTransitionFadeTimer;
extern u8 gCurFadeAlpha;
extern u8 gCurFadeBlue;
extern u8 gCurFadeGreen;
extern u8 gCurFadeRed;

/* Size: 0x2C bytes */
typedef struct LevelEffect {
    /* 0x00 */ u8 pad0[6];
    /* 0x06 */ u8 type;
    /* 0x07 */ u8 loaded;
    /* 0x08 */ u8 pad8[0x18 - 8];
    /* 0x18 */ s16 timer;
    /* 0x1A */ s16 age;
    /* 0x1C */ s16 id;
    /* 0x1E */ u8 pad1E[0x24 - 0x1E];
    /* 0x24 */ void *sprite;
    /* 0x28 */ s32 unk28;
} LevelEffect;
extern LevelEffect D_80102760[32];
void tex_free(void *tex);
void mempool_free(void *data);

/* JFG: clear the level effects. */
void func_80068450(void) {
    s32 i;
    LevelEffect *effect;

    for (i = 0, effect = D_80102760; i < 32; i++, effect++) {
        effect->type = 0;
        effect->loaded = 0;
    }
}

INCLUDE_ASM("asm/nonmatchings/69050", func_80068494);

void func_80068A14(LevelEffect *effect);

/* JFG: free the loaded level effects and clear their types. */
void func_80068874(void) {
    s32 i;
    LevelEffect *effect;

    for (i = 0, effect = D_80102760; i < 32; i++, effect++) {
        if (effect->loaded) {
            func_80068A14(effect);
        }
        effect->type = 0;
    }
}

void func_80068A14(LevelEffect *effect);

/* JFG: free the loaded level effects. */
void func_800688D8(void) {
    s32 i;
    LevelEffect *effect = D_80102760;

    for (i = 0; i < 32; i++, effect++) {
        if (effect->loaded) {
            func_80068A14(effect);
        }
    }
}

void func_800689BC(LevelEffect *effect);

/* JFG: release every active level effect whose id (s16 at 0x1C) matches. */
void func_80068938(s32 id) {
    s32 i;
    LevelEffect *effect;

    for (i = 0, effect = D_80102760; i < 32; i++, effect++) {
        if (effect->type && id == *(s16 *) ((u8 *) effect + 0x1C)) {
            func_800689BC(effect);
        }
    }
}

void func_800689B4(s32 arg0) {
}

void *func_80056798(s32 assetID, s32 flags);

void func_800689BC(LevelEffect *effect) {
    if (!effect->loaded) {
        if (effect->type == 4) {
            effect->sprite = func_80056798(0x57, 0);
        }
        effect->loaded = TRUE;
    }
}

void sprite_free(void *sprite);

void func_80068A14(LevelEffect *effect) {
    if (effect->loaded) {
        if (effect->type == 4) {
            sprite_free(effect->sprite);
        }
        effect->loaded = FALSE;
    }
}

s32 func_800577A8(void *sprite, s32 *arg1, s32 arg2, s32 *frame, s32 updateRate);

/* JFG: count down the level effects, releasing the expired ones and animating the sprites. */
void func_80068A64(s32 updateRate) {
    s32 i;
    s32 arg;
    LevelEffect *effect;

    for (i = 0, effect = D_80102760; i < 32; i++, effect++) {
        if (effect->type) {
            effect->timer -= updateRate;
            effect->age += updateRate;
            if (effect->timer <= 0) {
                effect->timer = 0;
                if (effect->loaded) {
                    func_80068A14(effect);
                }
                effect->type = 0;
            } else if (effect->type == 4) {
                arg = 1;
                if (func_800577A8(effect->sprite, &arg, 0xF, &effect->unk28, updateRate)) {
                    effect->timer = 0;
                }
            }
        }
    }
}

INCLUDE_ASM("asm/nonmatchings/69050", func_80068B54);

void func_80068DF8(void **cone) {
    if (cone[0] != NULL) {
        tex_free(cone[0]);
    }
    if (cone[1] != NULL) {
        tex_free(cone[1]);
    }
    mempool_free(cone);
}

typedef struct EffectMesh {
    /* 0x00 */ void *tex1;
    /* 0x04 */ void *tex2;
    /* 0x08 */ u8 *verts[2]; // double-buffered vertices
    /* 0x10 */ u8 *tris;
    /* 0x14 */ u8 numPoints;
    /* 0x15 */ u8 segments;
    /* 0x16 */ u8 unk16; // the current vertex buffer
    /* 0x17 */ u8 pad17;
    /* 0x18 */ f32 unk18;
    /* 0x1C */ s16 unk1C;
    /* 0x1E */ s16 unk1E;
    /* 0x20 */ s16 unk20;
    /* 0x22 */ s16 x;
    /* 0x24 */ s16 y;
    /* 0x26 */ s16 z;
} EffectMesh;
void *mempool_alloc_safe(s32 size, u32 colourTag);
void *load_texture(s32 assetIndex);
void func_800691F0(EffectMesh *mesh, s16 x, s16 y, s16 z, s32 arg4, s32 arg5, f32 arg6, f32 arg7);
void func_80069000(EffectMesh *mesh, s16 x, s16 y, s16 z, s32 arg4, s32 arg5, f32 arg6, f32 arg7);
void func_800694CC(EffectMesh *mesh, s32 arg1);

/* JFG: allocate an effect mesh with its three vertex buffers (segments + 1 points), load its two textures
   and build it (a single point when segments is 0). */
EffectMesh *effect_mesh_create(s16 x, s16 y, s16 z, s16 arg3, s16 arg4, f32 arg5, f32 arg6, s32 segments, s32 tex1,
                          s32 tex2) {
    s32 sizeA;
    s32 sizeB;
    EffectMesh *mesh;

    if (segments == 0) {
        sizeA = 0x280;
        sizeB = 0x154;
    } else {
        sizeA = segments * 16;
        sizeB = segments * 10 + 10;
    }
    mesh = mempool_alloc_safe(sizeof(EffectMesh) + sizeA + (sizeB * 2), -1);
    if (mesh != NULL) {
        if (tex1 >= 0) {
            mesh->tex1 = load_texture(tex1);
        } else {
            mesh->tex1 = NULL;
        }
        if (tex2 >= 0) {
            mesh->tex2 = load_texture(tex2);
        } else {
            mesh->tex2 = NULL;
        }
        mesh->tris = (u8 *) mesh + 0x28;
        mesh->verts[0] = mesh->tris + sizeA;
        mesh->verts[1] = mesh->verts[0] + sizeB;
        mesh->numPoints = segments + 1;
        mesh->unk16 = 0;
        mesh->segments = segments;
        mesh->unk1E = arg4;
        mesh->unk1C = arg3;
        mesh->unk20 = arg6;
        mesh->unk18 = arg5;
        mesh->x = x;
        mesh->y = y;
        mesh->z = z;
        if (segments + 1 == 1) {
            func_800691F0(mesh, x, y, z, arg3, arg4, arg5, arg6);
        } else {
            func_80069000(mesh, x, y, z, arg3, arg4, arg5, arg6);
        }
        func_800694CC(mesh, 0);
    }
    return mesh;
}

INCLUDE_ASM("asm/nonmatchings/69050", func_80069000);

INCLUDE_ASM("asm/nonmatchings/69050", func_800691F0);

INCLUDE_ASM("asm/nonmatchings/69050", func_800694CC);

INCLUDE_ASM("asm/nonmatchings/69050", func_8006999C);

#ifdef NON_MATCHING
void func_80055C00();

/* NON_MATCHING: 8 words: in the segments branch numPoints and its * 8 sit in a0/a1 (target v1/a0), segments in v0 (target a2) */
/* JFG: draw an effect mesh: one textured fan of 18 points (segments 0) or a strip of segments triangles. */
void func_80069BE8(Gfx **dList, EffectMesh *mesh, s32 flags, u8 alpha) {
    s32 hasTex;

    if (mesh != NULL) {
        gDPPipeSync((*dList)++);
        gDPSetPrimColor((*dList)++, 0, 0, 255, 255, 255, alpha);
        gDPSetEnvColor((*dList)++, 255, 255, 255, 0);
        if (mesh->tex1 != NULL) {
            hasTex = TRUE;
        } else {
            hasTex = FALSE;
        }
        if (mesh->segments == 0) {
            gSPVertexJFG((*dList)++, OS_K0_TO_PHYSICAL(mesh->verts[mesh->unk16]), 18, 0);
            func_80055C00(dList, mesh->tex2, flags, 0);
            gSPPolygon((*dList)++, OS_K0_TO_PHYSICAL(mesh->tris), 16, hasTex);
            func_80055C00(dList, mesh->tex1, flags, 0);
            gSPPolygon((*dList)++, OS_K0_TO_PHYSICAL(mesh->tris + 0x200), 8, hasTex);
            func_80055C00(dList, mesh->tex2, flags, 0);
            gSPPolygon((*dList)++, OS_K0_TO_PHYSICAL(mesh->tris + 0x100), 16, hasTex);
        } else {
            func_80055C00(dList, mesh->tex1, flags, 0);
            gDma1p((*dList)++, G_VTX, OS_K0_TO_PHYSICAL(mesh->verts[mesh->unk16]),
                   mesh->numPoints * 8 + 8 + mesh->numPoints * 2,
                   (mesh->numPoints * 8) | ((u32) OS_K0_TO_PHYSICAL(mesh->verts[mesh->unk16]) & 6));
            gSPPolygon((*dList)++, OS_K0_TO_PHYSICAL(mesh->tris), mesh->segments, hasTex);
        }
        gDPPipeSync((*dList)++);
        gDPSetPrimColor((*dList)++, 0, 0, 255, 255, 255, 255);
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/69050", func_80069BE8);
#endif

f32 func_80049C30(s32 angle);
f32 func_80049C34(s32 angle);

/* JFG: rotate count points (rotY, then rotX), add (x, y, z) and write them as white vertices. */
void vertices_rotate_translate(s32 count, s16 x, s16 y, s16 z, s16 rotX, s16 rotY, f32 *src, Vertex *dst) {
    f32 sinY;
    f32 cosY;
    f32 sinX;
    f32 cosX;
    f32 px;
    f32 py;
    f32 pz;
    f32 t;

    sinY = func_80049C34(rotY);
    cosY = func_80049C30(rotY);
    sinX = func_80049C34(rotX);
    cosX = func_80049C30(rotX);
    while (count--) {
        px = src[0];
        py = src[1];
        pz = src[2];
        src += 3;
        t = pz * cosY + py * sinY;
        dst->x = (s32) (px * cosX + t * sinX) + x;
        dst->y = (s32) (py * cosY - pz * sinY) + y;
        dst->z = (s32) (t * cosX - px * sinX) + z;
        dst->r = 255;
        dst->g = 255;
        dst->b = 255;
        dst->a = 255;
        dst++;
    }
}

INCLUDE_ASM("asm/nonmatchings/69050", func_8006A0C0);

INCLUDE_ASM("asm/nonmatchings/69050", func_8006A6B0);

void tex_free(void *tex);
void mempool_free(void *data);

void func_8006A894(u8 *obj) {
    void *tex = *(void **) (obj + 0x30);
    if (tex != NULL) {
        tex_free(tex);
    }
    mempool_free(obj);
}

void func_8006A8D4(u8 *ripple) {
    if (*(void **) (ripple + 0x70) != NULL) {
        tex_free(*(void **) (ripple + 0x70));
    }
    if (*(void **) (ripple + 0x84) != NULL) {
        func_8006A894(*(void **) (ripple + 0x84));
    }
}

INCLUDE_ASM("asm/nonmatchings/69050", func_8006A924);

INCLUDE_ASM("asm/nonmatchings/69050", func_8006B090);

INCLUDE_ASM("asm/nonmatchings/69050", func_8006B3CC);

/* JFG: a textured two-triangle quad with two vertex frames (ripple_quad_draw) */
typedef struct RippleTexture {
    /* 0x00 */ u8 pad0[6];
    /* 0x06 */ s16 flags;
} RippleTexture;

typedef struct RippleQuad {
    /* 0x00 */ u8 tris[0x20];     // two Triangles
    /* 0x20 */ u8 vtx[2][0x28];   // two frames of vertices
    /* 0x70 */ RippleTexture *tex;
    /* 0x74 */ u8 frame;
    /* 0x75 */ u8 pad75;
    /* 0x76 */ s16 alpha;
    /* 0x78 */ s16 texFrame;
    /* 0x7A */ u8 pad7A[0x84 - 0x7A];
    /* 0x84 */ void *child;
} RippleQuad;

extern s32 D_800A6950;
extern Gfx D_800A6958[];
void func_80055C00(Gfx **dList, RippleTexture *tex, s32 flags, s32 frame);
void func_8006B3CC(void *child, Gfx **dList);

/* DKR f3ddkr.h: runs a display list of numberOfCommands commands */
#define gDkrDmaDisplayList(pkt, address, numberOfCommands)                                           \
    {                                                                                                \
        Gfx *_g = (Gfx *) (pkt);                                                                     \
        _g->words.w0 = (_SHIFTL(7, 24, 8) | _SHIFTL((numberOfCommands), 16, 8) |                      \
                        _SHIFTL((numberOfCommands * 8), 0, 16));                                     \
        _g->words.w1 = (unsigned int) (address);                                                     \
    }

/* JFG: draw a textured quad (faded by its alpha and D_800A6950), then its child, then reset the colours. */
void ripple_quad_draw(RippleQuad *obj, Gfx **dList) {
    s32 alpha;

    if (obj != NULL && obj->tex != NULL) {
        if (obj->alpha != 0) {
            func_80055C00(dList, obj->tex, 0xF, obj->texFrame << 8);
            gDkrDmaDisplayList((*dList)++, (u8 *) D_800A6958 + 0x80000000, 2);
            if (obj->tex->flags & 0x40) {
                alpha = obj->texFrame & 0xFF;
            } else {
                alpha = 255;
            }
            gDPSetPrimColor((*dList)++, 0, 0, 255, 255, 255, ((255 - (D_800A6950 >> 1)) * obj->alpha) >> 8);
            gDPSetEnvColor((*dList)++, alpha, alpha, alpha, alpha);
            gSPVertexJFG((*dList)++, (u8 *) obj->vtx[obj->frame] + 0x80000000, 5, 0);
            gSPPolygon((*dList)++, (u32) obj + 0x80000000, 2, 1);
            rendermode_reset(dList);
        }
        if (obj->child != NULL) {
            func_8006B3CC(obj->child, dList);
        }
        gDPPipeSync((*dList)++);
        gDPSetPrimColor((*dList)++, 0, 0, 255, 255, 255, 255);
        gDPSetEnvColor((*dList)++, 255, 255, 255, 255);
    }
}

void *mempool_alloc_safe(s32 size, u32 colourTag);
void func_8006F874(void);

INCLUDE_ASM("asm/nonmatchings/69050", func_8006B8C4);

extern s32 gTransitionsDisabled;

/* Sets the flag that blocks new screen transitions (DKR's name) (0x8006B95C). */
void enable_new_screen_transitions(void) {
    gTransitionsDisabled = 0x1;
}

extern s32 gTransitionsDisabled;

/* Clears the flag that blocks new screen transitions (DKR's name) (0x8006B96C). */
void disable_new_screen_transitions(void) {
    gTransitionsDisabled = 0;
}

extern s32 D_800A6784;
extern s8 gTransitionFadeIn;

/* Returns whether a transition is still fading (0x8006B978). */
s32 check_fadeout_transition(void) {
    if (D_800A6784 != 0 && (sTransitionFadeTimer != 0 || (gTransitionEndTimer != 0 && gTransitionFadeIn != 0))) {
        return 1;
    }
    return 0;
}

extern s32 D_800A6788;  // DKR: sLevelTransitionDelayTimer
extern s32 D_80102D38;  // DKR: gCurFadeTransition
void transition_end(void);
extern s16 D_800A6810[]; // barn door (horizontal) coords
extern s16 D_800A6848[]; // barn door (vertical) coords
extern s16 D_800A68BC[]; // barn door (diagonal) coords
extern u8 D_800A6880[];
extern u8 D_800A688C[];
extern u8 D_800A6898[];
extern u8 D_800A68B0[];
extern u8 D_800A68EC[];
extern u8 D_800A68F8[];
extern u8 D_800A6904[];
extern u8 D_800A6918[];
void transition_fullscreen_start(FadeTransition *transition);
void func_8006C230(FadeTransition *transition, s32 numVerts, s32 numTris, s16 *coords, u8 *nextPos, u8 *targetPos,
                   u8 *alpha1, u8 *alpha2, u8 *tris); // DKR: transition_init_shape
void transition_init_blank(FadeTransition *transition);

/**
 * DKR: transition_begin. Resets the state, then sets up the transition from its description.
 * JFG: also sets D_800A6784; the shape transitions share one init function.
 */
s32 transition_begin(FadeTransition *transition) {
    if (gTransitionsDisabled) {
        return 0;
    }
    transition_end();
    D_800A6784 = 1;
    if (transition == NULL) {
        return 0;
    }
    sTransitionFadeTimer = transition->duration;
    sTransitionFadeDuration = transition->duration;
    gTransitionEndTimer = transition->endTimer;
    gTransitionFadeIn = !(transition->type & FADE_FLAG_OUT);
    D_80102D38 = transition->type & 0x3F;
    gTransitionInvert = transition->type & 0x40;
    D_800A6788 = 0;
    if (!gTransitionFadeIn && D_800A6788 == 0) {
        D_800A6788 = 2;
    }
    if (sTransitionFadeTimer > 0) {
        gLastFadeRed = gCurFadeRed;
        gLastFadeGreen = gCurFadeGreen;
        gLastFadeBlue = gCurFadeBlue;
        gCurFadeRed = transition->red;
        gCurFadeGreen = transition->green;
        gCurFadeBlue = transition->blue;
        switch (D_80102D38) {
            case 0:
                transition_fullscreen_start(transition);
                break;
            case 1:
                func_8006C230(transition, 12, 8, D_800A6810, D_800A6880, D_800A688C, D_800A68B0, D_800A68B0,
                              D_800A6898);
                break;
            case 2:
                func_8006C230(transition, 12, 8, D_800A6848, D_800A6880, D_800A688C, D_800A68B0, D_800A68B0,
                              D_800A6898);
                break;
            case 3:
            case 4:
            case 5:
                func_8006C230(transition, 10, 6, D_800A68BC, D_800A68EC, D_800A68F8, D_800A6918, D_800A6918,
                              D_800A6904);
                break;
            case 6:
                transition_init_blank(transition);
                break;
        }
    }
    return sTransitionStatus;
}

void func_8006BF50(s32 updateRate);
void func_8006C77C(s32 updateRate);
void func_8006CE78(s32 updateRate);

/* DKR: transition_update. Returns the transition status. */
s32 transition_update(s32 updateRate) {
    if (D_800A6788) {
        D_800A6788--;
        updateRate = 0;
    } else if (updateRate >= 6) {
        updateRate = 5;
    }

    if (sTransitionStatus != 0) {
        if (sTransitionFadeTimer == 0) {
            sTransitionStatus = -1;
        }
        if (sTransitionFadeTimer == 0 && gTransitionEndTimer == 0) {
            transition_end();
        } else {
            if (sTransitionFadeTimer > 0) {
                sTransitionTaskNum = 1 - sTransitionTaskNum;
            }
            switch (D_80102D38) {
                case 0:
                    func_8006BF50(updateRate);
                    break;
                case 1:
                case 2:
                case 3:
                case 4:
                case 5:
                    func_8006C77C(updateRate);
                    break;
                case 6:
                    func_8006CE78(updateRate);
                    break;
            }
        }
    }
    return sTransitionStatus;
}

void set_ortho_matrix_height(f32 height);
void mtx_ortho(Gfx **dList, Mtx **mtx);
void transition_render_fullscreen(Gfx **dList, Mtx **matrix, Vertex **vtx);
void transition_render_barndoor_hor(Gfx **dList, Mtx **matrix, Vertex **vtx);
void transition_render_barndoor_vert(Gfx **dList, Mtx **matrix, Vertex **vtx);
void transition_render_barndoor_diag(Gfx **dList, Mtx **matrix, Vertex **vtx);
void transition_render_blank(Gfx **dList, Mtx **matrix, Vertex **vtx);
void func_8004046C(Gfx **dList, Mtx **mtx);

/* DKR: transition_render. */
void transition_render(Gfx **dList, Mtx **mtx, Vertex **vtx) {
    if (sTransitionStatus) {
        if (osTvType == OS_TV_PAL) {
            set_ortho_matrix_height(1.4f);
        } else {
            set_ortho_matrix_height(1.2f);
        }
        mtx_ortho(dList, mtx);
        set_ortho_matrix_height(1.0f);
        switch (D_80102D38) {
            case 0:
                transition_render_fullscreen(dList, mtx, vtx);
                break;
            case 1:
                transition_render_barndoor_hor(dList, mtx, vtx);
                break;
            case 2:
                transition_render_barndoor_vert(dList, mtx, vtx);
                break;
            case 3:
            case 4:
            case 5:
                transition_render_barndoor_diag(dList, mtx, vtx);
                break;
            case 6:
                transition_render_blank(dList, mtx, vtx);
                break;
        }
        func_8004046C(dList, mtx);
    }
}

/* JFG: clear the transition state (0x8006BE88). */
void transition_end(void) {
    sTransitionStatus = 0;
    sTransitionTaskNum = 0;
}

/**
 * Sets the transition timer and opacity to positive or negative depending on whether to fade in or out.
 */
void transition_fullscreen_start(FadeTransition *transition) {
    if (transition->type & FADE_FLAG_OUT) {
        sTransitionOpacity = 255.0f;
        gTransitionOpacityVel = -255.0f / (f32) sTransitionFadeTimer;
    } else {
        sTransitionOpacity = 0.0f;
        gTransitionOpacityVel = 255.0f / (f32) sTransitionFadeTimer;
    }
    sTransitionStatus = TRANSITION_ACTIVE;
}

/**
 * Slowly dade in or out by increasing or decreasing opacity.
 */
void transition_update_fullscreen(s32 updateRate) {
    s32 var_v0;
    do {
        var_v0 = TRUE;
        if ((sTransitionFadeTimer) > 0) {
            if (updateRate < sTransitionFadeTimer) {
                sTransitionFadeTimer -= updateRate;
                sTransitionOpacity += gTransitionOpacityVel * updateRate;
            } else {
                updateRate -= sTransitionFadeTimer;
                var_v0 = FALSE;
                if (gTransitionOpacityVel < 0.0f) {
                    sTransitionOpacity = 0.0f;
                } else {
                    sTransitionOpacity = 255.0f;
                }
                sTransitionFadeTimer = 0 * updateRate; // Fakematch
            }
            if (sTransitionOpacity < 0.0f) {
                sTransitionOpacity = 0.0f;
            } else if (sTransitionOpacity > 255.0f) {
                sTransitionOpacity = 255.0f;
            }
            gCurFadeAlpha = sTransitionOpacity;
        } else if (gTransitionEndTimer != 0xFFFF) {
            if (updateRate < gTransitionEndTimer) {
                gTransitionEndTimer -= updateRate;
            } else {
                updateRate -= gTransitionEndTimer;
                gTransitionEndTimer = 0;
                if (gTransitionInvert != 0) {
                    gTransitionInvert = 0;
                    sTransitionFadeTimer = sTransitionFadeDuration;
                    gTransitionOpacityVel = -gTransitionOpacityVel;
                    var_v0 = FALSE;
                }
            }
        }
    } while (var_v0 == FALSE && updateRate > 0);
}

extern Gfx dTransitionFadeSettings[];
void fb_size(u32 *width, u32 *height);

/* Draws a fullscreen fill rectangle that fades in or out (0x8006C124). */
/**
 * Draws a simple fillrect covering the whole screen that fades in or out.
 * DKR: transition_render_fullscreen.
 */
void transition_render_fullscreen(Gfx **dList, UNUSED Mtx **matrix, UNUSED Vertex **vtx) {
    u32 width;
    u32 height;

    fb_size(&width, &height);
    gSPDisplayList((*dList)++, dTransitionFadeSettings);
    gDPSetPrimColor((*dList)++, 0, 0, gCurFadeRed, gCurFadeGreen, gCurFadeBlue, gCurFadeAlpha);
    gDPSetCombineMode((*dList)++, G_CC_PRIMITIVE, G_CC_PRIMITIVE);
    gDPFillRectangle((*dList)++, 0, 0, width, height);
    rendermode_reset(dList);
}

INCLUDE_ASM("asm/nonmatchings/69050", func_8006C230);

/**
 * Slowly update the vertices of the transition based on what the next frames target is.
 * That target is set based on a velocity.
 */
void transition_update_shape(s32 updateRate) {
    s32 i;
    f32 updateRateF;

    updateRateF = updateRate;
    if (sTransitionFadeTimer > 0) {
        if (updateRate < sTransitionFadeTimer) {
            for (i = 0; i < gTransitionVertexCount; i++) {
                gTransitionNextVtx[i * 3] += updateRateF * gTransitionVtxStep[i * 3];
                gTransitionNextVtx[i * 3 + 1] += updateRateF * gTransitionVtxStep[i * 3 + 1];
                gTransitionNextVtx[i * 3 + 2] += updateRateF * gTransitionVtxStep[i * 3 + 2];
            }
            sTransitionFadeTimer -= updateRate;
        } else {
            for (i = 0; i < gTransitionVertexCount; i++) {
                gTransitionNextVtx[i * 3] = gTransitionVertexTarget[i * 3];
                gTransitionNextVtx[i * 3 + 1] = gTransitionVertexTarget[i * 3 + 1];
                gTransitionNextVtx[i * 3 + 2] = gTransitionVertexTarget[i * 3 + 2];
            }
            sTransitionFadeTimer = 0;
        }
        for (i = 0; i < gTransitionVertexCount; i++) {
            sTransitionVtx[sTransitionTaskNum][i].x = gTransitionNextVtx[i * 3];
            sTransitionVtx[sTransitionTaskNum][i].y = gTransitionNextVtx[i * 3 + 1];
            sTransitionVtx[sTransitionTaskNum][i].a = (s32) gTransitionNextVtx[i * 3 + 2];
        }
    } else if (gTransitionEndTimer != 0xFFFF) {
        if (updateRate < gTransitionEndTimer) {
            gTransitionEndTimer -= updateRate;
        } else {
            gTransitionEndTimer = 0;
        }
    }
}

/* Renders the horizontal barn door transition (0x8006CA30). */
/**
 * Renders a transition effect on screen that will close in from both sides horizonally.
 * DKR: transition_render_barndoor_hor (JFG: 13 vertices).
 */
void transition_render_barndoor_hor(Gfx **dList, UNUSED Mtx **matrix, UNUSED Vertex **vtx) {
    rendermode_reset(dList);
    gSPDisplayList((*dList)++, dTransitionShapeSettings);
    gSPVertexJFG((*dList)++, OS_K0_TO_PHYSICAL(sTransitionVtx[sTransitionTaskNum]), 13, 0);
    gSPPolygon((*dList)++, OS_K0_TO_PHYSICAL(sTransitionTris[sTransitionTaskNum]), 8, TRIN_DISABLE_TEXTURE);
    rendermode_reset(dList);
}

/* Renders the vertical barn door transition (0x8006CB28). */
/**
 * Renders a transition effect on screen that will close in from both sides vertically.
 * DKR: transition_render_barndoor_vert (JFG: 13 vertices).
 */
void transition_render_barndoor_vert(Gfx **dList, UNUSED Mtx **matrix, UNUSED Vertex **vtx) {
    rendermode_reset(dList);
    gSPDisplayList((*dList)++, dTransitionShapeSettings);
    gSPVertexJFG((*dList)++, OS_K0_TO_PHYSICAL(sTransitionVtx[sTransitionTaskNum]), 13, 0);
    gSPPolygon((*dList)++, OS_K0_TO_PHYSICAL(sTransitionTris[sTransitionTaskNum]), 8, TRIN_DISABLE_TEXTURE);
    rendermode_reset(dList);
}

/* Renders the diagonal barn door transition (0x8006CC20). */
/**
 * Renders a transition effect closing in from the corners.
 * DKR: transition_render_barndoor_diag (JFG: 11 vertices).
 */
void transition_render_barndoor_diag(Gfx **dList, UNUSED Mtx **matrix, UNUSED Vertex **vtx) {
    rendermode_reset(dList);
    gSPDisplayList((*dList)++, dTransitionShapeSettings);
    gSPVertexJFG((*dList)++, OS_K0_TO_PHYSICAL(sTransitionVtx[sTransitionTaskNum]), 11, 0);
    gSPPolygon((*dList)++, OS_K0_TO_PHYSICAL(sTransitionTris[sTransitionTaskNum]), 6, TRIN_DISABLE_TEXTURE);
    rendermode_reset(dList);
}

/**
 * Set the transition colour target, then set the velocity for the colour to fade to.
 */
void transition_init_blank(UNUSED FadeTransition *transition) {
    gLastFadeRed <<= 16;
    gLastFadeGreen <<= 16;
    gLastFadeBlue <<= 16;
    gCurFadeAlpha = 255;
    sTransitionOpacity = 255.0f;
    gTransitionOpacityVel = 0.0f;
    gLastFadeRedStep = ((gCurFadeRed << 16) - gLastFadeRed) / sTransitionFadeTimer;
    gLastFadeGreenStep = ((gCurFadeGreen << 16) - gLastFadeGreen) / sTransitionFadeTimer;
    gLastFadeBlueStep = ((gCurFadeBlue << 16) - gLastFadeBlue) / sTransitionFadeTimer;
    sTransitionStatus = TRANSITION_ACTIVE;
}

/**
 * Shifts the colours of the transition over time while the timer ticks down.
 */
void transition_update_blank(s32 updateRate) {
    s32 breakout = FALSE;
    while (!breakout) {
        breakout = TRUE;
        if (sTransitionFadeTimer > 0) {
            gLastFadeRed += gLastFadeRedStep * updateRate;
            gLastFadeGreen += gLastFadeGreenStep * updateRate;
            gLastFadeBlue += gLastFadeBlueStep * updateRate;
            if (updateRate >= sTransitionFadeTimer) {
                gLastFadeRed = gCurFadeRed << 16;
                gLastFadeGreen = gCurFadeGreen << 16;
                gLastFadeBlue = gCurFadeBlue << 16;
                updateRate -= sTransitionFadeTimer;
                sTransitionFadeTimer = 0;
            } else {
                sTransitionFadeTimer -= updateRate;
            }
        } else {
            if (gTransitionEndTimer != 0xFFFF) {
                gTransitionEndTimer -= updateRate;
                if (gTransitionEndTimer <= 0) {
                    gTransitionEndTimer = 0;
                }
            }
        }
    }
}

/* Fills the whole screen with the transition colour (0x8006CFC0). */
/**
 * Fill the whole screen with a solid colour.
 * DKR: transition_render_blank.
 */
void transition_render_blank(Gfx **dList, UNUSED Mtx **matrix, UNUSED Vertex **vtx) {
    u32 width;
    u32 height;

    fb_size(&width, &height);
    gSPDisplayList((*dList)++, dTransitionFadeSettings);
    gDPSetPrimColor((*dList)++, 0, 0, (gLastFadeRed >> 16), (gLastFadeGreen >> 16), (gLastFadeBlue >> 16), 255);
    gDPSetCombineMode((*dList)++, G_CC_PRIMITIVE, G_CC_PRIMITIVE);
    gDPFillRectangle((*dList)++, 0, 0, width, height);
    rendermode_reset(dList);
}

extern s32 D_80103B80[2];
extern s32 D_80103B88[2];
extern s32 D_80103B90;
extern s32 D_80103B94;

void func_8006D0D8(void) {
    D_80103B80[0] = 0;
    D_80103B80[1] = 0;
    D_80103B88[0] = 0;
    D_80103B88[1] = 0;
    D_80103B90 = 0;
    D_80103B94 = -1;
}

/* JFG: clip the line (x1, y1)-(x2, y2) to the rectangle; FALSE if nothing of it is left. */
s32 line_clip_rect(s16 *px1, s16 *py1, s16 *px2, s16 *py2, s32 left, s32 top, s32 right, s32 bottom) {
    s32 xa;
    s32 ya;
    s32 xb;
    s32 yb;
    s32 swapped;
    s32 dx;
    s32 dy;
    s32 tmp;
    s32 tmp2;

    if (*px2 >= *px1) {
        xa = *px1;
        ya = *py1;
        xb = *px2;
        yb = *py2;
        swapped = FALSE;
    } else {
        xa = *px2;
        ya = *py2;
        xb = *px1;
        yb = *py1;
        swapped = TRUE;
    }
    if (xb < left || right < xa) {
        return FALSE;
    }
    dx = xb - xa;
    if (xa != xb) {
        dy = yb - ya;
        if (right < xb) {
            yb = (right - xa) * dy / dx + ya;
            xb = right;
        }
        if (xa < left) {
            ya = (left - xa) * dy / dx + ya;
            xa = left;
        }
    }
    if (yb < ya) {
        tmp = xa;
        tmp2 = ya;
        xa = xb;
        ya = yb;
        xb = tmp;
        yb = tmp2;
        swapped ^= 1;
    }
    if (yb < top || bottom < ya) {
        return FALSE;
    }
    dx = xb - xa;
    if (ya != yb) {
        dy = yb - ya;
        if (bottom < yb) {
            xb = (bottom - ya) * dx / dy + xa;
            yb = bottom;
        }
        if (ya < top) {
            xa = (top - ya) * dx / dy + xa;
            ya = top;
        }
    }
    if (swapped) {
        *px1 = xb;
        *py1 = yb;
        *px2 = xa;
        *py2 = ya;
    } else {
        *px1 = xa;
        *py1 = ya;
        *px2 = xb;
        *py2 = yb;
    }
    return TRUE;
}

typedef struct ScreenLine {
    /* 0x00 */ s16 x;
    /* 0x02 */ s16 y;
    /* 0x04 */ s32 u : 12;
    /* 0x04 */ s32 v : 12;
    /* 0x07 */ s32 flags : 8;
} ScreenLine;
extern ScreenLine D_80102DC0[2][0x94];

void menu_layout_box_get(s32 box, s32 *left, s32 *top, s32 *right, s32 *bottom);
s32 line_clip_rect(s16 *x1, s16 *y1, s16 *x2, s16 *y2, s32 left, s32 top, s32 right, s32 bottom);

/* JFG: queue a line clipped to the current window (or the screen); sorted unless the type is 2, 12 or 13. */
void screen_line_add(s16 x1, s16 y1, s16 x2, s16 y2, u8 colour) {
    s32 left;
    s32 top;
    s32 right;
    s32 bottom;
    ScreenLine *entry;
    u32 width;
    u32 height;
    s32 tmp;

    if (D_80103B80[D_80103B90] < 0x94) {
        if (D_80103B94 != -1) {
            menu_layout_box_get(D_80103B94, &left, &top, &right, &bottom);
            left++;
            top++;
            right -= 2;
            bottom -= 2;
        } else {
            fb_size(&width, &height);
            left = 1;
            top = 1;
            right = width - 2;
            bottom = height - 2;
        }
        if (line_clip_rect(&x1, &y1, &x2, &y2, left, top, right, bottom)) {
            if (x1 != x2 || y1 != y2) {
                if ((colour & 0xF) != 12 && (colour & 0xF) != 13 && (colour & 0xF) != 2) {
                    if (x2 < x1) {
                        tmp = x1;
                        x1 = x2;
                        x2 = tmp;
                    }
                    if (y2 < y1) {
                        tmp = y1;
                        y1 = y2;
                        y2 = tmp;
                    }
                }
                entry = &D_80102DC0[D_80103B90][D_80103B80[D_80103B90]++];
                entry->x = x1;
                entry->u = y1;
                entry->y = x2;
                entry->v = y2;
                entry->flags = colour;
            }
        }
    }
}

/* JFG: draw a line with D_80103B94 temporarily set to `window`. */
void func_8006D58C(s16 x1, s16 y1, s16 x2, s16 y2, u8 colour, s32 window) {
    s32 prevWindow = D_80103B94;

    D_80103B94 = window;
    screen_line_add(x1, y1, x2, y2, colour);
    D_80103B94 = prevWindow;
}

extern s32 D_80103B94;

void func_8006D600(s32 arg0) {
    D_80103B94 = arg0;
}

typedef struct DebugText {
    /* 0x00 */ s16 x;
    /* 0x02 */ s16 y;
    /* 0x04 */ char text[32];
} DebugText;
extern DebugText D_80103700[2][16];
extern u8 D_800A4FC4;

/* JFG: queue a 32-character text line at (x, y) if it is on screen (up to 16 per frame). */
void func_8006D60C(s16 x, s16 y, char *text) {
    DebugText *entry;
    char *dst;
    s32 i;
    u32 width;
    u32 height;

    if (!D_800A4FC4) {
        fb_size(&width, &height);
        if (x > 0 && x <= (s32) (width - 6) && y > 0 && y <= (s32) (height - 6)) {
            if (D_80103B88[D_80103B90] < 16) {
                entry = &D_80103700[D_80103B90][D_80103B88[D_80103B90]++];
                entry->x = x;
                entry->y = y;
                dst = entry->text;
                i = 32;
                while (i--) {
                    *dst++ = *text++;
                }
            }
        }
    }
}

/* JFG: write a number as decimal text; with `decimal` set, the last digit goes after a '.' (0x8006D70C). */
void s32_to_decimal_string(char *buf, s32 value, s32 decimal) {
    s32 divisor = 1000000;
    s32 started = FALSE;
    s32 digit;

    if (value < 0) {
        value = -value;
        *buf++ = '-';
    }
    do {
        digit = value / divisor;
        value %= divisor;
        if (digit != 0 || started || divisor == 100) {
            *buf++ = digit + '0';
            started = TRUE;
        }
        divisor /= 10;
        if (divisor == 10) {
            if (decimal) {
                *buf++ = '.';
            } else {
                divisor = 0;
            }
        }
    } while (divisor != 0);
    *buf = '\0';
}

INCLUDE_ASM("asm/nonmatchings/69050", func_8006D7F0);

/* JFG: add (signed) amounts to the channels of an RGBA5551 colour, clamped to 0..31 (0x8006DA1C). */
void rgba5551_add_clamped(u16 *colour, s8 addRed, s8 addGreen, s8 addBlue) {
    s16 red = ((*colour >> 11) & 0x1F) + addRed;
    s16 green = ((*colour >> 6) & 0x1F) + addGreen;
    s16 blue = ((*colour >> 1) & 0x1F) + addBlue;

    if (red >= 32) {
        red = 31;
    }
    if (green >= 32) {
        green = 31;
    }
    if (blue >= 32) {
        blue = 31;
    }
    if (red < 0) {
        red = 0;
    }
    if (green < 0) {
        green = 0;
    }
    if (blue < 0) {
        blue = 0;
    }
    *colour = (red << 11) | (green << 6) | (blue << 1) | 1;
}

/* JFG: brighten the red and green channels of an RGBA5551 colour (saturating) (0x8006DAE8). */
void rgba5551_brighten_rg(u16 *colour, u8 addRed, u8 addGreen) {
    s16 red = ((*colour >> 11) & 0x1F) + addRed;
    s16 green = ((*colour >> 6) & 0x1F) + addGreen;
    s16 blue = (*colour >> 1) & 0x1F;

    if (red >= 32) {
        red = 31;
    }
    if (green >= 32) {
        green = 31;
    }
    *colour = (red << 11) | (green << 6) | (blue << 1) | 1;
}

#ifdef NON_MATCHING
/* NON_MATCHING: frame 0x60 (target 0x50: width/height at sp+0x3C/0x38) and register numbering differ; same code otherwise */
/* JFG: brighten the green of a 2-pixel-wide line in the framebuffer (horizontal, or vertical when vertical is set). */
void func_8006DB6C(u16 *fb, s32 x1, s32 y1, s32 x2, s32 y2, s32 vertical) {
    u16 *center;
    u16 *after;
    u16 *before;
    u16 *after2;
    u32 width, height;
    s32 count;
    s32 step;
    u16 pixel;
    s32 green;
    s32 newGreen;

    fb_size(&width, &height);
    if ((!vertical && x2 < x1) || (vertical && y2 < y1)) {
        count = x1;
        x1 = x2;
        x2 = count;
        count = y1;
        y1 = y2;
        y2 = count;
    }
    count = y2 - y1 + 1;
    step = width;
    center = &fb[y1 * width + x1];
    if (!vertical) {
        after = center + width;
        before = center - width;
        after2 = after + width;
        count = x2 - x1 + 1;
        step = 1;
        if (y1 < y2) {
            step = width + 1;
        }
        if (y2 < y1) {
            step -= width;
        }
    } else {
        after = center + 1;
        before = center - 1;
        after2 = center + 2;
        if (x1 < x2) {
            step = width + 1;
        }
        if (x2 < x1) {
            step -= 1;
        }
    }
    while (count--) {
        *before = *before;
        before += step;
        *after2 = *after2;
        after2 += step;
        pixel = *center;
        green = pixel & 0x7C0;
        newGreen = green + 0x200;
        if (newGreen & 0xF800) {
            newGreen = 0x7C0;
        }
        *center = (pixel ^ green) | newGreen;
        center += step;
        pixel = *after;
        green = pixel & 0x7C0;
        newGreen = green + 0x200;
        if (newGreen & 0xF800) {
            newGreen = 0x7C0;
        }
        *after = (pixel ^ green) | newGreen;
        after += step;
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/69050", func_8006DB6C);
#endif

INCLUDE_ASM("asm/nonmatchings/69050", func_8006DD20);

int sprintf(char *s, const char *fmt, ...);
void func_8006DD20(u16 *framebuffer, u8 glyph, s32 x, s32 y, s32 arg4);
extern u8 D_800A6BA8[12]; // JFG: the digit glyphs, then '-' and blank
extern u16 *D_800FECB0;   // JFG: the previous frame's image

#ifdef NON_MATCHING
/* NON_MATCHING: frame 0x88 (target 0x80, buffer at sp+0x54) and one delay slot; the code is otherwise the same */
/* JFG: draw number into the previous frame, right-aligned in width digits (nothing if it is wider). */
void func_8006DF90(s32 x, s32 y, s32 number, s32 width, s32 arg4) {
    char buffer[40];
    s32 len = 0;
    s32 i = 0;
    u8 glyph;
    char c;

    sprintf(buffer, "%d", number);
    while (buffer[len] != '\0') {
        len++;
    }
    if (len <= width) {
        do {
            glyph = D_800A6BA8[11];
            if (len < width) {
                len++;
            } else {
                c = buffer[i++];
                if (c == '-') {
                    glyph = D_800A6BA8[10];
                } else if (c >= '0' && c <= '9') {
                    glyph = D_800A6BA8[c - '0'];
                }
            }
            func_8006DD20(D_800FECB0, glyph, x, y, arg4);
            x += 10;
        } while (buffer[i] != '\0');
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/69050", func_8006DF90);
#endif

/* JFG: queue an entry in the current buffer (up to 0x94). */
void func_8006E0C4(s32 x, s32 u, s32 y, s32 v, s32 flag) {
    ScreenLine *entry;

    if (D_80103B80[D_80103B90] < 0x94) {
        entry = &D_80102DC0[D_80103B90][D_80103B80[D_80103B90]++];
        entry->x = x;
        entry->y = y;
        entry->u = u;
        entry->v = v;
        entry->flags = 0xE;
        if (flag) {
            entry->flags |= 0x10;
        }
    }
}

INCLUDE_ASM("asm/nonmatchings/69050", func_8006E188);

extern s32 D_800A6D00;
extern u8 D_800A6D08;
extern u8 D_800A6D0C;

void func_8006EDF0(s32 arg0) {
    if (arg0) {
        D_800A6D00 = 0x20;
    } else {
        D_800A6D00 = 1;
    }
    D_800A6D08 = 1;
    D_800A6D0C = 1;
}

void func_8006EE28(s32 arg0) {
    if (arg0) {
        D_800A6D00 = 0;
    } else {
        D_800A6D00 = 0x20;
    }
    D_800A6D08 = 0;
}

extern s32 D_800A6D00;

s32 func_8006EE50(void) {
    return D_800A6D00;
}

extern u8 D_80102CE0;
extern u8 D_80102CE1;
extern u8 D_80102CE2;
extern u8 D_80102CE3;
extern u8 D_80102CE4;
extern u8 D_80102CE5;
extern s32 D_800A6D04;
extern s32 gVideoLastFramebuffer; // JFG: the framebuffer to show next
extern Gfx D_800A6BB8[];
s32 rand_range(s32 min, s32 max);
void func_8006F550(Gfx **dList, s32 type, s32 a, s32 b, s32 c, s32 d, s32 e, s32 f, s32 g);

/**
 * JFG: while D_800A6D00 runs (1..32, counting up or down by D_800A6D08), fade the D_80102CE0 colours,
 * redraw the last frame once D_800A6D0C is set, tint the screen and print three random 8-bit binary strings.
 */
void glitch_effect_draw(Gfx **dList) {
    s32 i;
    s32 j;
    s32 value;
    s32 y;
    u32 width;
    u32 height;
    char text[20];
    char *p;

    if (D_800A6D00) {
        fb_size(&width, &height);
        if (D_800A6D00 < 33) {
            D_80102CE0 = ((D_800A6D00 * -255) >> 5) + 255;
            D_80102CE1 = ((D_800A6D00 * -16) >> 5) + 255;
            D_80102CE2 = ((D_800A6D00 * -196) >> 5) + 255;
            D_80102CE3 = 0;
            D_80102CE4 = (D_800A6D00 * 16) >> 5;
            D_80102CE5 = (D_800A6D00 * 4) >> 5;
            if (D_800A6D08) {
                D_800A6D00++;
            } else {
                D_800A6D00--;
            }
        }
        if (D_800A6D0C) {
            D_800A6D04 = 1;
            func_8006F550(dList, gVideoLastFramebuffer, width, height, 0, 0, width, height, 0);
        }
        D_800A6D0C = 1;
        gDPSetPrimColor((*dList)++, 0, 0, 0, 255, 0, D_800A6D00 * 5);
        gSPDisplayList((*dList)++, D_800A6BB8);
        rendermode_reset(dList);
        y = 200;
        i = 3;
        while (i--) {
            value = rand_range(0, 255);
            p = text;
            j = 8;
            while (j--) {
                *p++ = (value & 1) + '0';
                value >>= 1;
            }
            *p = '\0';
            func_8006D60C(40, y, text);
            y += 6;
        }
    }
}

extern u8 D_800A6D0C;

void func_8006F078(void) {
    D_800A6D0C = 0;
}

INCLUDE_ASM("asm/nonmatchings/69050", func_8006F084);

typedef struct ScreenEffect {
    /* 0x00 */ s32 type;
    /* 0x04 */ s16 unk4;
    /* 0x06 */ s16 unk6;
    /* 0x08 */ s16 unk8;
    /* 0x0A */ s16 unkA;
    /* 0x0C */ s16 unkC;
    /* 0x0E */ s16 unkE;
    /* 0x10 */ s32 unk10;
} ScreenEffect;
extern s32 D_800A6E18;
extern ScreenEffect D_80102CE8[4];

/* JFG: queue a screen effect (up to 4 per frame). */
void func_8006F424(s32 type, s32 a, s32 b, s32 c, s32 d, s32 e, s32 f, s32 g) {
    ScreenEffect *fx;

    if (D_800A6E18 < 4) {
        fx = &D_80102CE8[D_800A6E18++];
        fx->type = type;
        fx->unk4 = a;
        fx->unk6 = b;
        fx->unk8 = c;
        fx->unkA = d;
        fx->unkC = e;
        fx->unkE = f;
        fx->unk10 = g;
    }
}

void func_8006F550(Gfx **dList, s32 type, s32 a, s32 b, s32 c, s32 d, s32 e, s32 f, s32 g);

/* JFG: draw the queued screen effects and empty the queue. */
void func_8006F4A0(Gfx **dList) {
    s32 i;
    ScreenEffect *fx = D_80102CE8;

    for (i = 0; i < D_800A6E18; i++, fx++) {
        func_8006F550(dList, fx->type, fx->unk4, fx->unk6, fx->unk8, fx->unkA, fx->unkC, fx->unkE, fx->unk10);
    }
    D_800A6E18 = 0;
}

INCLUDE_ASM("asm/nonmatchings/69050", func_8006F550);

typedef void (*ScreenEffectFunc)(s32 slot, s32 arg, s32 arg2);
extern s32 D_80102D40[4];
extern s32 D_80102D54;
extern void *D_80102D60[4];
extern void *D_80102D78[4];
extern u8 D_80102D9C[4];
extern ScreenEffectFunc D_800A6E1C[4];
void func_80054530();
void func_800531C8(void *addr);

/* JFG: clear the four effect slots; a slot is flagged when its handler is the unloaded-module trap. */
void func_8006F874(void) {
    s32 i;

    D_80102D54 = 0;
    i = 4;
    while (i--) {
        D_80102D60[i] = NULL;
        D_80102D78[i] = NULL;
        D_80102D9C[i] = (D_800A6E1C[i] == (ScreenEffectFunc) func_80054530);
    }
}

/* JFG: call the handlers of the effect slots flagged in D_80102D54, then clear the flags. */
void func_8006F8E4(void) {
    s32 i = 4;

    while (i--) {
        if ((1 << i) & D_80102D54) {
            func_800531C8(D_800A6E1C + i);
            if (D_800A6E1C[i] != NULL) {
                D_800A6E1C[i](i, D_80102D40[i], 0);
            }
        }
    }
    D_80102D54 = 0;
}

#ifdef NON_MATCHING
/* NON_MATCHING: from the clear loop on, temps numbered one lower and the handler in v0 (target v1) */
/* JFG: start effect slot (1-4) with a width x height byte pair: two cleared buffers of that size, then its handler. */
void func_8006F998(s32 slot, u8 *dims) {
    u8 *buf1;
    u8 *buf2;
    s32 count;

    slot--;
    D_80102D54 |= 1 << slot;
    D_80102D40[slot] = (s32) dims;
    if (D_80102D60[slot] == NULL) {
        D_80102D60[slot] = buf1 = mempool_alloc_safe(dims[1] * dims[0], 0xFFFF);
        D_80102D78[slot] = buf2 = mempool_alloc_safe(dims[1] * dims[0], 0xFFFF);
        if (D_80102D60[slot] == NULL || buf2 == NULL) {
            D_80102D60[slot] = NULL;
            D_80102D78[slot] = NULL;
            return;
        }
        count = dims[1] * dims[0];
        while (count--) {
            *buf1++ = 0;
            *buf2++ = 0;
        }
        func_800531C8(D_800A6E1C + slot);
        if (D_800A6E1C[slot] != NULL) {
            D_800A6E1C[slot](slot, D_80102D40[slot], 1);
        }
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/69050", func_8006F998);
#endif

void mempool_free(void *data);

/* JFG: free the four effect slots' buffers; flagged slots get the unloaded-module trap back. */
void func_8006FB18(void) {
    s32 i = 4;

    while (i--) {
        if (D_80102D60[i] != NULL) {
            mempool_free(D_80102D60[i]);
            mempool_free(D_80102D78[i]);
            D_80102D60[i] = NULL;
            D_80102D78[i] = NULL;
        }
        if (D_80102D9C[i]) {
            D_800A6E1C[i] = (ScreenEffectFunc) func_80054530;
        }
    }
    D_80102D54 = 0;
}
