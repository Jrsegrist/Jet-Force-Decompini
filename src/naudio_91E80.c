#include "common.h"

/* n_audio's maths (built -g -mips2, see the Makefile): a float two-argument arc-tangent, from the arc-cosine. */

extern f32 D_800AF940; // .rodata: pi
extern f32 D_800AF944; // .rodata: pi / 2
extern f32 D_800AF948; // .rodata: -pi / 2
extern f32 D_800AF94C; // .rodata
extern f32 D_800AF950; // .rodata
extern f32 D_800AF954; // .rodata
extern f32 D_800AF958; // .rodata

f32 sqrtf(f32 x);
f32 func_80092C50(f32 x); // the float arc-cosine

f32 func_80091280(f32 y, f32 x) {
    f32 angle = 0.0f;
    f32 len;

    if (y == 0.0f) {
        if (x >= 0.0f) {
            angle = 0.0f;
        } else {
            angle = D_800AF940;
        }
    } else if (x == 0.0f) {
        if (y > 0.0f) {
            angle = D_800AF944;
        } else {
            angle = D_800AF948;
        }
    } else {
        len = sqrtf(y * y + x * x);
        if (y > x) {
            angle = func_80092C50(x / len);
            if (y < 0.0f) {
                angle = D_800AF94C - angle;
            }
        } else {
            angle = D_800AF950 - func_80092C50(y / len);
            if (x < 0.0f) {
                angle = D_800AF954 - angle;
            }
            if (angle < 0.0f) {
                angle = angle + D_800AF958;
            }
        }
    }
    return angle;
}
