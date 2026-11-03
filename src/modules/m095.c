/*
 * Runlink module 95: a flame weapon. Init clears a user's weapon data and
 * keeps it in a list of up to four; the fire, update, limit and hit
 * functions are still in assembly; the draw function draws the flames as
 * sprites; the fini function removes the light of each player user and frees it.
 */
#include "module.h"

/* A flame (size 0x34) */
typedef struct M095Flame {
    /* 0x00 */ f32 x;
    /* 0x04 */ f32 y;
    /* 0x08 */ f32 z;
    /* 0x0C */ u8 padC[0x18 - 0xC];
    /* 0x18 */ f32 size;
    /* 0x1C */ f32 unk1C;
    /* 0x20 */ u8 pad20[0x2C - 0x20];
    /* 0x2C */ s16 life;
    /* 0x2E */ u8 pad2E[0x31 - 0x2E];
    /* 0x31 */ u8 alpha;
    /* 0x32 */ u8 pad32[0x34 - 0x32];
} M095Flame;

/* A user's weapon data (size 0x624) */
typedef struct M095Data {
    /* 0x000 */ Object *owner;
    /* 0x004 */ u8 pad4[0x8 - 0x4];
    /* 0x008 */ M095Flame flames[30];
    /* 0x620 */ u32 newest : 6; /* the newest flame + 1 */
    /* 0x620 */ u32 count : 6;
} M095Data;

/* What a flame user's obj->header points at */
typedef struct M095Header {
    /* 0x00 */ u8 pad0[0xAC];
    /* 0xAC */ void **textures;
} M095Header;

/* The owner's light (in the player's data) */
typedef struct M095Light {
    /* 0x0 */ u8 pad0[0x4];
    /* 0x4 */ s32 light;
} M095Light;

/* What a player object keeps in unk68 */
typedef struct M095Player {
    /* 0x000 */ u8 pad0[0x564];
    /* 0x564 */ M095Light *flameLight;
} M095Player;

extern void func_80098360(void *data, s32 size); /* clears */
extern void func_8002179C(s32 light);            /* DKR light_remove */
extern void func_8004AA80(void *data);           /* DKR mempool_free */
extern void func_80055B80(Gfx **dl);             /* resets the render settings */
extern void render_sprite_billboard(Gfx **dl, Mtx **mtx, struct Vertex **vtx, Object *obj, void *texture, s32 flags, s32 alpha);

static M095Data *sUsers[4];
static s32 sUserCount;

void func_m095_00000(Object *owner, M095Data *data) {
    func_80098360(data, 0x624);
    data->owner = owner;
    sUsers[sUserCount++] = data;
}

INCLUDE_ASM("asm/nonmatchings/modules/m095", func_m095_0005C);

INCLUDE_ASM("asm/nonmatchings/modules/m095", func_m095_0059C);

INCLUDE_ASM("asm/nonmatchings/modules/m095", func_m095_00D38);

INCLUDE_ASM("asm/nonmatchings/modules/m095", func_m095_0110C);

/* Draws the flames as sprites, newest first, skipping ones that would overlap the last drawn */
void func_m095_0148C(Object *obj, M095Data *data, Gfx **dlp, Mtx **mtxp, struct Vertex **vtxp) {
    UNUSED s32 pad[4];
    Object sprite;
    s32 i;
    u32 n;
    M095Flame *f;
    f32 gap;
    Gfx *dl;
    Mtx *mtx;
    struct Vertex *vtx;
    void *texture;

    dl = *dlp;
    mtx = *mtxp;
    vtx = *vtxp;
    sprite.trans.rotation.y_rotation = 0;
    sprite.trans.rotation.x_rotation = 0;
    sprite.trans.rotation.z_rotation = 0;
    func_80055B80(&dl);
    gDPPipeSync(dl++);
    gDPSetEnvColor(dl++, 255, 255, 255, 255);
    if (obj->behaviorId == 1) {
        texture = ((M095Header *) obj->header)->textures[3];
    } else {
        texture = ((M095Header *) obj->header)->textures[0];
    }
    i = data->newest - 1;
    gap = 0.0f;
    for (n = 0; n < data->count; n++) {
        if (i < 0) {
            i += 30;
        }
        if (gap <= 0.0f) {
            f = &data->flames[i];
            if (f->life > 0) {
                sprite.unk28 = f->unk1C;
                sprite.trans.x_position = f->x;
                sprite.trans.y_position = f->y;
                sprite.trans.z_position = f->z;
                sprite.trans.scale = f->size;
                gDPPipeSync(dl++);
                gDPSetPrimColor(dl++, 0, 0, 255, 255, 255, f->alpha);
                render_sprite_billboard(&dl, &mtx, &vtx, &sprite, texture, 2, f->alpha);
            }
            gap += f->size;
        }
        gap -= 1.5f;
        i--;
    }
    gDPSetPrimColor(dl++, 0, 0, 255, 255, 255, 255);
    func_80055B80(&dl);
    *dlp = dl;
    *mtxp = mtx;
    *vtxp = vtx;
}

void func_m095_0175C(void) {
    s32 i;
    Object *owner;
    M095Data *data;
    M095Player *player;

    i = sUserCount;
    while (i--) {
        data = sUsers[i];
        owner = data->owner;
        if (owner->behaviorId == 1) {
            player = owner->unk68;
            if (player->flameLight->light != 0) {
                func_8002179C(player->flameLight->light);
            }
            func_8004AA80(player->flameLight);
            player->flameLight = NULL;
        }
    }
}
#pragma weak __rl_fini = func_m095_0175C

INCLUDE_ASM("asm/nonmatchings/modules/m095", data);

INCLUDE_ASM("asm/nonmatchings/modules/m095", rodata);
