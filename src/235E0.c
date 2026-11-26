/*
 * Vertex shading routines that 217C0.c's object shading calls. The ROM's code is hand-written assembly, so the
 * build keeps it; the NON_MATCHING bodies are C written from it.
 */
#include "common.h"

#ifdef NON_MATCHING
/* JFG: 217C0.c's light */
typedef struct DirLight {
    /* 0x00 */ f32 dir[3];
    /* 0x0C */ s32 unkC;
    /* 0x10 */ s32 unk10;
    /* 0x14 */ u8 shift;
    /* 0x15 */ u8 max;
    /* 0x16 */ u8 range;
    /* 0x17 */ u8 min;
    /* 0x18 */ u8 *table;
    /* 0x1C */ s16 angle1;
    /* 0x1E */ s16 angle0;
} DirLight;

f32 sqrtf(f32 x);
f32 func_80049C30(s32 angle); // cosine of a 16-bit angle
f32 func_80049C34(s32 angle); // sine of a 16-bit angle
void func_80023600(u16 *image, s32 width, s32 height, s32 x0, s32 y0, s32 x1, s32 y1, u32 colour);

/* The assembly's cvt.w.s: round to the nearest integer, ties to even (the FPU's rounding mode at boot). For values
 * well inside the s32 range, which is all these routines convert. */
static s32 round_to_s32(f32 x) {
    s32 i;
    f32 frac;

    i = (s32) x;
    frac = x - (f32) i;
    if (frac > 0.5f || (frac == 0.5f && (i & 1))) {
        i++;
    } else if (frac < -0.5f || (frac == -0.5f && (i & 1))) {
        i--;
    }
    return i;
}

/* The assembly's unaligned big-endian loads and stores (lwl/lwr, ldl/ldr, swl/swr). */
static u32 read_be32(u8 *p) {
    return ((u32) p[0] << 24) | ((u32) p[1] << 16) | ((u32) p[2] << 8) | p[3];
}

static void write_be32(u8 *p, u32 v) {
    p[0] = v >> 24;
    p[1] = v >> 16;
    p[2] = v >> 8;
    p[3] = v;
}
#endif

#ifdef NON_MATCHING
/* NON_MATCHING: hand-written assembly in the ROM; verified against it by simulation.
 * JFG: one light's diffuse shading of a model's vertex colours. The command stream is groups of a count byte (0
 * ends it) and a light-direction index into buf (s8 x, y, z at buf + 3 * index), then count unaligned big-endian
 * words: a byte offset into normals (top 16 bits; s8 x, y, z then the base colour's r, g, b), the number of extra
 * vertices (bits 12-15) and a vertex index (bits 0-11), followed by the extra indices packed 12 bits each. Every
 * listed vertex (10 bytes each) gets the base colour scaled by the light level at its bytes 6-9, alpha 0xFF. */
void func_800229E0(void *arg0, void *arg1, DirLight *light, void *buf, void *arg4) {
    u8 *vertices;
    u8 *cmd;
    s8 *dirs;
    u8 *normals;
    s8 *dir;
    s8 *normal;
    s32 low;
    s32 dotMax;
    s32 high;
    s32 scale;
    s32 left;
    s32 lx;
    s32 ly;
    s32 lz;
    s32 dot;
    s32 level;
    s32 extra;
    u32 word;
    u32 base;
    u32 colour;
    u32 packedHi;
    u32 packedLo;

    vertices = arg0;
    cmd = arg1;
    dirs = buf;
    normals = arg4;
    low = light->min;
    dotMax = light->unkC;
    high = light->max;
    scale = light->unk10;
    left = -1; // read a group header first
    while (TRUE) {
        if (left <= 0) {
            left = cmd[0];
            if (left == 0) {
                return;
            }
            dir = &dirs[cmd[1] * 3];
            cmd += 2;
            lx = dir[0];
            ly = dir[1];
            lz = dir[2];
        }
        word = read_be32(cmd);
        left--;
        normal = (s8 *) &normals[word >> 16];
        dot = lx * normal[0] + ly * normal[1] + lz * normal[2];
        base = read_be32((u8 *) normal + 2);
        if (dot < 0) {
            level = low;
        } else if (dotMax < dot) {
            level = high;
        } else {
            level = ((dot * scale) >> 14) + low;
        }
        colour = (((base & 0xFF00FF) * level) & 0xFF00FF00) | (((base & 0xFF00) * level) & 0xFF0000) | 0xFF;
        write_be32(&vertices[(word & 0xFFF) * 10 + 6], colour);
        extra = (word >> 12) & 0xF;
        if (extra == 0) {
            cmd += 4;
            continue;
        }
        /* Like the assembly, only the first 64 bits are read (here as two words): indices after the fifth come out
         * as 0. */
        packedHi = read_be32(cmd + 4);
        packedLo = read_be32(cmd + 8);
        cmd += (extra * 12 + 39) >> 3;
        do {
            write_be32(&vertices[(packedHi >> 20) * 10 + 6], colour);
            packedHi = (packedHi << 12) | (packedLo >> 20);
            packedLo <<= 12;
        } while (--extra != 0);
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/235E0", func_800229E0);
#endif

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

#ifdef NON_MATCHING
/* NON_MATCHING: hand-written assembly in the ROM; verified against it by simulation.
 * JFG: turn count light directions (the first three floats of each 0x20-byte light) into each of n matrices'
 * space, scaled to length 127, as s8 x, y, z in buf: light i, matrix j at buf + (i * n + j) * 3. A zero direction
 * gives (127, 0, 0); one whose squared length is exactly 127 is used unscaled. count and n must be at least 1 when
 * the other is not 0, as the assembly counts both down to zero. */
void func_800231C8(s32 count, void *lights, s32 n, MtxF *matrix, void *buf) {
    f32 *light;
    u8 *column;
    u8 *out;
    s32 stride;
    s32 i;
    f32 x;
    f32 y;
    f32 z;
    f32 a;
    f32 b;
    f32 c;
    f32 lengthSq;
    f32 scale;

    stride = n * 3;
    column = buf;
    if (count == 0 || n == 0) {
        return;
    }
    do {
        light = lights;
        out = column;
        i = count;
        do {
            x = light[0];
            y = light[1];
            z = light[2];
            light += 8;
            a = x * (*matrix)[0][0] + y * (*matrix)[0][1] + z * (*matrix)[0][2];
            b = x * (*matrix)[1][0] + y * (*matrix)[1][1] + z * (*matrix)[1][2];
            c = x * (*matrix)[2][0] + y * (*matrix)[2][1] + z * (*matrix)[2][2];
            lengthSq = a * a + b * b + c * c;
            if (lengthSq <= 0.0f) {
                out[0] = 0x7F;
                out[1] = 0;
                out[2] = 0;
            } else {
                if (lengthSq != 127.0f) {
                    scale = 127.0f / sqrtf(lengthSq);
                    a *= scale;
                    b *= scale;
                    c *= scale;
                }
                out[0] = round_to_s32(a);
                out[1] = round_to_s32(b);
                out[2] = round_to_s32(c);
            }
            out += stride;
        } while (--i != 0);
        matrix++;
        column += 3;
    } while (--n != 0);
}
#else
INCLUDE_ASM("asm/nonmatchings/235E0", func_800231C8);
#endif

#ifdef NON_MATCHING
/* NON_MATCHING: hand-written assembly in the ROM; verified against it by simulation.
 * JFG: environment-mapped texture coordinates. The command stream is groups of a count byte (0 ends it), a
 * direction index into dirs (s8 x, y, z at dirs + 3 * index) and a u16 of flags and scales (bit 15: s from x
 * instead of y; bit 14: t from z instead of y; bits 7-13 and 0-6: the s and t scale shifts), then count entries:
 * an s16 byte offset into normals (s8 x, y, z) and a list of s16 byte offsets into vertices, each with bit 0 set
 * when another follows. Each listed vertex gets one word, s in the top half and t in the bottom: for the normal n
 * and direction l, (n.l) * n - l * 0x2000 on the chosen axis, shifted down and centred. */
void func_8002333C(void *arg0, void *arg1, void *arg2, void *buf) {
    u8 *vertices;
    u8 *cmd;
    s8 *normals;
    s8 *dir;
    s8 *normal;
    u32 count;
    u32 flags;
    s32 shiftS;
    s32 shiftT;
    u32 centreS;
    u32 centreT;
    s32 lx;
    s32 ly;
    s32 lz;
    s32 nx;
    s32 ny;
    s32 nz;
    s32 dot;
    u32 s;
    u32 t;
    u32 word;
    s32 offset;

    vertices = arg0;
    cmd = arg1;
    normals = arg2;
    while ((count = cmd[0]) != 0) {
        dir = (s8 *) buf + cmd[1] * 3;
        flags = *(u16 *) (cmd + 2);
        shiftS = (flags >> 7) & 0x7F;
        shiftT = flags & 0x7F;
        /* The assembly's sllv/srlv use the low 5 bits of the shift. */
        centreS = 0x20U << (shiftS & 31);
        centreT = 0x20U << (shiftT & 31);
        shiftS = (16 - shiftS) & 31;
        shiftT = (16 - shiftT) & 31;
        lx = dir[0];
        ly = dir[1];
        lz = dir[2];
        cmd += 4;
        do {
            normal = normals + *(s16 *) cmd;
            cmd += 2;
            nx = normal[0];
            ny = normal[1];
            nz = normal[2];
            dot = nx * lx + ny * ly + nz * lz;
            if (!(flags & 0x8000)) {
                s = dot * ny - ly * 0x2000;
            } else {
                s = dot * nx - lx * 0x2000;
            }
            s = (s >> shiftS) + centreS;
            if (!(flags & 0x4000)) {
                t = dot * ny - ly * 0x2000;
            } else {
                t = dot * nz - lz * 0x2000;
            }
            t = (t >> shiftT) + centreT;
            word = (s << 16) | t;
            do {
                offset = *(s16 *) cmd;
                cmd += 2;
                *(u32 *) (vertices + (offset & ~1)) = word;
            } while (offset & 1);
        } while (--count != 0);
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/235E0", func_8002333C);
#endif

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

#ifdef NON_MATCHING
/* NON_MATCHING: hand-written assembly in the ROM; verified against it by simulation.
 * JFG: draw a line from (x0, y0) to (x1, y1) into a width x height RGBA5551 image, nothing if an end is outside it.
 * colour's bytes from the top are red, green, blue and a blend shift k: 0 draws the colour, otherwise each pixel
 * becomes (old * (2^k - 1) + colour) >> k per channel (red and blue added in one word, as the assembly does). */
void func_80023600(u16 *image, s32 width, s32 height, s32 x0, s32 y0, s32 x1, s32 y1, u32 colour) {
    u32 redBlue;
    u32 green;
    u32 pixel;
    u32 rb;
    u32 g;
    s32 shift;
    s32 dx;
    s32 dy;
    s32 major;
    s32 minor;
    s32 error;
    s32 n;
    s32 step;
    s32 side;
    u16 *p;

    if ((u32) x0 >= (u32) width || (u32) y0 >= (u32) height || (u32) x1 >= (u32) width ||
        (u32) y1 >= (u32) height) {
        return;
    }
    redBlue = ((((colour >> 24) & 0xFF) << 8) | (((colour >> 8) & 0xFF) >> 2)) & 0xF83E;
    green = (((colour >> 16) & 0xFF) << 3) & 0x7C0;
    dx = x1 - x0;
    if (dx < 0) {
        dx = x0 - x1;
    }
    dy = y1 - y0;
    if (dy < 0) {
        dy = y0 - y1;
    }
    if (dy < dx) {
        major = dx;
        minor = dy;
        step = 1;
        side = width;
        if (x0 < x1) {
            if (y1 < y0) {
                side = -side;
            }
        } else {
            if (y0 < y1) {
                side = -side;
            }
            x0 = x1;
            y0 = y1;
        }
    } else {
        major = dy;
        minor = dx;
        step = width;
        side = 1;
        if (y0 < y1) {
            if (x1 < x0) {
                side = -1;
            }
        } else {
            if (x0 < x1) {
                side = -1;
            }
            x0 = x1;
            y0 = y1;
        }
    }
    p = image + y0 * width + x0;
    n = major;
    error = -((major + 1) >> 1); // the assembly's (-major) >> 1
    shift = colour & 0xFF;
    if (shift == 0) {
        pixel = redBlue | green | 1;
        *p = pixel;
        while (TRUE) {
            error += minor;
            n--;
            p += step;
            if (error >= 0) {
                error -= major;
                p += side;
            }
            if (n < 0) {
                break;
            }
            *p = pixel;
        }
    } else {
        /* The assembly's sllv/srlv use the low 5 bits of the shift. */
        shift &= 31;
        do {
            error += minor;
            n--;
            rb = *p & 0xF83E;
            rb = (rb << shift) - rb;
            g = *p & 0x7C0;
            g = (g << shift) - g;
            rb += redBlue;
            g += green;
            rb >>= shift;
            g >>= shift;
            *p = (rb & 0xF83E) | (g & 0x7C0) | 1;
            p += step;
            if (error >= 0) {
                error -= major;
                p += side;
            }
        } while (n >= 0);
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/235E0", func_80023600);
#endif

#ifdef NON_MATCHING
/* NON_MATCHING: hand-written assembly in the ROM; verified against it by simulation.
 * JFG: draw count streaks into a width x height image with func_80023600: each runs from a point (x, y, z) to the
 * point moved by -2 * (offsetX, offsetY, offsetZ), both turned by angle about the y axis and projected onto the
 * image centre with -scale / depth (depth = z cos - x sin); a streak with either end at a depth of -1 or more
 * is skipped. colours holds one colour word per point. count must be at least 1. */
void func_800237D8(u16 *image, s32 width, s32 height, s32 count, f32 *points, u32 *colours, s32 angle, f32 offsetX,
                   f32 offsetY, f32 offsetZ, f32 scale) {
    f32 sine;
    f32 cosine;
    f32 moveX;
    f32 moveY;
    f32 moveZ;
    f32 centreX;
    f32 centreY;
    f32 negScale;
    f32 x;
    f32 y;
    f32 z;
    f32 depth0;
    f32 depth1;
    f32 proj0;
    f32 proj1;

    sine = func_80049C34(angle);
    cosine = func_80049C30(angle);
    moveX = offsetX * -2.0f;
    moveY = offsetY * -2.0f;
    moveZ = offsetZ * -2.0f;
    centreX = width >> 1;
    centreY = height >> 1;
    negScale = -scale;
    do {
        x = points[0];
        y = points[1];
        z = points[2];
        points += 3;
        depth0 = z * cosine - x * sine;
        if (depth0 < -1.0f) {
            depth1 = (z + moveZ) * cosine - (x + moveX) * sine;
            if (depth1 < -1.0f) {
                proj0 = negScale / depth0;
                proj1 = negScale / depth1;
                func_80023600(image, width, height, round_to_s32(centreX + (x * cosine + z * sine) * proj0),
                              round_to_s32(centreY - y * proj0),
                              round_to_s32(centreX + ((x + moveX) * cosine + (z + moveZ) * sine) * proj1),
                              round_to_s32(centreY - (y + moveY) * proj1), *colours);
            }
        }
        colours++;
    } while (--count != 0);
}
#else
INCLUDE_ASM("asm/nonmatchings/235E0", func_800237D8);
#endif
