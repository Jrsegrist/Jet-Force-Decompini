#include "common.h"

INCLUDE_ASM("asm/nonmatchings/66B80", func_80065F80);

INCLUDE_ASM("asm/nonmatchings/66B80", func_80066594);

INCLUDE_ASM("asm/nonmatchings/66B80", func_800665D8);

void func_80066594(void *arg0, s32 *a, s32 *b, s32 *c, s32 *d);

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

INCLUDE_ASM("asm/nonmatchings/66B80", func_80066830);

INCLUDE_ASM("asm/nonmatchings/66B80", func_80066860);

INCLUDE_ASM("asm/nonmatchings/66B80", func_80066A68);
