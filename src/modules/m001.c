/* Runlink module 1: a pool of 32 small particles (0x80 bytes each). */
#include "common.h"

extern void *func_80056798(s32, s32);
extern void func_80056C14(void *);
extern void *func_80055694(s32);
extern void func_80055A00(void *);

typedef struct M001Data {
    s32 unk0[8];
    void *unk20; /* loaded with func_80056798 */
    void *unk24; /* loaded with func_80056798 */
    void *unk28; /* loaded with func_80055694 */
    s32 count;   /* particles in use */
} M001Data;

static M001Data D_m001_00C30 = {
    { 0x40000103, 0, 0, 0, 0x40000302, 0, 0, 0 },
    NULL,
    NULL,
    NULL,
    0,
};

static u8 D_m001_00C70[0x20][0x80];
static s32 D_m001_01C70; /* set once the resources are loaded */

INCLUDE_ASM("asm/nonmatchings/modules/m001", func_m001_00000);

void func_m001_00120(void) {
    if (D_m001_00C30.unk20 != NULL) {
        func_80056C14(D_m001_00C30.unk20);
    }
    if (D_m001_00C30.unk24 != NULL) {
        func_80056C14(D_m001_00C30.unk24);
    }
    if (D_m001_00C30.unk28 != NULL) {
        func_80055A00(D_m001_00C30.unk28);
    }
    D_m001_01C70 = 0;
}

INCLUDE_ASM("asm/nonmatchings/modules/m001", func_m001_00188);

INCLUDE_ASM("asm/nonmatchings/modules/m001", func_m001_002A0);

INCLUDE_ASM("asm/nonmatchings/modules/m001", func_m001_00600);

INCLUDE_ASM("asm/nonmatchings/modules/m001", func_m001_00678);

INCLUDE_ASM("asm/nonmatchings/modules/m001", func_m001_00978);

/* The module's shutdown function (runlink fini) */
void func_m001_00C0C(void) {
    func_m001_00120();
}
#pragma weak __rl_fini = func_m001_00C0C

INCLUDE_ASM("asm/nonmatchings/modules/m001", rodata);
