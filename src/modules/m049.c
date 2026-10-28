/*
 * Runlink module 49: a soft shadow drawn from the shape of an object's model.
 * Each frame it draws a filled disc for every collision sphere of the model
 * (projected from above) into a 64x64 4-bit texture, blurs it and flips
 * between two such textures, sizing the shadow to the spheres' extent.
 */
#include "module.h"

/* The texture header the game draws (DKR's TextureHeader, structs.h) */
typedef struct M049Texture {
    /* 0x00 */ u8 width;
    /* 0x01 */ u8 height;
    /* 0x02 */ u8 format;
    /* 0x03 */ u8 unk3;
    /* 0x04 */ u8 unk4;
    /* 0x05 */ u8 unk5;
    /* 0x06 */ s16 unk6;
    /* 0x08 */ u16 unk8;
    /* 0x0A */ s16 cmdSize;
    /* 0x0C */ Gfx *cmd;
    /* 0x10 */ s16 unk10;
    /* 0x12 */ s16 unk12;
    /* 0x14 */ s16 unk14;
    /* 0x16 */ s16 textureSize;
    /* 0x18 */ u8 unk18;
    /* 0x19 */ u8 unk19;
    /* 0x1A */ u8 unk1A;
    /* 0x1B */ u8 unk1B;
    /* 0x1C */ u8 unk1C;
    /* 0x1D */ u8 unk1D;
    /* 0x1E */ u8 unk1E;
    /* 0x1F */ u8 unk1F;
} M049Texture;

/* The shadow's own state */
typedef struct M049Shadow {
    /* 0x00 */ f32 scale;
    /* 0x04 */ M049Texture *current;
    /* 0x08 */ s16 unk8;
    /* 0x0A */ s16 unkA;
    /* 0x0C */ u8 flags;
    /* 0x0D */ u8 unkD;
    /* 0x0E */ u8 unkE;
    /* 0x0F */ u8 unkF;
    /* 0x10 */ u8 pad10[0x20 - 0x10];
    /* 0x20 */ M049Texture *textures[2];
    /* 0x28 */ void *allocations[2];
    /* 0x30 */ f32 size;
    /* 0x34 */ u8 width;
    /* 0x35 */ u8 height;
    /* 0x36 */ s8 frame;
} M049Shadow;

/* What object->shadow points at */
typedef struct M049ShadowData {
    /* 0x00 */ f32 scale;
    /* 0x04 */ u8 pad4[0xC - 0x4];
    /* 0x0C */ u8 flags;
    /* 0x0D */ u8 padD;
    /* 0x0E */ u8 unkE;
    /* 0x0F */ u8 padF[0x1C - 0xF];
    /* 0x1C */ M049Shadow *shadow;
} M049ShadowData;

typedef struct M049Sphere {
    /* 0x0 */ u8 pad0[8];
    /* 0x8 */ f32 radius;
} M049Sphere;

typedef struct M049ModelHeader {
    /* 0x00 */ u8 pad0[0x2F];
    /* 0x2F */ u8 numSpheres;
    /* 0x30 */ u8 pad30[0x38 - 0x30];
    /* 0x38 */ M049Sphere *spheres;
} M049ModelHeader;

typedef struct M049Model {
    /* 0x00 */ M049ModelHeader *header;
    /* 0x04 */ u8 pad4[0xA - 0x4];
    /* 0x0A */ s8 animated;
    /* 0x0B */ u8 padB[0x70 - 0xB];
    /* 0x70 */ Vec3f *positions; /* of the spheres */
} M049Model;

#define SHADOW_DATA(obj) ((M049ShadowData *) (obj)->shadow)

extern void *func_8004A60C(s32 size, s32 colour); /* allocates */
extern void func_8004AA80(void *data);            /* frees */

/*
 * NON_MATCHING: func_m049_00000, func_m049_001C8 and the static circle
 * functions (0069C, 0078C, 00878) match, as do .data and .rodata. Still
 * different (register allocation only): func_m049_00204 (the extent loop's
 * temporaries and the spilled sphere offset), 00668 (the unrolled clear
 * schedules its pointer step differently), 009A8 (a0/a1 swapped) and 00A84
 * (the order the blur sums are formed in). The default build uses the
 * assembly for everything from 00204 on (the static functions are only
 * called from there) and for the data.
 */
#define TEXTURE_LOAD_SIZE (7 * sizeof(Gfx))

#ifdef NON_MATCHING
/* Loads the texture (4-bit intensity, 64x64, clamped) */
static Gfx sTextureLoad[] = {
    gsDPSetTextureImage(G_IM_FMT_RGBA, G_IM_SIZ_16b, 32, 0),
    gsDPSetTile(G_IM_FMT_RGBA, G_IM_SIZ_16b, 0, 0, G_TX_LOADTILE, 0, 0, 0, 0, 0, 0, 0),
    gsDPLoadSync(),
    gsDPLoadBlock(G_TX_LOADTILE, 0, 0, 1023, 512),
    gsDPPipeSync(),
    gsDPSetTile(G_IM_FMT_I, G_IM_SIZ_4b, 4, 0, G_TX_RENDERTILE, 0, G_TX_CLAMP, 0, 0, G_TX_CLAMP, 0, 0),
    gsDPSetTileSize(G_TX_RENDERTILE, 0, 0, 63 << G_TEXTURE_IMAGE_FRAC, 63 << G_TEXTURE_IMAGE_FRAC),
};

static s32 sWidth;
static s32 sHalfWidth;
static s32 sHeight;
static s32 sHalfHeight;
static u8 *sPixels;
static s32 sUnused;
static u8 sSpans[64][2]; /* per row: the leftmost and rightmost pixel, 0x80 when set */

static void func_m049_00A84(void);
static void func_m049_0069C(s32 x, s32 y, s32 radius);
#else
/* the texture load list, in the assembly data */
extern Gfx rl_m049_00BD0[];
#define sTextureLoad rl_m049_00BD0
#endif

s32 func_m049_00000(Object *obj) {
    M049Shadow *s = SHADOW_DATA(obj)->shadow;
    s32 i;
    M049Texture *tex;

    s->width = 64;
    s->height = 64;
    s->frame = 0;
    if ((s->allocations[0] = func_8004A60C(((s->width * s->height) >> 1) + 0x28, 0xFF7F7FFF)) == NULL) {
        return 0;
    }
    if ((s->allocations[1] = func_8004A60C(((s->width * s->height) >> 1) + 0x28, 0xFF7F7FFF)) == NULL) {
        return 0;
    }
    s->textures[0] = (M049Texture *) (((u32) s->allocations[0] & ~7) + 8);
    s->textures[1] = (M049Texture *) (((u32) s->allocations[1] & ~7) + 8);
    for (i = 0; i < 2; i++) {
        tex = s->textures[i];
        tex->width = s->width;
        tex->height = s->height;
        tex->format = 3;
        tex->unk3 = 0;
        tex->unk4 = 0;
        tex->unk5 = 0;
        tex->unk6 = 0;
        tex->unk8 = 1;
        tex->cmdSize = TEXTURE_LOAD_SIZE;
        tex->cmd = sTextureLoad;
        tex->unk10 = 0;
        tex->unk12 = 0;
        tex->unk14 = 0;
        tex->textureSize = (s->width * s->height) >> 1;
        tex->unk18 = 0;
        tex->unk19 = 0;
        tex->unk1A = 0;
        tex->unk1B = 0;
        tex->unk1C = 2;
        tex->unk1D = 0;
        tex->unk1E = 2;
        tex->unk1F = 0;
    }
    s->scale = SHADOW_DATA(obj)->scale;
    s->flags = SHADOW_DATA(obj)->flags | 0x20;
    s->unkD = 1;
    s->unkE = SHADOW_DATA(obj)->unkE;
    s->unkF = 0;
    s->unk8 = 0;
    s->unkA = 0;
    s->current = s->textures[0];
    return 1;
}

void func_m049_001C8(M049Shadow *s) {
    func_8004AA80(s->allocations[0]);
    func_8004AA80(s->allocations[1]);
}

#ifdef NON_MATCHING

s32 func_m049_00204(Object *obj) {
    M049Model *model;
    M049ModelHeader *hdr;
    M049Shadow *s;
    s32 *p;
    s32 n;
    f32 x0;
    f32 z0;
    f32 minX;
    f32 maxX;
    f32 minZ;
    f32 maxZ;
    f32 dx;
    f32 pixelScale;
    f32 dz;
    f32 r;
    f32 size;
    f32 diff;
    s32 radius;
    s32 i;
    s32 x;
    s32 z;
    s32 pad[4];

    model =((M049Model **) obj->modelInstances)[obj->modelIndex];
    if (model->animated) {
        return 0;
    }
    hdr = model->header;
    s = SHADOW_DATA(obj)->shadow;
    s->frame ^= 1;
    s->current = s->textures[s->frame];
    sWidth = s->width;
    sHeight = s->height;
    p = (s32 *) ((u8 *) s->current + 0x20);
    n = (sWidth >> 3) * sHeight;
    while (n--) {
        *p++ = 0;
    }
    x0 = obj->trans.x_position;
    z0 = obj->trans.z_position;
    if (hdr->numSpheres != 0) {
        minZ = minX = 32000.0f;
        maxZ = maxX = -32000.0f;
        for (i = 0; i < hdr->numSpheres; i++) {
            r = hdr->spheres[i].radius * obj->trans.scale;
            dx = model->positions[i].x - x0;
            dz = model->positions[i].z - z0;
            if (maxX < dx + r) {
                maxX = dx + r;
            }
            if (dx - r < minX) {
                minX = dx - r;
            }
            if (maxZ < dz + r) {
                maxZ = dz + r;
            }
            if (dz - r < minZ) {
                minZ = dz - r;
            }
        }
        if (minX < 0.0f) {
            minX = -minX;
        }
        if (minZ < 0.0f) {
            minZ = -minZ;
        }
        if (maxX < minX) {
            maxX = minX;
        }
        if (maxZ < minZ) {
            maxZ = minZ;
        }
        if (maxZ < maxX) {
            size = maxX + 2.0f;
        } else {
            size = maxZ + 2.0f;
        }
        size += size;
        diff = 100.0f / s->size * size - 100.0f;
        if (diff > -5.0f && diff < 5.0f) {
            size = s->size;
        } else {
            s->size = size;
        }
        s->scale = 0.05f * size;
        pixelScale = 64.0f / size;
    }
    sHalfWidth = sWidth >> 1;
    sHalfHeight = sHeight >> 1;
    sPixels = (u8 *) s->current + 0x20;
    for (i = 0; i < hdr->numSpheres; i++) {
        dx = (model->positions[i].x - x0) * pixelScale;
        dz = (model->positions[i].z - z0) * pixelScale;
        radius = hdr->spheres[i].radius * obj->trans.scale * pixelScale;
        x = dx;
        if (-sHalfWidth - radius < x && x < sHalfWidth + radius) {
            z = dz;
            if (-sHalfHeight - radius < z && z < sHalfHeight + radius) {
                func_m049_0069C(x, z, radius);
            }
        }
    }
    func_m049_00A84();
    return 1;
}

static void func_m049_00668(void) {
    s32 i;
    s32 *p;

    for (i = 0, p = (s32 *) sSpans; i < 32; i++) {
        *p++ = 0;
    }
}

static void func_m049_0078C(s32 cx, s32 cy, s32 x, s32 y);
static void func_m049_00878(s32 cx, s32 cy, s32 dx, s32 dy);
static void func_m049_009A8(void);

/* Draws a filled disc (midpoint circle) */
static void func_m049_0069C(s32 cx, s32 cy, s32 radius) {
    s32 d;
    s32 dEast;
    s32 dSouthEast;
    s32 x;
    s32 y;

    func_m049_00668();
    x = 0;
    y = radius;
    d = 1 - radius;
    dEast = 3;
    dSouthEast = -2 * radius + 5;
    func_m049_0078C(cx, cy, 0, radius);
    while (x < y) {
        x++;
        if (d < 0) {
            d += dEast;
            dEast += 2;
            dSouthEast += 2;
        } else {
            d += dSouthEast;
            dEast += 2;
            dSouthEast += 4;
            y--;
        }
        func_m049_0078C(cx, cy, x, y);
    }
    func_m049_009A8();
}

/* Marks the eight points of a circle */
static void func_m049_0078C(s32 cx, s32 cy, s32 x, s32 y) {
    func_m049_00878(cx, cy, x, y);
    func_m049_00878(cx, cy, y, x);
    func_m049_00878(cx, cy, y, -x);
    func_m049_00878(cx, cy, x, -y);
    func_m049_00878(cx, cy, -x, -y);
    func_m049_00878(cx, cy, -y, -x);
    func_m049_00878(cx, cy, -y, x);
    func_m049_00878(cx, cy, -x, y);
}

/* Widens a row's span to include a point */
static void func_m049_00878(s32 x, s32 y, s32 dx, s32 dy) {
    u8 *span;

    x = x + dx + sHalfWidth;
    y = y + dy + sHalfHeight;
    if (x < 0) {
        x = 0;
    } else if (x >= sWidth) {
        x = sWidth - 1;
    }
    if (y < 0) {
        y = 0;
    } else if (y >= sHeight) {
        y = sHeight - 1;
    }
    x |= 0x80;
    span = sSpans[y];
    if (!(span[0] & 0x80) && !(span[1] & 0x80)) {
        span[0] = x;
    } else if ((span[0] & 0x80) && !(span[1] & 0x80)) {
        if (x < span[0]) {
            span[1] = span[0];
            span[0] = x;
        } else {
            span[1] = x;
        }
    } else if ((span[0] & 0x80) && (span[1] & 0x80)) {
        if (x < span[0]) {
            span[0] = x;
        } else if (span[1] < x) {
            span[1] = x;
        }
    }
}

/* Fills the spans into the texture */
static void func_m049_009A8(void) {
    u8 *row = sPixels;
    s32 y;
    u8 *span;
    s32 x;

    for (y = 0, span = sSpans[0]; y < sHeight; y++, span += 2) {
        if (span[0] & 0x80) {
            if (span[1] & 0x80) {
                for (x = span[0] & ~0x80; x <= (span[1] & ~0x80); x++) {
                    if (x & 1) {
                        row[x >> 1] |= 8;
                    } else {
                        row[x >> 1] |= 0x80;
                    }
                }
            }
        }
        row += sWidth >> 1;
    }
}

/* Blurs the texture (each byte holds two 4-bit pixels) */
static void func_m049_00A84(void) {
    u8 *p = sPixels;
    s32 y = sHeight - 2;
    s32 x;
    s32 sum;

    while (y--) {
        for (x = 0; x < (sWidth >> 1) - 1; x++, p++) {
            sum = (p[0] >> 4) + (p[0] & 0xF) + (p[1] >> 4) + (p[0x20] >> 4) + (p[0x21] >> 4) + (p[0x40] >> 4) +
                  (p[0x40] & 0xF) + (p[0x41] >> 4);
            p[0x20] &= 0xF0;
            p[0x20] |= (sum >> 3) & 0xF;
            p[0x21] &= 0xF;
            p[0x21] |= (((p[0] & 0xF) + (p[1] >> 4) + (p[1] & 0xF) + (p[0x20] & 0xF) + (p[0x21] & 0xF) +
                         (p[0x40] & 0xF) + (p[0x41] >> 4) + (p[0x41] & 0xF))
                        << 1) &
                       0xF0;
        }
        p++;
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m049", func_m049_00204);

INCLUDE_ASM("asm/nonmatchings/modules/m049", func_m049_00668);

INCLUDE_ASM("asm/nonmatchings/modules/m049", func_m049_0069C);

INCLUDE_ASM("asm/nonmatchings/modules/m049", func_m049_0078C);

INCLUDE_ASM("asm/nonmatchings/modules/m049", func_m049_00878);

INCLUDE_ASM("asm/nonmatchings/modules/m049", func_m049_009A8);

INCLUDE_ASM("asm/nonmatchings/modules/m049", func_m049_00A84);

INCLUDE_ASM("asm/nonmatchings/modules/m049", data);

INCLUDE_ASM("asm/nonmatchings/modules/m049", rodata);

INCLUDE_ASM("asm/nonmatchings/modules/m049", bss);
#endif
