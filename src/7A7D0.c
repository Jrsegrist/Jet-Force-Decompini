#include "common.h"

extern void *D_801047E4;
extern void *D_801047E8;
extern void *D_801047EC;
void *mempool_alloc_safe(s32 size, u32 colourTag);

void func_80079BD0(void) {
    D_801047E4 = mempool_alloc_safe(0x140, 0xFFFF);
    D_801047E8 = mempool_alloc_safe(0x400, 0xFFFF);
    D_801047EC = mempool_alloc_safe(0x400, 0xFFFF);
}

INCLUDE_ASM("asm/nonmatchings/7A7D0", func_80079C20);

extern void *D_801047F4;
extern void *D_801047F8;
extern void *D_801047FC;
extern void *D_80104800;
void mempool_free(void *data);

void func_80079CC4(void) {
    mempool_free(D_80104800);
    mempool_free(D_801047F4);
    mempool_free(D_801047F8);
    mempool_free(D_801047FC);
}

INCLUDE_ASM("asm/nonmatchings/7A7D0", func_80079D1C);

INCLUDE_ASM("asm/nonmatchings/7A7D0", func_8007A018);

INCLUDE_ASM("asm/nonmatchings/7A7D0", func_8007A324);

INCLUDE_ASM("asm/nonmatchings/7A7D0", func_8007A534);

INCLUDE_ASM("asm/nonmatchings/7A7D0", func_8007B99C);

INCLUDE_ASM("asm/nonmatchings/7A7D0", func_8007BB00);

INCLUDE_ASM("asm/nonmatchings/7A7D0", func_8007BCD0);

INCLUDE_ASM("asm/nonmatchings/7A7D0", func_8007C2E8);

INCLUDE_ASM("asm/nonmatchings/7A7D0", func_8007CCF0);

INCLUDE_ASM("asm/nonmatchings/7A7D0", func_8007CF28);

INCLUDE_ASM("asm/nonmatchings/7A7D0", func_8007D0B4);

INCLUDE_ASM("asm/nonmatchings/7A7D0", func_8007D168);

INCLUDE_ASM("asm/nonmatchings/7A7D0", func_8007DE98);

INCLUDE_ASM("asm/nonmatchings/7A7D0", func_8007E150);

INCLUDE_ASM("asm/nonmatchings/7A7D0", func_8007E328);

extern s32 D_801047E0;
extern void *D_801047E4;

void *func_8007E484(s32 *count) {
    *count = D_801047E0;
    return D_801047E4;
}

INCLUDE_ASM("asm/nonmatchings/7A7D0", func_8007E4A0);

INCLUDE_ASM("asm/nonmatchings/7A7D0", func_8007E504);

INCLUDE_ASM("asm/nonmatchings/7A7D0", func_8007E5EC);

INCLUDE_ASM("asm/nonmatchings/7A7D0", func_8007E794);

INCLUDE_ASM("asm/nonmatchings/7A7D0", func_8007E9E0);

INCLUDE_ASM("asm/nonmatchings/7A7D0", func_8007EB6C);

INCLUDE_ASM("asm/nonmatchings/7A7D0", func_8007F280);

INCLUDE_ASM("asm/nonmatchings/7A7D0", func_8007FC0C);

INCLUDE_ASM("asm/nonmatchings/7A7D0", func_800805F4);

INCLUDE_ASM("asm/nonmatchings/7A7D0", func_800809F4);

INCLUDE_ASM("asm/nonmatchings/7A7D0", func_80081658);

INCLUDE_ASM("asm/nonmatchings/7A7D0", func_80081CB8);

INCLUDE_ASM("asm/nonmatchings/7A7D0", func_80082118);

INCLUDE_ASM("asm/nonmatchings/7A7D0", func_800822D8);

INCLUDE_ASM("asm/nonmatchings/7A7D0", func_80082D80);

INCLUDE_ASM("asm/nonmatchings/7A7D0", func_800835E0);

INCLUDE_ASM("asm/nonmatchings/7A7D0", func_80083914);
