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

void *mempool_alloc_safe(s32 size, u32 colourTag);
s32 rand_range(s32 min, s32 max);
extern s32 D_800FE9B0; // JFG: squad module flags
extern s32 D_800FE9F4; // JFG: a squad timer (240 to 360 frames)

/* JFG: set up the squad lists before the objects are loaded. */
void func_80051420(void) {
    D_800FE9AC = (s32 *) mempool_alloc_safe(0x400, 0xFFFF00FF);
    D_800FE9B0 = 1;
    D_800FE9A4 = NULL;
    D_800FE9A0 = 0;
    D_800FE9F0 = 0;
    D_800FE9F4 = rand_range(0xF0, 0x168);
}

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

s32 func_80053370(s32 module); // JFG: whether a runlink module is loaded
void func_80054530();          // a module function (patched in by the runtime linker)

/* JFG: pass an event to the squad module (3) when it is loaded. */
void func_80052734(s32 a0, s16 a1, s16 a2, s16 a3, u8 a4, u8 a5, u8 a6) {
    if (func_80053370(3)) {
        func_80054530(a0, a1, a2, a3, a4, a5, a6);
    }
}

extern void *D_800FEA08;
extern s32 D_800FEA0C;
extern void *D_800FEA10;
extern s32 D_800FEA14;

/* JFG: keep two objects and their word at 0x68 (zero for none). */
void func_80052798(u8 *a, u8 *b) {
    if ((D_800FEA08 = a) != NULL) {
        D_800FEA0C = *(s32 *) (a + 0x68);
    } else {
        D_800FEA0C = 0;
    }
    if ((D_800FEA10 = b) != NULL) {
        D_800FEA14 = *(s32 *) (b + 0x68);
    } else {
        D_800FEA14 = 0;
    }
}

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

/* JFG: set a module's destruct timer (10 bits) and its other 6 bits. */
void func_800541B0(s32 index, u16 timer, u8 value) {
    D_800FEB58[index].timer = timer;
    D_800FEB58[index].unk = value;
}

INCLUDE_ASM("asm/nonmatchings/51AD0", func_80054204);

INCLUDE_ASM("asm/nonmatchings/51AD0", func_800542C4);

INCLUDE_ASM("asm/nonmatchings/51AD0", func_80054368);
