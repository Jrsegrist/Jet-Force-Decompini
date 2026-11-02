/*
 * Runlink module 80: a small flapping object drawn from its own six vertices
 * and eight textured triangles (two vertex buffers, both set up from the
 * same corner positions). Init copies the triangles' vertex indices once and
 * masks their texture coordinates to the size of the chosen texture; control
 * (002A0, with its helper 01324) moves and animates it.
 */
#include "module.h"
#include "gfx.h"

/* What init gets */
typedef struct M080Setup {
    /* 0x00 */ u8 pad0[0xC];
    /* 0x0C */ u8 textureIndex;
    /* 0x0D */ u8 scale; /* in hundredths */
} M080Setup;

/* The first bytes of a texture */
typedef struct M080Texture {
    /* 0x0 */ u8 width;
    /* 0x1 */ u8 height;
} M080Texture;

/* What obj->unk68 points at */
typedef struct M080State {
    /* 0x000 */ Triangle tris[8];
    /* 0x080 */ Vertex verts[2][6];
    /* 0x0F8 */ M080Texture *texture;
    /* 0x0FC */ u8 unkFC;
    /* 0x0FD */ u8 unkFD;
    /* 0x0FE */ u8 unkFE;
    /* 0x100 */ s32 unk100;
    /* 0x104 */ s16 unk104;
    /* 0x106 */ s16 unk106;
    /* 0x108 */ f32 unk108;
    /* 0x10C */ f32 unk10C;
    /* 0x110 */ s32 unk110;
    /* 0x114 */ s32 unk114;
} M080State;

/* .data (asm): the triangles, then the six corner positions */
extern Triangle rl_m080_01490[8];
extern s16 rl_m080_01510[6][3];
/* .rodata (asm): 0.01f, the scale unit */
extern f32 rl_m080_01540[];

#ifdef NON_MATCHING
/* 36 diffs: pos/dest pointer registers in the corner loop, and the scale multiply's load order (0.01f read from the asm .rodata) */
void func_m080_00000(Object *obj, M080Setup *setup, s32 reinit) {
    M080State *s = obj->unk68;
    s32 i;
    s32 uMask;
    s32 vMask;
    f32 scale;
    s16 *pos;

    if (!reinit) {
        obj->velocity.y = 0.0f;
        s->unkFE = 0;
        s->unk100 = 0;
        s->unk104 = 0;
        s->unkFD = 0;
        s->unk106 = 0x180;
        s->unk108 = 0.0f;
        for (i = 0; i < 8; i++) {
            s->tris[i].flags = rl_m080_01490[i].flags;
            s->tris[i].vi0 = rl_m080_01490[i].vi0;
            s->tris[i].vi1 = rl_m080_01490[i].vi1;
            s->tris[i].vi2 = rl_m080_01490[i].vi2;
        }
        pos = rl_m080_01510[0];
        i = 0;
        while (TRUE) {
            s->verts[0][i].x = pos[0];
            s->verts[0][i].y = pos[1];
            s->verts[0][i].z = pos[2];
            s->verts[0][i].r = 0xFF;
            s->verts[0][i].g = 0xFF;
            s->verts[0][i].b = 0xFF;
            s->verts[0][i].a = 0xFF;
            s->verts[1][i].x = pos[0];
            s->verts[1][i].y = pos[1];
            s->verts[1][i].z = pos[2];
            s->verts[1][i].r = 0xFF;
            s->verts[1][i].g = 0xFF;
            s->verts[1][i].b = 0xFF;
            s->verts[1][i].a = 0xFF;
            pos += 3;
            if (++i == 6) {
                break;
            }
        }
        s->unkFC = 1;
    }
    uMask = 0;
    scale = setup->scale;
    obj->trans.scale = scale * rl_m080_01540[0];
    if (setup->textureIndex < obj->header->numberOfModelIds) {
        s->texture = obj->modelInstances[setup->textureIndex];
    } else {
        s->texture = obj->modelInstances[0];
    }
    if (s->texture != NULL) {
        uMask = (s->texture->width - 1) << 5;
        vMask = (s->texture->height - 1) << 5;
    } else {
        vMask = 0;
    }
    for (i = 0; i < 8; i++) {
        s->tris[i].uv[0][0] = rl_m080_01490[i].uv[0][0] & uMask;
        s->tris[i].uv[0][1] = rl_m080_01490[i].uv[0][1] & vMask;
        s->tris[i].uv[1][0] = rl_m080_01490[i].uv[1][0] & uMask;
        s->tris[i].uv[1][1] = rl_m080_01490[i].uv[1][1] & vMask;
        s->tris[i].uv[2][0] = rl_m080_01490[i].uv[2][0] & uMask;
        s->tris[i].uv[2][1] = rl_m080_01490[i].uv[2][1] & vMask;
    }
    s->unk10C = 1.0f;
    s->unk110 = 0x100;
    s->unk114 = 0x100;
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m080", func_m080_00000);
#endif

INCLUDE_ASM("asm/nonmatchings/modules/m080", func_m080_002A0);

INCLUDE_ASM("asm/nonmatchings/modules/m080", func_m080_01324);

INCLUDE_ASM("asm/nonmatchings/modules/m080", data);

INCLUDE_ASM("asm/nonmatchings/modules/m080", rodata);
