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

extern void func_8004AA80(void *data); /* DKR mempool_free */
extern void free_3d_model(void *model);

INCLUDE_ASM("asm/nonmatchings/modules/m004", func_m004_00000);

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
