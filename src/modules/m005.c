/*
 * Runlink module 5: two fields of particles around the camera. The first is a
 * field of points spread through a box in front of the camera, which the
 * camera's movement scrolls through; each point is drawn as a one-pixel
 * rectangle, darker the further away it is. The second is a field of
 * coloured streaks spread through a box around the camera, moved by a wind
 * and by the camera's own movement, and drawn by main-segment code.
 */
#include "module.h"

typedef struct M005Point {
    /* 0x0 */ f32 x;
    /* 0x4 */ f32 y;
    /* 0x8 */ f32 z;
} M005Point;

/* A field's box, fade and movement. Size 0x48 */
typedef struct M005Field {
    /* 0x00 */ f32 minX;
    /* 0x04 */ f32 maxX;
    /* 0x08 */ f32 sizeX;
    /* 0x0C */ f32 minY;
    /* 0x10 */ f32 maxY;
    /* 0x14 */ f32 sizeY;
    /* 0x18 */ f32 minZ;
    /* 0x1C */ f32 maxZ;
    /* 0x20 */ f32 sizeZ;
    /* 0x24 */ f32 fade;
    /* 0x28 */ f32 fadeScale; /* 255 / fade */
    /* 0x2C */ s32 hasCamera; /* lastCamera is valid */
    /* 0x30 */ Vec3f vel;
    /* 0x3C */ Vec3f lastCamera;
} M005Field;

extern void *func_8004A57C(s32 size, s32 colour); /* DKR mempool_alloc_safe */
extern void func_8004AA80(void *data);            /* DKR mempool_free */
extern void func_80054EA0(u32 *width, u32 *height); /* the frame buffer size */
extern void func_80055B80(Gfx **dl);              /* resets the render settings */
extern ObjectTransform *func_80041DE0(void);      /* the camera */
/* moves the points by (dx, dy, dz), wrapping them round in the box */
extern void func_800234D0(s32 count, M005Point *points, f32 dx, f32 dy, f32 dz, f32 minX, f32 maxX, f32 sizeX,
                          f32 minY, f32 maxY, f32 sizeY, f32 minZ, f32 maxZ, f32 sizeZ);
/* draws the streaks */
extern void func_800237D8(s32 arg0, s32 arg1, s32 arg2, s32 count, M005Point *points, u32 *colours, s32 angle,
                          f32 velX, f32 velY, f32 velZ, f32 arg10);

static s32 sPointCount = 0;
static M005Point *sPoints = NULL;
static s32 sStreakCount = 0;
static s32 sStreaksShown = 1;
static M005Point *sStreaks = NULL;

/* The render settings for the points: flat primitive colour */
static Gfx sPointSetup[] = {
    gsDPPipeSync(),
    gsSPClearGeometryMode(G_ZBUFFER | G_FOG),
    gsDPSetCombineMode(G_CC_PRIMITIVE, G_CC_PRIMITIVE),
    gsDPSetOtherMode(0x000C0F, 0x0F0A4000),
    gsSPEndDisplayList(),
};

static M005Field sPointField;
static u16 *sPointColours; /* 256 shades (5551), after the points */
static s32 sUnusedBss;
static M005Field sStreakField;
static u32 *sStreakColours; /* one per streak, after the streaks */

void *func_m005_00000(void) {
    return sPoints;
}

void func_m005_00010(void) {
    if (sPoints != NULL) {
        func_8004AA80(sPoints);
        sPoints = NULL;
        sPointColours = NULL;
    }
}

void func_m005_0004C(s32 count, s32 sizeX, s32 sizeY, s32 sizeZ, u32 colourFrom, u32 colourTo, s32 fade) {
    M005Point *pt;
    u16 *colours;
    u32 i;
    s32 r;
    s32 g;
    s32 b;
    u32 dr;
    u32 dg;
    u32 db;

    sPoints = func_8004A57C(count * sizeof(M005Point) + 0x200, -1);
    sPointColours = (u16 *) ((u8 *) sPoints + count * sizeof(M005Point));
    sPointCount = count;
    sPointField.sizeX = sizeX;
    sPointField.minX = sPointField.sizeX * -0.5f;
    sPointField.maxX = sPointField.sizeX * 0.5f;
    sPointField.sizeY = sizeY;
    sPointField.minY = sPointField.sizeY * -0.5f;
    sPointField.maxY = sPointField.sizeY * 0.5f;
    sPointField.sizeZ = sizeZ;
    sPointField.minZ = 1.0f;
    sPointField.maxZ = sPointField.sizeZ + 1.0f;
    sPointField.fade = fade;
    sPointField.fadeScale = 255.0f / sPointField.fade;
    sPointField.hasCamera = 0;
    sizeX <<= 7;
    sizeY <<= 7;
    sizeZ = (sizeZ + 1) << 8;
    for (i = 0, pt = sPoints; (s32) i < sPointCount; i++) {
        pt->x = func_80048E2C(-sizeX, sizeX) * (1.0f / 256);
        pt->y = func_80048E2C(-sizeY, sizeY) * (1.0f / 256);
        pt->z = func_80048E2C(0x100, sizeZ) * (1.0f / 256);
        pt++;
    }
    r = (colourFrom >> 24) & 0xFF;
    g = (colourFrom >> 16) & 0xFF;
    b = (colourFrom >> 8) & 0xFF;
    dr = ((colourTo >> 24) & 0xFF) - r;
    dg = ((colourTo >> 16) & 0xFF) - g;
    db = ((colourTo >> 8) & 0xFF) - b;
    for (i = 0, colours = sPointColours; i < 256; i++) {
        *colours++ = (((((s32) (dr * i) >> 8) + r) & 0xF8) << 8) | (((((s32) (dg * i) >> 8) + g) & 0xF8) << 3) |
                     (((((s32) (db * i) >> 8) + b) & 0xF8) >> 2) | 1;
    }
}

void func_m005_00434(f32 velX, f32 velY, f32 velZ, s32 updateRate) {
    M005Field *f = &sPointField;

    f->vel.x = velX;
    f->vel.y = velY;
    f->vel.z = velZ;
    if (sPoints != NULL) {
        velX *= updateRate;
        velY *= updateRate;
        velZ *= updateRate;
        func_800234D0(sPointCount, sPoints, velX, velY, velZ, f->minX, f->maxX, f->sizeX, f->minY, f->maxY,
                      f->sizeY, f->minZ, f->maxZ, f->sizeZ);
    }
}

#ifdef NON_MATCHING
/* the frame size matches but width/height sit two words higher, and two constants swap registers */
void func_m005_0050C(Gfx **dl, f32 focal) {
    s32 unused1;
    s32 unused2;
    u32 width;
    u32 height;
    Gfx *gfx;
    M005Point *pt;
    s32 i;
    f32 z;
    f32 scale;
    s32 x;
    s32 y;
    s32 shade;

    func_80054EA0(&width, &height);
    gfx = *dl;
    i = sPointCount;
    pt = sPoints;
    gSPDisplayList(gfx++, sPointSetup);
    while (i--) {
        z = pt->z;
        if (z >= 8.0f && z < 300.0f) {
            scale = focal / z;
            x = (s32) (pt->x * scale) + (width >> 1);
            y = (height >> 1) - (s32) (pt->y * scale);
            if (x >= 0 && y >= 0 && x < (s32) width && y < (s32) height) {
                gfx->words.w0 = 0xFA000000;
                shade = 255 - (s32) ((z - 8.0f) * 0.8732877f);
                gfx->words.w1 = (shade << 24) | (shade << 16) | (shade << 8) | 0xFF;
                gfx++;
                gfx->words.w0 = 0xF6000000 | ((x + 1) << 14) | ((y + 1) << 2);
                gfx->words.w1 = (x << 14) | (y << 2);
                gfx++;
            }
        }
        pt++;
    }
    *dl = gfx;
    func_80055B80(dl);
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m005", func_m005_0050C);
#endif

void *func_m005_00708(void) {
    return sStreaks;
}

void func_m005_00718(void) {
    if (sStreaks != NULL) {
        func_8004AA80(sStreaks);
        sStreaks = NULL;
    }
}

#ifdef NON_MATCHING
/* the colour components are hoisted and spilled differently, and the frame is 0x30 bytes smaller */
void func_m005_0074C(s32 count, s32 sizeX, s32 sizeY, s32 sizeZ, u32 colourFrom, u32 colourTo, s32 fade) {
    M005Point *pt;
    u32 *colours;
    s32 i;
    s32 v;

    pt = func_8004A57C(count << 4, -1);
    sStreakField.sizeX = sizeX;
    sStreakField.minX = sStreakField.sizeX * -0.5f;
    sStreakField.maxX = sStreakField.sizeX * 0.5f;
    sStreakField.sizeY = sizeY;
    sStreakField.minY = sStreakField.sizeY * -0.5f;
    sStreakField.maxY = sStreakField.sizeY * 0.5f;
    sStreakField.sizeZ = sizeZ;
    sStreakField.minZ = sStreakField.sizeZ * -0.5f;
    sStreakField.maxZ = sStreakField.sizeZ * 0.5f;
    sStreakField.fade = fade;
    sStreakField.fadeScale = 255.0f / sStreakField.fade;
    colours = (u32 *) (pt + count);
    sStreaks = pt;
    sStreakColours = colours;
    sStreakCount = count;
    sStreakField.hasCamera = 0;
    sizeX <<= 7;
    sizeY <<= 7;
    sizeZ <<= 7;
    for (i = 0; i < sStreakCount; i++) {
        pt->x = func_80048E2C(-sizeX, sizeX) * (1.0f / 256);
        pt->y = func_80048E2C(-sizeY, sizeY) * (1.0f / 256);
        pt->z = func_80048E2C(-sizeZ, sizeZ) * (1.0f / 256);
        v = func_80048E2C(0, 0xFF);
        pt++;
        *colours++ = (((((s32) ((((colourTo >> 24) & 0xFF) - ((colourFrom >> 24) & 0xFF)) * v) >> 8) +
                        ((colourFrom >> 24) & 0xFF))
                       << 24) |
                      ((((s32) ((((colourTo >> 16) & 0xFF) - ((colourFrom >> 16) & 0xFF)) * v) >> 8) +
                        ((colourFrom >> 16) & 0xFF))
                       << 16) |
                      ((((s32) ((((colourTo >> 8) & 0xFF) - ((colourFrom >> 8) & 0xFF)) * v) >> 8) +
                        ((colourFrom >> 8) & 0xFF))
                       << 8) |
                      (((s32) (((colourTo & 0xFF) - (colourFrom & 0xFF)) * v) >> 8) + (colourFrom & 0xFF)));
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m005", func_m005_0074C);
#endif

void func_m005_00A58(f32 velX, f32 velY, f32 velZ, s32 updateRate) {
    M005Field *f = &sStreakField;
    ObjectTransform *cam;
    f32 dx;
    f32 dz;
    f32 dy;
    f32 inv;

    cam = func_80041DE0();
    if (f->hasCamera && updateRate != 0) {
        inv = 1.0f / updateRate;
        dx = (f->lastCamera.x - cam->x_position) * inv;
        dy = (f->lastCamera.y - cam->y_position) * inv;
        dz = (f->lastCamera.z - cam->z_position) * inv;
    } else {
        dx = 0.0f;
        dy = 0.0f;
        dz = 0.0f;
    }
    f->lastCamera.x = cam->x_position;
    f->lastCamera.y = cam->y_position;
    f->lastCamera.z = cam->z_position;
    f->hasCamera = 1;
    velX += dx;
    velY += dy;
    velZ += dz;
    f->vel.x = velX;
    f->vel.y = velY;
    f->vel.z = velZ;
    if (sStreaks != NULL) {
        velX *= updateRate;
        velY *= updateRate;
        velZ *= updateRate;
        func_800234D0(sStreakCount, sStreaks, velX, velY, velZ, f->minX, f->maxX, f->sizeX, f->minY, f->maxY,
                      f->sizeY, f->minZ, f->maxZ, f->sizeZ);
    }
}

void func_m005_00BF8(s32 shown) {
    sStreaksShown = shown;
}

void func_m005_00C04(void) {
    sStreakField.hasCamera = 0;
}

void func_m005_00C10(s32 arg0, s32 arg1, s32 arg2, f32 arg3, f32 density) {
    s32 count;
    M005Field *f = &sStreakField;

    if (sStreaksShown) {
        count = sStreakCount * density;
        if (sStreaks != NULL && count > 0) {
            func_800237D8(arg0, arg1, arg2, count, sStreaks, sStreakColours,
                          func_80041DE0()->rotation.y_rotation + 0x8000, f->vel.x, f->vel.y, f->vel.z, arg3);
        }
    }
}

#ifndef NON_MATCHING
INCLUDE_ASM("asm/nonmatchings/modules/m005", rodata);
#endif
