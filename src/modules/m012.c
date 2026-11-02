/*
 * Runlink module 12: the frame of the in-game menus. It opens by growing a
 * 3D frame from a line to its full size (a spinning bar, then the width,
 * then the height), fades the background behind it, and closes the same way
 * in reverse; while it is open it loads the menu textures, and once it has
 * closed it frees them and can start a level change.
 */
#include "module.h"
#include "gfx.h"

/* The frame's state */
typedef struct M012Frame {
    /* 0x00 */ u32 flags; /* 1: open, 2: restore the music, 4: active */
    /* 0x04 */ s32 width;
    /* 0x08 */ s32 height;
    /* 0x0C */ s32 targetWidth;
    /* 0x10 */ s32 targetHeight;
    /* 0x14 */ s16 angle;
    /* 0x16 */ u8 pad16[2];
    /* 0x18 */ f32 distance;
    /* 0x1C */ u8 pad1C[0x28 - 0x1C];
    /* 0x28 */ s16 unk28;
    /* 0x2A */ u8 alpha;
    /* 0x2B */ u8 unk2B;
    /* 0x2C */ u8 texturesFreed;
    /* 0x2D */ s8 fade;
    /* 0x2E */ u8 soundPlayed;
    /* 0x2F */ u8 pad2F;
    /* 0x30 */ Vertex *vertices;
} M012Frame;

/* A texture the frame keeps loaded while it is open */
typedef struct M012Texture {
    /* 0x0 */ s32 unk0;
    /* 0x4 */ s32 unk4;
} M012Texture;

extern void func_80059E1C(s16 *assets); /* loads menu textures */
extern void func_80059CCC(s16 *assets); /* frees them */
extern void mempool_free_queue_flush(void);
extern void func_8005414C(void);
extern void load_texture_at(s32 arg0, s32 arg1);
extern void func_8004AA80(void *data); /* frees */
extern void func_8003F468(void);
extern void func_80004C64(s32 arg0);

static s16 sFrameVertices[][3] = {
    { -44, 0, 4 },  { -44, 8, 4 },  { -40, 0, 4 },  { -40, 12, 4 },  { -32, 0, 4 },  { -32, 12, 4 }, { 0, 0, 4 },
    { 0, 12, 4 },   { 8, 0, 4 },    { 8, 12, 4 },   { 32, 0, 4 },    { 32, 12, 4 },  { 40, 0, 4 },   { 40, 12, 4 },
    { 44, 0, 4 },   { 44, 8, 4 },   { -44, 0, -4 }, { -44, 8, -4 },  { -40, 12, -4 }, { 44, 0, -4 },
};
static Triangle sTriangles1998[] = {
    { 0, 0, 1, 0x11, { { 0, 0 }, { 0, 0 }, { 0, 0 } } },
    { 0, 0, 0x11, 0x10, { { 0, 0 }, { 0, 0 }, { 0, 0 } } },
    { 0, 1, 3, 0x12, { { 0, 0 }, { 0, 0 }, { 0, 0 } } },
    { 0, 1, 0x12, 0x11, { { 0, 0 }, { 0, 0 }, { 0, 0 } } },
    { 0, 0, 3, 1, { { 0, 0x120 }, { 0, 0x1A0 }, { 0, 0x1A0 } } },
    { 0, 0, 2, 3, { { 0, 0x120 }, { 0, 0x120 }, { 0, 0x1A0 } } },
    { 0, 0xE, 0xF, 0xD, { { 0, 0x120 }, { 0, 0x1A0 }, { 0, 0x1A0 } } },
    { 0, 0xE, 0xD, 0xC, { { 0, 0x120 }, { 0, 0x1A0 }, { 0, 0x120 } } },
    { 0, 0, 0x10, 0x13, { { 0, 0 }, { 0, 0 }, { 0, 0 } } },
    { 0, 0, 0x13, 0xE, { { 0, 0 }, { 0, 0 }, { 0, 0 } } },
};
static Triangle sTriangles1A38[] = {
    { 0, 2, 7, 3, { { 0, 0x120 }, { 0x7E0, 0x340 }, { 0, 0x340 } } },
    { 0, 2, 6, 7, { { 0, 0x120 }, { 0x7E0, 0x120 }, { 0x7E0, 0x340 } } },
};
static Triangle sTriangles1A58[] = {
    { 0, 6, 9, 7, { { 0, 0x120 }, { 0x1E0, 0x340 }, { 0, 0x340 } } },
    { 0, 6, 8, 9, { { 0, 0x120 }, { 0x1E0, 0x120 }, { 0x1E0, 0x340 } } },
    { 0, 0xA, 0xD, 0xB, { { 0x1E0, 0x120 }, { 0, 0x340 }, { 0x1E0, 0x340 } } },
    { 0, 0xA, 0xC, 0xD, { { 0x1E0, 0x120 }, { 0, 0x120 }, { 0, 0x340 } } },
};
static Triangle sTriangles1A98[] = {
    { 0, 0xB, 9, 8, { { 0x3E0, 0x340 }, { 0, 0x340 }, { 0, 0x120 } } },
    { 0, 8, 0xA, 0xB, { { 0, 0x120 }, { 0x3E0, 0x120 }, { 0x3E0, 0x340 } } },
};
static Triangle sTriangles1AB8[] = {
    { 0, 2, 5, 3, { { 0, 0x120 }, { 0x1E0, 0x340 }, { 0, 0x340 } } },
    { 0, 2, 4, 5, { { 0, 0x120 }, { 0x1E0, 0x120 }, { 0x1E0, 0x340 } } },
    { 0, 0xA, 0xD, 0xB, { { 0x1E0, 0x120 }, { 0, 0x340 }, { 0x1E0, 0x340 } } },
    { 0, 0xA, 0xC, 0xD, { { 0x1E0, 0x120 }, { 0, 0x120 }, { 0, 0x340 } } },
};
static Triangle sTriangles1AF8[] = {
    { 0, 4, 0xB, 5, { { 0, 0x120 }, { 0x3E0, 0x340 }, { 0, 0x340 } } },
    { 0, 4, 0xA, 0xB, { { 0, 0x120 }, { 0x3E0, 0x120 }, { 0x3E0, 0x340 } } },
};
static Gfx sDisplayList1B18[] = {
    { { 0xE7000000, 0x00000000 } }, { { 0xBA001001, 0x00000000 } }, { { 0xBA000E02, 0x00000000 } },
    { { 0xB9000002, 0x00000000 } }, { { 0xBA001301, 0x00000000 } }, { { 0xBA000C02, 0x00000000 } },
    { { 0xB6000000, 0x00010001 } }, { { 0xE7000000, 0x00000000 } }, { { 0xB8000000, 0x00000000 } },
};
static Gfx sDisplayList1B60[] = {
    { { 0xFC119623, 0xFF2FFFFF } },
    { { 0xEF000C0F, 0x00504340 } },
};
static Gfx sDisplayList1B70[] = {
    { { 0xFC71FEE3, 0xFFFEFB7D } },
    { { 0xEF000C0F, 0x00504340 } },
};
static s16 sAssets[] = { 0x35, 0x36, 0x37, 0x38, 0x34, -1 };
/* The modules to unload when the frame opens */
static u8 sModules[] = { 1, 2, 3, 7, 8, 0xD, 0xE, 0xF, 0x10, 0x11, 0xA, 4, 0x1C, 0xFF };
static u8 sLoaded = 0;
static s16 sTextureList[] = { 0xCB, 0xCC, 0xCD, 0xCE, 0xCF, 0xD0, 0xD1, 0xD2, 0xD3, 0xD4,
                              0xD5, 0xD6, 0xD7, 0x300, 0x17D, 0xC4, 0x123, 0x135, -1 };

static M012Frame sFrame;
/* The level change to start when the frame has closed (-1: none) */
static s32 sNewLevel;
static s32 sNewLevelArg1;
static s32 sNewLevelArg2;
static s32 sNewLevelArg3;
s32 D_m012_01C24;
static s32 sEnabled;
static s32 sTextureCount;
static M012Texture *sTextures;

static void free_textures(void);

void func_m012_00000(void) {
    if (sLoaded) {
        func_80059CCC(sAssets);
    }
    sLoaded = 1;
    sFrame.width = 10;
    sFrame.height = 0;
    sFrame.unk28 = 0;
    sFrame.unk2B = 0;
    sFrame.alpha = 0;
    sFrame.texturesFreed = 1;
    sFrame.fade = 4;
    sFrame.distance = 230.0f;
    func_80059E1C(sAssets);
    sEnabled = 1;
    sNewLevel = -1;
}

void func_m012_00094(void) {
    if (sLoaded) {
        if (sFrame.texturesFreed == 0) {
            free_textures();
        }
        func_80059CCC(sAssets);
    }
    sLoaded = 0;
}

void func_m012_000E4(s32 width, s32 height) {
    sFrame.targetWidth = width;
    sFrame.targetHeight = height;
    sFrame.flags |= 5;
    sFrame.soundPlayed = 0;
}

#ifdef NON_MATCHING
/* IDO materialises the struct's address for the read-modify-write; the original used %lo twice */
void func_m012_00108(s32 flags) {
    func_80001248(0x172, 0);
    sFrame.flags = (sFrame.flags & ~1) | flags;
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m012", func_m012_00108);
#endif

void func_m012_0014C(s32 fade) {
    sFrame.fade = fade;
}

void func_m012_00158(s32 enabled) {
    sEnabled = enabled;
}

INCLUDE_ASM("asm/nonmatchings/modules/m012", func_m012_00164);

INCLUDE_ASM("asm/nonmatchings/modules/m012", func_m012_008A8);

static void free_textures(void) {
    s32 i;

    mempool_free_queue_flush();
    func_8005414C();
    for (i = 0; i < sTextureCount; i++) {
        load_texture_at(sTextures[i].unk0, sTextures[i].unk4);
    }
    func_8004AA80(sTextures);
    func_8003F468();
    sFrame.texturesFreed = 1;
    func_80004C64(0);
}

INCLUDE_ASM("asm/nonmatchings/modules/m012", func_m012_00A64);

INCLUDE_ASM("asm/nonmatchings/modules/m012", func_m012_01018);

INCLUDE_ASM("asm/nonmatchings/modules/m012", func_m012_01444);

u8 func_m012_01820(void) {
    return sFrame.alpha;
}

#ifdef NON_MATCHING
/* the struct's address and the result swap v0 and v1 */
s32 func_m012_01830(void) {
    return sFrame.width == sFrame.targetWidth && sFrame.height == sFrame.targetHeight;
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m012", func_m012_01830);
#endif

u8 func_m012_01870(void) {
    return sFrame.texturesFreed;
}

INCLUDE_ASM("asm/nonmatchings/modules/m012", func_m012_01880);

void func_m012_018F4(s32 level, s32 arg1, s32 arg2, s32 arg3) {
    sNewLevel = level;
    sNewLevelArg1 = arg1;
    sNewLevelArg2 = arg2;
    sNewLevelArg3 = arg3;
}

INCLUDE_ASM("asm/nonmatchings/modules/m012", rodata);
