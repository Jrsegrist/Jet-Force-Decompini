#include "common.h"

INCLUDE_ASM("asm/nonmatchings/66B80", func_80065F80);

/* JFG: unpack a two-word command: the top byte, the next byte, the low half and the second word. */
void func_80066594(void *arg0, s32 *a, s32 *b, s32 *c, s32 *d) {
    *a = (((u32 *) arg0)[0] >> 24) & 0xFF;
    *d = (((u32 *) arg0)[0] >> 16) & 0xFF;
    *c = ((u32 *) arg0)[0] & 0xFFFF;
    *b = ((u32 *) arg0)[1];
}

/* JFG: unpack a two-word command: the top byte, the middle half, the low byte and the second word. */
void func_800665D8(u32 *arg0, s32 *a, s32 *b, s32 *c, s32 *d) {
    *a = (arg0[0] >> 24) & 0xFF;
    *b = (arg0[0] >> 8) & 0xFFFF;
    *c = arg0[0] & 0xFF;
    *d = arg0[1];
}

s32 func_80066620(void *arg0) {
    s32 a, b, c, d;
    s32 pad[4];
    func_80066594(arg0, &a, &b, &c, &d);
    return 8;
}

s32 func_80066650(void *arg0) {
    s32 a, b, c, d;
    s32 pad[4];
    func_80066594(arg0, &a, &b, &c, &d);
    return 8;
}

s32 func_80066680(void *arg0) {
    s32 a, b, c, d;
    s32 pad[8];
    func_80066594(arg0, &a, &b, &c, &d);
    return 8;
}

s32 func_800666B0(s32 arg0, s32 arg1) {
    return 0x8;
}

s32 func_800666C0(void *arg0) {
    s32 a, b, c, d;
    s32 pad[2];
    func_80066594(arg0, &a, &b, &c, &d);
    return 8;
}

s32 func_800666F0(void *arg0) {
    s32 a, b, c, d;
    s32 pad[2];
    func_80066594(arg0, &a, &b, &c, &d);
    return 8;
}

s32 func_80066720(void *arg0) {
    s32 a, b, c, d;
    s32 pad[2];
    func_80066594(arg0, &a, &b, &c, &d);
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

// Returns 8, but only its second argument is spilled: no C spelling found yet.
INCLUDE_ASM("asm/nonmatchings/66B80", func_80066784);

INCLUDE_ASM("asm/nonmatchings/66B80", func_80066790);

extern char D_800AE158[]; // JFG: a format string (.rodata)

/* JFG: format the command's argument into a local buffer (the result is unused); 8 bytes long. */
s32 func_80066830(UNUSED s32 arg0, s32 arg1) {
    char buffer[0x50];
    UNUSED s32 pad[2];

    sprintf(buffer, D_800AE158, arg1);
    return 8;
}

INCLUDE_ASM("asm/nonmatchings/66B80", func_80066860);

INCLUDE_ASM("asm/nonmatchings/66B80", func_80066A68);
