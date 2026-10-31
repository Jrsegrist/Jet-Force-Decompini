/*
 * Runlink module 81: an effect library of sparks that gather in on a point.
 * Other modules create an effect with a spark count, a size range, a spread
 * and a speed, set how many of its sparks should be live (a level from 0 to
 * 1), update it every frame and draw it at a position. Each new spark starts
 * out at a random direction and distance and falls in towards the centre,
 * faster and faster, until it reaches it; up to four new ones start each
 * frame. Up to four effects exist at once; they share one texture.
 */
#include "module.h"
#include "gfx.h"

/* Size 0x18 */
typedef struct M081Spark {
    /* 0x00 */ u8 active;
    /* 0x01 */ u8 size;
    /* 0x02 */ s16 alpha;
    /* 0x04 */ Vec3f offset; /* from the centre at the start */
    /* 0x10 */ f32 scale;    /* how much of the offset is left: 1 to 0 */
    /* 0x14 */ f32 speed;    /* of the scale's fall */
} M081Spark;

/* Size 0x18 */
typedef struct M081Effect {
    /* 0x00 */ u8 liveCount;
    /* 0x01 */ u8 unk1;
    /* 0x02 */ u8 targetCount;
    /* 0x03 */ u8 count;
    /* 0x04 */ u8 buffer; /* which vertex set is current */
    /* 0x05 */ u8 minSize;
    /* 0x06 */ u8 maxSize;
    /* 0x07 */ u8 spread; /* the pitch range of new sparks, in 1/256ths of a half turn */
    /* 0x08 */ f32 speed;
    /* 0x0C */ M081Spark *sparks;
    /* 0x10 */ Vertex *verts[2]; /* four per spark */
} M081Effect;

extern void *func_8004A57C(s32 size, s32 colour); /* DKR mempool_alloc_safe */
extern void func_8004AA80(void *data);            /* DKR mempool_free */
extern u8 *func_80055694(s32 id);                 /* DKR load_texture */
extern void func_80055A00(u8 *tex);               /* DKR tex_free */
extern MtxF *get_camera_matrix(void);
extern Mtx *get_projection_matrix_s16(void);
extern void func_80048F90(MtxF *out, ObjectTransform *trans); /* the matrix of a transform */
extern void func_80048C7C(MtxF *a, MtxF *b, MtxF *out);       /* out = a * b */
extern void func_80048B60(MtxF *mtx, f32 x, f32 y, f32 z, f32 *ox, f32 *oy, f32 *oz); /* DKR mathMtxXFMF */
extern void func_80055C00(Gfx **dl, u8 *tex, s32 flags, s32 offset); /* sets a texture */
extern void func_80041DA0(Gfx **dl);                                  /* mtx_pop */

static s32 sEffectCount = 0;
static s32 sUnused = 0;
/* A spark's quad (the texture's width set by load_textures) */
static Triangle sQuadTris[2] = {
    { 0x40, 0, 1, 2, { { 0, 0 }, { 0, 0 }, { 0, 0 } } },
    { 0x40, 1, 2, 3, { { 0, 0 }, { 0, 0 }, { 0, 0 } } },
};
static u8 *sTexture = NULL;

static M081Effect sEffects[4];

/*
 * NON_MATCHING: func_m081_00080 (the stack slot of `effect`, 0x18 rather than
 * 0x1C, and the registers of the vertex colour loop differ). load_textures
 * matches, but only func_m081_00080 calls it, so it stays assembly with it.
 */
#ifdef NON_MATCHING
static void load_textures(void) {
    Triangle *tris;
    s32 w;

    sTexture = func_80055694(0x257);
    if (sTexture != NULL) {
        tris = sQuadTris;
        w = (sTexture[0] - 1) << 5;
        tris[0].uv[1][0] = w;
        tris[0].uv[2][1] = w;
        tris[1].uv[0][0] = w;
        tris[1].uv[1][1] = w;
        tris[1].uv[2][0] = w;
        tris[1].uv[2][1] = w;
        sEffects[0].sparks = NULL;
        sEffects[1].sparks = NULL;
        sEffects[2].sparks = NULL;
        sEffects[3].sparks = NULL;
    }
}

M081Effect *func_m081_00080(s32 count, s32 minSize, s32 maxSize, f32 spread, f32 speed) {
    M081Effect *effect;
    Vertex *v;
    s32 i;
    M081Effect *ret;
    s32 j;

    ret = NULL;
    if (sEffectCount < 4) {
        if (sEffectCount == 0) {
            load_textures();
        }
        i = 0;
        while (sEffects[i].sparks != NULL) {
            i++;
        }
        effect = &sEffects[i];
        effect->liveCount = 0;
        effect->unk1 = 0;
        effect->targetCount = 0;
        effect->count = count;
        effect->buffer = 0;
        effect->minSize = minSize;
        effect->maxSize = maxSize;
        effect->spread = (s32) (spread * 1.4223f);
        effect->speed = speed * -0.01f;
        ret = effect;
        effect->sparks = func_8004A57C(count * (sizeof(M081Spark) + 8 * sizeof(Vertex)), 0xFF00FF);
        if (effect->sparks == NULL) {
            return NULL;
        }
        for (i = 0; i < count; i++) {
            ret->sparks[i].active = 0;
        }
        ret->verts[0] = (Vertex *) &ret->sparks[count];
        ret->verts[1] = (Vertex *) ((u8 *) ret->verts[0] + (count << 2) * sizeof(Vertex));
        for (i = 0; i < 2; i++) {
            v = ret->verts[i];
            for (j = 0; j < count * 4; j++) {
                v->r = 255;
                v->g = 255;
                v->b = 255;
                v->a = 255;
                v++;
            }
        }
        sEffectCount++;
    }
    return ret;
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m081", func_m081_00000);

INCLUDE_ASM("asm/nonmatchings/modules/m081", func_m081_00080);

/* The two float constants of func_m081_00080, which come first in the .rodata */
static const f32 sCreateConsts[2] = { 1.4223f, -0.01f };
#endif

void func_m081_0034C(M081Effect *effect) {
    if (effect->sparks != NULL) {
        func_8004AA80(effect->sparks);
        effect->sparks--;
        if (sEffectCount == 1 && sTexture != NULL) {
            func_80055A00(sTexture);
            sTexture = NULL;
        }
        sEffectCount--;
    }
}

void func_m081_003D8(M081Effect *effect, f32 level) {
    if (level < 0.0f) {
        level = 0.0f;
    }
    if (level > 1.0f) {
        level = 1.0f;
    }
    effect->targetCount = (s32) (effect->count * level);
}

void func_m081_00464(M081Effect *effect, s32 updateRate) {
    M081Spark *spark;
    s32 i;
    s32 n;
    s32 live;
    s32 yaw;
    s32 pitch;
    f32 speed;

    if (effect->liveCount != 0) {
        spark = effect->sparks;
        for (i = 0; i < effect->count; i++, spark++) {
            if (spark->active) {
                spark->alpha += updateRate << 4;
                if (spark->alpha >= 256) {
                    spark->alpha = 255;
                }
                spark->speed += 0.005f * (f32) updateRate;
                spark->scale -= spark->speed * (f32) updateRate;
                if (spark->scale <= 0.0f) {
                    spark->active = 0;
                    effect->liveCount--;
                }
            }
        }
    }
    if ((live = effect->liveCount) < effect->targetCount) {
        if ((n = effect->targetCount - live) > 4) {
            n = 4;
        }
    } else {
        n = 0;
    }
    while (n--) {
        spark = effect->sparks;
        while (spark->active) {
            spark++;
        }
        yaw = func_80048E2C(-0x8000, 0x7FFF);
        pitch = func_80048E2C(0, effect->spread << 7);
        speed = func_80048E2C(60, 100) * effect->speed;
        spark->active = 1;
        spark->alpha = 0;
        spark->size = func_80048E2C(effect->minSize, effect->maxSize);
        spark->offset.x = speed * func_80049C34(pitch) * func_80049C34(yaw);
        spark->offset.y = func_80049C30(pitch) * speed;
        spark->offset.z = speed * func_80049C34(pitch) * func_80049C30(yaw);
        spark->scale = 1.0f;
        spark->speed = 0.0f;
        effect->liveCount++;
    }
}

/*
 * NON_MATCHING: the same length and the same instructions in most places, but
 * the locals sit 8 bytes lower than in the original (it has fewer words above
 * ox), the matrix zeroes share the f24 zero and the env colour multiplies by
 * 255 with multu rather than shifts.
 */
#ifdef NON_MATCHING
void func_m081_006F8(Gfx **dl, s32 arg1, M081Effect *effect, Vec3s *rot, Vec3f *pos, MtxF *mtx) {
    MtxF *camera;
    M081Spark *spark;
    Vertex *v;
    s32 i;
    s32 ratio;
    s32 x;
    s32 y;
    s32 z;
    f32 zero;
    f32 scale;
    f32 ox;
    f32 oy;
    f32 oz;
    MtxF m;
    MtxF m2;
    MtxF m1;
    ObjectTransform trans;
    f32 *src;
    f32 *dst;

    if (effect->liveCount != 0) {
        camera = get_camera_matrix();
        zero = 0.0f;
        trans.rotation.y_rotation = rot->y_rotation;
        trans.rotation.x_rotation = rot->x_rotation;
        trans.rotation.z_rotation = rot->z_rotation;
        trans.scale = 1.0f;
        trans.x_position = zero;
        trans.y_position = zero;
        trans.z_position = zero;
        i = 0;
        src = (f32 *) mtx;
        for (dst = (f32 *) m; dst < (f32 *) m[4]; dst++) {
            *dst = *src++;
        }
        m[3][0] = 0.0f;
        m[3][1] = 0.0f;
        m[3][2] = 0.0f;
        func_80048F90(&m2, &trans);
        func_80048C7C(&m2, &m, &m1);
        m1[3][0] = pos->x;
        m1[3][1] = pos->y;
        m1[3][2] = pos->z;
        func_80048C7C(&m1, camera, &m2);
        gSPMatrix((*dl)++, OS_K0_TO_PHYSICAL(get_projection_matrix_s16()), G_MTX_PROJECTION);
        func_80055C00(dl, sTexture, 0x216, 0);
        ratio = (effect->liveCount << 16) / effect->count;
        gDPSetEnvColor((*dl)++, ((-ratio * 255) >> 16) + 255, (ratio * 255) >> 16, 0, 255);
        effect->buffer = 1 - effect->buffer;
        v = effect->verts[effect->buffer];
        spark = effect->sparks;
        for (i = 0; i < effect->count; i++, spark++) {
            if (spark->active) {
                scale = spark->scale;
                func_80048B60(&m2, spark->offset.x * scale, spark->offset.y * scale, spark->offset.z * scale, &ox,
                              &oy, &oz);
                scale = spark->scale;
                x = ox;
                y = oy;
                z = oz;
                gDPSetPrimColor((*dl)++, 0, 0, (s32) (scale * zero) + 255, (s32) (scale * zero) + 255,
                                (s32) (scale * -170.0f) + 255, 255);
                gSPVertexJFG((*dl)++, OS_K0_TO_PHYSICAL(v), 5, 0);
                gSPPolygon((*dl)++, OS_K0_TO_PHYSICAL(sQuadTris), 2, 1);
                v->x = x - spark->size;
                v->y = spark->size + y;
                v->z = z;
                v->a = spark->alpha;
                v++;
                v->x = spark->size + x;
                v->y = spark->size + y;
                v->z = z;
                v->a = spark->alpha;
                v++;
                v->x = x - spark->size;
                v->y = y - spark->size;
                v->z = z;
                v->a = spark->alpha;
                v++;
                v->x = spark->size + x;
                v->y = y - spark->size;
                v->z = z;
                v->a = spark->alpha;
                v++;
            }
        }
        func_80041DA0(dl);
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m081", func_m081_006F8);
#endif
