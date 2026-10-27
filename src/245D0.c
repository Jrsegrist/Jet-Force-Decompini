#include "common.h"

/* Size: 0x50 bytes */
typedef struct CamLight {
    /* 0x00 */ u8 pad0[6];
    /* 0x06 */ u8 unk6;
    /* 0x07 */ s8 unk7;
    /* 0x08 */ u8 pad8[4];
    /* 0x0C */ f32 pos[3];
    /* 0x18 */ u8 pad18[0x3C - 0x18];
    /* 0x3C */ void *unk3C; // JFG: a sprite
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

void func_80023CB4(CamLight *light);

/* JFG: delete the 50 camera lights. */
void func_80023A00(void) {
    CamLight *light = D_800F5B60;
    s32 i = 50;
    while (i--) {
        func_80023CB4(light);
        light++;
    }
}

INCLUDE_ASM("asm/nonmatchings/245D0", func_80023A48);

extern s32 D_800A1108; // JFG: the number of camera lights in use
extern void *D_800A1110; // JFG: a sprite shared by the camera lights
void sprite_free(void *sprite);

/* JFG: delete a camera light (the shared sprite goes with the last one). */
void func_80023CB4(CamLight *light) {
    if (light != NULL && light->unk6) {
        light->unk6 = 0;
        if (light->unk3C != NULL) {
            sprite_free(light->unk3C);
            light->unk3C = NULL;
        }
        if (--D_800A1108 == 0) {
            if (D_800A1110 != NULL) {
                sprite_free(D_800A1110);
                D_800A1110 = NULL;
            }
        }
    }
}

void func_80023DAC(CamLight *light);

/* JFG: update the active camera lights. */
void func_80023D48(void) {
    CamLight *light;
    s32 i;

    if (D_800A1108 > 0) {
        light = D_800F5B60;
        i = 49;
        do {
            if (light->unk6) {
                func_80023DAC(light);
            }
            light++;
        } while (i--);
    }
}

INCLUDE_ASM("asm/nonmatchings/245D0", func_80023DAC);

/* JFG: what func_80018808 writes back */
typedef struct HitResult {
    /* 0x00 */ f32 unk0[11];
} HitResult;

s32 func_80018808(s32 arg0, f32 *from, f32 *to, HitResult *result, s32 flags, s32 arg5);

/* JFG: fade the light in while nothing is between it and the position, out otherwise. */
void func_80023FA4(f32 *pos, CamLight *light) {
    HitResult result;

    if (func_80018808(0, pos + 3, light->pos, &result, 0x2404, 0) == 0) {
        light->unk7 += 5;
        if (light->unk7 > 16) {
            light->unk7 = 16;
        }
    } else {
        light->unk7 -= 5;
        if (light->unk7 < 0) {
            light->unk7 = 0;
        }
    }
}

INCLUDE_ASM("asm/nonmatchings/245D0", func_80024044);

INCLUDE_ASM("asm/nonmatchings/245D0", func_80024330);

INCLUDE_ASM("asm/nonmatchings/245D0", func_80024430);

INCLUDE_ASM("asm/nonmatchings/245D0", func_80024634);
