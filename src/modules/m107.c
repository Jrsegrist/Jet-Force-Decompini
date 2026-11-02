/*
 * Runlink module 107: the invincibility pick-up and the shield it gives. The
 * pick-up spins and fades in; a player who touches it gets a shield object
 * (three rings of particles that orbit the player, each with a light and a
 * colour cycle) for the pick-up's duration, and the pick-up comes back after
 * its respawn time.
 */
#include "module.h"
#include "gfx.h"

/* The pick-up's state (at 0x88 in the object) */
typedef struct M107State {
    /* 0x0 */ s16 duration;
    /* 0x2 */ s16 respawnTime;
    /* 0x4 */ s16 timer;
    /* 0x6 */ s16 fade;
} M107State;

/* The shield's level entry, three rings' worth of parameters */
typedef struct M107Entry {
    /* 0x00 */ s16 objectID;
    /* 0x02 */ s16 unk2;
    /* 0x04 */ s16 x;
    /* 0x06 */ s16 y;
    /* 0x08 */ s16 z;
    /* 0x0A */ s16 radius;
    /* 0x0C */ s16 height;
    /* 0x0E */ s16 unkE[3];
    /* 0x14 */ s16 unk14[3];
    /* 0x1A */ s16 unk1A[3];
    /* 0x20 */ s16 unk20[3];
    /* 0x26 */ s16 unk26[3];
    /* 0x2C */ s16 unk2C[3];
    /* 0x32 */ s16 colourCycle[3];
    /* 0x38 */ s16 unk38[3];
    /* 0x3E */ s16 unk3E[3];
    /* 0x44 */ f32 scale[3];
    /* 0x50 */ s16 unk50;
    /* 0x54 */ Object *owner;
} M107Entry;

/* The colour cycle of 55130.c (colour_cycle_init) */
typedef struct M107ColourCycle {
    /* 0x0 */ u8 data[0x10];
} M107ColourCycle;

/* The shield's data (its object's unk68) */
typedef struct M107Shield {
    /* 0x000 */ u8 pad0[0x438];
    /* 0x438 */ s16 radius;
    /* 0x43A */ s16 height;
    /* 0x43C */ s16 unk43C[3];
    /* 0x442 */ s16 unk442[3];
    /* 0x448 */ s16 unk448[3];
    /* 0x44E */ s16 unk44E[3];
    /* 0x454 */ Object *owner;
    /* 0x458 */ s16 unk458[3];
    /* 0x45E */ s16 unk45E[3];
    /* 0x464 */ s16 unk464[3];
    /* 0x46A */ s16 unk46A[3];
    /* 0x470 */ f32 scale[3];
    /* 0x47C */ f32 unk47C[3];
    /* 0x488 */ M107ColourCycle cycles[3];
    /* 0x4B8 */ f32 targetScaleX;
    /* 0x4BC */ f32 targetScaleY;
    /* 0x4C0 */ s16 targetHeight;
    /* 0x4C2 */ s16 timer;
    /* 0x4C4 */ f32 scaleX;
    /* 0x4C8 */ f32 scaleY;
    /* 0x4CC */ u8 pad4CC[0x4F0 - 0x4CC];
    /* 0x4F0 */ s16 angle[3];
    /* 0x4F6 */ s16 buffer;
    /* 0x4F8 */ s32 lights[3];
    /* 0x504 */ s32 sound;
} M107Shield;

/* What a player object keeps in its unk68, as far as this module uses it */
typedef struct M107Player {
    /* 0x000 */ u8 pad0[0x5BC];
    /* 0x5BC */ Object *shield;
    /* 0x5C0 */ u8 pad5C0[0x5C8 - 0x5C0];
    /* 0x5C8 */ s16 invincibleTime;
} M107Player;

extern s32 func_8007E150(f32 x, f32 y, f32 z, f32 radius, s32 arg4, Object **list); /* objects in range */
extern Object *func_80005F14(void *entry, s32 arg1); /* DKR spawn_object */
extern void colour_cycle_init(M107ColourCycle *cycle, s32 assetID);
extern s32 func_800210A4(s32 arg0, LightParams *params); /* adds a light */

static s16 sAngles[4] = { 0x1000, 0, 0x3000, 0 };
static M107Entry sShieldEntry = {
    0x133, 0x5800, 0, 0, 0, 100, 120,
    { 0x8000, 0x8000, 0x4000 },
    { 0x300, 0x400, 0x200 },
    { -2, 3, 1 },
    { 0x800, 0x1000, 0x500 },
    { 0xF, 0, 4 },
    { -0xA0, 0x100, 0xA0 },
    { 0xF, 0x11, 0xA },
    { 0, 0x4000, 0x8000 },
    { 0x100, 0x80, 0x200 },
    { 0.1f, 0.15f, 0.075f },
    0x11,
    NULL,
};

static Triangle sTriangles[16];
static s16 sCircle[32][2];
static u8 sTablesBuilt;

void func_m107_00000(Object *obj, LevelObjectEntryCommon *entry) {
    M107State *state = OBJ_STATE(obj, M107State);

    state->duration = ((s8 *) entry)[0xA] * 60;
    state->respawnTime = ((s8 *) entry)[0xB] * 60;
    state->timer = 0;
    state->fade = 255;
}

void func_m107_00038(Object *obj, s32 updateRate) {
    M107State *state = OBJ_STATE(obj, M107State);
    M107Player *player;
    Object *list[8];
    Object *other;
    s32 count;
    s32 i;

    count = func_8007E150(obj->trans.x_position, obj->trans.y_position, obj->trans.z_position, 40.0f, 0, list);
    if (state->fade == 0) {
        state->timer -= updateRate;
        if (state->timer <= 0) {
            state->timer = 0;
            if (count == 0) {
                state->fade = updateRate;
            }
        }
    } else {
        obj->trans.rotation.y_rotation += updateRate << 8;
        state->fade += updateRate * 4;
        if (state->fade >= 0x100) {
            state->fade = 0xFF;
        }
        if (count != 0) {
            for (i = 0; i < count; i++) {
                other = list[i];
                player = other->unk68;
                if (player->invincibleTime == 0) {
                    if (player->shield == NULL) {
                        sShieldEntry.owner = other;
                        sShieldEntry.x = other->trans.x_position;
                        sShieldEntry.y = other->trans.y_position;
                        sShieldEntry.z = other->trans.z_position;
                        player->shield = func_80005F14(&sShieldEntry, 1);
                        if (player->shield != NULL) {
                            player->shield->level_entry = NULL;
                        }
                    }
                    if (player->shield != NULL) {
                        ((M107Shield *) player->shield->unk68)->timer = state->duration;
                    }
                    player->invincibleTime = state->duration;
                    state->timer = state->respawnTime;
                    if (state->timer != 0) {
                        state->fade = 0;
                    } else {
                        func_800079B8(obj);
                    }
                    func_m006_011E0(player, 0x89);
                }
            }
        }
    }
}

#ifdef NON_MATCHING
/* Sets up the shield; the triangle-table loop's temporaries get other registers */
void func_m107_0027C(Object *obj, M107Entry *entry) {
    M107Shield *shield = obj->unk68;
    s32 i;
    s32 angle;
    s32 u;
    s32 v;
    Triangle *tri;
    s32 pad;
    LightParams light;

    for (i = 0; i < 3; i++) {
        shield->unk43C[i] = entry->unkE[i];
        shield->unk442[i] = entry->unk14[i];
        shield->unk448[i] = entry->unk1A[i];
        shield->unk44E[i] = entry->unk20[i];
        shield->unk458[i] = entry->unk26[i];
        shield->unk45E[i] = entry->unk2C[i];
        shield->unk464[i] = entry->unk38[i];
        shield->unk46A[i] = entry->unk3E[i];
        shield->scale[i] = entry->scale[i];
        colour_cycle_init(&shield->cycles[i], entry->colourCycle[i]);
        shield->angle[i] = sAngles[i];
        light.type = 1;
        light.unk1 = 3;
        light.flags = 0x40;
        light.unk4 = 0;
        light.unk3 = -1;
        light.unk6 = 0;
        light.unk8 = 0;
        light.unkA = 0;
        light.a = 255;
        light.radius = 120;
        light.radius2 = 120;
        light.unk14 = -1;
        light.unk16 = 0;
        light.unk17 = 0;
        shield->lights[i] = func_800210A4(0, &light);
    }
    shield->radius = entry->radius;
    shield->height = entry->height;
    shield->scaleX = 1.0f;
    shield->scaleY = 1.0f;
    shield->targetScaleX = 1.0f;
    shield->targetScaleY = 1.0f;
    shield->targetHeight = shield->height;
    shield->owner = entry->owner;
    if (!sTablesBuilt) {
        tri = sTriangles;
        v = 0;
        u = 0x7E0;
        for (i = 0; i < 8; i++) {
            tri[0].flags = 0x40;
            tri[0].vi0 = v;
            tri[0].vi1 = v + 2;
            tri[0].vi2 = v + 1;
            tri[0].uv[0][0] = u;
            tri[0].uv[0][1] = 0;
            tri[0].uv[1][0] = u - 0xFC;
            tri[0].uv[1][1] = 0;
            tri[0].uv[2][0] = u;
            tri[0].uv[2][1] = 0x1E0;
            tri[1].flags = 0x40;
            tri[1].vi0 = v + 2;
            tri[1].vi1 = v + 1;
            tri[1].vi2 = v + 3;
            tri[1].uv[0][0] = u - 0xFC;
            tri[1].uv[0][1] = 0;
            tri[1].uv[1][0] = u;
            tri[1].uv[1][1] = 0x1E0;
            tri[1].uv[2][0] = u - 0xFC;
            tri[1].uv[2][1] = 0x1E0;
            tri += 2;
            v += 2;
            u -= 0xFC;
        }
        angle = 0;
        for (i = 0; i < 32; i++) {
            sCircle[i][0] = func_80049C34(angle) * 100.0f;
            sCircle[i][1] = func_80049C30(angle) * 100.0f;
            angle += 0x800;
        }
    }
    func_80003410(0xF9, obj->trans.x_position, obj->trans.y_position, obj->trans.z_position, 1, &shield->sound);
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m107", func_m107_0027C);
#endif

INCLUDE_ASM("asm/nonmatchings/modules/m107", func_m107_005C0);

void func_m107_00D08(Object *obj, f32 scaleX, f32 scaleY, s32 height) {
    M107Shield *shield = obj->unk68;

    shield->targetHeight = height;
    shield->targetScaleX = scaleX;
    shield->targetScaleY = scaleY;
}

INCLUDE_ASM("asm/nonmatchings/modules/m107", func_m107_00D24);

INCLUDE_ASM("asm/nonmatchings/modules/m107", rodata);
