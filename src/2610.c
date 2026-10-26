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

typedef struct AudioMgrHandle {
    /* 0x00 */ u8 initialised;
    /* 0x04 */ s32 unk4;
    /* 0x08 */ void *unk8;
} AudioMgrHandle;
extern AudioMgrHandle D_800F1B68;
extern u8 D_800F1B78[];
void func_800024C4(void);

void *func_80002694(AudioMgrHandle **out) {
    if (!D_800F1B68.initialised) {
        D_800F1B68.unk4 = 0;
        D_800F1B68.unk8 = D_800F1B78;
        D_800F1B68.initialised = TRUE;
    }
    *out = &D_800F1B68;
    return func_800024C4;
}

INCLUDE_ASM("asm/nonmatchings/2610", func_800026CC);

extern s32 D_800A06DC;
extern s32 D_800A06E4;

void func_800027D8(s32 mode) {
    if ((D_800A06DC = mode) == 1) {
        D_800A06E4 = 0;
    }
}

extern s32 D_800A06E8;

s32 func_800027FC(void) {
    return D_800A06E8;
}

extern s32 D_800A06C0;

s32 func_8000280C(void) {
    return D_800A06C0 == 0;
}
