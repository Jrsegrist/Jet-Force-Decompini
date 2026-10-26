/* Runlink module 45: the 6105 CIC challenge exchanged over the serial interface. */
#include "common.h"

extern u8 *D_800A3324; /* 64-byte serial interface buffer */
extern u8 D_80105340;  /* last controller command */

extern void func_8009A7C0(void);
extern void func_8009A804(void);
extern void func_8009A830(s32, void *);
extern void func_80098050(void *, s32);
extern void func_800982B0(void *, s32);

void func_m045_00000(u8 *data, s32 arg1) {
    u32 i;
    u8 *buf = D_800A3324;

    for (i = 0; i < 0x2E;) {
        buf[i++] = 0xFF;
    }
    buf[0x2E] = 0xF;
    buf[0x2F] = 0xF;
    for (i = 0; i < 0x10; i++) {
        buf[0x30 + i] = data[i];
    }
    func_8009A7C0();
    D_80105340 = 0xFE;
    func_80098050(buf, 0x40);
    func_8009A830(1, buf);
    func_8009A804();
}

void func_m045_000C8(s32 arg0) {
    func_8009A7C0();
    D_80105340 = 0xFE;
    func_800982B0(D_800A3324, 0x40);
    func_8009A830(0, D_800A3324);
    func_8009A804();
}

void func_m045_0011C(u8 *out) {
    u8 *buf = D_800A3324;
    s32 i;

    for (i = 0; i < 0x10; i++) {
        out[i] = buf[0x30 + i];
    }
}
