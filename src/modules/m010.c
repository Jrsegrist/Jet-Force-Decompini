/*
 * Runlink module 10: sparks. Up to 80 sparks (40 in a multiplayer game),
 * each a textured quad (two triangles) that flies from where it was added,
 * falls under a settable downward acceleration and fades out over its life.
 * The draw function sorts them by camera depth before drawing them as
 * streaks from their previous to their current position.
 */
#include "module.h"

typedef union M010Colour {
    s32 word;
    u8 c[4]; /* r, g, b, a */
} M010Colour;

typedef struct M010Spark {
    /* 0x00 */ u8 flags0; /* two of the game's 16-byte textured triangles */
    /* 0x01 */ u8 vi00;
    /* 0x02 */ u8 vi01;
    /* 0x03 */ u8 vi02;
    /* 0x04 */ s16 uv0[3][2];
    /* 0x10 */ u8 flags1;
    /* 0x11 */ u8 vi10;
    /* 0x12 */ u8 vi11;
    /* 0x13 */ u8 vi12;
    /* 0x14 */ s16 uv1[3][2];
    /* 0x20 */ u8 *texture;
    /* 0x24 */ M010Colour colour1;
    /* 0x28 */ M010Colour colour2;
    /* 0x2C */ Vec3f pos;
    /* 0x38 */ Vec3f prevPos;
    /* 0x44 */ Vec3f vel;
    /* 0x50 */ f32 size;
    /* 0x54 */ s16 timer;
    /* 0x56 */ s16 alpha;
} M010Spark;

extern u8 D_800A4FC4; /* a multiplayer game */
extern void *func_8004A57C(s32 size, s32 colour); /* DKR mempool_alloc_safe */
extern void func_8004AA80(void *data);            /* DKR mempool_free */
extern u8 *func_80055694(s32 id);                 /* DKR load_texture */
extern void func_80055A00(void *tex);             /* DKR tex_free */
extern f32 get_distance_to_camera(f32 x, f32 y, f32 z);    /* the point's depth from the camera */
extern void func_8000A300(void *arg0, void *arg1, u8 *texture, void *tris, Vec3f *pos, Vec3f *prevPos, f32 size,
                          s32 colour1, s32 colour2, s32 flags);

static s32 sCount = 0;
static f32 sFallAccel = 0.15f;

static M010Spark *sSparks[80];

void func_m010_00000(f32 accel) {
    sFallAccel = accel;
}

#ifdef NON_MATCHING
/* registers only: the original keeps the texture in a0 and the reread alpha/timer in a1/a2 */
void *func_m010_0000C(SparkParams *params) {
    M010Spark *spark = NULL;
    s32 max;
    s32 w;
    s32 h;

    if (D_800A4FC4) {
        max = 40;
    } else {
        max = 80;
    }
    if (sCount < max) {
        spark = func_8004A57C(sizeof(M010Spark), 0xFF7F7FFF);
        if (spark != NULL) {
            spark->texture = func_80055694(params->unk18);
            if (spark->texture != NULL) {
                w = (spark->texture[0] - 1) << 5;
                h = (spark->texture[1] - 1) << 5;
                spark->flags0 = 0x40;
                spark->vi00 = 0;
                spark->uv0[0][0] = w;
                spark->uv0[0][1] = 0;
                spark->vi01 = 1;
                spark->uv0[1][0] = w;
                spark->uv0[1][1] = h;
                spark->vi02 = 2;
                spark->uv0[2][0] = 0;
                spark->uv0[2][1] = 0;
                spark->flags1 = 0x40;
                spark->vi10 = 1;
                spark->uv1[0][0] = w;
                spark->uv1[0][1] = h;
                spark->vi11 = 2;
                spark->uv1[1][0] = 0;
                spark->uv1[1][1] = 0;
                spark->vi12 = 3;
                spark->uv1[2][0] = 0;
                spark->uv1[2][1] = h;
                spark->colour1.word = params->unk1C;
                spark->colour2.word = params->unk20;
                spark->alpha = params->unk1B;
                spark->timer = (u8) params->unk1A * 6;
                spark->colour1.c[3] = spark->alpha;
                if (spark->timer < 32) {
                    spark->colour1.c[3] = (spark->alpha * spark->timer) >> 5;
                }
                spark->colour2.c[3] = 0xFF;
                spark->pos.x = params->x_position;
                spark->pos.y = params->y_position;
                spark->pos.z = params->z_position;
                spark->prevPos.x = params->x_position;
                spark->prevPos.y = params->y_position;
                spark->prevPos.z = params->z_position;
                spark->vel.z = -params->speed;
                func_8004944C((Vec3s *) params, &spark->vel);
                spark->size = params->unk14;
                sSparks[sCount] = spark;
                sCount++;
            }
        } else {
            func_8004AA80(spark);
            spark = NULL;
        }
    }
    return spark;
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m010", func_m010_0000C);
#endif

void func_m010_00208(M010Spark *spark) {
    s32 i;
    M010Spark **p;

    i = sCount;
    p = sSparks;
    while (i--) {
        if (spark == *p) {
            while (i--) {
                p[0] = p[1];
                p++;
            }
            if (spark->texture != NULL) {
                func_80055A00(spark->texture);
            }
            func_8004AA80(spark);
            sCount--;
            return;
        }
        p++;
    }
}

void func_m010_002C0(void) {
    while (sCount != 0) {
        func_m010_00208(sSparks[0]);
    }
}

void func_m010_00318(s32 updateRate) {
    s32 i;
    s32 j;
    M010Spark *spark;

    if (updateRate != 0) {
        i = 0;
        while (i < sCount) {
            spark = sSparks[i++];
            spark->timer -= updateRate;
            if (spark->timer > 0) {
                spark->prevPos.x = spark->pos.x;
                spark->prevPos.y = spark->pos.y;
                spark->prevPos.z = spark->pos.z;
                j = updateRate;
                while (j--) {
                    spark->pos.x += spark->vel.x;
                    spark->pos.y += spark->vel.y;
                    spark->pos.z += spark->vel.z;
                    spark->vel.y -= sFallAccel;
                }
                spark->colour1.c[3] = spark->alpha;
                if (spark->timer < 32) {
                    spark->colour1.c[3] = (spark->alpha * spark->timer) >> 5;
                }
            } else {
                func_m010_00208(spark);
                i--;
            }
        }
    }
}

void func_m010_00460(void *arg0, void *arg1) {
    s32 i;
    s32 j;
    f32 tmpf;
    f32 depth[80];
    M010Spark *spark;

    i = 0;
    while (i < sCount) {
        spark = sSparks[i];
        depth[i] = get_distance_to_camera(spark->pos.x, spark->pos.y, spark->pos.z);
        i++;
    }
    j = sCount;
    while (j--) {
        for (i = 0; i < j; i++) {
            if (depth[i + 1] < depth[i]) {
                tmpf = depth[i];
                depth[i] = depth[i + 1];
                depth[i + 1] = tmpf;
                spark = sSparks[i];
                sSparks[i] = sSparks[i + 1];
                sSparks[i + 1] = spark;
            }
        }
    }
    i = 0;
    while (i < sCount) {
        spark = sSparks[i++];
        func_8000A300(arg0, arg1, spark->texture, spark, &spark->pos, &spark->prevPos, spark->size, spark->colour1.word,
                      spark->colour2.word, 0x200);
    }
}
