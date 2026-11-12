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

/* The level model's textures (func_8001B650; DKR's LevelModel, reduced) */
typedef struct LevelModelTexture {
    /* 0x0 */ void *texture;
    /* 0x4 */ u8 pad4[4];
} LevelModelTexture;
typedef struct LevelModelTextures {
    /* 0x00 */ LevelModelTexture *textures;
    /* 0x04 */ u8 pad4[0x18 - 0x4];
    /* 0x18 */ s16 count; /* DKR numberOfTextures */
} LevelModelTextures;

/* A texture the frame keeps loaded while it is open */
typedef struct M012Texture {
    /* 0x0 */ s32 unk0;
    /* 0x4 */ s32 unk4;
} M012Texture;

extern void menu_assetgroup_load(s16 *assets); /* loads menu textures */
extern void menu_assetgroup_free(s16 *assets); /* frees them */
extern void mempool_free_queue_flush(void);
extern void runlink_slots_unload_all(void);
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
        menu_assetgroup_free(sAssets);
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
    menu_assetgroup_load(sAssets);
    sEnabled = 1;
    sNewLevel = -1;
}

void func_m012_00094(void) {
    if (sLoaded) {
        if (sFrame.texturesFreed == 0) {
            free_textures();
        }
        menu_assetgroup_free(sAssets);
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

INCLUDE_ASM("asm/nonmatchings/modules/m012/ro", func_m012_00164);

extern LevelModelTextures *func_8001B650(void); /* the level model */
extern void *func_8004A57C(s32 size, s32 colour); /* DKR mempool_alloc_safe */
extern s32 tex_find_id(void *texture);
extern void func_80055A00(void *texture);        /* frees a texture */
extern void model_cache_textures_release(s16 *keep);
static void func_m012_01880(void);

/* Keeps the level's textures (their ids) while the frame is open, and frees what the models no longer
   need (called from 00164) */
void func_m012_008A8(void) {
    LevelModelTextures *model = func_8001B650();
    M012Texture *t;
    s32 i;

    sTextureCount = model->count;
    sTextures = func_8004A57C(sTextureCount * sizeof(M012Texture), -1);
    t = sTextures;
    for (i = 0; i < model->count; i++) {
        t->unk4 = (s32) model->textures[i].texture;
        t->unk0 = tex_find_id(model->textures[i].texture);
        func_80055A00(model->textures[i].texture);
        t++;
    }
    model_cache_textures_release(sTextureList);
    sFrame.texturesFreed = FALSE;
    func_m012_01880();
    func_80004C64(1);
}

static void free_textures(void) {
    s32 i;

    mempool_free_queue_flush();
    runlink_slots_unload_all();
    for (i = 0; i < sTextureCount; i++) {
        load_texture_at(sTextures[i].unk0, sTextures[i].unk4);
    }
    func_8004AA80(sTextures);
    func_8003F468();
    sFrame.texturesFreed = 1;
    func_80004C64(0);
}

/* A menu image's texture (4E610.c's RectTexture) */
typedef struct M012Image {
    /* 0x00 */ u8 pad0[0xA];
    /* 0x0A */ s16 numberOfCommands;
    /* 0x0C */ Gfx *cmd;
} M012Image;

extern Gfx *D_800FF398;
extern Mtx *D_800FF39C;
extern Vertex *D_800FF3A0;
extern M012Image *D_800FF3F8[]; /* the loaded menu images */
extern void set_active_camera(s32 camera);            /* set_active_camera */
extern f32 func_8003F7C4(void);                   /* the field of view */
extern void func_8003F7F4(f32 fov, s32 arg1);     /* sets the field of view */
extern void viewport_apply(Gfx **dl, s32 arg1);    /* viewport_apply */
extern void func_80040FE4(Gfx **dl);              /* viewport_reset */
extern void func_80040E64(Gfx **dl, Mtx **mtx);   /* mtx_perspective */
extern void func_80055B80(Gfx **dl);
extern void func_80055C00(Gfx **dl, void *tex, s32 flags, s32 arg3); /* loads a texture */
extern void *func_80055B40(M012Image *tex, s32 frame); /* set_animated_texture_header */
extern void func_m006_0107C(s32 *x, s32 *y);
extern void rl_m012_01018(void);
extern void rl_m012_01444(s32 distance, s32 y, s16 angle, s32 side);

/* DKR include/f3ddkr.h: G_DMADL (7) runs numberOfCommands commands from address (the length as `<< 3`: IDO
   folds `(n - 1) * 8` into `n * 8 - 8`, the ROM shifts n - 1) */
#define gDkrDmaDisplayList(pkt, address, numberOfCommands)                                           \
    {                                                                                                \
        Gfx *_g = (Gfx *) (pkt);                                                                     \
        _g->words.w0 = (_SHIFTL(0x07, 24, 8) | _SHIFTL((numberOfCommands), 16, 8) |                 \
                        _SHIFTL(((numberOfCommands) << 3), 0, 16));                                  \
        _g->words.w1 = (unsigned int) (address);                                                     \
    }

/* The flipped texture rectangle in the older microcode's form (RDPHALF_2, RDPHALF_CONT) */
#define gSPTextureRectangleFlipOld(pkt, xl, yl, xh, yh, tile, s, t, dsdx, dtdy)                    \
    {                                                                                             \
        Gfx *_g = (Gfx *) (pkt);                                                                  \
                                                                                                  \
        _g->words.w0 = (_SHIFTL(G_TEXRECTFLIP, 24, 8) | _SHIFTL(xh, 12, 12) | _SHIFTL(yh, 0, 12)); \
        _g->words.w1 = (_SHIFTL(tile, 24, 3) | _SHIFTL(xl, 12, 12) | _SHIFTL(yl, 0, 12));         \
        gImmp1(pkt, G_RDPHALF_2, (_SHIFTL(s, 16, 16) | _SHIFTL(t, 0, 16)));                       \
        gImmp1(pkt, G_RDPHALF_CONT, (_SHIFTL(dsdx, 16, 16) | _SHIFTL(dtdy, 0, 16)));              \
    }

/*
 * Draws the frame: its four sides, the background's fade behind it and, while it is narrow, the image
 * stretched over it
 */
void func_m012_00A64(void) {
    M012Image *tex;
    Gfx *cmd;
    s32 x1;
    s32 x2;
    s32 y1;
    s32 y2;
    M012Frame *frame; /* (set before the calls: the ROM addresses each field with its own lui) */
    f32 scale;
    s32 alpha;
    f32 fov;

    frame = &sFrame;
    if (sEnabled) {
        if (frame->flags & 4) {
            set_active_camera(0);
            fov = func_8003F7C4();
            func_8003F7F4(45.0f, 1);
            viewport_apply(&D_800FF398, 0);
            func_80040FE4(&D_800FF398);
            func_80040E64(&D_800FF398, &D_800FF39C);
            frame->vertices = D_800FF3A0;
            rl_m012_01018();
            rl_m012_01444(frame->distance, frame->height >> 1, frame->angle, 1);
            rl_m012_01444(frame->distance, frame->height >> 1, frame->angle + 0x8000, 1);
            rl_m012_01444(frame->distance, -(frame->height >> 1), frame->angle, 0);
            rl_m012_01444(frame->distance, -(frame->height >> 1), frame->angle + 0x8000, 0);
            x2 = frame->width >> 1;
            x1 = -x2;
            y1 = frame->height >> 1;
            y2 = -y1;
            func_m006_0107C(&x1, &y1);
            func_m006_0107C(&x2, &y2);
            y2++;
            y1--;
            if (x1 < 0) {
                x1 = 0;
            }
            if (x2 >= 320) {
                x2 = 319;
            }
            if (y1 < 0) {
                y1 = 0;
            }
            if (y1 >= 240) {
                y1 = 239;
            }
            if (frame->height > 0) {
                if (frame->alpha > 0) {
                    func_80055C00(&D_800FF398, NULL, 0, 0);
                    gDPSetEnvColor(D_800FF398++, 0, 0x28, 0x32, frame->alpha);
                    gDPSetPrimColor(D_800FF398++, 0, 0, 0, 5, 0x14, 0);
                    gDkrDmaDisplayList(D_800FF398++, (u8 *) sDisplayList1B70 + 0x80000000, 2);
                    gDPFillRectangle(D_800FF398++, x1, y1, x2, y2);
                }
                if (frame->width < 151) {
                    x1 *= 4;
                    y1 *= 4;
                    x2 *= 4;
                    y2 *= 4;
                    alpha = 255 - frame->width * 255 / 150;
                    gDPPipeSync(D_800FF398++);
                    gDPSetPrimColor(D_800FF398++, 0, 0, 255, 255, 255, alpha);
                    gSPDisplayList(D_800FF398++, sDisplayList1B18);
                    gDkrDmaDisplayList(D_800FF398++, (u8 *) sDisplayList1B60 + 0x80000000, 2);
                    tex = D_800FF3F8[52];
                    cmd = tex->cmd;
                    D_800FF398->words.w0 = cmd->words.w0;
                    D_800FF398->words.w1 = (u32) func_80055B40(tex, 0) + 0x80000000;
                    D_800FF398++;
                    cmd++;
                    gDkrDmaDisplayList(D_800FF398++, (u32) cmd + 0x80000000, tex->numberOfCommands - 1);
                    scale = 16.0f / frame->width;
                    scale *= 1024.0f;
                    gSPTextureRectangleFlipOld(D_800FF398++, x1, y1, x2, y2, 0, 0, 0, -1024, (s32) scale);
                }
            }
            func_80055B80(&D_800FF398);
            func_8003F7F4(fov, 1);
        }
    }
}

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

extern s32 runlink_module_loaded(s32 module);
extern void runlink_module_move_to_slot(s32 module);

/* Moves each of sModules that is loaded to its slot */
static void func_m012_01880(void) {
    u8 *module;

    for (module = sModules; *module != 0xFF; module++) {
        if (runlink_module_loaded(*module)) {
            runlink_module_move_to_slot(*module);
        }
    }
}

void func_m012_018F4(s32 level, s32 arg1, s32 arg2, s32 arg3) {
    sNewLevel = level;
    sNewLevelArg1 = arg1;
    sNewLevelArg2 = arg2;
    sNewLevelArg3 = arg3;
}

/* (.rodata: C, and the assembly functions' own from ro/) */
