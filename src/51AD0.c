#include "common.h"

INCLUDE_ASM("asm/nonmatchings/51AD0", func_80050ED0);

extern s32 D_800FE9F0;
extern void *D_800A38E4;

void *func_80050F44(s32 *count) {
    *count = D_800FE9F0;
    return D_800A38E4;
}

INCLUDE_ASM("asm/nonmatchings/51AD0", func_80050F60);

INCLUDE_ASM("asm/nonmatchings/51AD0", func_8005122C);

extern s32 D_800FE9A0;
extern s32 *D_800FE9AC;

void func_8005131C(s32 arg0) {
    D_800FE9AC[D_800FE9A0] = arg0;
    D_800FE9A0++;
}

extern s32 *D_800FE9A4;
extern s32 D_800FE9A8;

void func_80051350(s32 arg0) {
    D_800FE9A4[D_800FE9A8] = arg0;
    if (++D_800FE9A8 >= D_800FE9A0) {
        D_800FE9A8 = 0;
    }
}

INCLUDE_ASM("asm/nonmatchings/51AD0", func_800513A0);

INCLUDE_ASM("asm/nonmatchings/51AD0", func_80051420);

INCLUDE_ASM("asm/nonmatchings/51AD0", func_80051484);

INCLUDE_ASM("asm/nonmatchings/51AD0", func_80051508);

INCLUDE_ASM("asm/nonmatchings/51AD0", func_800515D0);

INCLUDE_ASM("asm/nonmatchings/51AD0", func_80051660);

INCLUDE_ASM("asm/nonmatchings/51AD0", func_800516EC);

INCLUDE_ASM("asm/nonmatchings/51AD0", func_80051880);

INCLUDE_ASM("asm/nonmatchings/51AD0", func_80051CCC);

INCLUDE_ASM("asm/nonmatchings/51AD0", func_80051EA4);

extern u32 *D_800A38E0;
u32 *asset_table_load(u32 assetIndex);

void func_80052678(void) {
    D_800A38E0 = asset_table_load(0x3E);
}

INCLUDE_ASM("asm/nonmatchings/51AD0", func_8005269C);

INCLUDE_ASM("asm/nonmatchings/51AD0", func_80052734);

extern void *D_800FEA08;
extern s32 D_800FEA0C;
extern void *D_800FEA10;
extern s32 D_800FEA14;

INCLUDE_ASM("asm/nonmatchings/51AD0", func_80052798);

INCLUDE_ASM("asm/nonmatchings/51AD0", func_800527F0);

INCLUDE_ASM("asm/nonmatchings/51AD0", func_80052874);

INCLUDE_ASM("asm/nonmatchings/51AD0", func_80052A00);

INCLUDE_ASM("asm/nonmatchings/51AD0", func_80052AB8);

INCLUDE_ASM("asm/nonmatchings/51AD0", func_80052CFC);

INCLUDE_ASM("asm/nonmatchings/51AD0", func_800531C8);

/* Size: 0x20 bytes */
typedef struct ModuleEntry {
    s32 unk0;
    u8 pad4[0x1C];
} ModuleEntry;
extern ModuleEntry *D_800FEAA0;

s32 func_80053370(s32 index) {
    return D_800FEAA0[index].unk0;
}

INCLUDE_ASM("asm/nonmatchings/51AD0", func_8005338C);

INCLUDE_ASM("asm/nonmatchings/51AD0", func_8005342C);

INCLUDE_ASM("asm/nonmatchings/51AD0", func_8005372C);

INCLUDE_ASM("asm/nonmatchings/51AD0", func_80053934);

INCLUDE_ASM("asm/nonmatchings/51AD0", func_800539E0);

INCLUDE_ASM("asm/nonmatchings/51AD0", func_80053C44);

INCLUDE_ASM("asm/nonmatchings/51AD0", func_80053D2C);

INCLUDE_ASM("asm/nonmatchings/51AD0", func_8005414C);

typedef struct DestructTimer {
    u16 timer : 10;
    u16 unk : 6;
} DestructTimer;
extern DestructTimer *D_800FEB58;

INCLUDE_ASM("asm/nonmatchings/51AD0", func_800541B0);

INCLUDE_ASM("asm/nonmatchings/51AD0", func_80054204);

INCLUDE_ASM("asm/nonmatchings/51AD0", func_800542C4);

INCLUDE_ASM("asm/nonmatchings/51AD0", func_80054368);
