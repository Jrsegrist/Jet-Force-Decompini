/*
 * Hand-written maths: matrices from transforms and rotations. The ROM's code is hand-written assembly (odd FPRs
 * as singles, no IDO scheduling), so the build keeps it; the NON_MATCHING bodies are C written from it.
 */
#include "common.h"

/* DKR include/structs.h's ObjectTransform (Size 0x18) */
typedef struct ObjectTransform {
    /* 0x00 */ s16 y_rotation;
    /* 0x02 */ s16 x_rotation;
    /* 0x04 */ s16 z_rotation;
    /* 0x06 */ s16 flags;
    /* 0x08 */ f32 scale;
    /* 0x0C */ f32 x_position;
    /* 0x10 */ f32 y_position;
    /* 0x14 */ f32 z_position;
} ObjectTransform;

f32 func_80049C30(s32 angle); /* cosine of a 16-bit angle (hand-written, asm/49800.s) */
f32 func_80049C34(s32 angle); /* sine of a 16-bit angle (hand-written, asm/49800.s) */

/*
 * The C bodies below are functionally equivalent to the hand-written assembly (which IDO can't produce): each
 * product and sum is grouped as the assembly computes it, so the results are bit-identical.
 */

#ifdef NON_MATCHING
/* NON_MATCHING: hand-written assembly; this C gives the same results.
 * DKR: mtxf_from_transform (arguments swapped): scale, then roll, pitch and yaw, then the position. */
void func_80049DC0(ObjectTransform *trans, MtxF *mtx) {
    f32 yRotSine;
    f32 yRotCosine;
    f32 xRotSine;
    f32 xRotCosine;
    f32 zRotSine;
    f32 zRotCosine;
    f32 scale;

    yRotSine = func_80049C34(trans->y_rotation);
    yRotCosine = func_80049C30(trans->y_rotation);
    xRotSine = func_80049C34(trans->x_rotation);
    xRotCosine = func_80049C30(trans->x_rotation);
    zRotSine = func_80049C34(trans->z_rotation);
    zRotCosine = func_80049C30(trans->z_rotation);
    scale = trans->scale;

    (*mtx)[0][0] = ((zRotSine * yRotSine) * xRotSine + zRotCosine * yRotCosine) * scale;
    (*mtx)[0][1] = (zRotSine * xRotCosine) * scale;
    (*mtx)[0][2] = ((zRotSine * yRotCosine) * xRotSine - yRotSine * zRotCosine) * scale;
    (*mtx)[0][3] = 0.0f;
    (*mtx)[1][0] = ((yRotSine * zRotCosine) * xRotSine - zRotSine * yRotCosine) * scale;
    (*mtx)[1][1] = (zRotCosine * xRotCosine) * scale;
    (*mtx)[1][2] = ((zRotCosine * yRotCosine) * xRotSine + zRotSine * yRotSine) * scale;
    (*mtx)[1][3] = 0.0f;
    (*mtx)[2][0] = (yRotSine * xRotCosine) * scale;
    (*mtx)[2][1] = -xRotSine * scale;
    (*mtx)[2][2] = (xRotCosine * yRotCosine) * scale;
    (*mtx)[2][3] = 0.0f;
    (*mtx)[3][0] = trans->x_position;
    (*mtx)[3][1] = trans->y_position;
    (*mtx)[3][2] = trans->z_position;
    (*mtx)[3][3] = 1.0f;
}
#else
INCLUDE_ASM("asm/nonmatchings/4A9C0", func_80049DC0);
#endif

#ifdef NON_MATCHING
/* NON_MATCHING: hand-written assembly; this C gives the same results.
 * func_80049DC0 without the scale. */
void func_80049F08(ObjectTransform *trans, MtxF *mtx) {
    f32 yRotSine;
    f32 yRotCosine;
    f32 xRotSine;
    f32 xRotCosine;
    f32 zRotSine;
    f32 zRotCosine;

    yRotSine = func_80049C34(trans->y_rotation);
    yRotCosine = func_80049C30(trans->y_rotation);
    xRotSine = func_80049C34(trans->x_rotation);
    xRotCosine = func_80049C30(trans->x_rotation);
    zRotSine = func_80049C34(trans->z_rotation);
    zRotCosine = func_80049C30(trans->z_rotation);

    (*mtx)[0][0] = (zRotSine * yRotSine) * xRotSine + zRotCosine * yRotCosine;
    (*mtx)[0][1] = zRotSine * xRotCosine;
    (*mtx)[0][2] = (zRotSine * yRotCosine) * xRotSine - yRotSine * zRotCosine;
    (*mtx)[0][3] = 0.0f;
    (*mtx)[1][0] = (yRotSine * zRotCosine) * xRotSine - zRotSine * yRotCosine;
    (*mtx)[1][1] = zRotCosine * xRotCosine;
    (*mtx)[1][2] = (zRotCosine * yRotCosine) * xRotSine + zRotSine * yRotSine;
    (*mtx)[1][3] = 0.0f;
    (*mtx)[2][0] = yRotSine * xRotCosine;
    (*mtx)[2][1] = -xRotSine;
    (*mtx)[2][2] = xRotCosine * yRotCosine;
    (*mtx)[2][3] = 0.0f;
    (*mtx)[3][0] = trans->x_position;
    (*mtx)[3][1] = trans->y_position;
    (*mtx)[3][2] = trans->z_position;
    (*mtx)[3][3] = 1.0f;
}
#else
INCLUDE_ASM("asm/nonmatchings/4A9C0", func_80049F08);
#endif

#ifdef NON_MATCHING
/* NON_MATCHING: hand-written assembly; this C gives the same results.
 * DKR: mtxf_from_inverse_transform's order (yaw, pitch, roll), with JFG's scale; the position row is the
 * position (already negated by the caller) through the scaled rotation. */
void func_8004A014(ObjectTransform *trans, MtxF *mtx) {
    f32 yRotSine;
    f32 yRotCosine;
    f32 xRotSine;
    f32 xRotCosine;
    f32 zRotSine;
    f32 zRotCosine;
    f32 scale;
    f32 x, y, z;

    yRotSine = func_80049C34(trans->y_rotation);
    yRotCosine = func_80049C30(trans->y_rotation);
    xRotSine = func_80049C34(trans->x_rotation);
    xRotCosine = func_80049C30(trans->x_rotation);
    zRotSine = func_80049C34(trans->z_rotation);
    zRotCosine = func_80049C30(trans->z_rotation);
    scale = trans->scale;

    (*mtx)[0][0] = (yRotCosine * zRotCosine - (yRotSine * xRotSine) * zRotSine) * scale;
    (*mtx)[0][1] = (yRotCosine * zRotSine + (yRotSine * xRotSine) * zRotCosine) * scale;
    (*mtx)[0][2] = (yRotSine * -xRotCosine) * scale;
    (*mtx)[0][3] = 0.0f;
    (*mtx)[1][0] = (-xRotCosine * zRotSine) * scale;
    (*mtx)[1][1] = (xRotCosine * zRotCosine) * scale;
    (*mtx)[1][2] = xRotSine * scale;
    (*mtx)[1][3] = 0.0f;
    (*mtx)[2][0] = (yRotSine * zRotCosine + (yRotCosine * xRotSine) * zRotSine) * scale;
    (*mtx)[2][1] = (yRotSine * zRotSine - (yRotCosine * xRotSine) * zRotCosine) * scale;
    (*mtx)[2][2] = (yRotCosine * xRotCosine) * scale;
    (*mtx)[2][3] = 0.0f;
    (*mtx)[3][3] = 1.0f;

    x = trans->x_position;
    y = trans->y_position;
    z = trans->z_position;
    (*mtx)[3][0] = x * (*mtx)[0][0] + y * (*mtx)[1][0] + z * (*mtx)[2][0];
    (*mtx)[3][1] = x * (*mtx)[0][1] + y * (*mtx)[1][1] + z * (*mtx)[2][1];
    (*mtx)[3][2] = x * (*mtx)[0][2] + y * (*mtx)[1][2] + z * (*mtx)[2][2];
}
#else
INCLUDE_ASM("asm/nonmatchings/4A9C0", func_8004A014);
#endif

#ifdef NON_MATCHING
/* NON_MATCHING: hand-written assembly; this C gives the same results.
 * func_8004A014 without the scale. */
void func_8004A1A0(ObjectTransform *trans, MtxF *mtx) {
    f32 yRotSine;
    f32 yRotCosine;
    f32 xRotSine;
    f32 xRotCosine;
    f32 zRotSine;
    f32 zRotCosine;
    f32 x, y, z;

    yRotSine = func_80049C34(trans->y_rotation);
    yRotCosine = func_80049C30(trans->y_rotation);
    xRotSine = func_80049C34(trans->x_rotation);
    xRotCosine = func_80049C30(trans->x_rotation);
    zRotSine = func_80049C34(trans->z_rotation);
    zRotCosine = func_80049C30(trans->z_rotation);

    (*mtx)[0][0] = yRotCosine * zRotCosine - (yRotSine * xRotSine) * zRotSine;
    (*mtx)[0][1] = yRotCosine * zRotSine + (yRotSine * xRotSine) * zRotCosine;
    (*mtx)[0][2] = yRotSine * -xRotCosine;
    (*mtx)[0][3] = 0.0f;
    (*mtx)[1][0] = -xRotCosine * zRotSine;
    (*mtx)[1][1] = xRotCosine * zRotCosine;
    (*mtx)[1][2] = xRotSine;
    (*mtx)[1][3] = 0.0f;
    (*mtx)[2][0] = yRotSine * zRotCosine + (yRotCosine * xRotSine) * zRotSine;
    (*mtx)[2][1] = yRotSine * zRotSine - (yRotCosine * xRotSine) * zRotCosine;
    (*mtx)[2][2] = yRotCosine * xRotCosine;
    (*mtx)[2][3] = 0.0f;
    (*mtx)[3][3] = 1.0f;

    x = trans->x_position;
    y = trans->y_position;
    z = trans->z_position;
    (*mtx)[3][0] = x * (*mtx)[0][0] + y * (*mtx)[1][0] + z * (*mtx)[2][0];
    (*mtx)[3][1] = x * (*mtx)[0][1] + y * (*mtx)[1][1] + z * (*mtx)[2][1];
    (*mtx)[3][2] = x * (*mtx)[0][2] + y * (*mtx)[1][2] + z * (*mtx)[2][2];
}
#else
INCLUDE_ASM("asm/nonmatchings/4A9C0", func_8004A1A0);
#endif

#ifdef NON_MATCHING
/* NON_MATCHING: hand-written assembly; this C gives the same results.
 * A 4-vector times each row of a matrix: out[i] = in . mtx[i] (the input is read once, before any store). */
void func_8004A2FC(MtxF *mtx, f32 *in, f32 *out) {
    s32 i;
    f32 x, y, z, w;

    x = in[0];
    y = in[1];
    z = in[2];
    w = in[3];
    for (i = 0; i < 4; i++) {
        out[i] = x * (*mtx)[i][0] + y * (*mtx)[i][1] + z * (*mtx)[i][2] + w * (*mtx)[i][3];
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/4A9C0", func_8004A2FC);
#endif
