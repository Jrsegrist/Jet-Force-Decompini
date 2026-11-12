#include "common.h"

/* n_audio's maths (built -g -mips2, see the Makefile): a float arc-cosine from the fixed-point table one. */

extern f32 D_800AF980; // .rodata: the fixed-point scale (0x7FFF)
extern f32 D_800AF984; // .rodata
extern f32 D_800AF988; // .rodata

u16 func_800930D0(s16 x); // the fixed-point arc-cosine

f32 func_80092C50(f32 x) {
    s16 fixed;
    u16 angle;

    if (x >= 1.0f) {
        fixed = 0x7FFF;
    } else if (x <= -1.0f) {
        fixed = -0x7FFF;
    } else {
        fixed = x * D_800AF980;
    }
    angle = func_800930D0(fixed);
    return angle * D_800AF984 / D_800AF988;
}
