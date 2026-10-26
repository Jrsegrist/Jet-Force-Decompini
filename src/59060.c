/*
 * Menus. Functions in C are from the Diddy Kong Racing decompilation
 * (CC0 1.0), src/menu.c, or marked "JFG"; see CREDITS.md.
 */
#include "common.h"

extern u8 gLanguage;

INCLUDE_ASM("asm/nonmatchings/59060", func_80058460);

INCLUDE_ASM("asm/nonmatchings/59060", func_80058590);

INCLUDE_ASM("asm/nonmatchings/59060", func_800586E0);

INCLUDE_ASM("asm/nonmatchings/59060", func_8005886C);

INCLUDE_ASM("asm/nonmatchings/59060", func_800589F0);

extern u8 D_800A51B0;

u8 func_80058A5C(void) {
    return D_800A51B0;
}

INCLUDE_ASM("asm/nonmatchings/59060", func_80058A6C);

INCLUDE_ASM("asm/nonmatchings/59060", func_80058C54);

INCLUDE_ASM("asm/nonmatchings/59060", func_80058D5C);

INCLUDE_ASM("asm/nonmatchings/59060", func_80058E78);

INCLUDE_ASM("asm/nonmatchings/59060", func_80058EF0);

INCLUDE_ASM("asm/nonmatchings/59060", func_800595F4);

INCLUDE_ASM("asm/nonmatchings/59060", func_80059814);

INCLUDE_ASM("asm/nonmatchings/59060", func_80059864);

INCLUDE_ASM("asm/nonmatchings/59060", func_80059938);

INCLUDE_ASM("asm/nonmatchings/59060", func_80059A04);

INCLUDE_ASM("asm/nonmatchings/59060", func_80059A98);

INCLUDE_ASM("asm/nonmatchings/59060", func_80059CCC);

INCLUDE_ASM("asm/nonmatchings/59060", func_80059D2C);

INCLUDE_ASM("asm/nonmatchings/59060", func_80059E1C);

INCLUDE_ASM("asm/nonmatchings/59060", func_80059E7C);

INCLUDE_ASM("asm/nonmatchings/59060", func_80059FE8);

INCLUDE_ASM("asm/nonmatchings/59060", func_8005A048);

INCLUDE_ASM("asm/nonmatchings/59060", func_8005A0D0);

INCLUDE_ASM("asm/nonmatchings/59060", func_8005A51C);

s32 func_8005A794(void) {
    return 0;
}

s32 func_8005A79C(void) {
    return 0;
}

INCLUDE_ASM("asm/nonmatchings/59060", func_8005A7A4);

/**
 * Returns the current language being used by the game.
 * JFG: a stored setting (DKR decodes it from the EEPROM settings).
 * Official Name: frontGetLanguage
 */
s32 get_language(void) {
    return gLanguage;
}

INCLUDE_ASM("asm/nonmatchings/59060", func_8005A7E0);

INCLUDE_ASM("asm/nonmatchings/59060", func_8005A804);

INCLUDE_ASM("asm/nonmatchings/59060", func_8005A834);

extern u8 D_800A51A4;
extern u8 D_800A51A8;

/* JFG: remember the current screen mode byte (func_8005A8D8 returns it). */
void func_8005A8C4(void) {
    D_800A51A4 = D_800A51A8;
}

u8 func_8005A8D8(void) {
    return D_800A51A4;
}

INCLUDE_ASM("asm/nonmatchings/59060", func_8005A8E8);

extern s8 D_800FF387;

s8 func_8005A950(void) {
    return D_800FF387;
}

INCLUDE_ASM("asm/nonmatchings/59060", func_8005A960);

extern u8 D_800FF38C;

u8 func_8005A98C(void) {
    return D_800FF38C;
}

INCLUDE_ASM("asm/nonmatchings/59060", func_8005A99C);

extern u16 D_800FF388;

u16 func_8005A9E4(void) {
    return D_800FF388;
}

INCLUDE_ASM("asm/nonmatchings/59060", func_8005A9F4);

extern u16 D_800FF38A;

u16 func_8005AA30(void) {
    return D_800FF38A;
}

INCLUDE_ASM("asm/nonmatchings/59060", func_8005AA40);

INCLUDE_ASM("asm/nonmatchings/59060", func_8005AA7C);

INCLUDE_ASM("asm/nonmatchings/59060", func_8005AA94);

INCLUDE_ASM("asm/nonmatchings/59060", func_8005AAB8);

INCLUDE_ASM("asm/nonmatchings/59060", func_8005AAD0);

extern u8 D_800FF311;

void func_8005AAE8(s32 arg0) {
    D_800FF311 = arg0;
}
