/*
 * Runlink module 28: a beam that follows a track. Its object spawns the beam
 * object (the module's other behaviour) and every frame hands it its own
 * position, facing and strength, or turns it off while hidden.
 */
#include "module.h"

typedef struct M028Setup {
    /* 0x00 */ s16 objectID;
    /* 0x02 */ u8 kind; /* 10: spawned by another object, with the defaults */
    /* 0x03 */ u8 pad3;
    /* 0x04 */ s16 x;
    /* 0x06 */ s16 y;
    /* 0x08 */ s16 z;
    /* 0x0A */ s16 yRotation;
    /* 0x0C */ s16 xRotation;
    /* 0x0E */ s16 strength;
    /* 0x10 */ u8 unk10;
    /* 0x11 */ u8 pad11[0x14 - 0x11];
    /* 0x14 */ Object *owner;
} M028Setup;

/* The level entry the beam object is spawned from. Size 0x2C */
typedef struct M028BeamEntry {
    /* 0x00 */ s16 objectID;
    /* 0x02 */ u8 size;
    /* 0x03 */ u8 pad3;
    /* 0x04 */ s16 x;
    /* 0x06 */ s16 y;
    /* 0x08 */ s16 z;
    /* 0x0A */ u8 padA[0x10 - 0xA];
    /* 0x10 */ Object *owner;
    /* 0x14 */ s16 shape[3][3];
    /* 0x26 */ s16 strength;
    /* 0x28 */ u8 unk28;
    /* 0x29 */ u8 unk29;
    /* 0x2A */ u8 unk2A;
} M028BeamEntry;

/* The beam's state (its object's unk68) */
typedef struct M028BeamState {
    /* 0x000 */ u8 pad0[0xA78];
    /* 0xA78 */ s16 strength;
    /* 0xA7A */ s16 strength2;
    /* 0xA7C */ u8 padA7C;
    /* 0xA7D */ u8 on;
} M028BeamState;

/* The state in the object */
typedef struct M028State {
    /* 0x0 */ s32 unk0;
    /* 0x4 */ s32 strength;
} M028State;

extern Object *func_80005F14(void *entry, s32 arg1); /* DKR spawn_object */
extern void func_80003614(s32 handle);               /* stops a positioned sound */

void func_m028_0031C(Object *obj, s32 arg1);

extern s32 rl_m028_01BD4; /* the beam's sound */

INCLUDE_ASM("asm/nonmatchings/modules/m028", func_m028_00000);

INCLUDE_ASM("asm/nonmatchings/modules/m028", func_m028_0031C);

void func_m028_01624(Object *obj, M028Setup *setup) {
    M028BeamEntry entry;
    Object **beam = obj->unk68;

    entry.objectID = 0xB6;
    entry.size = sizeof(M028BeamEntry);
    entry.x = setup->x;
    entry.y = setup->y;
    entry.z = setup->z;
    if (setup->objectID == 0x1BD) {
        entry.unk2A = 1;
    } else {
        entry.unk2A = 0;
    }
    if (setup->kind == 10) {
        entry.unk28 = 1;
        entry.strength = 250;
        entry.owner = obj;
    } else {
        entry.unk28 = setup->unk10;
        entry.strength = setup->strength;
        entry.owner = setup->owner;
    }
    entry.shape[0][0] = 0;
    entry.shape[1][0] = 0x800;
    entry.shape[2][0] = 0x2000;
    entry.shape[0][1] = 0x4000;
    entry.shape[1][1] = -0x1200;
    entry.shape[2][1] = 0x2000;
    entry.shape[0][2] = 0;
    entry.shape[1][2] = 0x800;
    entry.shape[2][2] = -0x2000;
    entry.unk29 = 1;
    *beam = func_80005F14(&entry, 1);
    (*beam)->level_entry = NULL;
    obj->trans.rotation.y_rotation = setup->yRotation;
    obj->trans.rotation.x_rotation = setup->xRotation;
    (*beam)->trans.rotation.y_rotation = setup->yRotation;
    (*beam)->trans.rotation.x_rotation = setup->xRotation;
    OBJ_STATE(obj, M028State)->strength = entry.strength;
    func_m028_0031C(*beam, 2);
}

void func_m028_01798(Object *obj, s32 updateRate) {
    Object **beams = obj->unk68;
    Object *beam = *beams;

    if (obj->trans.flags & 0x400) {
        ((M028BeamState *) beam->unk68)->strength = 0;
        ((M028BeamState *) beam->unk68)->on = 0;
    } else {
        ((M028BeamState *) beam->unk68)->strength = OBJ_STATE(obj, M028State)->strength;
        ((M028BeamState *) beam->unk68)->strength2 = OBJ_STATE(obj, M028State)->strength;
        ((M028BeamState *) beam->unk68)->on = 1;
    }
    beam->trans.x_position = obj->trans.x_position;
    beam->trans.y_position = obj->trans.y_position;
    beam->trans.z_position = obj->trans.z_position;
    beam->trans.rotation.y_rotation = obj->trans.rotation.y_rotation;
    beam->trans.rotation.x_rotation = obj->trans.rotation.x_rotation;
    beam->trans.rotation.z_rotation = obj->trans.rotation.z_rotation;
}

INCLUDE_ASM("asm/nonmatchings/modules/m028", func_m028_01840);

void func_m028_01B80(void) {
    if (rl_m028_01BD4 != 0) {
        func_80003614(rl_m028_01BD4);
    }
}
#pragma weak __rl_fini = func_m028_01B80

INCLUDE_ASM("asm/nonmatchings/modules/m028", data);

INCLUDE_ASM("asm/nonmatchings/modules/m028", rodata);
