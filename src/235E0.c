/*
 * Vertex shading routines that 217C0.c's object shading calls. The ROM's code is hand-written assembly, so the
 * build keeps it; the NON_MATCHING bodies are C written from it.
 */
#include "common.h"

INCLUDE_ASM("asm/nonmatchings/235E0", func_800229E0);

INCLUDE_ASM("asm/nonmatchings/235E0", func_80022BC4);

INCLUDE_ASM("asm/nonmatchings/235E0", func_80022E98);

#ifdef NON_MATCHING
/* NON_MATCHING: hand-written assembly; this C gives the same results.
 * A light's 128-entry shade table: entry i holds the colour times (2i + 1), red and blue in one word (masked to
 * their bytes 3 and 1) and green in another (bytes 2 and 0). */
void func_80023170(s32 red, s32 green, s32 blue, u8 *table) {
    u32 *out;
    u32 rb;
    u32 g;
    u32 stepRB;
    u32 stepG;
    s32 i;

    out = (u32 *) table;
    rb = ((u32) red << 16) | (u32) blue;
    g = (u32) green << 8;
    stepRB = rb << 1;
    stepG = g << 1;
    for (i = 0x80; i != 0; i--) {
        *out++ = (rb & 0xFF00FF00) | (g & 0x00FF00FF);
        rb += stepRB;
        g += stepG;
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/235E0", func_80023170);
#endif

INCLUDE_ASM("asm/nonmatchings/235E0", func_800231C8);

INCLUDE_ASM("asm/nonmatchings/235E0", func_8002333C);

#ifdef NON_MATCHING
/* NON_MATCHING: hand-written assembly; this C gives the same results.
 * Move count points (x, y, z) by (dx, dy, dz) and wrap each moved axis into [min, max) by its range. count must
 * be at least 1: like the assembly, the loop counts down to zero after the first point. */
void func_800234D0(s32 count, f32 *pos, f32 dx, f32 dy, f32 dz, f32 minX, f32 maxX, f32 rangeX, f32 minY, f32 maxY,
                   f32 rangeY, f32 minZ, f32 maxZ, f32 rangeZ) {
    s32 moveX;
    s32 moveY;
    s32 moveZ;
    f32 v;

    moveX = dx != 0.0f;
    moveY = dy != 0.0f;
    moveZ = dz != 0.0f;
    do {
        if (moveX) {
            v = pos[0] + dx;
            if (v < minX) {
                v += rangeX;
            } else if (!(v < maxX)) {
                v -= rangeX;
            }
            pos[0] = v;
        }
        if (moveY) {
            v = pos[1] + dy;
            if (v < minY) {
                v += rangeY;
            } else if (!(v < maxY)) {
                v -= rangeY;
            }
            pos[1] = v;
        }
        if (moveZ) {
            v = pos[2] + dz;
            if (v < minZ) {
                v += rangeZ;
            } else if (!(v < maxZ)) {
                v -= rangeZ;
            }
            pos[2] = v;
        }
        pos += 3;
    } while (--count != 0);
}
#else
INCLUDE_ASM("asm/nonmatchings/235E0", func_800234D0);
#endif

INCLUDE_ASM("asm/nonmatchings/235E0", func_80023600);

INCLUDE_ASM("asm/nonmatchings/235E0", func_800237D8);
