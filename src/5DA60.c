/*
 * Particles. Functions in C here are from the Diddy Kong Racing decompilation
 * (CC0 1.0), src/particles.c; see CREDITS.md.
 */
#include "common.h"

/* DKR src/particles.h and include/structs.h (reduced) */
typedef struct Vec3f {
    f32 x, y, z;
} Vec3f;

typedef struct Vec3s {
    s16 y_rotation, x_rotation, z_rotation;
} Vec3s;

/* JFG: DKR's flags, size 0x28 */
typedef struct ParticleBehaviour {
    /* 0x00 */ s32 flags;
    /* 0x04 */ Vec3f emitterPos;
    /* 0x10 */ f32 sourceDistance;
    /* 0x14 */ Vec3s sourceRotation;
    /* 0x1A */ u8 pad1A[0x22 - 0x1A];
    /* 0x22 */ Vec3s emissionDirection;
    /* 0x28 */ u8 pad28[0x30 - 0x28];
    /* 0x30 */ Vec3f velocityModifier;
    /* 0x3C */ f32 emissionSpeed;
    /* 0x40 */ s16 spawnInterval;
    /* 0x42 */ s16 burstCount;
    /* 0x44 */ u8 pad44[0x50 - 0x44];
    /* 0x50 */ f32 unk50; // JFG: line segment length scale
    /* 0x54 */ u8 pad54[0x58 - 0x54];
    /* 0x58 */ f32 movementParam;
    /* 0x5C */ s32 randomizationFlags;
    /* 0x60 */ s32 sourceDistanceRange;
    /* 0x64 */ Vec3s sourceDirRange;
    /* 0x6A */ Vec3s emissionDirRange;
    /* 0x70 */ s32 emissionSpeedRange;
    /* 0x74 */ struct {
        s32 x, y, z;
    } velocityModifierRange;
    /* 0x80 */ u8 pad80[0x94 - 0x80];
    /* 0x94 */ s32 movementParamRange;
} ParticleBehaviour;

#define PARTICLE_POINT 0x400
#define PARTICLE_LINE 0x4000
#define PARTICLE_ACTIVE 0x8000

/* JFG layout, size 0x24 */
typedef struct ParticleEmitter {
    /* 0x00 */ ParticleBehaviour *behaviour;
    /* 0x04 */ s32 flags;
    /* 0x08 */ u8 sourceRotationCounter;
    /* 0x09 */ u8 emissionDirRotationCounter;
    /* 0x0A */ s16 descriptorID;
    /* 0x0C */ s16 colourIndex;
    /* 0x0E */ Vec3s sourceRotation;
    /* 0x14 */ Vec3s emissionDirection;
    /* 0x1A */ s16 posX, posY, posZ;
    /* 0x20 */ s16 unk20;
    /* 0x22 */ u8 index; // into the line or point particle buffers, 0xFF if none
    /* 0x23 */ s8 unk23;
} ParticleEmitter;

/* JFG: the free-emitter pool, size 0x28 */
typedef struct PoolEmitter {
    /* 0x00 */ ParticleEmitter emitter;
    /* 0x24 */ u8 active;
    /* 0x25 */ u8 pad25[3];
} PoolEmitter;

typedef struct ParticleObjHeader {
    /* 0x00 */ u8 pad0[0x1C];
    /* 0x1C */ s16 unk1C;
    /* 0x1E */ u8 pad1E[0x22 - 0x1E];
    /* 0x22 */ s8 emitterCount;
} ParticleObjHeader;

typedef struct ParticleObjModel {
    /* 0x00 */ u8 pad0[0x30];
    /* 0x30 */ u8 emitterCount;
} ParticleObjModel;

typedef struct ParticleObj {
    /* 0x00 */ u8 pad0[0x1A];
    /* 0x1A */ s16 activeEmitters;
    /* 0x1C */ u8 pad1C[0x40 - 0x1C];
    /* 0x40 */ ParticleObjHeader *header;
    /* 0x44 */ u8 pad44[0x48 - 0x44];
    /* 0x48 */ s16 behaviorId;
    /* 0x4A */ u8 pad4A[0x68 - 0x4A];
    /* 0x68 */ struct {
        u8 pad0[0x20];
        ParticleObjModel *model;
    } *unk68;
    /* 0x6C */ u8 pad6C[4];
    /* 0x70 */ ParticleEmitter *emitters;
    /* 0x74 */ u8 pad74[0x84 - 0x74];
    /* 0x84 */ u32 particleEmittersEnabled;
} ParticleObj;

/* JFG: a point emitter's particles, size 0xC0 */
typedef struct PointParticleSet {
    /* 0x00 */ s32 points[15];
    /* 0x3C */ s32 count;
    /* 0x40 */ s32 numVerts;
    /* 0x44 */ Vec3f verts[8];
    /* 0xA4 */ u8 unkA4; // set when the emitter lets go of it
    /* 0xA5 */ u8 padA5[0xA8 - 0xA5];
    /* 0xA8 */ s32 texFlags;
    /* 0xAC */ s32 flags;
    /* 0xB0 */ f32 frame;
    /* 0xB4 */ s16 texUpdateRate;
    /* 0xB6 */ u8 padB6[2];
    /* 0xB8 */ struct TextureHeader *texture;
    /* 0xBC */ struct ParticleEmitter *emitter;
} PointParticleSet;

/* JFG: one segment of a line emitter: its two ends, fade and colour */
typedef struct LineSeg {
    /* 0x00 */ s16 x0, y0, z0;
    /* 0x06 */ s16 x1, y1, z1;
    /* 0x0C */ s16 fadeDelay;
    /* 0x0E */ union {
        s16 value; // 8.8 fixed point
        u8 hi;
    } alpha;
    /* 0x10 */ s16 colourDelay;
    /* 0x12 */ s16 colourIndex;
    /* 0x14 */ s16 life;
    /* 0x16 */ s16 fadeRate;
    /* 0x18 */ u8 r, g, b;
} LineSeg;

/* JFG: a line emitter's data, size 0x148 */
typedef struct LineParticleSet {
    /* 0x000 */ s32 count;
    /* 0x004 */ LineSeg *segs[9];
    /* 0x028 */ u8 pad28[0x124 - 0x28];
    /* 0x124 */ u8 unk124; // set when the emitter lets go of it
    /* 0x125 */ u8 pad125[0x128 - 0x125];
    /* 0x128 */ struct TextureHeader *texture;
    /* 0x12C */ struct ParticleBehaviour *behaviour;
    /* 0x130 */ u32 *colours; // RGBA, cycled through
    /* 0x134 */ s16 numColours;
    /* 0x136 */ u8 pad136[2];
    /* 0x138 */ s32 unk138;
    /* 0x13C */ s32 flags;
    /* 0x140 */ f32 frame;
    /* 0x144 */ s16 unk144;
    /* 0x146 */ u8 pad146[2];
} LineParticleSet;

extern PointParticleSet *D_800A5D88;
extern LineParticleSet *D_800A5D84;

void free_particle_buffers(void);
void free_particle_vertices_triangles(void);
void particle_free_dummy(void);
void emitter_init_with_pos(ParticleEmitter *emitter, s32 behaviourID, s32 particleID, s16 posX, s16 posY, s16 posZ);

extern ParticleBehaviour **gParticleBehavioursAssetTable;
extern s32 gParticleBehavioursAssetTableCount;

typedef struct Vertex Vertex;
typedef struct Triangle Triangle;
typedef struct ObjectSegment ObjectSegment;

typedef struct TextureHeader TextureHeader;
/* JFG layout (55130.c has more) */
struct TextureHeader {
    /* 0x00 */ u8 pad0[0x12];
    /* 0x12 */ u16 numOfTextures;
};

void mempool_free(void *data);
void tex_free(TextureHeader *tex);
s32 *asset_table_load(u32 assetIndex);

/* DKR include/structs.h, size 10 */
struct Vertex {
    /* 0x00 */ s16 x, y, z;
    /* 0x06 */ u8 r, g, b, a;
};

/* DKR include/f3ddkr.h (JFG: the length is numTris << 4, which IDO does not fold into an expression) */
#define G_TRIN 5
#define gSPPolygon(dl, ptr, numTris, texEnabled)                                                        \
    {                                                                                                   \
        Gfx *_g = (Gfx *) (dl);                                                                         \
        _g->words.w0 = _SHIFTL((((numTris) - 1) << 4) | (texEnabled), 16, 8) | _SHIFTL(G_TRIN, 24, 8) | \
                       _SHIFTL(((numTris) << 4), 0, 16);                                                \
        _g->words.w1 = (unsigned int) (ptr);                                                            \
    }
/* JFG: loads n + 1 vertices (this file's form of the vertex command) */
#define gSPVertexN1(pkt, v, n) gDma1p(pkt, G_VTX, v, (n) * 8 + (n) * 2 + 8, ((n) * 8) | ((u32) (v) & 6))

void func_80055C00(Gfx **dList, TextureHeader *tex, s32 flags, s32 frame);
void func_80048C00(); // DKR: mtxf_transform_dir (mtx, in, out)

struct ObjectSegment {
    /* 0x00 */ s16 y_rotation;
    /* 0x02 */ u8 pad2[0xC - 2];
    /* 0x0C */ Vec3f pos;
};

typedef struct AudioPoint AudioPoint;
typedef struct SpriteAsset SpriteAsset;
TextureHeader *load_texture(s32 assetIndex);
void sprite_free(SpriteAsset *sprite);

/**
 * Deallocate all memory buffers used by particles.
 */
void reset_particles(void) {
    free_particle_buffers();
    free_particle_vertices_triangles();
    particle_free_dummy();
}

extern void **D_800A5D74; // JFG: the particle assets (sprites, textures or models)
extern s32 D_800A5D78;    // JFG: the number of particle assets
void free_3d_model(void *model);

/* JFG: free the particle assets (the type of each is in the top bits of asset 0x3C's entries). */
void particle_assets_free(void) {
    s32 i;
    s16 *types;

    if (D_800A5D74 != NULL) {
        types = (s16 *) asset_table_load(0x3C);
        for (i = 0; i < D_800A5D78; i++) {
            switch (types[i] & 0xC000) {
                case 0x8000:
                    sprite_free(D_800A5D74[i]);
                    break;
                case 0xC000:
                    tex_free(D_800A5D74[i]);
                    break;
                case 0:
                    free_3d_model(D_800A5D74[i]);
                    break;
            }
        }
        mempool_free(types);
        mempool_free(D_800A5D74);
        D_800A5D74 = NULL;
    }
}

typedef struct WeatherParticleList WeatherParticleList;
extern WeatherParticleList *D_801008E0[5];
extern s32 D_800A5D7C; // JFG: the number of line particle sets
extern s32 D_800A5D80; // JFG: the number of point particle sets
extern s32 D_800A5DB4;
extern void *D_800A5F48;
extern void *D_800A5F80;
extern PoolEmitter *D_800A5DAC;
extern s32 D_800A5DB0;
void free_particle_list(WeatherParticleList **listPtr);

/* JFG: free the particle lists, the line and point sets (and their textures) and the other particle buffers. */
void particle_buffers_free(void) {
    WeatherParticleList **list;
    LineParticleSet *line;
    PointParticleSet *point;
    s32 i;

    for (list = D_801008E0; list < D_801008E0 + 5; list++) {
        free_particle_list(list);
    }
    D_800A5DB4 = 0;
    if (D_800A5D84 != NULL) {
        line = D_800A5D84;
        for (i = 0; i < D_800A5D7C; i++, line++) {
            if (line->unk124 && line->texture != NULL) {
                tex_free(line->texture);
            }
        }
        mempool_free(D_800A5D84);
        D_800A5D84 = NULL;
    }
    if (D_800A5D88 != NULL) {
        point = D_800A5D88;
        for (i = 0; i < D_800A5D80; i++, point++) {
            if (point->unkA4 && point->texture != NULL) {
                tex_free(point->texture);
            }
        }
        mempool_free(D_800A5D88);
        D_800A5D88 = NULL;
    }
    if (D_800A5F48 != NULL) {
        mempool_free(D_800A5F48);
        D_800A5F48 = NULL;
    }
    if (D_800A5F80 != NULL) {
        mempool_free(D_800A5F80);
        D_800A5F80 = NULL;
    }
    if (D_800A5DAC != NULL) {
        mempool_free(D_800A5DAC);
        D_800A5DAC = NULL;
        D_800A5DB0 = 0;
    }
}

extern Vertex *D_800A5D8C[2];

/* Frees the particle vertex buffers (0x8005D19C). */
void free_particle_vertex_buffers(void) {
    if (D_800A5D8C[0] != NULL) {
        mempool_free(D_800A5D8C[0]);
        D_800A5D8C[0] = NULL;
        D_800A5D8C[1] = NULL;
    }
}

/* Size: 0x78 bytes */
typedef struct WeatherParticle {
    /* 0x00 */ u8 pad0[0x2C];
    /* 0x2C */ s16 kind; // 0/1 texture, 2 sprite, 3 model
    /* 0x2E */ u8 unk2E;
    /* 0x2F */ u8 pad2F;
    /* 0x30 */ Vec3f pos;
    /* 0x3C */ u8 pad3C[0x48 - 0x3C];
    /* 0x48 */ s32 unk48;
    /* 0x4C */ void *asset;
    /* 0x50 */ u8 pad50[0x58 - 0x50];
    /* 0x58 */ PoolEmitter *pool; // JFG: the pool emitter it came from, if any
    /* 0x5C */ u8 pad5C[0x78 - 0x5C];
} WeatherParticle;

struct WeatherParticleList {
    /* 0x00 */ u8 pad0[0x14];
    /* 0x14 */ WeatherParticle *particles;
    /* 0x18 */ s32 count;
    /* 0x1C */ s32 activeCount;
    /* 0x20 */ u32 *freeMask; // one bit per free particle slot
    /* 0x24 */ s32 lastMaskWord;
    /* 0x28 */ s32 full;      // set when an allocation found the list full
};

void free_3d_model(void *model);

/* JFG: free a particle list and the assets of its particles (0x8005D1DC). */
void free_particle_list(WeatherParticleList **listPtr) {
    WeatherParticleList *list = *listPtr;
    WeatherParticle *p;
    s32 i;

    if (list != NULL) {
        p = list->particles;
        for (i = 0; i < list->count; i++, p++) {
            switch (p->kind) {
                case 2:
                    sprite_free(p->asset);
                    break;
                case 0:
                case 1:
                    tex_free(p->asset);
                    break;
                case 3:
                    free_3d_model(p->asset);
                    break;
            }
        }
        mempool_free(list);
        *listPtr = NULL;
    }
}

/* A sprite particle (kind 2) as sprite_particles_render reads it */
typedef struct SpriteParticle {
    /* 0x00 */ u8 pad0[0x2C];
    /* 0x2C */ s16 kind;
    /* 0x2E */ u8 pad2E[0x4C - 0x2E];
    /* 0x4C */ struct ParticleSprite *sprite;
    /* 0x50 */ s32 flags;
    /* 0x54 */ u8 pad54[0x64 - 0x54];
    /* 0x64 */ s16 alpha; // 8.8 fixed point
    /* 0x66 */ u8 pad66[0x6F - 0x66];
    /* 0x6F */ u8 brightness;
    /* 0x70 */ u8 colourMode; // 0: plain, else with an environment colour (2: swapped)
    /* 0x71 */ u8 r, g, b;
    /* 0x74 */ u8 envR, envG, envB;
    /* 0x77 */ u8 pad77;
} SpriteParticle;

typedef struct ParticleSprite {
    /* 0x00 */ u8 pad0[6];
    /* 0x06 */ s16 flags;
    /* 0x08 */ u8 primR, primG, primB;
    /* 0x0B */ u8 envR, envG, envB;
} ParticleSprite;

void draw_tint_pair_set(u8 a, u8 b, u8 c, u8 d, u8 e, u8 f);
void draw_tint_pair_off(void);
void render_sprite_billboard(Gfx **dList, Mtx **mtx, Vertex **vtx, SpriteParticle *particle, ParticleSprite *sprite, s32 flags,
                   s32 alpha);

/* JFG: draw a list's sprite particles (kind 2) with their colours (brightness-scaled, and with an environment
   colour or the sprite's own colours), then reset the colours. The environment colours of a particle without
   one carry over from the previous particle. */
void sprite_particles_render(Gfx **dList, Mtx **mtx, Vertex **vtx, WeatherParticleList *list) {
    s32 i;
    SpriteParticle *p;
    ParticleSprite *sprite;
    s32 alpha;
    s32 envR;
    s32 envG;
    s32 envB;
    UNUSED s32 pad1;
    s32 flags;
    s32 r;
    s32 g;
    s32 b;
    s32 mode;
    s32 f;
    UNUSED s32 pad0;

    if (list == NULL) {
        return;
    }
    p = (SpriteParticle *) list->particles;
    for (i = 0; i < list->count; i++, p++) {
        if (p->kind == 2) {
            mode = p->colourMode;
            sprite = p->sprite;
            r = p->r;
            g = p->g;
            b = p->b;
            flags = 2;
            if (mode) {
                envR = p->envR;
                envG = p->envG;
                envB = p->envB;
            }
            alpha = (p->alpha >> 8) & 0xFF;
            if (p->flags & 0x800) {
                f = p->brightness;
                r = (f * r) >> 8;
                g = (f * g) >> 8;
                b = (f * b) >> 8;
                if (mode) {
                    envR = (f * envR) >> 8;
                    envG = (f * envG) >> 8;
                    envB = (f * envB) >> 8;
                }
            }
            if (sprite->flags & 0x200) {
                if (!mode) {
                    envR = (sprite->envR * r) >> 8;
                    envG = (sprite->envG * g) >> 8;
                    envB = (sprite->envB * b) >> 8;
                    r = (sprite->primR * r) >> 8;
                    g = (sprite->primG * g) >> 8;
                    b = (sprite->primB * b) >> 8;
                }
                if (mode != 2) {
                    draw_tint_pair_set(r, g, b, envR, envG, envB);
                } else {
                    draw_tint_pair_set(envR, envG, envB, r, g, b);
                }
            } else if (((r << 24) | (g << 16) | (b << 8) | alpha) != 0) {
                gDPPipeSync((*dList)++);
                gDPSetPrimColor((*dList)++, 0, 0, r, g, b, alpha);
            }
            if (alpha != 255) {
                flags = 6;
            }
            render_sprite_billboard(dList, mtx, vtx, p, sprite, flags, alpha);
            if (sprite->flags & 0x200) {
                draw_tint_pair_off();
            }
        }
    }
    gDPPipeSync((*dList)++);
    gDPSetPrimColor((*dList)++, 0, 0, 255, 255, 255, 255);
    gDPSetEnvColor((*dList)++, 255, 255, 255, 255);
}

typedef struct ObjectTransform {
    /* 0x00 */ s16 y_rotation;
    /* 0x02 */ s16 x_rotation;
    /* 0x04 */ s16 z_rotation;
    /* 0x06 */ s16 flags;
    /* 0x08 */ f32 scale;
    /* 0x0C */ f32 x_position;
    /* 0x10 */ f32 y_position;
    /* 0x14 */ f32 z_position;
} ObjectTransform;

typedef struct ParticleModelHeader {
    /* 0x00 */ u8 pad0[0x74];
    /* 0x74 */ u8 *dl;
} ParticleModelHeader;

typedef struct ParticleModel {
    /* 0x00 */ ParticleModelHeader *header;
    /* 0x04 */ u8 *vertices;
    /* 0x08 */ u8 pad8[3];
    /* 0x0B */ s8 frame;
    /* 0x0C */ u8 padC[4];
    /* 0x10 */ u8 *frames[1];
} ParticleModel;

/* A model particle (kind 3) as model_particles_render reads it */
typedef struct ModelParticle {
    /* 0x00 */ s16 y_rotation;
    /* 0x02 */ s16 x_rotation;
    /* 0x04 */ s16 z_rotation;
    /* 0x06 */ u8 pad6[2];
    /* 0x08 */ f32 scale;
    /* 0x0C */ f32 x, y, z;
    /* 0x18 */ u8 pad18[0x2C - 0x18];
    /* 0x2C */ s16 kind;
    /* 0x2E */ u8 pad2E[0x4C - 0x2E];
    /* 0x4C */ ParticleModel *model;
    /* 0x50 */ s32 flags;
    /* 0x54 */ u8 pad54[0x6F - 0x54];
    /* 0x6F */ u8 brightness;
    /* 0x70 */ u8 pad70;
    /* 0x71 */ u8 r, g, b;
    /* 0x74 */ u8 pad74[4];
} ModelParticle;

/* A display list command written as its two words */
#define fast3d_cmd(pkt, word0, word1) \
    {                                 \
        Gfx *_g = (Gfx *) (pkt);      \
        _g->words.w0 = word0;         \
        _g->words.w1 = word1;         \
    }

void mtx_cam_push(Gfx **dList, Mtx **mtx, void *trans, f32 scaleY, f32 offsetY);
void mtx_pop(Gfx **dList);

/* JFG: draw a list's model particles (kind 3): each one's matrix, its colour (brightness-scaled when flag 0x800
   is set) and its model's current frame, then reset the colour and pop the matrix. */
void model_particles_render(Gfx **dList, Mtx **mtx, Vertex **vtx, WeatherParticleList *list) {
    s32 i;
    ModelParticle *p;
    ObjectTransform trans;
    ParticleModel *model;
    s32 f;
    s32 r;
    s32 g;
    s32 b;

    if (list == NULL) {
        return;
    }
    p = (ModelParticle *) list->particles;
    gDPPipeSync((*dList)++);
    for (i = 0; i < list->count; i++, p++) {
        if (p->kind == 3) {
            model = p->model;
            trans.z_rotation = p->z_rotation;
            trans.x_rotation = p->x_rotation;
            trans.y_rotation = p->y_rotation;
            trans.x_position = p->x;
            trans.y_position = p->y;
            trans.z_position = p->z;
            trans.scale = p->scale;
            mtx_cam_push(dList, mtx, &trans, 1.0f, 0.0f);
            gDPPipeSync((*dList)++);
            if (p->flags & 0x800) {
                f = p->brightness;
                r = (p->r * f) >> 8;
                g = (p->g * f) >> 8;
                b = (p->b * f) >> 8;
                gDPSetPrimColor((*dList)++, 0, 0, r, g, b, 255);
            } else {
                gDPSetPrimColor((*dList)++, 0, 0, 255, 255, 255, 255);
            }
            fast3d_cmd((*dList)++, ((u32) (model->frames[model->frame] + 0x80000000) & 0xFFFFFF) | 0xBF000000,
                       (u32) (model->vertices + 0x80000000));
            gSPDisplayList((*dList)++, model->header->dl + 0x80000000);
            fast3d_cmd((*dList)++, 0xBF000000, 0);
        }
    }
    gDPPipeSync((*dList)++);
    gDPSetPrimColor((*dList)++, 0, 0, 255, 255, 255, 255);
    mtx_pop(dList);
}

INCLUDE_ASM("asm/nonmatchings/5DA60", func_8005D93C);

/**
 * Initializes the emitter with the specified particle type and behaviour.
 */
void emitter_init(ParticleEmitter *emitter, s32 behaviourID, s32 particleID) {
    ParticleBehaviour *behaviour;

    if (behaviourID < gParticleBehavioursAssetTableCount) {
        behaviour = gParticleBehavioursAssetTable[behaviourID];
        emitter_init_with_pos(emitter, behaviourID, particleID, behaviour->emitterPos.x, behaviour->emitterPos.y,
                              behaviour->emitterPos.z);
    }
}

/* JFG: emitter_init without the behaviour's position, plus a trigger byte (0x8005ED54). */
void emitter_init_at_origin(ParticleEmitter *emitter, s32 behaviourID, s32 particleID, s32 trigger) {
    if (behaviourID < gParticleBehavioursAssetTableCount) {
        emitter_init_with_pos(emitter, behaviourID, particleID, 0, 0, 0);
        emitter->unk23 = trigger;
    }
}

/**
 * Initializes the emitter with the specified particle type, behaviour and position.
 * JFG: line and point emitters only get their flags; no buffers are allocated here.
 */
void emitter_init_with_pos(ParticleEmitter *emitter, s32 behaviourID, s32 particleID, s16 posX, s16 posY, s16 posZ) {
    ParticleBehaviour *behaviour;

    behaviour = gParticleBehavioursAssetTable[behaviourID];
    emitter->descriptorID = particleID;
    emitter->colourIndex = 0;
    emitter->posX = posX;
    emitter->behaviour = behaviour;
    emitter->posY = posY;
    emitter->posZ = posZ;
    emitter->unk20 = 0;
    emitter->unk23 = -1;
    if (behaviour->flags & PARTICLE_LINE) {
        emitter->flags = PARTICLE_LINE;
        emitter->index = 0xFF;
    } else if (behaviour->flags & PARTICLE_POINT) {
        emitter->flags = PARTICLE_POINT;
        emitter->index = 0xFF;
    } else {
        emitter->flags = 0;
        emitter->sourceRotation.y_rotation = behaviour->sourceRotation.y_rotation;
        emitter->sourceRotation.x_rotation = behaviour->sourceRotation.x_rotation;
        emitter->sourceRotation.z_rotation = behaviour->sourceRotation.z_rotation;
        emitter->emissionDirection.y_rotation = behaviour->emissionDirection.y_rotation;
        emitter->emissionDirection.x_rotation = behaviour->emissionDirection.x_rotation;
        emitter->emissionDirection.z_rotation = behaviour->emissionDirection.z_rotation;
        emitter->index = 0;
    }
}

extern PoolEmitter *D_800A5DAC;
extern s32 D_800A5DB0;

/* JFG: take a free emitter from the pool and initialise it; NULL if none is free (0x8005EE78). */
ParticleEmitter *emitter_alloc_from_pool(s32 behaviourID, s32 particleID) {
    s32 i;
    PoolEmitter *entry = D_800A5DAC;

    for (i = 0; i < D_800A5DB0; i++, entry++) {
        if (!entry->active) {
            entry->active = TRUE;
            entry->emitter.colourIndex = 0;
            emitter_init_with_pos(&entry->emitter, behaviourID, particleID, 0, 0, 0);
            return &entry->emitter;
        }
    }
    return NULL;
}

typedef struct ParticleDesc ParticleDesc;
extern ParticleDesc **D_800A5D9C;
s32 point_particle_set_alloc(ParticleDesc *desc, ParticleBehaviour *behaviour, ParticleEmitter *emitter);
s32 line_particle_set_alloc(); // JFG: takes (desc, behaviour); the caller below also passes the emitter

/**
 * Initializes the emitter if it has been activated for an object.
 * JFG: line and point emitters get a particle buffer; no point opacity or line reference.
 * DKR: obj_enable_emitter
 */
void obj_enable_emitter(ParticleObj *obj, s32 emitterIndex) {
    ParticleBehaviour *behaviour;
    ParticleEmitter *emitter;
    ParticleDesc *desc;

    emitter = &obj->emitters[emitterIndex];
    desc = D_800A5D9C[emitter->descriptorID];
    behaviour = emitter->behaviour;
    emitter->colourIndex = 0;
    if (emitter->flags & PARTICLE_LINE) {
        emitter->index = line_particle_set_alloc(desc, behaviour, emitter);
    } else {
        if (behaviour->flags & 1) {
            emitter->sourceRotationCounter = 0;
            emitter->sourceRotation.y_rotation = behaviour->sourceRotation.y_rotation;
            emitter->sourceRotation.x_rotation = behaviour->sourceRotation.x_rotation;
            emitter->sourceRotation.z_rotation = behaviour->sourceRotation.z_rotation;
        }
        if (behaviour->flags & 4) {
            emitter->emissionDirRotationCounter = 0;
            emitter->emissionDirection.y_rotation = behaviour->emissionDirection.y_rotation;
            emitter->emissionDirection.x_rotation = behaviour->emissionDirection.x_rotation;
            emitter->emissionDirection.z_rotation = behaviour->emissionDirection.z_rotation;
        }
    }
    if (emitter->flags & PARTICLE_POINT) {
        emitter->index = point_particle_set_alloc(desc, behaviour, emitter);
    }
    emitter->flags |= PARTICLE_ACTIVE;
    obj->activeEmitters++;
}

/* JFG: the part of a particle description that line and point sets take */
typedef struct LineDesc {
    /* 0x00 */ s32 unk0;
    /* 0x04 */ u8 unk4;
    /* 0x05 */ u8 shape : 4; // point count and D_800A5F78 shape index
    /* 0x05 */ u8 unk5 : 4;
    /* 0x06 */ s16 textureID; // -1 for none
    /* 0x08 */ s16 unk8;
} LineDesc;

extern f32 D_800AD81C; // 0.01f, ahead of this file's .rodata split
extern f32 *D_800A5F78[]; // JFG: point shapes, (x, y) pairs

/* JFG: give an emitter a point particle set: reuse the one it let go of, or claim a free one (index, 0xFF if none).
   (The two empty ifs before the second loop give the ROM's loop entry; found by the permuter.) */
s32 point_particle_set_alloc(ParticleDesc *desc, ParticleBehaviour *behaviour, ParticleEmitter *emitter) {
    s32 found;
    s32 i;
    s32 j;
    TextureHeader *tex;
    PointParticleSet *set;
    s32 numFrames;
    Vec3f *v;
    f32 *src;

    if (D_800A5D88 == NULL) {
        return 0xFF;
    }
    set = D_800A5D88;
    found = 0xFF;
    for (i = 0; i < D_800A5D80; i++, set++) {
        if (emitter == set->emitter && set->unkA4 == 1) {
            set->unkA4 = 2;
            return i;
        }
    }
    if (set) {}
    set = D_800A5D88;
    if (!i) {}
    for (i = 0; i < D_800A5D80; i++) {
        if (set->unkA4 == 0) {
            found = i;
            i = D_800A5D80;
        } else {
            set++;
        }
    }
    if (found != 0xFF) {
        set->unkA4 = 2;
        set->numVerts = ((LineDesc *) desc)->shape;
        v = set->verts;
        src = D_800A5F78[((LineDesc *) desc)->shape];
        for (j = 0; j < set->numVerts; j++) {
            v->x = src[0];
            v->y = src[1];
            v->z = 0.0f;
            src += 2;
            v++;
        }
        set->texFlags = ((LineDesc *) desc)->unk0;
        set->flags = behaviour->flags;
        if (((LineDesc *) desc)->textureID == -1) {
            set->texture = NULL;
            set->texUpdateRate = 0;
            set->frame = 0.0f;
        } else {
            tex = load_texture(((LineDesc *) desc)->textureID);
            set->texture = tex;
            set->texUpdateRate = ((LineDesc *) desc)->unk8;
            numFrames = tex->numOfTextures >> 8;
            if (behaviour->flags & 0x800) {
                set->frame = rand_range(0, numFrames - 1);
            } else if ((set->texFlags & 1) == 2) {
                set->frame = numFrames - D_800AD81C;
            } else {
                set->frame = 0.0f;
            }
        }
        set->emitter = emitter;
    }
    return found;
}

/* JFG: claim a free line particle set for a description; returns its index, 0xFF if none is free. */
s32 line_particle_set_alloc(ParticleDesc *desc, ParticleBehaviour *behaviour) {
    s32 found;
    s32 i;
    TextureHeader *tex;
    LineParticleSet *set;
    s32 numFrames;

    if (D_800A5D84 == NULL) {
        return 0xFF;
    }
    set = D_800A5D84;
    found = 0xFF;
    for (i = 0; i < D_800A5D7C; i++) {
        if (set->unk124 == 0) {
            set->unk124 = 2;
            set->behaviour = behaviour;
            set->colours = NULL;
            found = i;
            i = D_800A5D7C;
        } else {
            set++;
        }
    }
    if (found != 0xFF) {
        set->unk138 = ((LineDesc *) desc)->unk0;
        set->flags = behaviour->flags;
        if (((LineDesc *) desc)->textureID == -1) {
            set->texture = NULL;
            set->unk144 = 0;
            set->frame = 0.0f;
        } else {
            tex = load_texture(((LineDesc *) desc)->textureID);
            set->texture = tex;
            set->unk144 = ((LineDesc *) desc)->unk8;
            numFrames = tex->numOfTextures >> 8;
            if (behaviour->flags & 0x800) {
                set->frame = rand_range(0, numFrames - 1);
            } else if ((set->unk138 & 1) == 2) {
                set->frame = numFrames - 0.01f;
            } else {
                set->frame = 0.0f;
            }
        }
    }
    return found;
}

/* JFG: switch off an object's emitter and release its line/point buffer (0x8005F438). */
void obj_disable_emitter(ParticleObj *obj, s32 emitterIndex) {
    ParticleEmitter *emitter = &obj->emitters[emitterIndex];
    s32 flags;
    LineParticleSet *line;
    PointParticleSet *point;

    emitter->flags &= ~PARTICLE_ACTIVE;
    obj->activeEmitters--;
    flags = obj->emitters[emitterIndex].flags;
    if (flags & PARTICLE_LINE) {
        line = &D_800A5D84[emitter->index];
        emitter->index = 0xFF;
        line->unk124 = TRUE;
    } else if (flags & PARTICLE_POINT) {
        point = &D_800A5D88[emitter->index];
        emitter->index = 0xFF;
        point->unkA4 = TRUE;
    }
}

/* JFG: switch off every active emitter of an object (0x8005F4F4). */
void obj_disable_all_emitters(ParticleObj *obj) {
    s32 i;
    s32 count;
    ParticleObjHeader *header = obj->header;
    ParticleObjModel *model;

    if (header->unk1C == 0x27) {
        model = obj->unk68->model;
        count = model->emitterCount;
    } else {
        count = header->emitterCount;
    }
    for (i = 0; i < count; i++) {
        if (obj->emitters[i].flags & PARTICLE_ACTIVE) {
            obj_disable_emitter(obj, i);
        }
    }
}

extern f32 D_800A5DDC;

void func_8005F5A0(f32 arg0) {
    D_800A5DDC = arg0;
}

void obj_trigger_emitter(ParticleObj *obj, ParticleEmitter *emitter); // DKR: obj_trigger_emitter

/**
 * Update all particle emitters attached to the object and spawn new particles.
 * JFG: the emitter count can come from the object's model; disabled emitters are switched off at once.
 */
void obj_spawn_particle(ParticleObj *obj, s32 updateRate) {
    s32 i;
    u32 bits;
    s32 count;
    ParticleObjModel *model;

    bits = obj->particleEmittersEnabled;
    i = 0;
    if (obj->behaviorId == 0x27) {
        model = obj->unk68->model;
        count = model->emitterCount;
    } else {
        count = obj->header->emitterCount;
    }
    for (; i < count; i++) {
        if (bits & 1) {
            if (!(obj->emitters[i].flags & PARTICLE_ACTIVE)) {
                obj_enable_emitter(obj, i);
            }
            if (obj->emitters[i].flags & PARTICLE_LINE) {
                obj->emitters[i].colourIndex += updateRate;
                obj_trigger_emitter(obj, &obj->emitters[i]);
            } else if (obj->emitters[i].flags & PARTICLE_POINT) {
                obj->emitters[i].colourIndex += updateRate;
                obj_trigger_emitter(obj, &obj->emitters[i]);
            } else {
                obj->emitters[i].colourIndex += updateRate;
                if (obj->emitters[i].colourIndex >= obj->emitters[i].behaviour->spawnInterval) {
                    obj_trigger_emitter(obj, &obj->emitters[i]);
                }
            }
        } else if (obj->emitters[i].flags & PARTICLE_ACTIVE) {
            obj_disable_emitter(obj, i);
        }
        bits >>= 1;
    }
    D_800A5DDC = 0.0f;
}

/* JFG: the part of a general particle obj_trigger_emitter touches */
typedef struct TrigParticle {
    /* 0x00 */ u8 pad0[0x1C];
    /* 0x1C */ Vec3f velocity;
    /* 0x28 */ u8 pad28[0x50 - 0x28];
    /* 0x50 */ s32 flags;
} TrigParticle;
extern f32 D_801008F4; // JFG: a velocity taken off a new particle while it catches up (x, y, z)
extern f32 D_801008F8;
extern f32 D_801008FC;
s32 func_80060434(void *arg0, ParticleEmitter *emitter, s32 arg2);
s32 particle_update(s32 particle, s32 updateRate);
void func_80061698(ParticleObj *obj, ParticleEmitter *emitter);
void point_particle_set_add(void *arg0, ParticleEmitter *emitter);

/**
 * Generates particles from a specific emitter attached to the object.
 * JFG: line and point emitters also wait for the spawn interval; a general particle spawned late is
 * caught up without D_801008F4's velocity and flagged 0x80000.
 * DKR: obj_trigger_emitter
 */
void obj_trigger_emitter(ParticleObj *obj, ParticleEmitter *emitter) {
    s32 i;
    ParticleBehaviour *behaviour;
    TrigParticle *particle;

    behaviour = emitter->behaviour;
    if (emitter->flags & PARTICLE_LINE) {
        if (emitter->colourIndex >= behaviour->spawnInterval) {
            emitter->colourIndex -= behaviour->spawnInterval;
            func_80061698(obj, emitter);
        }
    } else if (emitter->flags & PARTICLE_POINT) {
        if (emitter->colourIndex >= behaviour->spawnInterval) {
            emitter->colourIndex -= behaviour->spawnInterval;
            point_particle_set_add(obj, emitter);
        }
    } else {
        while (emitter->colourIndex >= behaviour->spawnInterval) {
            emitter->colourIndex -= behaviour->spawnInterval;
            for (i = 0; i < behaviour->burstCount; i++) {
                particle = (TrigParticle *) func_80060434(obj, emitter, -1);
                if (particle != NULL) {
                    if (emitter->colourIndex != 0) {
                        particle->velocity.x -= D_801008F4;
                        particle->velocity.y -= D_801008F8;
                        particle->velocity.z -= D_801008FC;
                        particle_update((s32) particle, emitter->colourIndex);
                        particle->velocity.x += D_801008F4;
                        particle->velocity.y += D_801008F8;
                        particle->velocity.z += D_801008FC;
                    }
                    particle->flags |= 0x80000;
                }
            }
        }
    }
}

/* JFG: the parts of a particle setup_particle_velocity sets */
typedef struct VelParticle {
    /* 0x00 */ u8 pad0[0x1C];
    /* 0x1C */ Vec3f velocity;
    /* 0x28 */ u8 pad28[0x48 - 0x28];
    /* 0x48 */ Vec3s *parentObj; // its rotation comes first
} VelParticle;

/* JFG: a model's attachment table entry */
typedef struct VelAttach {
    /* 0x00 */ u16 unk0;
    /* 0x02 */ u16 matrix; // the joint matrix the point moves with
} VelAttach;

typedef struct VelModelData {
    /* 0x00 */ u8 pad0[0x30];
    /* 0x30 */ VelAttach *attach;
    /* 0x34 */ u8 pad34[0x4E - 0x34];
    /* 0x4E */ s8 unk4E;
} VelModelData;

typedef struct VelModel {
    /* 0x00 */ VelModelData *data;
    /* 0x04 */ u8 pad4[0xA - 0x4];
    /* 0x0A */ s8 unkA;
    /* 0x0B */ s8 matrixIndex;
    /* 0x0C */ u8 padC[0x10 - 0xC];
    /* 0x10 */ u8 *matrices[1];
} VelModel;

typedef struct VelObject {
    /* 0x00 */ u8 pad0[0x1C];
    /* 0x1C */ Vec3f velocity;
    /* 0x28 */ u8 pad28[0x3A - 0x28];
    /* 0x3A */ s8 modelIndex;
    /* 0x3B */ u8 pad3B[0x40 - 0x3B];
    /* 0x40 */ struct {
        u8 pad0[0x1E];
        s8 unk1E;
    } *header;
    /* 0x44 */ u8 pad44[0x6C - 0x44];
    /* 0x6C */ VelModel **models;
} VelObject;

void func_80048C00(u8 *mtx, Vec3f *in, Vec3f *out, VelModelData *data);
f32 sqrtf(f32 x);
void func_800496B0(s32 arg0, Vec3s *rotation, Vec3f *in, Vec3f *out); // DKR: vec3f_rotate
void func_8004944C(Vec3s *rotation, Vec3f *vec);                     // DKR: vec3f_rotate_py
extern f32 D_801008F4; // JFG: the velocity the last particle got (x, y, z)
extern f32 D_801008F8;
extern f32 D_801008FC;

#ifdef NON_MATCHING
/* NON_MATCHING: the target loads model->data into a0 and copies it to a3 for the call (mine loads a3), which
   shifts the FPRs of the normalisation */
/**
 * Initializes the particle velocity.
 * JFG: the emission direction of an emitter at a model point turns with that point's joint matrix (keeping the
 * emission speed), and the base velocity is kept in D_801008F4.
 * DKR: setup_particle_velocity
 */
void func_8005F960(VelParticle *particle, VelObject *obj, ParticleEmitter *emitter, ParticleBehaviour *behaviour) {
    s32 randomizationFlags;
    Vec3f sourceVel;
    Vec3s angle;
    VelModel *model;
    f32 len;
    f32 scale;
    f32 speed;

    if (behaviour->flags & 0x70) {
        particle->velocity.x = behaviour->velocityModifier.x;
        particle->velocity.y = behaviour->velocityModifier.y;
        particle->velocity.z = behaviour->velocityModifier.z;
    } else {
        particle->velocity.x = 0.0f;
        particle->velocity.y = 0.0f;
        particle->velocity.z = 0.0f;
    }
    randomizationFlags = behaviour->randomizationFlags & 0x700;
    if (randomizationFlags) {
        if (randomizationFlags & 0x100) {
            particle->velocity.x += (f32) rand_range(-behaviour->velocityModifierRange.x,
                                                     behaviour->velocityModifierRange.x) * 0.0000152587890625f;
        }
        if (randomizationFlags & 0x200) {
            particle->velocity.y += (f32) rand_range(-behaviour->velocityModifierRange.y,
                                                     behaviour->velocityModifierRange.y) * 0.0000152587890625f;
        }
        if (randomizationFlags & 0x400) {
            particle->velocity.z += (f32) rand_range(-behaviour->velocityModifierRange.z,
                                                     behaviour->velocityModifierRange.z) * 0.0000152587890625f;
        }
    }
    switch (behaviour->flags & 0x70) {
        case 0x10:
            particle->velocity.x += obj->velocity.x;
            particle->velocity.y += obj->velocity.y;
            particle->velocity.z += obj->velocity.z;
            break;
        case 0x40:
            particle->velocity.x *= obj->velocity.x;
            particle->velocity.y *= obj->velocity.y;
            particle->velocity.z *= obj->velocity.z;
            break;
    }
    D_801008F4 = particle->velocity.x;
    D_801008F8 = particle->velocity.y;
    D_801008FC = particle->velocity.z;
    if (behaviour->flags & 4) {
        randomizationFlags = behaviour->randomizationFlags;
        sourceVel.x = 0;
        sourceVel.y = 0;
        sourceVel.z = -behaviour->emissionSpeed;
        if (randomizationFlags & 0x10) {
            sourceVel.z +=
                (f32) rand_range(-behaviour->emissionSpeedRange, behaviour->emissionSpeedRange) * 0.0000152587890625f;
        }
        speed = -sourceVel.z;
        if (randomizationFlags & 0x60) {
            angle.y_rotation = emitter->emissionDirection.y_rotation;
            if (randomizationFlags & 0x20) {
                angle.y_rotation +=
                    rand_range(-behaviour->emissionDirRange.y_rotation, behaviour->emissionDirRange.y_rotation);
            }
            angle.x_rotation = emitter->emissionDirection.x_rotation;
            if (randomizationFlags & 0x40) {
                angle.x_rotation +=
                    rand_range(-behaviour->emissionDirRange.x_rotation, behaviour->emissionDirRange.x_rotation);
            }
        } else {
            angle.y_rotation = emitter->emissionDirection.y_rotation;
            angle.x_rotation = emitter->emissionDirection.x_rotation;
        }
        func_8004944C(&angle, &sourceVel);
        if (obj->header->unk1E != 0) {
            model = obj->models[obj->modelIndex];
        } else {
            model = NULL;
        }
        if (emitter->unk23 != -1 && model != NULL && model->data->unk4E != 0) {
            if (model->unkA != 0) {
                sourceVel.x = 0.0f;
                sourceVel.y = 0.0f;
                sourceVel.z = 0.0f;
            } else {
                func_80048C00((model->data->attach[emitter->unk23].matrix << 6) + model->matrices[model->matrixIndex],
                              &sourceVel, &sourceVel, model->data);
                len = sqrtf(sourceVel.z * sourceVel.z + (sourceVel.x * sourceVel.x + sourceVel.y * sourceVel.y));
                if (len == 0.0f) {
                    scale = speed;
                } else {
                    scale = speed / len;
                }
                sourceVel.x *= scale;
                sourceVel.y *= scale;
                sourceVel.z *= scale;
            }
        } else if (particle->parentObj != NULL) {
            func_800496B0(1, particle->parentObj, &sourceVel, &sourceVel);
        }
        particle->velocity.x += sourceVel.x;
        particle->velocity.y += sourceVel.y;
        particle->velocity.z += sourceVel.z;
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/5DA60", func_8005F960);
#endif

/* JFG: the parts of a particle setup_particle_position sets */
typedef struct PosParticle {
    /* 0x00 */ u8 pad0[0xC];
    /* 0x0C */ Vec3f position;
    /* 0x18 */ u8 pad18[0x2E - 0x18];
    /* 0x2E */ u8 movementType;
    /* 0x2F */ u8 pad2F;
    /* 0x30 */ Vec3f localPos;
    /* 0x3C */ f32 movementParam;
} PosParticle;

typedef struct PosModelData {
    /* 0x00 */ u8 pad0[0x4E];
    /* 0x4E */ s8 unk4E;
} PosModelData;

typedef struct PosModel {
    /* 0x00 */ PosModelData *data;
    /* 0x04 */ u8 pad4[0x68 - 0x4];
    /* 0x68 */ Vec3f *points;
} PosModel;

typedef struct PosObjHeader {
    /* 0x00 */ u8 pad0[0x1E];
    /* 0x1E */ s8 unk1E;
} PosObjHeader;

typedef struct PosObject {
    /* 0x00 */ s16 rotation[3];
    /* 0x06 */ u8 pad6[2];
    /* 0x08 */ f32 scale;
    /* 0x0C */ f32 x;
    /* 0x10 */ f32 y;
    /* 0x14 */ f32 z;
    /* 0x18 */ u8 pad18[0x3A - 0x18];
    /* 0x3A */ s8 modelIndex;
    /* 0x3B */ u8 pad3B[0x40 - 0x3B];
    /* 0x40 */ PosObjHeader *header;
    /* 0x44 */ u8 pad44[0x6C - 0x44];
    /* 0x6C */ PosModel **models;
} PosObject;

void func_800496B0(s32 arg0, Vec3s *rotation, Vec3f *in, Vec3f *out); // DKR: vec3f_rotate
void func_8004944C(Vec3s *rotation, Vec3f *vec);                     // DKR: vec3f_rotate_py

/**
 * Initializes the particle position.
 * JFG: an emitter with a model point (unk23) starts there (scaled, or relative to the object), and the movement
 * parameter is randomised by flag 0x80000.
 * DKR: setup_particle_position
 */
void setup_particle_position(PosParticle *particle, PosObject *obj, ParticleEmitter *emitter, ParticleBehaviour *behaviour) {
    s32 randomizationFlags;
    Vec3f sourcePos;
    Vec3s sourceDir;
    PosModel *model;

    if (emitter->unk23 == -1) {
        particle->localPos.x = emitter->posX;
        particle->localPos.y = emitter->posY;
        particle->localPos.z = emitter->posZ;
    } else {
        if (obj->header->unk1E == 0) {
            model = obj->models[obj->modelIndex];
        } else {
            model = NULL;
        }
        if (model != NULL && model->points != NULL) {
            particle->localPos.x = model->points[emitter->unk23].x;
            particle->localPos.y = model->points[emitter->unk23].y;
            particle->localPos.z = model->points[emitter->unk23].z;
            if (model->data->unk4E == 0) {
                particle->localPos.x *= obj->scale;
                particle->localPos.y *= obj->scale;
                particle->localPos.z *= obj->scale;
                func_800496B0(1, (Vec3s *) obj->rotation, &particle->localPos, &particle->localPos);
            } else {
                particle->localPos.x -= obj->x;
                particle->localPos.y -= obj->y;
                particle->localPos.z -= obj->z;
            }
        } else {
            particle->localPos.x = 0.0f;
            particle->localPos.y = 0.0f;
            particle->localPos.z = 0.0f;
        }
    }
    particle->movementParam = behaviour->movementParam;
    if (behaviour->randomizationFlags & 0x80000) {
        particle->movementParam +=
            (f32) rand_range(-behaviour->movementParamRange, behaviour->movementParamRange) * 0.0000152587890625f;
    }
    if (behaviour->flags & 1) {
        sourcePos.x = 0.0f;
        sourcePos.y = 0.0f;
        sourcePos.z = -behaviour->sourceDistance;
        randomizationFlags = behaviour->randomizationFlags;
        if (randomizationFlags & 1) {
            sourcePos.z +=
                (f32) rand_range(-behaviour->sourceDistanceRange, behaviour->sourceDistanceRange) * 0.0000152587890625f;
        }
        if (randomizationFlags & 6) {
            sourceDir.y_rotation = emitter->sourceRotation.y_rotation;
            if (randomizationFlags & 2) {
                sourceDir.y_rotation +=
                    rand_range(-behaviour->sourceDirRange.y_rotation, behaviour->sourceDirRange.y_rotation);
            }
            sourceDir.x_rotation = emitter->sourceRotation.x_rotation;
            if (randomizationFlags & 4) {
                sourceDir.x_rotation +=
                    rand_range(-behaviour->sourceDirRange.x_rotation, behaviour->sourceDirRange.x_rotation);
            }
            func_8004944C(&sourceDir, &sourcePos);
        } else {
            func_800496B0(1, &emitter->sourceRotation, &sourcePos, &sourcePos);
        }
        particle->localPos.x += sourcePos.x;
        particle->localPos.y += sourcePos.y;
        particle->localPos.z += sourcePos.z;
    }
    if (particle->movementType != 4 && emitter->unk23 == -1) {
        func_800496B0(1, (Vec3s *) obj->rotation, &particle->localPos, &particle->localPos);
    }
    particle->position.x = particle->localPos.x;
    particle->position.y = particle->localPos.y;
    particle->position.z = particle->localPos.z;
    if (particle->movementType == 4) {
        func_800496B0(1, (Vec3s *) obj->rotation, &particle->position, &particle->position);
    }
    particle->position.x += obj->x;
    particle->position.y += obj->y;
    particle->position.z += obj->z;
}

typedef struct EmitterSpec {
    /* 0x00 */ s16 behaviourID;
    /* 0x02 */ s16 descriptorID;
    /* 0x04 */ s16 trigger;
} EmitterSpec;

typedef struct EmitterObjHeader {
    /* 0x00 */ u8 pad0[0xB0];
    /* 0xB0 */ EmitterSpec *emitters;
} EmitterObjHeader;

typedef struct EmitterObj {
    /* 0x00 */ u8 pad0[0x40];
    /* 0x40 */ EmitterObjHeader *header;
    /* 0x44 */ u8 pad44[0x80 - 0x44];
    /* 0x80 */ ParticleEmitter *emitters;
    /* 0x84 */ u8 pad84[0x91 - 0x84];
    /* 0x91 */ u8 emitterCount;
} EmitterObj;

typedef struct EmittedParticle {
    /* 0x00 */ u8 pad0[0x1C];
    /* 0x1C */ Vec3f velocity;
} EmittedParticle;

s32 func_80060434(void *arg0, ParticleEmitter *emitter, s32 arg2);

/* JFG: set up one of an object's emitters from its header's table and fire it, giving the new particle a velocity. */
void obj_fire_emitter(EmitterObj *obj, f32 velX, f32 velY, f32 velZ, s32 index) {
    ParticleEmitter *emitter;
    ParticleBehaviour *behaviour;
    EmittedParticle *particle;

    if (index < obj->emitterCount) {
        emitter = &obj->emitters[index];
        behaviour = gParticleBehavioursAssetTable[obj->header->emitters[index].behaviourID];
        emitter->behaviour = behaviour;
        emitter->descriptorID = obj->header->emitters[index].descriptorID;
        emitter->flags = 0;
        emitter->colourIndex = 0;
        emitter->posX = 0;
        emitter->posY = 0;
        emitter->posZ = 0;
        emitter->unk20 = 0;
        emitter->sourceRotation.y_rotation = behaviour->sourceRotation.y_rotation;
        emitter->sourceRotation.x_rotation = behaviour->sourceRotation.x_rotation;
        emitter->sourceRotation.z_rotation = behaviour->sourceRotation.z_rotation;
        emitter->emissionDirection.y_rotation = behaviour->emissionDirection.y_rotation;
        emitter->emissionDirection.x_rotation = behaviour->emissionDirection.x_rotation;
        emitter->emissionDirection.z_rotation = behaviour->emissionDirection.z_rotation;
        emitter->unk23 = -1;
        particle = (EmittedParticle *) func_80060434(obj, emitter, obj->header->emitters[index].trigger);
        if (particle != NULL && (velX != 0.0f || velY != 0.0f || velZ != 0.0f)) {
            particle->velocity.x = velX;
            particle->velocity.y = velY;
            particle->velocity.z = velZ;
        }
    }
}

INCLUDE_ASM("asm/nonmatchings/5DA60", func_80060434);

/**
 * JFG: take a free particle of a kind from its list: the lowest free slot when slot is -1,
 * otherwise the highest. NULL if the list is full (then its full flag is set).
 */
WeatherParticle *particle_alloc(s32 kind, s32 slot) {
    WeatherParticleList *list;
    WeatherParticle *p = NULL;
    s32 i = 0;
    s32 bit;

    if (kind >= 5) {
        return NULL;
    }
    list = D_801008E0[kind];
    if (list != NULL) {
        if (list->activeCount >= list->count) {
            if (list->full == 0) {
                list->full = 1;
            }
        } else {
            if (slot == -1) {
                while (list->freeMask[i] == 0 && i <= list->lastMaskWord) {
                    i++;
                }
                if (i > list->lastMaskWord) {
                    return NULL;
                }
                bit = 0;
                while (!(list->freeMask[i] & (1 << bit))) {
                    bit++;
                }
                list->freeMask[i] &= ~(1 << bit);
                i = (i << 5) + bit;
            } else {
                i = list->lastMaskWord;
                while (i > 0 && list->freeMask[i] == 0) {
                    i--;
                }
                if (list->freeMask[i] == 0) {
                    return NULL;
                }
                bit = 31;
                while (!(list->freeMask[i] & (1 << bit))) {
                    bit--;
                }
                list->freeMask[i] &= ~(1 << bit);
                i = (i << 5) + bit;
            }
            if (i >= list->count) {
                return NULL;
            }
            p = &list->particles[i];
            p->kind = kind;
            list->activeCount++;
        }
    }
    return p;
}

/* JFG: free a particle: its sprite or model, its pool emitter's hold on it and its texture, then give its slot
   back to its kind's list (kind 0x80 marks a free particle). */
void particle_free(WeatherParticle *p) {
    s32 index;
    WeatherParticleList *list;

    switch (p->kind) {
        case 0x80:
            return;
        default:
            return;
        case 2:
            if (p->asset != NULL) {
                sprite_free(p->asset);
                p->asset = NULL;
            }
            break;
        case 3:
            if (p->asset != NULL) {
                free_3d_model(p->asset);
                p->asset = NULL;
            }
            break;
        case 0:
        case 1:
        case 4:
            break;
    }
    if (p->pool != NULL) {
        p->pool->active = 0;
    }
    list = D_801008E0[p->kind];
    if (list->activeCount > 0) {
        if (p->asset != NULL) {
            tex_free(p->asset);
        }
        list->activeCount--;
        index = p - list->particles;
        p->kind = 0x80;
        list->freeMask[index >> 5] |= 1 << (index & 0x1F);
    }
}

/* JFG: a particle that bounces off the level (the part particles_update_all touches) */
typedef struct BounceParticle {
    /* 0x00 */ u8 pad0[0xC];
    /* 0x0C */ Vec3f pos;
    /* 0x18 */ u8 pad18[0x1C - 0x18];
    /* 0x1C */ Vec3f velocity;
    /* 0x28 */ u8 pad28[0x2F - 0x28];
    /* 0x2F */ u8 bounces;
    /* 0x30 */ u8 pad30[0x50 - 0x30];
    /* 0x50 */ s32 flags;
    /* 0x54 */ u8 pad54[0x5C - 0x54];
    /* 0x5C */ s16 destroyTimer;
} BounceParticle;

/* JFG: a particle to test against the level this frame, and where it was */
typedef struct ParticleSweep {
    /* 0x00 */ BounceParticle *queued;
    /* 0x04 */ Vec3f from;
} ParticleSweep;

extern u8 D_800A5DE0;              // JFG: the sweep slot to fill next (the one with the most bounces)
extern ParticleSweep D_80100908[5];

/* JFG: the parts of a particle particle_update touches */
typedef struct UpdParticle {
    /* 0x00 */ u8 pad0[0xC];
    /* 0x0C */ Vec3f position;
    /* 0x18 */ u8 pad18[0x28 - 0x18];
    /* 0x28 */ f32 frame;
    /* 0x2C */ s16 kind;
    /* 0x2E */ u8 movementType;
    /* 0x2F */ u8 bounces;
    /* 0x30 */ u8 pad30[0x44 - 0x30];
    /* 0x44 */ u8 *colours;
    /* 0x48 */ u8 pad48[0x4C - 0x48];
    /* 0x4C */ void *texture;
    /* 0x50 */ s32 flags;
    /* 0x54 */ u8 pad54[0x58 - 0x54];
    /* 0x58 */ ParticleEmitter *emitter;
    /* 0x5C */ s16 destroyTimer;
    /* 0x5E */ u8 pad5E[0x64 - 0x5E];
    /* 0x64 */ s16 opacity;
    /* 0x66 */ s16 opacityVel;
    /* 0x68 */ s16 opacityTimer;
    /* 0x6A */ s16 colourIndex;
    /* 0x6C */ s16 colourTimer;
    /* 0x6E */ u8 numColours;
    /* 0x6F */ u8 pad6F[0x71 - 0x6F];
    /* 0x71 */ u8 r;
    /* 0x72 */ u8 g;
    /* 0x73 */ u8 b;
    /* 0x74 */ u8 pad74[0x77 - 0x74];
    /* 0x77 */ u8 texUpdateRate;
} UpdParticle;

struct Particle;
void move_particle_basic_parent(struct Particle *particle);
void move_particle_attached_to_parent(struct Particle *particle);
void move_particle_with_acceleration(struct Particle *particle);
void move_particle_basic(struct Particle *particle);
void move_particle_forward(struct Particle *particle);
s32 sprite_anim_advance(void *sprite, s32 *flags, s32 updateRate, f32 *frame, s32 speed);
s32 texture_anim_advance(TextureHeader *tex, s32 *flags, s32 updateRate, f32 *frame, s32 arg4);
extern s32 gParticleUpdateRate;

#define particle ((UpdParticle *) particleAddr)
/**
 * Call a function to update the current particle texture frame if applicable.
 * Applies movement, then the colour cycle and the opacity.
 * JFG: a particle flagged 0x200000 skips a frame, one flagged 0x80000 only catches up; general particles that
 * bounce (0x10000) queue for the level test in D_80100908; the colour cycle is per particle. Returns TRUE when
 * the particle was freed.
 * DKR: particle_update
 */
s32 particle_update(s32 particleAddr, s32 updateRate) {
    Vec3f prevPos;
    s32 found;
    s32 i;
    u8 *colour;

    if (particle->flags & 0x200000) {
        particle->flags &= ~0x200000;
        return FALSE;
    }
    gParticleUpdateRate = updateRate;
    if (particle->flags & 0x80000) {
        particle->flags ^= 0x80000;
        if (particle->movementType == 3) {
            gParticleUpdateRate = 0;
            move_particle_attached_to_parent((struct Particle *) particle);
            gParticleUpdateRate = updateRate;
        }
    } else if (particle->kind != 5) {
    particle->destroyTimer -= gParticleUpdateRate;
    if (particle->destroyTimer <= 0) {
        particle_free((WeatherParticle *) particle);
        return TRUE;
    }
    if (particle->kind == 2 && (particle->flags & 1)) {
        sprite_anim_advance(particle->texture, &particle->flags, particle->texUpdateRate, &particle->frame,
                      gParticleUpdateRate);
    } else if (particle->kind == 1 || particle->kind == 0) {
        if (particle->texture != NULL) {
            texture_anim_advance(particle->texture, &particle->flags, particle->texUpdateRate, &particle->frame,
                          gParticleUpdateRate);
        }
    }
    if (!(particle->flags & 0x20000)) {
        if (particle->flags & 0x10000) {
            prevPos.x = particle->position.x;
            prevPos.y = particle->position.y;
            prevPos.z = particle->position.z;
        }
        if (particle->movementType == 2) {
            move_particle_with_acceleration((struct Particle *) particle);
        } else if (particle->movementType == 3) {
            move_particle_attached_to_parent((struct Particle *) particle);
        } else if (particle->movementType == 4) {
            move_particle_basic_parent((struct Particle *) particle);
        } else if (particle->movementType == 5) {
            move_particle_forward((struct Particle *) particle);
        } else {
            move_particle_basic((struct Particle *) particle);
        }
        if (particle->flags & 0x10000) {
            found = FALSE;
            if (D_80100908[D_800A5DE0].queued != NULL) {
                if (particle->bounces < D_80100908[D_800A5DE0].queued->bounces ||
                    D_80100908[D_800A5DE0].queued->destroyTimer < particle->destroyTimer) {
                    found = TRUE;
                }
            } else {
                found = TRUE;
            }
            if (found) {
                D_80100908[D_800A5DE0].queued = (BounceParticle *) particle;
                D_80100908[D_800A5DE0].from.x = prevPos.x;
                D_80100908[D_800A5DE0].from.y = prevPos.y;
                D_80100908[D_800A5DE0].from.z = prevPos.z;
                D_800A5DE0 = 0;
                for (i = 1; i < 5; i++) {
                    if (D_80100908[i].queued != NULL) {
                        if (D_80100908[D_800A5DE0].queued->bounces < D_80100908[i].queued->bounces) {
                            D_800A5DE0 = i;
                        }
                    } else {
                        D_800A5DE0 = i;
                        i = 5;
                    }
                }
            }
        }
    }
    if (particle->flags & 0x400) {
        if (particle->colourTimer == 0) {
            particle->colourIndex += gParticleUpdateRate;
            while (particle->colourIndex >= particle->numColours) {
                particle->colourIndex -= particle->numColours;
            }
            colour = &particle->colours[particle->colourIndex * 4];
            particle->r = colour[0];
            particle->g = colour[1];
            particle->b = colour[2];
        } else {
            particle->colourTimer -= gParticleUpdateRate;
            if (particle->colourTimer < 0) {
                particle->colourIndex -= particle->colourTimer;
            }
        }
    }
    if (particle->opacityTimer == 0) {
        particle->opacity += gParticleUpdateRate * particle->opacityVel;
    } else {
        particle->opacityTimer -= gParticleUpdateRate;
        if (particle->opacityTimer < 0) {
            particle->opacity -= particle->opacityTimer * particle->opacityVel;
            particle->opacityTimer = 0;
        }
    }
    if (particle->emitter != NULL) {
        particle->emitter->colourIndex += updateRate;
        obj_trigger_emitter((ParticleObj *) particle, particle->emitter);
    }
    }
    return FALSE;
}
#undef particle

/* JFG: the part of a line particle description that a new segment takes */
typedef struct LineSegDesc {
    /* 0x00 */ s32 flags; // 0x400: colours from the colour list, 0x4000: along the velocity
    /* 0x04 */ union {
        u32 kind; // top 4 bits: kind (5 = line)
        u16 axis; // low 4 bits: the axis the segment lies on
    } u4;
    /* 0x08 */ u8 pad8[2];
    /* 0x0A */ s16 life;
    /* 0x0C */ u8 padC[2];
    /* 0x0E */ u8 alphaStart;
    /* 0x0F */ u8 alphaEnd;
    /* 0x10 */ s16 fadeDelay;
    /* 0x12 */ s16 colourDelay;
    /* 0x14 */ f32 length;
    /* 0x18 */ u8 pad18;
    /* 0x19 */ u8 r, g, b;
    /* 0x1C */ u8 pad1C[3];
    /* 0x1F */ u8 numColours;
    /* 0x20 */ u32 *colours;
} LineSegDesc;

/* JFG: the parts of an emitting object a line segment needs */
typedef struct LineSegObj {
    /* 0x00 */ Vec3s rotation;
    /* 0x06 */ u8 pad6[6];
    /* 0x0C */ Vec3f pos;
    /* 0x18 */ u8 pad18[4];
    /* 0x1C */ Vec3f velocity;
} LineSegObj;

#ifdef NON_MATCHING
/* NON_MATCHING: ROM length; from the line-set address on the t-registers are numbered one lower (the ROM keeps the
   emitter's behaviour in a1) and the second axis swap uses other FPRs. */
/* JFG: add a segment to a line emitter's set (at most 9): its life, colour and fade from the description, its two
   ends on either side of the emitter's position (rotated by the object), along an axis or the velocity. */
#define obj ((LineSegObj *) objAddr)
void func_80061698(ParticleObj *objAddr, ParticleEmitter *emitter) {
    LineSegDesc *desc;
    LineParticleSet *set;
    LineSeg *seg;
    f32 scale;
    f32 size;
    f32 lenSq;
    Vec3f pos;
    Vec3f dir;
    s32 axis;
    f32 t;

    desc = (LineSegDesc *) D_800A5D9C[emitter->descriptorID];
    if ((desc->u4.kind >> 28) == 5 && emitter->index != 0xFF) {
        set = &D_800A5D84[emitter->index];
        if (set->count == 9) {
            return;
        }
        {
            size = desc->length * emitter->behaviour->unk50;
            seg = set->segs[set->count];
            set->count++;
            seg->life = desc->life;
            if (desc->flags & 0x400) {
                if (set->colours == NULL) {
                    set->numColours = desc->numColours;
                    set->colours = desc->colours;
                }
                seg->r = (set->colours[0] & 0xFF000000) >> 24;
                seg->g = (set->colours[0] & 0xFF0000) >> 16;
                seg->b = (set->colours[0] & 0xFF00) >> 8;
            } else {
                seg->r = desc->r;
                seg->g = desc->g;
                seg->b = desc->b;
            }
            seg->fadeDelay = desc->fadeDelay;
            seg->colourIndex = 0;
            seg->colourDelay = desc->colourDelay;
            seg->alpha.value = desc->alphaStart << 8;
            if (seg->fadeDelay < seg->life) {
                seg->fadeRate = ((desc->alphaEnd - desc->alphaStart) << 8) / (seg->life - seg->fadeDelay);
            } else {
                seg->fadeRate = 0;
            }
            pos.x = emitter->posX;
            pos.y = emitter->posY;
            pos.z = emitter->posZ;
            func_800496B0(1, &obj->rotation, &pos, &pos);
            pos.x += obj->pos.x;
            pos.y += obj->pos.y;
            pos.z += obj->pos.z;
            if (emitter->behaviour->flags & 0x1000) {
                size = size *
                       sqrtf(obj->velocity.x * obj->velocity.x + obj->velocity.y * obj->velocity.y +
                             obj->velocity.z * obj->velocity.z) *
                       0.1f;
            }
            axis = desc->u4.axis & 0xF;
            if (!(desc->flags & 0x4000)) {
                dir.y = 0.0f;
                dir.z = 0.0f;
                dir.x = 0.0f;
                switch (axis) {
                    default:
                        dir.x = size;
                        break;
                    case 2:
                        dir.z = size;
                        break;
                    case 1:
                        dir.y = size;
                        break;
                }
                func_800496B0(1, &obj->rotation, &dir, &dir);
            } else {
                dir.x = obj->velocity.x;
                dir.y = obj->velocity.y;
                dir.z = obj->velocity.z;
                lenSq = dir.z * dir.z + (dir.x * dir.x + dir.y * dir.y);
                if (lenSq < 0.01f) {
                    scale = 1.0f;
                } else {
                    scale = size / sqrtf(lenSq);
                }
                dir.x *= scale;
                dir.y *= scale;
                dir.z *= scale;
                switch (axis) {
                    case 0:
                        t = dir.x;
                        dir.x = -dir.z;
                        dir.z = t;
                        break;
                    case 1:
                        t = dir.y;
                        dir.y = -dir.z;
                        dir.z = t;
                        break;
                }
            }
            seg->x0 = dir.x + pos.x;
            seg->y0 = dir.y + pos.y;
            seg->z0 = dir.z + pos.z;
            seg->x1 = pos.x - dir.x;
            seg->y1 = pos.y - dir.y;
            seg->z1 = pos.z - dir.z;
        }
    }
}
#undef obj
#else
INCLUDE_ASM("asm/nonmatchings/5DA60", func_80061698);
#endif

s32 texture_anim_advance(TextureHeader *tex, s32 *flags, s32 updateRate, f32 *frame, s32 arg4);

/*
 * JFG: update a line set: move the segments whose life ran out to the end, then age the rest (colour cycle
 * and fade) and animate the texture; a released set that is empty frees its texture.
 */
void line_particle_set_update(LineParticleSet *set, s32 updateRate) {
    s32 i;
    s32 j;
    LineSeg *seg;
    LineSeg *expired[9];

    i = 0;
    while (i < set->count && set->segs[i]->life < updateRate) {
        i++;
    }
    if (i > 0) {
        set->count -= i;
        for (j = 0; j < i; j++) {
            expired[j] = set->segs[j];
        }
        j = 0;
        for (; i < 9; j++, i++) {
            set->segs[j] = set->segs[i];
        }
        for (i = 0; j < 9; j++, i++) {
            set->segs[j] = expired[i];
        }
    }
    if (set->unk124 == 1 && set->count == 0) {
        set->unk124 = 0;
        if (set->texture != NULL) {
            tex_free(set->texture);
        }
    } else {
        for (i = 0; i < set->count; i++) {
            seg = set->segs[i];
            seg->life -= updateRate;
            if (set->colours != NULL) {
                if (seg->colourDelay == 0) {
                    seg->colourIndex += updateRate;
                    if (seg->colourIndex >= set->numColours) {
                        seg->colourIndex -= set->numColours;
                    }
                    seg->r = (set->colours[seg->colourIndex] & 0xFF000000) >> 24;
                    seg->g = (set->colours[seg->colourIndex] & 0xFF0000) >> 16;
                    seg->b = (set->colours[seg->colourIndex] & 0xFF00) >> 8;
                } else {
                    seg->colourDelay -= updateRate;
                    if (seg->colourDelay < 0) {
                        seg->colourIndex -= seg->colourDelay;
                    }
                }
            }
            if (seg->fadeDelay == 0) {
                seg->alpha.value += updateRate * seg->fadeRate;
            } else {
                seg->fadeDelay -= updateRate;
                if (seg->fadeDelay < 0) {
                    seg->alpha.value -= seg->fadeDelay * seg->fadeRate;
                    seg->fadeDelay = 0;
                }
            }
        }
        if (set->unk124) {
            if (set->flags & 0x800) {
                set->frame = rand_range(0, (set->texture->numOfTextures >> 8) - 1);
            } else {
                texture_anim_advance(set->texture, &set->unk138, set->unk144, &set->frame, updateRate);
            }
        }
    }
}

s32 particle_update(s32 particle, s32 updateRate);
s32 texture_anim_advance(TextureHeader *tex, s32 *flags, s32 updateRate, f32 *frame, s32 arg4);

/* JFG: update a point set's particles, pack the live ones to the front, then free or animate its texture. */
void point_particle_set_update(PointParticleSet *set, s32 updateRate) {
    s32 removed = FALSE;
    s32 i;
    s32 j;
    s32 found;
    TextureHeader *tex;

    for (i = 0; i < set->count; i++) {
        if (particle_update(set->points[i], updateRate)) {
            set->points[i] = 0;
            removed = TRUE;
        }
    }
    if (removed) {
        set->count = 0;
        for (i = 0; i < 15; i++) {
            found = FALSE;
            for (j = i; j < 15; j++) {
                if (set->points[j] != 0) {
                    found = TRUE;
                    if (i != j) {
                        set->points[i] = set->points[j];
                        set->points[j] = 0;
                    }
                    j = 15;
                    set->count++;
                }
            }
            if (!found) {
                set->points[i] = 0;
            }
        }
    }
    if (set->unkA4 == 1 && set->count == 0) {
        tex = set->texture;
        set->unkA4 = 0;
        if (tex != NULL) {
            tex_free(tex);
            set->texture = NULL;
        }
    }
    if (set->unkA4) {
        tex = set->texture;
        if (set->flags & 0x800) {
            set->frame = rand_range(0, (tex->numOfTextures >> 8) - 1);
        } else {
            texture_anim_advance(tex, &set->texFlags, set->texUpdateRate, &set->frame, updateRate);
        }
    }
}

INCLUDE_ASM("asm/nonmatchings/5DA60", func_80062178);

extern s32 D_800A5D80;
void func_80062178(Gfx **dList, Vertex **vtx, PointParticleSet *set);

/* JFG: render the flagged point particle sets (0x80062A00). */
void point_particle_sets_render(Gfx **dList, Vertex **vtx) {
    s32 i;
    PointParticleSet *item;

    if (D_800A5D88 != NULL) {
        item = D_800A5D88;
        for (i = 0; i < D_800A5D80; i++, item++) {
            if (item->unkA4) {
                func_80062178(dList, vtx, item);
            }
        }
    }
}

extern s32 D_800A5D7C;   // JFG: the number of line particle sets
extern u8 D_800A5DE8[]; // JFG: the triangle strip the line particles share

/* JFG: render the active line particle sets with two vertices per segment (their textures animated). */
void line_particles_render(Gfx **dList, Vertex **vtxList) {
    Vertex *vtx;
    Vertex *start;
    LineParticleSet *set;
    LineSeg *seg;
    Gfx *gfx;
    s32 i;
    s32 j;
    s32 n;

    if (D_800A5D84 != NULL) {
        gfx = *dList;
        vtx = *vtxList;
        set = D_800A5D84;
        for (i = 0; i < D_800A5D7C; i++, set++) {
            if (set->unk124 && set->count >= 2) {
                start = vtx;
                for (j = 0; j < set->count; j++) {
                    seg = set->segs[j];
                    vtx[0].x = seg->x0;
                    vtx[0].y = seg->y0;
                    vtx[0].z = seg->z0;
                    vtx[0].r = seg->r;
                    vtx[0].g = seg->g;
                    vtx[0].b = seg->b;
                    vtx[0].a = seg->alpha.hi;
                    vtx[1].x = seg->x1;
                    vtx[1].y = seg->y1;
                    vtx[1].z = seg->z1;
                    vtx[1].r = seg->r;
                    vtx[1].g = seg->g;
                    vtx[1].b = seg->b;
                    vtx[1].a = seg->alpha.hi;
                    vtx += 2;
                }
                func_80055C00(&gfx, set->texture, 0x12, set->frame * 65536.0f);
                n = set->count * 2;
                gSPVertexN1(gfx++, (u8 *) start + 0x80000000, n);
                gSPPolygon(gfx++, D_800A5DE8 + 0x80000000, n - 2, 1);
            }
        }
        gDPPipeSync(gfx++);
        *dList = gfx;
        *vtxList = vtx;
    }
}

struct ParticleDesc {
    /* 0x00 */ u8 pad0[4];
    /* 0x04 */ u32 flags; // top 4 bits: kind (4 = point)
};
extern ParticleDesc **D_800A5D9C;
s32 func_80060434(void *arg0, ParticleEmitter *emitter, s32 arg2);

/* JFG: add a new particle to the emitter's point set (at most 15) (0x80062D5C). */
void point_particle_set_add(void *arg0, ParticleEmitter *emitter) {
    ParticleDesc *desc = D_800A5D9C[emitter->descriptorID];
    PointParticleSet *set;
    s32 particle;

    if (emitter->index != 0xFF) {
        if ((desc->flags >> 28) == 4) {
            set = &D_800A5D88[emitter->index];
            if (set->count != 15) {
                particle = func_80060434(arg0, emitter, -1);
                if (particle != 0) {
                    set->points[set->count] = particle;
                    set->count++;
                }
            }
        }
    }
}

typedef struct ObjPos {
    /* 0x00 */ u8 pad0[0xC];
    /* 0x0C */ Vec3f pos;
} ObjPos;

/* DKR include/structs.h Particle, reduced (JFG offsets) */
typedef struct Particle {
    /* 0x00 */ Vec3s rotation;
    /* 0x06 */ u8 pad6[2];
    /* 0x08 */ f32 scale;
    /* 0x0C */ Vec3f position;
    /* 0x18 */ f32 scaleVelocity;
    /* 0x1C */ Vec3f velocity;
    /* 0x28 */ u8 pad28[0x30 - 0x28];
    /* 0x30 */ Vec3f localPos;
    /* 0x3C */ f32 forwardVel; // also downOffset / downAcceleration in DKR
    /* 0x40 */ f32 gravity;
    /* 0x44 */ u8 pad44[0x48 - 0x44];
    /* 0x48 */ ObjPos *parentObj;
    /* 0x4C */ u8 pad4C[0x5E - 0x4C];
    /* 0x5E */ Vec3s angularVelocity;
} Particle;

extern s32 gParticleUpdateRate;
void func_800496B0(s32 arg0, Vec3s *rotation, Vec3f *in, Vec3f *out); // DKR: vec3f_rotate
void func_8004944C(Vec3s *rotation, Vec3f *vec);                     // DKR: vec3f_rotate_py

/* Moves a particle by its velocities, offset by its parent's position (0x80062E0C). */
/**
 * Apply translation, rotation and scale based off the velocities of the particle.
 * Applies further translation based on the position of the parent.
 * DKR: move_particle_basic_parent.
 */
void move_particle_basic_parent(Particle *particle) {
    s32 i;
    ObjPos *parent;

    i = gParticleUpdateRate;
    while (i-- > 0) {
        particle->localPos.x += particle->velocity.x;
        particle->localPos.y += particle->velocity.y;
        particle->velocity.y -= particle->gravity;
        particle->localPos.z += particle->velocity.z;
        particle->rotation.y_rotation += particle->angularVelocity.y_rotation;
        particle->rotation.x_rotation += particle->angularVelocity.x_rotation;
        particle->rotation.z_rotation += particle->angularVelocity.z_rotation;
        particle->scale += particle->scaleVelocity;
    }
    particle->position.x = particle->localPos.x;
    particle->position.y = particle->localPos.y;
    particle->position.z = particle->localPos.z;
    parent = particle->parentObj;
    if (parent) {
        particle->position.x += parent->pos.x;
        particle->position.y += parent->pos.y;
        particle->position.z += parent->pos.z;
    }
}

/* Rotates and scales a particle, placing it below its local position relative to the parent (0x80062F0C). */
/**
 * Applies rotation and scale based on the particle's velocities.
 * Translates the particle downward in the object's local coordinate system by downOffset.
 * Applies additional translation based on the parent's position.
 * DKR: move_particle_attached_to_parent.
 */
void move_particle_attached_to_parent(Particle *particle) {
    s32 i = gParticleUpdateRate;
    ObjPos *parent;

    while (i-- > 0) {
        particle->rotation.y_rotation += particle->angularVelocity.y_rotation;
        particle->rotation.x_rotation += particle->angularVelocity.x_rotation;
        particle->rotation.z_rotation += particle->angularVelocity.z_rotation;
        particle->scale += particle->scaleVelocity;
    }

    particle->position.x = 0.0f;
    particle->position.y = -particle->forwardVel;
    particle->position.z = 0.0f;
    func_800496B0(1, &particle->rotation, &particle->position, &particle->position);
    particle->position.x += particle->localPos.x;
    particle->position.y += particle->localPos.y;
    particle->position.z += particle->localPos.z;

    parent = particle->parentObj;
    if (parent != NULL) {
        particle->position.x += parent->pos.x;
        particle->position.y += parent->pos.y;
        particle->position.z += parent->pos.z;
    }
}

/* Moves a particle with a downward local acceleration (0x80063030). */
/**
 * Apply translation, rotation and scale based off the velocities of the particle.
 * Apply additional acceleration always directed downward in the local coordinate system
 * DKR: move_particle_with_acceleration.
 */
void move_particle_with_acceleration(Particle *particle) {
    s32 i = 0;
    Vec3f acceleration;

    while (i++ < gParticleUpdateRate) {
        particle->position.x += particle->velocity.x;
        particle->position.y += particle->velocity.y;
        particle->position.z += particle->velocity.z;
        particle->scale += particle->scaleVelocity;
        particle->rotation.y_rotation += particle->angularVelocity.y_rotation;
        particle->rotation.x_rotation += particle->angularVelocity.x_rotation;
        particle->rotation.z_rotation += particle->angularVelocity.z_rotation;

        acceleration.x = 0.0f;
        acceleration.y = -particle->forwardVel;
        acceleration.z = 0.0f;
        func_800496B0(1, &particle->rotation, &acceleration, &acceleration);
        particle->velocity.x += acceleration.x;
        particle->velocity.y += acceleration.y;
        particle->velocity.y -= particle->gravity;
        particle->velocity.z += acceleration.z;
    }
}

/* Moves a particle by its velocities and gravity (0x80063190). */
/**
 * Apply translation, rotation and scale based off the velocities of the particle.
 * DKR: move_particle_basic.
 */
void move_particle_basic(Particle *particle) {
    s32 i = 0;

    while (i++ < gParticleUpdateRate) {
        particle->position.x += particle->velocity.x;
        particle->position.y += particle->velocity.y;
        particle->velocity.y -= particle->gravity;
        particle->position.z += particle->velocity.z;
        particle->scale += particle->scaleVelocity;
        particle->rotation.y_rotation += particle->angularVelocity.y_rotation;
        particle->rotation.x_rotation += particle->angularVelocity.x_rotation;
        particle->rotation.z_rotation += particle->angularVelocity.z_rotation;
    }
}

/* Moves a particle forward along its rotation (0x80063244). */
/**
 * Apply translation, rotation and scale based off the velocities of the particle.
 * Velocity is set from forward velocity.
 * The particle always moves forward in the local coordinate system
 * DKR: move_particle_forward.
 */
void move_particle_forward(Particle *particle) {
    s32 i = 0;

    while (i++ < gParticleUpdateRate) {
        particle->velocity.x = 0.0f;
        particle->velocity.y = 0.0f;
        particle->velocity.z = -particle->forwardVel;
        func_8004944C(&particle->rotation, &particle->velocity);
        particle->position.x += particle->velocity.x;
        particle->position.y += particle->velocity.y - particle->gravity;
        particle->position.z += particle->velocity.z;
        particle->scale += particle->scaleVelocity;
        particle->rotation.y_rotation += particle->angularVelocity.y_rotation;
        particle->rotation.x_rotation += particle->angularVelocity.x_rotation;
        particle->rotation.z_rotation += particle->angularVelocity.z_rotation;
    }
}


/* JFG: what func_80016208 fills in (12D50.c SweepHit) */
typedef struct ParticleHit {
    /* 0x00 */ void *obj;
    /* 0x04 */ f32 pos[3];
    /* 0x10 */ f32 normal[3];
    /* 0x1C */ u8 pad1C[0x28 - 0x1C];
} ParticleHit;

void line_particle_set_update(LineParticleSet *set, s32 updateRate);
void point_particle_set_update(PointParticleSet *set, s32 updateRate);
void func_8001CE10(s32 arg0, f32 *from, f32 *to, f32 *radius, s32 arg4, s32 arg5, s32 arg6);
s32 func_80016208(void *obj, f32 *from, f32 *to, f32 radius, s32 arg4, void *hit);
f32 sqrtf(f32 x);

/*
 * JFG: update every particle (the four particle lists, the line sets and the point sets), then sweep each
 * particle queued in D_80100908 against the level: one that hits bounces off (reflected about the hit's
 * vector, scaled by 0.66), or comes to rest (flag 0x20000) when slow on a floor.
 */
void particles_update_all(s32 updateRate) {
    s32 i;
    WeatherParticleList **listPtr;
    WeatherParticleList *list;
    WeatherParticle *p;
    LineParticleSet *line;
    PointParticleSet *point;
    BounceParticle *particle;
    f32 len;
    f32 dot;
    f32 two;
    f32 radius;
    ParticleHit hit;

    D_800A5DE0 = 0;
    for (i = 0; i < 5; i++) {
        D_80100908[i].queued = NULL;
    }
    for (listPtr = D_801008E0; listPtr < D_801008E0 + 4; listPtr++) {
        list = *listPtr;
        if (list != NULL) {
            p = list->particles;
            for (i = 0; i < list->count; i++, p++) {
                if (p->kind != 0x80) {
                    particle_update((s32) p, updateRate);
                }
            }
        }
    }
    if (D_800A5D84 != NULL) {
        line = D_800A5D84;
        for (i = 0; i < D_800A5D7C; i++, line++) {
            if (line->unk124) {
                line_particle_set_update(line, updateRate);
            }
        }
    }
    if (D_800A5D88 != NULL) {
        point = D_800A5D88;
        for (i = 0; i < D_800A5D80; i++, point++) {
            if (point->unkA4) {
                point_particle_set_update(point, updateRate);
            }
        }
    }
    for (i = 0; i < 5; i++) {
        particle = D_80100908[i].queued;
        if (particle != NULL) {
            radius = 5.0f;
            func_8001CE10(1, &D_80100908[i].from.x, &particle->pos.x, &radius, 0xCE002000, 0, 0);
            if (func_80016208(NULL, &D_80100908[i].from.x, &particle->pos.x, radius, 1, &hit)) {
                if (hit.pos[1] > 0.75f) {
                    particle->bounces++;
                }
                len = sqrtf(particle->velocity.x * particle->velocity.x + particle->velocity.y * particle->velocity.y +
                            particle->velocity.z * particle->velocity.z);
                if (len != 0.0f) {
                    particle->velocity.x /= -len;
                    particle->velocity.y /= -len;
                    particle->velocity.z /= -len;
                }
                if (len < 0.8f && hit.pos[1] > 0.75f) {
                    particle->velocity.x = 0.0f;
                    particle->velocity.y = 0.0f;
                    particle->velocity.z = 0.0f;
                    particle->flags |= 0x20000;
                } else {
                    dot = hit.pos[0] * particle->velocity.x + hit.pos[1] * particle->velocity.y +
                          hit.pos[2] * particle->velocity.z;
                    len *= 0.66f;
                    two = (f32) 2 * dot;
                    particle->velocity.x = (hit.pos[0] * two - particle->velocity.x) * len;
                    particle->velocity.y = (hit.pos[1] * two - particle->velocity.y) * len;
                    particle->velocity.z = (hit.pos[2] * two - particle->velocity.z) * len;
                }
            }
        }
    }
}


extern WeatherParticleList *D_801008E0[5];
extern s32 D_800A5DD8;
void set_active_camera(s32 arg0);
void viewport_apply(Gfx **dList, Mtx **matrix);
void func_8005D93C(Gfx **dList, Vertex **vtx, WeatherParticleList *list);
void line_particles_render(Gfx **dList, Vertex **vtx);
void sprite_particles_render(Gfx **dList, Mtx **matrix, Vertex **vtx, WeatherParticleList *list);
void model_particles_render(Gfx **dList, Mtx **matrix, Vertex **vtx, WeatherParticleList *list);

/* JFG: render all the particles into the current vertex buffer, then flip buffers (0x80063730). */
void render_all_particles(Gfx **dList, Mtx **matrix) {
    Vertex *vtx = D_800A5D8C[D_800A5DD8];

    gDPPipeSync((*dList)++);
    set_active_camera(0);
    viewport_apply(dList, matrix);
    func_8005D93C(dList, &vtx, D_801008E0[0]);
    func_8005D93C(dList, &vtx, D_801008E0[1]);
    line_particles_render(dList, &vtx);
    point_particle_sets_render(dList, &vtx);
    sprite_particles_render(dList, matrix, &vtx, D_801008E0[2]);
    model_particles_render(dList, matrix, &vtx, D_801008E0[3]);
    D_800A5DD8 ^= 1;
}

/* JFG: move the particles of all five lists by the object's position (0x8006382C). */
void particles_offset_by_position(ObjPos *obj) {
    s32 i;
    s32 j;
    WeatherParticleList *list;
    WeatherParticle *p;

    for (i = 0; i < 5; i++) {
        list = D_801008E0[i];
        p = list->particles;
        for (j = 0; j < list->count; j++, p++) {
            if (p->kind != 0x80 && p->unk2E == 3) {
                p->pos.x += obj->pos.x;
                p->pos.y += obj->pos.y;
                p->pos.z += obj->pos.z;
                p->unk48 = 0;
            }
        }
    }
}
