/*
 * Runlink module 4: copies of a model (its vertex and triangle buffers and
 * their shadows), built and freed for other modules.
 */
#include "module.h"

/* A model copy and the buffers it owns */
typedef struct M004Copy {
    /* 0x00 */ u8 pad0[0x24];
    /* 0x24 */ void *unk24;
    /* 0x28 */ void *unk28;
    /* 0x2C */ void *unk2C;
    /* 0x30 */ u8 pad30[0x34 - 0x30];
    /* 0x34 */ void *unk34; /* optional */
    /* 0x38 */ void *buffers[(0x68 - 0x38) / 4];
    /* 0x68 */ void *unk68;
    /* 0x6C */ u8 pad6C[0xB8 - 0x6C];
    /* 0xB8 */ u8 numBuffers;
} M004Copy;

/* The copy as its set-up sees it */
typedef struct M004Setup {
    /* 0x00 */ f32 scale;
    /* 0x04 */ void *unk4;
    /* 0x08 */ s16 unk8;
    /* 0x0A */ s16 unkA;
    /* 0x0C */ u8 unkC;
    /* 0x0D */ u8 unkD;
    /* 0x0E */ u8 unkE;
    /* 0x0F */ u8 unkF;
    /* 0x10 */ u8 pad10[0x20 - 0x10];
    /* 0x20 */ u8 *unk20;
    /* 0x24 */ u8 pad24[0x60 - 0x24];
    /* 0x60 */ void **model;
    /* 0x64 */ void **model2;
    /* 0x68 */ u8 pad68[0xAC - 0x68];
    /* 0xAC */ f32 unkAC;
    /* 0xB0 */ u8 padB0[0xBA - 0xB0];
    /* 0xBA */ u8 unkBA;
} M004Setup;

/* The 0x20-byte header written before the copy's buffer */
typedef struct M004Block {
    /* 0x00 */ u8 unk0;
    /* 0x01 */ u8 unk1;
    /* 0x02 */ u8 unk2;
    /* 0x03 */ u8 unk3;
    /* 0x04 */ u8 unk4;
    /* 0x05 */ u8 unk5;
    /* 0x06 */ s16 unk6;
    /* 0x08 */ s16 unk8;
    /* 0x0A */ s16 unkA;
    /* 0x0C */ void *unkC;
    /* 0x10 */ s16 unk10;
    /* 0x12 */ s16 unk12;
    /* 0x14 */ s16 unk14;
    /* 0x16 */ s16 unk16;
    /* 0x18 */ u8 unk18;
    /* 0x19 */ u8 unk19;
    /* 0x1A */ u8 unk1A;
    /* 0x1B */ u8 unk1B;
    /* 0x1C */ u8 unk1C;
    /* 0x1D */ u8 unk1D;
    /* 0x1E */ u8 unk1E;
    /* 0x1F */ u8 unk1F;
} M004Block;

/* What obj->shadow points at here */
typedef struct M004Shadow {
    /* 0x00 */ f32 scale;
    /* 0x04 */ u8 pad4[0xC - 0x4];
    /* 0x0C */ u8 unkC;
    /* 0x0D */ u8 unkD;
    /* 0x0E */ u8 unkE;
    /* 0x0F */ u8 padF[0x1C - 0xF];
    /* 0x1C */ M004Setup *copy;
} M004Shadow;

extern s32 rl_m004_01398(Object *obj, M004Setup *copy);
extern void rl_m004_01488(Object *obj, M004Setup *copy, s32 arg2, s32 arg3);
extern u8 rl_m004_01660[];

extern void func_8004AA80(void *data); /* DKR mempool_free */
extern void free_3d_model(void *model);

/* Sets up an object's shadow copy; returns FALSE when it cannot be made */
s32 func_m004_00000(Object *obj) {
    M004Block *b;
    M004Setup *c = ((M004Shadow *) obj->shadow)->copy;
    s8 index;
    u8 *m;

    if (!rl_m004_01398(obj, c)) {
        return FALSE;
    }
    c->unkAC = 1.0f;
    c->model = ((void ***) obj->modelInstances)[obj->modelIndex];
    index = ((s8 *) obj->header)[0x60];
    if (index >= 0) {
        c->model2 = ((void ***) obj->modelInstances)[index];
    } else {
        c->model2 = c->model;
    }
    m = *c->model;
    if (m[0x2F]) {
        c->unkBA = 1;
    }
    rl_m004_01488(obj, c, 0x400, 11);
    b = (M004Block *) (c->unk20 - 0x20);
    b->unk0 = 0x40;
    b->unk1 = 0x40;
    b->unk2 = 5;
    b->unk3 = 0;
    b->unk4 = 0;
    b->unk5 = 0;
    b->unk6 = 0;
    b->unk8 = 1;
    b->unkA = 0x38;
    b->unkC = rl_m004_01660;
    b->unk10 = 0;
    b->unk12 = 0;
    b->unk14 = 0;
    b->unk16 = 0x1000;
    b->unk18 = 0;
    b->unk19 = 0;
    b->unk1A = 0;
    b->unk1B = 0;
    b->unk1C = 2;
    b->unk1D = 0;
    b->unk1E = 2;
    b->unk1F = 0;
    c->unk4 = b;
    c->scale = ((M004Shadow *) obj->shadow)->scale;
    c->unkC = ((M004Shadow *) obj->shadow)->unkC | 0x20;
    c->unkD = ((M004Shadow *) obj->shadow)->unkD;
    c->unkE = ((M004Shadow *) obj->shadow)->unkE;
    c->unkF = 0;
    c->unk8 = 0;
    c->unkA = 0;
    return TRUE;
}

INCLUDE_ASM("asm/nonmatchings/modules/m004", func_m004_001BC);

/* Frees a copy's buffers */
void func_m004_002B8(M004Copy *copy) {
    s32 i;

    func_8004AA80(copy->unk24);
    func_8004AA80(copy->unk28);
    func_8004AA80(copy->unk2C);
    if (copy->unk34 != NULL) {
        func_8004AA80(copy->unk34);
    }
    if (copy->unk68 != NULL) {
        free_3d_model(copy->unk68);
    }
    for (i = 0; i < copy->numBuffers; i++) {
        func_8004AA80(copy->buffers[i]);
    }
}

INCLUDE_ASM("asm/nonmatchings/modules/m004", func_m004_0036C);

INCLUDE_ASM("asm/nonmatchings/modules/m004", func_m004_00CB8);

INCLUDE_ASM("asm/nonmatchings/modules/m004", func_m004_012BC);

INCLUDE_ASM("asm/nonmatchings/modules/m004", func_m004_01398);

INCLUDE_ASM("asm/nonmatchings/modules/m004", func_m004_01488);

INCLUDE_ASM("asm/nonmatchings/modules/m004", func_m004_015A8);

INCLUDE_ASM("asm/nonmatchings/modules/m004", data);

INCLUDE_ASM("asm/nonmatchings/modules/m004", rodata);

INCLUDE_ASM("asm/nonmatchings/modules/m004", bss);
