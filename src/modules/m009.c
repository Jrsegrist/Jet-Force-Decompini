/*
 * Runlink module 9: a front-end map screen (still mostly assembly).
 */
#include "module.h"

extern void menu_assetgroup_free(void *images); /* frees a list of loaded images */

extern void menu_assetgroup_load(void *images); /* loads a list of images */
extern void menu_assetgroup_reset(void *images);
extern void func_8005342C(s32 module); /* unloads a runlink module */
extern void func_m005_00010(void);
extern void rl_m009_03D0C(void *icon);
extern s32 D_800FECCC;

/* A world's icon on the map (size 0x30) */
typedef struct M009Icon {
    /* 0x00 */ u8 pad0[0x30];
} M009Icon;

/* The module's .data (still assembly) */
extern u8 rl_m009_043F8[];
extern u8 rl_m009_04400[]; /* the map's icons */
extern u8 rl_m009_04424[];
extern M009Icon rl_m009_0448C[11];
extern s32 rl_m009_04908;

/* The module's .bss (still assembly) */
extern s32 rl_m009_097B0;
extern s32 rl_m009_097B4;

INCLUDE_ASM("asm/nonmatchings/modules/m009", func_m009_00000);

void func_m009_003A8(void) {
    func_m005_00010();
    if (rl_m009_097B0) {
        func_8005342C(5);
    }
    menu_assetgroup_free(rl_m009_043F8);
    menu_assetgroup_free(rl_m009_04400);
    D_800FECCC = 0;
    rl_m009_04908 = 0;
}

void func_m009_00408(void) {
    s32 i;
    M009Icon *icon;

    menu_assetgroup_load(rl_m009_04400);
    menu_assetgroup_reset(rl_m009_04424);
    rl_m009_097B4 = 0;
    icon = rl_m009_0448C;
    for (i = 0; i < 11; i++, icon++) {
        rl_m009_03D0C(icon);
    }
}

/* A menu item (D_800FF780, size 0x20), as used here */
typedef struct M009MenuItem {
    /* 0x00 */ s16 x;
    /* 0x02 */ s16 y;
    /* 0x04 */ s16 z;
    /* 0x06 */ s16 object; /* its D_800FF3F8 object */
    /* 0x08 */ f32 scale;
    /* 0x0C */ f32 posX;
    /* 0x10 */ f32 posY;
    /* 0x14 */ u8 pad14[0x1C - 0x14];
    /* 0x1C */ s8 model;
    /* 0x1D */ u8 pad1D[0x20 - 0x1D];
} M009MenuItem;

/* A place on the map (size 0xC; a kind of 0xFF ends the list) */
typedef struct M009Place {
    /* 0x0 */ u8 kind;  /* 0: a world model, else a menu item */
    /* 0x1 */ u8 index;
    /* 0x2 */ u8 world;
    /* 0x3 */ u8 pad3[0xC - 0x3];
} M009Place;

/* A world model drawn on the map (size 0x4E0) */
typedef struct M009Model {
    /* 0x000 */ u8 pad0[0x4CC];
    /* 0x4CC */ s16 x;
    /* 0x4CE */ s16 y;
    /* 0x4D0 */ u8 pad4D0[0x4E0 - 0x4D0];
} M009Model;

extern M009MenuItem D_800FF780[];
extern Gfx *D_800FF398;
extern void *D_800FF39C;
extern void func_80040D64(Gfx **dl, void *mtx);
extern void rl_m009_02978(M009Model *model, s32 updateRate);
extern void rl_m009_030E8(M009Model *model);
static void func_m009_03908(s32 index);
extern M009Place rl_m009_0469C[]; /* (.data) */
extern M009Model rl_m009_04990[]; /* (.bss) */
extern f32 rl_m009_09790;
extern f32 rl_m009_09794;

/* Draws the icon of a world on the map at (x, y) */
void func_m009_00478(s32 world, s32 x, s32 y, s32 updateRate) {
    M009Place *p;
    s32 found;
    M009Model *model;

    rl_m009_09790 = 0.0f;
    p = rl_m009_0469C;
    rl_m009_09794 = 0.0f;
    found = 0;
    if (p->kind != 0xFF) {
        do {
            if (world == p->world) {
                found = 1;
                if (p->kind == 0) {
                    model = &rl_m009_04990[p->index];
                    model->x = x;
                    model->y = y;
                    func_80040D64(&D_800FF398, &D_800FF39C);
                    rl_m009_02978(model, updateRate);
                    rl_m009_030E8(model);
                } else {
                    D_800FF780[p->index].posX = x;
                    D_800FF780[p->index].posY = y;
                    D_800FF780[16].x += updateRate * 256;
                    D_800FF780[17].x += updateRate << 7;
                    D_800FF780[44].x += updateRate << 7;
                    D_800FF780[18].x -= updateRate * 160;
                    D_800FF780[14].x += updateRate << 8;
                    D_800FF780[20].x += updateRate << 8;
                    D_800FF780[43].x += updateRate << 8;
                    D_800FF780[43].y += updateRate << 5;
                    D_800FF780[15].x += updateRate << 8;
                    func_m009_03908(p->index);
                }
            }
            p++;
        } while (p->kind != 0xFF && !found);
    }
}

void func_m009_0066C(void) {
    menu_assetgroup_free(rl_m009_04400);
}

INCLUDE_ASM("asm/nonmatchings/modules/m009", func_m009_00690);

extern s32 rl_m009_0490C; /* .data (still assembly): the place picked */
extern f32 rl_m009_097A0; /* .bss (still assembly): where the map heads for */
extern f32 rl_m009_097A4;
extern s32 rl_m009_097A8; /* the time it takes */

/* Picks map place `index`: the map heads for its world model or menu item */
void func_m009_02880(s32 index) {
    M009Place *p;

    rl_m009_0490C = index;
    p = &rl_m009_0469C[index];
    if (p->kind == 0) {
        rl_m009_097A0 = rl_m009_04990[p->index].x;
        rl_m009_097A4 = rl_m009_04990[p->index].y;
    } else {
        rl_m009_097A0 = D_800FF780[p->index].posX;
        rl_m009_097A4 = D_800FF780[p->index].posY;
    }
    if (rl_m009_097A8 == 0) {
        rl_m009_097A8 = 15;
    }
}

INCLUDE_ASM("asm/nonmatchings/modules/m009", func_m009_02978);

INCLUDE_ASM("asm/nonmatchings/modules/m009", func_m009_02FB0);

extern void func_80042134(MtxF *mtx);                          /* identity */
extern void func_80049874(f32 x, f32 y, f32 z, MtxF *mtx);     /* scale */
extern void func_800498E8(f32 x, f32 y, f32 z, MtxF *mtx);     /* translation */
extern void func_80048D84(MtxF *mtx, void *out);               /* to a fixed-point Mtx */
extern void func_80055C00(Gfx **dl, void *tex, s32 flags, s32 arg3); /* loads a texture */

#ifdef NON_MATCHING
/* Only the triangle commands' stores are scheduled differently (16 words) */
/*
 * Draws a world model on the map: its body and, if it has one, its glowing layer, shrunk as its timer runs.
 * (Commands: the matrix, 25 vertices, 16 + 7 triangles; then pipe sync and the glow's prim colour.)
 */
static void func_m009_030E8(M009Model *model) {
    u32 *glow = *(u32 **) ((u8 *) model + 0x4C8);
    MtxF mtx;
    f32 scale = 1.0f - (*(s16 *) ((u8 *) model + 0x4D0) * 0.5f) / 60.0f;
    Gfx *g;

    func_80042134(&mtx);
    func_80049874(scale, scale, scale, &mtx);
    func_800498E8(model->x - rl_m009_09790, 120.0f - (model->y - rl_m009_09794), 0.0f, &mtx);
    func_80048D84(&mtx, D_800FF39C);
    g = D_800FF398++;
    g->words.w0 = 0x01000040;
    g->words.w1 = (u32) D_800FF39C + 0x80000000;
    D_800FF39C = (u8 *) D_800FF39C + 0x40;
    {
        Gfx *_g = D_800FF398++;
        u32 v = (u32) model + 0x80000170;

        _g->words.w0 = ((((v & 6) | 0xC0) & 0xFF) << 16) | 0x04000000 | 0xF8;
        _g->words.w1 = v;
    }
    func_80055C00(&D_800FF398, *(void **) ((u8 *) model + 0x4C0), 1, 0);
    {
        Gfx *_g = D_800FF398++;
        _g->words.w0 = 0x05F10100;
        _g->words.w1 = (u32) model + 0x80000000;
    }
    {
        Gfx *_g = D_800FF398++;
        _g->words.w0 = 0x05610070;
        _g->words.w1 = (u32) model + 0x80000100;
    }
    if (*(void **) ((u8 *) model + 0x4C4) != NULL) {
        func_80042134(&mtx);
        func_80049874(scale, scale, scale, &mtx);
        func_800498E8(model->x - rl_m009_09790, 120.0f - (model->y - rl_m009_09794), 0.0f, &mtx);
        func_80048D84(&mtx, D_800FF39C);
        g = D_800FF398++;
        g->words.w0 = 0x01000040;
        g->words.w1 = (u32) D_800FF39C + 0x80000000;
        D_800FF39C = (u8 *) D_800FF39C + 0x40;
        {
            Gfx *_g = D_800FF398++;
            u32 v = (u32) model + 0x800003D0;

            _g->words.w0 = ((((v & 6) | 0xC0) & 0xFF) << 16) | 0x04000000 | 0xF8;
            _g->words.w1 = v;
        }
        {
            Gfx *_g = D_800FF398++;
            _g->words.w0 = 0xE7000000;
            _g->words.w1 = 0;
        }
        func_80055C00(&D_800FF398, *(void **) ((u8 *) model + 0x4C4), 5, 0);
        g = D_800FF398++;
        g->words.w0 = 0xFA000000;
        g->words.w1 = glow[5];
        {
            Gfx *_g = D_800FF398++;
            _g->words.w0 = 0x05F10100;
            _g->words.w1 = (u32) model + 0x80000260;
        }
        {
            Gfx *_g = D_800FF398++;
            _g->words.w0 = 0x05610070;
            _g->words.w1 = (u32) model + 0x80000360;
        }
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m009", func_m009_030E8);
#endif

INCLUDE_ASM("asm/nonmatchings/modules/m009", func_m009_0343C);

/* A texture of a model, as 037E4 uses it (m135's 02834 is the same function) */
typedef struct M009Texture {
    /* 0x00 */ u8 pad0[0x12];
    /* 0x12 */ u16 frames; /* times 256 */
    /* 0x14 */ u8 pad14[2];
    /* 0x16 */ s16 frameSize;
} M009Texture;

/* A model's texture table entry (8 bytes) */
typedef struct M009ModelTexture {
    /* 0x0 */ M009Texture *texture;
    /* 0x4 */ u8 pad4[4];
} M009ModelTexture;

typedef struct M009Model3D {
    /* 0x00 */ u8 pad0[0x18];
    /* 0x18 */ M009ModelTexture *textures;
    /* 0x1C */ u8 pad1C[0x2C - 0x1C];
    /* 0x2C */ u8 batchCount;
} M009Model3D;

/* A batch's texture animation (8 bytes) */
typedef struct M009TexAnim {
    /* 0x0 */ s16 frame;
    /* 0x2 */ s16 frame2; /* < 0: none */
    /* 0x4 */ u32 flags;  /* the low byte: the texture; 0x10000: set by 037E4 */
} M009TexAnim;

typedef struct M009Instance {
    /* 0x00 */ M009Model3D *model;
    /* 0x04 */ u8 pad4[0xB - 0x4];
    /* 0x0B */ s8 buffer;
    /* 0x0C */ u8 padC[0x74 - 0xC];
    /* 0x74 */ M009TexAnim *texAnims;
    /* 0x78 */ s16 *texOffsets[2];
} M009Instance;

/* Sets the frames of the object's animated textures `frames` long to `frame` (the second one to the
   next frame), and writes each batch's texture offsets */
void func_m009_037E4(Object *obj, s32 frame, s32 frames) {
    M009Instance *inst = obj->modelInstances[obj->modelIndex];
    M009Model3D *model = inst->model;
    M009TexAnim *a = inst->texAnims;
    M009Texture *tex;
    s16 *out;
    s32 i;
    s32 idx;

    frames <<= 8;
    if (a == NULL) {
        return;
    }
    out = inst->texOffsets[inst->buffer];
    for (i = 0; i < model->batchCount; i++) {
        idx = a->flags & 0xFF;
        tex = model->textures[idx].texture;
        if ((a->flags & 0x10000) && frames == tex->frames) {
            a->frame = frame;
            if (a->frame2 >= 0) {
                frame += 0x100;
                if (frame >= tex->frames) {
                    frame -= 0x100;
                }
                a->frame2 = frame;
            }
        }
        if (a->frame2 >= 0) {
            *out++ = (a->frame2 >> 8) * tex->frameSize;
        }
        *out++ = (a->frame >> 8) * tex->frameSize;
        a++;
    }
}

extern Object *D_800FF3F8[]; /* the menu's objects */
extern void *D_800FF3A0;
extern void func_800221F0(Object *obj, void *model, void *shading, s32 arg3);
extern void func_8000DB94(Gfx **dl, void *mtx, void *vtx, Object *obj);

/* Draws menu item `index`'s object at the item's place on the map */
static void func_m009_03908(s32 index) {
    M009MenuItem *item = &D_800FF780[index];
    Object *obj = D_800FF3F8[item->object];

    if (item->object == 0x7F) {
        func_800221F0(obj, obj->modelInstances[0], obj->shading, 0);
    }
    obj->trans.x_position = item->posX - rl_m009_09790;
    obj->trans.y_position = 120.0f - (item->posY - rl_m009_09794);
    obj->trans.rotation.y_rotation = item->x;
    obj->trans.rotation.x_rotation = item->y;
    obj->trans.rotation.z_rotation = item->z;
    obj->modelIndex = item->model;
    obj->trans.scale = item->scale;
    func_8000DB94(&D_800FF398, &D_800FF39C, &D_800FF3A0, obj);
}

INCLUDE_ASM("asm/nonmatchings/modules/m009", func_m009_039F0);

INCLUDE_ASM("asm/nonmatchings/modules/m009", func_m009_03D0C);

INCLUDE_ASM("asm/nonmatchings/modules/m009", data);

INCLUDE_ASM("asm/nonmatchings/modules/m009", rodata);

INCLUDE_ASM("asm/nonmatchings/modules/m009", bss);

