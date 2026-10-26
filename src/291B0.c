#include "common.h"

INCLUDE_ASM("asm/nonmatchings/291B0", func_800285B0);

INCLUDE_ASM("asm/nonmatchings/291B0", func_8002863C);

INCLUDE_ASM("asm/nonmatchings/291B0", func_800286F4);

INCLUDE_ASM("asm/nonmatchings/291B0", func_80028954);

INCLUDE_ASM("asm/nonmatchings/291B0", func_80028E7C);

INCLUDE_ASM("asm/nonmatchings/291B0", func_80028F90);

INCLUDE_ASM("asm/nonmatchings/291B0", func_800291BC);

INCLUDE_ASM("asm/nonmatchings/291B0", func_80029688);

INCLUDE_ASM("asm/nonmatchings/291B0", func_8002993C);

INCLUDE_ASM("asm/nonmatchings/291B0", func_80029A20);

/* Size: 0x24 bytes */
typedef struct Blast {
    /* 0x00 */ u8 pad0[0x1C];
    /* 0x1C */ s16 unk1C;
    /* 0x1E */ u8 pad1E[6];
} Blast;
extern Blast D_800F6B60[16];
extern s32 D_800A11B4;

/* JFG: clear the 16 blast slots. */
void func_80029B78(void) {
    Blast *blast = D_800F6B60;
    s32 i = 16;
    while (i--) {
        blast->unk1C = 0;
        blast++;
    }
    D_800A11B4 = 0;
}

INCLUDE_ASM("asm/nonmatchings/291B0", func_80029BA4);

INCLUDE_ASM("asm/nonmatchings/291B0", func_80029CFC);

INCLUDE_ASM("asm/nonmatchings/291B0", func_8002A704);

INCLUDE_ASM("asm/nonmatchings/291B0", func_8002A7D8);

INCLUDE_ASM("asm/nonmatchings/291B0", func_8002A954);

INCLUDE_ASM("asm/nonmatchings/291B0", func_8002AA7C);

INCLUDE_ASM("asm/nonmatchings/291B0", func_8002AC3C);
