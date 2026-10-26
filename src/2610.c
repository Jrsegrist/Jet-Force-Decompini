#include "common.h"

/* The audio manager thread (part of DKR's AMAudioMgr __am). */
extern OSThread sAudioMgrThread;

INCLUDE_ASM("asm/nonmatchings/2610", func_80001A10);

INCLUDE_ASM("asm/nonmatchings/2610", func_80001D7C);

/**
 * From the Diddy Kong Racing decompilation (CC0 1.0), src/audiomgr.c.
 * Official Name: amGo
 */
void audioStartThread(void) {
    osStartThread(&sAudioMgrThread);
}

/**
 * From the Diddy Kong Racing decompilation (CC0 1.0), src/audiomgr.c.
 */
void audioStopThread(void) {
    osStopThread(&sAudioMgrThread);
}

extern s32 D_800A06EC;

s32 func_80001EE8(void) {
    return D_800A06EC;
}

INCLUDE_ASM("asm/nonmatchings/2610", func_80001EF8);

INCLUDE_ASM("asm/nonmatchings/2610", func_80002484);

INCLUDE_ASM("asm/nonmatchings/2610", func_800024C4);

INCLUDE_ASM("asm/nonmatchings/2610", func_80002694);

INCLUDE_ASM("asm/nonmatchings/2610", func_800026CC);

INCLUDE_ASM("asm/nonmatchings/2610", func_800027D8);

extern s32 D_800A06E8;

s32 func_800027FC(void) {
    return D_800A06E8;
}

INCLUDE_ASM("asm/nonmatchings/2610", func_8000280C);
