#include "common.h"

f32 func_80049C30(s32 angle);
f32 func_80049C34(s32 angle);

/* JFG: a perspective matrix like libultra's guPerspectiveF, with the 16-bit angle cosine/sine and the scale
   applied per element. */
void guPerspectiveF(f32 mf[4][4], u16 *perspNorm, f32 fovy, f32 aspect, f32 near, f32 far, f32 scale) {
    UNUSED s32 pad;
    s32 angle;
    f32 cot;
    f32 diff;

    angle = fovy * 91.022f;
    cot = func_80049C30(angle) / func_80049C34(angle);
    diff = near - far;
    mf[0][0] = cot / aspect * scale;
    mf[0][1] = 0.0f;
    mf[0][2] = 0.0f;
    mf[0][3] = 0.0f;
    mf[1][0] = 0.0f;
    mf[1][1] = cot * scale;
    mf[1][2] = 0.0f;
    mf[1][3] = 0.0f;
    mf[2][0] = 0.0f;
    mf[2][1] = 0.0f;
    mf[2][2] = (near + far) / diff * scale;
    mf[2][3] = -1.0f;
    mf[3][0] = 0.0f;
    mf[3][1] = 0.0f;
    mf[3][2] = 2.0f * near * far / diff * scale;
    mf[3][3] = 0.0f;
    if (perspNorm != NULL) {
        if (near + far <= 2.0f) {
            *perspNorm = 0xFFFF;
        } else {
            *perspNorm = (s32) (131072.0f / (near + far));
            if (*perspNorm <= 0) {
                *perspNorm = 1;
            }
        }
    }
}

INCLUDE_ASM("asm/nonmatchings/766C0", func_80075C40);
