#include "common.h"

INCLUDE_ASM("asm/nonmatchings/66B80", func_80065F80);

/* JFG: unpack a two-word command: the top byte, the next byte, the low half and the second word. */
void dl_cmd_unpack_bbh(void *arg0, s32 *a, s32 *b, s32 *c, s32 *d) {
    *a = (((u32 *) arg0)[0] >> 24) & 0xFF;
    *d = (((u32 *) arg0)[0] >> 16) & 0xFF;
    *c = ((u32 *) arg0)[0] & 0xFFFF;
    *b = ((u32 *) arg0)[1];
}

/* JFG: unpack a two-word command: the top byte, the middle half, the low byte and the second word. */
void dl_cmd_unpack_bhb(u32 *arg0, s32 *a, s32 *b, s32 *c, s32 *d) {
    *a = (arg0[0] >> 24) & 0xFF;
    *b = (arg0[0] >> 8) & 0xFFFF;
    *c = arg0[0] & 0xFF;
    *d = arg0[1];
}

s32 func_80066620(void *arg0) {
    s32 a, b, c, d;
    s32 pad[4];
    dl_cmd_unpack_bbh(arg0, &a, &b, &c, &d);
    return 8;
}

s32 func_80066650(void *arg0) {
    s32 a, b, c, d;
    s32 pad[4];
    dl_cmd_unpack_bbh(arg0, &a, &b, &c, &d);
    return 8;
}

s32 func_80066680(void *arg0) {
    s32 a, b, c, d;
    s32 pad[8];
    dl_cmd_unpack_bbh(arg0, &a, &b, &c, &d);
    return 8;
}

s32 func_800666B0(s32 arg0, s32 arg1) {
    return 0x8;
}

s32 func_800666C0(void *arg0) {
    s32 a, b, c, d;
    s32 pad[2];
    dl_cmd_unpack_bbh(arg0, &a, &b, &c, &d);
    return 8;
}

s32 func_800666F0(void *arg0) {
    s32 a, b, c, d;
    s32 pad[2];
    dl_cmd_unpack_bbh(arg0, &a, &b, &c, &d);
    return 8;
}

s32 func_80066720(void *arg0) {
    s32 a, b, c, d;
    s32 pad[2];
    dl_cmd_unpack_bbh(arg0, &a, &b, &c, &d);
    return 8;
}

s32 func_80066750(s32 arg0, s32 arg1, s32 arg2) {
    return 0x8;
}

s32 func_80066764(s32 arg0) {
    return 0x8;
}

s32 func_80066770(s32 arg0, s32 arg1, s32 arg2) {
    return 0x8;
}

/*
 * JFG: 8 bytes long. Only the second argument is spilled: IDO homes the parameters a function never
 * reads, so the first one is tested (the test's body was compiled out).
 */
s32 func_80066784(s32 arg0, UNUSED s32 arg1) {
    if (arg0) {
    }
    return 8;
}

INCLUDE_ASM("asm/nonmatchings/66B80", func_80066790);

extern char D_800AE158[]; // JFG: a format string (.rodata)

/* JFG: format the command's argument into a local buffer (the result is unused); 8 bytes long. */
s32 func_80066830(UNUSED s32 arg0, s32 arg1) {
    char buffer[0x50];
    UNUSED s32 pad[2];

    sprintf(buffer, D_800AE158, arg1);
    return 8;
}

extern char D_800AE174[]; // JFG: a format string (.rodata)

#ifdef NON_MATCHING
/* NON_MATCHING: the ROM keeps the case constants 2 and 3 in registers and the mode in two registers */
/*
 * JFG: the "set other mode" command: tells the render modes (the second word's low half) apart; the
 * prints of their names were compiled out. 8 bytes long.
 */
s32 func_80066860(u32 *cmd, s32 arg1) {
    char buffer[0x50];
    UNUSED s32 pad;
    s32 type;
    s32 w1;

    type = (cmd[0] >> 8) & 0xFF;
    w1 = cmd[1];
    sprintf(buffer, D_800AE174, arg1);
    switch (type) {
    case 0:
    case 2:
        break;
    case 3:
        w1 &= 0xFFFF;
        if (w1 == 0x2078 || w1 == 0x2038 || w1 == 0x49D8 || w1 == 0x2D58 || w1 == 0x2D18 ||
            w1 == 0x4DD8 || w1 == 0x2478 || w1 == 0x45D8 || w1 == 0x7858 || w1 == 0x7F58 ||
            w1 == 0x3078 || w1 == 0x3478 || w1 == 0x2278) {
        } else if (w1 == 0x7B || w1 == 0x2078 || w1 == 0x3078) {
        } else if (w1 == 0x2278 || w1 == 0x2048 || w1 == 0x2008 || w1 == 0x41C8 || w1 == 0x7048 ||
                   w1 == 0x7248 || w1 == 0x3048 || w1 == 0x2248) {
        } else if (w1 == 0x4B || w1 == 0x2048 || w1 == 0x3048) {
        } else if (w1 == 0x2248 || w1 == 0x2230 || w1 == 0x4A50 || w1 == 0x2E10 || w1 == 0x4E50 ||
                   w1 == 0x4B50 || w1 == 0x4F50 || w1 == 0x233 || w1 == 0x4000 || w1 == 0x4240 ||
                   w1 == 0x7008 || w1 == 0x4340) {
        } else if (w1 == 0x4203 || w1 == 0x4340) {
        } else if (w1 == 0) {
        } else if (w1 == 0x4040) {
        } else if (w1 != 0) {
            if (((w1 >> 8) & 3) == 0) {
            } else if (((w1 >> 8) & 3) == 1) {
            }
            if (((w1 >> 10) & 3) == 0) {
            } else if (((w1 >> 10) & 3) == 1) {
            }
        }
        break;
    case 0x10:
        break;
    }
    return 8;
}
#else
INCLUDE_ASM("asm/nonmatchings/66B80", func_80066860);
#endif

extern char D_800AE594[]; // JFG: a format string (.rodata)

#ifdef NON_MATCHING
/* NON_MATCHING: one word, the bit test's operand order (and t6,a3,v0 in the ROM) */
/* JFG: walks the geometry mode bits set in the command's second word (the prints were compiled out). */
s32 func_80066A68(u32 *cmd, s32 arg1, UNUSED s32 arg2) {
    char buffer[0x50];
    UNUSED s32 pad;
    u32 flags;
    s32 bit;
    s32 first;

    bit = 1;
    first = TRUE;
    flags = cmd[1];
    sprintf(buffer, D_800AE594, arg1, flags);
    for (; bit <= 0x100000; bit <<= 1) {
        if (bit & flags) {
            if (first) {
                first = FALSE;
            }
            switch (bit) {
                case 0x1:
                    break;
                case 0x2:
                    break;
                case 0x4:
                    break;
                case 0x200:
                    break;
                case 0x1000:
                    break;
                case 0x2000:
                    break;
                case 0x10000:
                    break;
                case 0x20000:
                    break;
                case 0x40000:
                    break;
                case 0x80000:
                    break;
                case 0x100000:
                    break;
            }
        }
    }
    return 8;
}
#else
INCLUDE_ASM("asm/nonmatchings/66B80", func_80066A68);
#endif
