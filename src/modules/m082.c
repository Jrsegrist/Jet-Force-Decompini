/*
 * Runlink module 82: an object that opens when the player comes to it, with a
 * camera that swings round to look at it while it does (m075's cabinet camera
 * works the same way).
 */
#include "module.h"

/* What init gets */
typedef struct M082Setup {
    /* 0x00 */ LevelObjectEntryCommon common;
    /* 0x0A */ u8 kind;
    /* 0x0B */ u8 scale;
    /* 0x0C */ s16 yaw;
    /* 0x0E */ s16 camHeight;
    /* 0x10 */ u16 camDist;
    /* 0x12 */ s16 camYaw;
} M082Setup;

/* Kept in obj->unk68 */
typedef struct M082State {
    /* 0x00 */ s32 unk0;
    /* 0x04 */ u8 unk4;
    /* 0x05 */ u8 kind;
    /* 0x06 */ s16 camYaw;
    /* 0x08 */ f32 camDist;
    /* 0x0C */ f32 camHeight;
} M082State;

extern ObjectTransform *func_80041DE0(void); /* the camera */
extern f32 func_80097140(f32 x);             /* sqrtf */
extern s32 func_80049C84(f32 x, f32 z);      /* the angle of a vector */
extern f32 func_80049BEC(f32 rate, s32 updateRate);
extern s32 func_80049D80(s32 from, s32 to); /* the difference between two angles */
extern s16 angle_lerp(s16 angle, s16 target, f32 rate); /* turns an angle towards another */

/* The static functions below, still assembly: called by their module offset names */
extern void rl_m082_00340(Object *obj, M082State *state, u32 kind);
extern s32 rl_m082_013C0(u8 kind);

/* The module's .rodata, still assembly: its first two floats (0.975, 0.9) and 0.01 */
extern f32 rl_m082_017F0[];
extern f32 rl_m082_01814;

/* The camera */
static f32 sCentreX;
static f32 sCentreY;
static f32 sCentreZ;
static f32 sCamDist;
static f32 sCamHeight;
static s16 sCamYaw;
static f32 sTargetDist;
static f32 sTargetHeight;
static s16 sTargetYaw;

/* Starts the camera's swing from where it is to the object's view */
void func_m082_00000(Object *obj) {
    f32 dx;
    f32 dz;
    M082State *state = obj->unk68;
    ObjectTransform *camera;

    camera = func_80041DE0();
    sCentreX = obj->trans.x_position;
    sCentreY = obj->trans.y_position + (480.0f * obj->trans.scale + 8.0f);
    sCentreZ = obj->trans.z_position;
    dx = camera->x_position - obj->trans.x_position;
    dz = camera->z_position - obj->trans.z_position;
    sCamDist = func_80097140(dx * dx + dz * dz);
    sCamHeight = camera->y_position - sCentreY;
    sCamYaw = 0x8000 - func_80049C84(dx, dz);
    sTargetDist = state->camDist;
    sTargetHeight = state->camHeight;
    sTargetYaw = state->camYaw - obj->trans.rotation.y_rotation;
}

/* The camera while the object is open */
void func_m082_00114(ObjectTransform *camera, s32 updateRate) {
    s32 turn;
    s32 i;
    s16 yaw;
    s16 pitch;
    f32 x;
    f32 z;
    f32 rate;

    rate = 1.0f - func_80049BEC(rl_m082_017F0[0], updateRate);
    sCamDist += (sTargetDist - sCamDist) * rate;
    sCamHeight += (sTargetHeight - sCamHeight) * rate;
    i = updateRate;
    while (i--) {
        turn = func_80049D80(sCamYaw, sTargetYaw) >> 4;
        if (turn < -0x200) {
            sCamYaw -= 0x200;
        } else if (turn > 0x200) {
            sCamYaw += 0x200;
        } else {
            sCamYaw += turn;
        }
    }
    x = func_80049C34(sCamYaw) * sCamDist;
    z = -func_80049C30(sCamYaw) * sCamDist;
    camera->x_position = sCentreX + x;
    camera->y_position = sCentreY + sCamHeight;
    camera->z_position = sCentreZ + z;
    yaw = 0x8000 - func_80049C84(x, z);
    pitch = func_80049C84(sCamHeight, sCamDist);
    rate = 1.0f - func_80049BEC(rl_m082_017F0[1], updateRate);
    camera->rotation.y_rotation = angle_lerp(camera->rotation.y_rotation, yaw, rate);
    camera->rotation.x_rotation = angle_lerp(camera->rotation.x_rotation, pitch, rate);
}

INCLUDE_ASM("asm/nonmatchings/modules/m082", func_m082_00340);

void func_m082_00478(Object *obj, M082Setup *setup) {
    M082State *state = obj->unk68;

    state->unk0 = 0;
    obj->trans.scale = rl_m082_01814 * (setup->scale * obj->header->scale);
    obj->trans.rotation.y_rotation = setup->yaw;
    rl_m082_00340(obj, state, 0);
    state->kind = setup->kind;
    if (rl_m082_013C0(state->kind)) {
        state->unk4 = 5;
    } else {
        state->unk4 = 0;
    }
    state->camHeight = setup->camHeight;
    state->camDist = setup->camDist;
    state->camYaw = setup->camYaw;
    obj_anim_set(obj, 0, 0.0f);
    func_80079D1C(obj, -1, 0, 8, 0, 0, 0, 4.0f, 4.0f, -1.0f, 1.0f, 1);
}

INCLUDE_ASM("asm/nonmatchings/modules/m082", func_m082_005D4);

INCLUDE_ASM("asm/nonmatchings/modules/m082", func_m082_00E6C);

INCLUDE_ASM("asm/nonmatchings/modules/m082", func_m082_00F8C);

INCLUDE_ASM("asm/nonmatchings/modules/m082", func_m082_013C0);

/* The level entry 0165C spawns (size 0x10) */
typedef struct M082Entry {
    /* 0x00 */ s16 objectID;
    /* 0x02 */ u8 size;
    /* 0x03 */ u8 unk3;
    /* 0x04 */ s16 x;
    /* 0x06 */ s16 y;
    /* 0x08 */ s16 z;
    /* 0x0A */ s8 unkA;
    /* 0x0B */ s8 unkB;
    /* 0x0C */ f32 unkC;
} M082Entry;

struct M082Linked;
extern struct M082Linked *rl_m082_017A0; /* .data (still assembly): the thing spawned last */
extern void func_m007_02824(void);
extern Object *func_80005F14(void *entry, s32 arg1); /* DKR spawn_object */

/* Spawns object `id` at (x, y, z) and keeps it (called from 00F8C) */
void func_m082_0165C(s16 id, f32 x, f32 y, f32 z, f32 a, f32 b, f32 c) {
    M082Entry entry;

    func_m007_02824();
    entry.objectID = id;
    entry.size = sizeof(M082Entry);
    entry.unk3 = 0;
    entry.x = x;
    entry.y = y;
    entry.z = z;
    entry.unkA = a;
    entry.unkB = b;
    entry.unkC = c;
    rl_m082_017A0 = (struct M082Linked *) func_80005F14(&entry, 1);
    if (rl_m082_017A0 != NULL) {
        ((Object *) rl_m082_017A0)->level_entry = NULL;
    }
}

/* What the module's first .data word points at, as far as 01778 uses it */
typedef struct M082Linked {
    /* 0x00 */ u8 pad0[0x88];
    /* 0x88 */ s32 unk88;
} M082Linked;

extern M082Linked *rl_m082_017A0; /* .data (still assembly) */

/* Sets the linked thing's flag at 0x88 and forgets it (called from 005D4) */
void func_m082_01778(void) {
    if (rl_m082_017A0 != NULL) {
        rl_m082_017A0->unk88 = 1;
        rl_m082_017A0 = NULL;
    }
}

INCLUDE_ASM("asm/nonmatchings/modules/m082", data);

INCLUDE_ASM("asm/nonmatchings/modules/m082", rodata);
