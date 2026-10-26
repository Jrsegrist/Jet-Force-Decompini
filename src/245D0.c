#include "common.h"

/* Size: 0x50 bytes */
typedef struct CamLight {
    /* 0x00 */ u8 pad0[6];
    /* 0x06 */ u8 unk6;
    /* 0x07 */ u8 pad7[0x3C - 7];
    /* 0x3C */ s32 unk3C;
    /* 0x40 */ u8 pad40[0x10];
} CamLight;
extern CamLight D_800F5B60[50];
extern s32 D_800A110C;

/* JFG: reset the 50 camera-light slots. */
void func_800239D0(void) {
    CamLight *light = D_800F5B60;
    s32 i = 50;
    while (i--) {
        light->unk6 = 0;
        light->unk3C = 0;
        light++;
    }
    D_800A110C = 0;
}

INCLUDE_ASM("asm/nonmatchings/245D0", func_80023A00);

INCLUDE_ASM("asm/nonmatchings/245D0", func_80023A48);

INCLUDE_ASM("asm/nonmatchings/245D0", func_80023CB4);

INCLUDE_ASM("asm/nonmatchings/245D0", func_80023D48);

INCLUDE_ASM("asm/nonmatchings/245D0", func_80023DAC);

INCLUDE_ASM("asm/nonmatchings/245D0", func_80023FA4);

INCLUDE_ASM("asm/nonmatchings/245D0", func_80024044);

INCLUDE_ASM("asm/nonmatchings/245D0", func_80024330);

INCLUDE_ASM("asm/nonmatchings/245D0", func_80024430);

INCLUDE_ASM("asm/nonmatchings/245D0", func_80024634);
