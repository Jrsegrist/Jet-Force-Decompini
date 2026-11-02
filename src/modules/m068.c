/*
 * Runlink module 68: a large boss and the objects it uses (still mostly
 * assembly).
 */
#include "module.h"

typedef struct M068Setup {
    LevelObjectEntryCommon common;
    /* 0x0A */ s16 xRotation;
    /* 0x0C */ s16 yRotation;
} M068Setup;

/* The boss's state (obj->unk68), as far as used here */
typedef struct M068State {
    /* 0x00 */ u8 pad0[0x58];
    /* 0x58 */ s32 sound58;
    /* 0x5C */ s32 sound5C;
    /* 0x60 */ u8 pad60[0x6C - 0x60];
    /* 0x6C */ s32 sound6C;
    /* 0x70 */ s32 sound70;
    /* 0x74 */ u8 pad74[0x76 - 0x74];
    /* 0x76 */ s16 hitByPlayer;
} M068State;

/* A piece drawn with its own matrix (obj->unk68) */
typedef struct M068Piece {
    /* 0x00 */ u8 pad0[0x10];
    /* 0x10 */ MtxF *base;
} M068Piece;

extern void func_80003614(s32 handle); /* stops a positioned sound */
extern void func_80049DC0(Object *obj, MtxF *out);
extern void func_80048C7C(MtxF *a, MtxF *b, MtxF *out); /* out = a * b */
extern void mtx_model_push(Gfx **dl, s32 arg1, MtxF *mtx);

/* The module's .data (still assembly): sounds playing */
extern s32 rl_m068_124EC;
extern s32 rl_m068_12600;
extern s32 rl_m068_12614;
extern s32 rl_m068_12618;

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_00000);

void func_m068_00014(Object *obj, M068Setup *setup) {
    obj->trans.rotation.y_rotation = setup->yRotation;
    obj->trans.rotation.x_rotation = setup->xRotation;
}

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_0002C);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_0003C);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_004D8);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_0092C);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_00C70);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_0104C);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_01330);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_01628);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_017EC);

/* Stops the boss's sounds */
void func_m068_023D4(Object *obj) {
    M068State *s = obj->unk68;

    if (s->sound6C != 0) {
        func_80003614(s->sound6C);
    }
    if (s->sound58 != 0) {
        func_80003614(s->sound58);
    }
    if (s->sound70 != 0) {
        func_80003614(s->sound70);
    }
    if (s->sound5C != 0) {
        func_80003614(s->sound5C);
    }
}

void func_m068_0246C(void) {
    if (rl_m068_12600 != 0) {
        func_80003614(rl_m068_12600);
    }
    if (rl_m068_12614 != 0) {
        func_80003614(rl_m068_12614);
    }
    if (rl_m068_12618 != 0) {
        func_80003614(rl_m068_12618);
    }
    if (rl_m068_124EC != 0) {
        func_80003614(rl_m068_124EC);
    }
}
#pragma weak __rl_fini = func_m068_0246C

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_024EC);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_039D4);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_03A78);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_03C70);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_03D58);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_03E24);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_03F0C);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_03FD8);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_040EC);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_0447C);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_04590);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_04600);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_046EC);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_047F8);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_04878);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_048C0);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_04938);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_04A60);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_05028);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_05100);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_053AC);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_054A0);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_05570);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_05668);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_0577C);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_05964);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_059F0);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_05C8C);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_05F60);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_06180);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_06244);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_06310);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_06590);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_067B4);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_06AE0);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_06E14);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_0714C);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_0758C);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_078F4);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_07D68);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_083C8);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_08674);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_0873C);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_087E0);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_088DC);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_09088);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_09218);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_092F0);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_093B8);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_0A208);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_0A5D8);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_0ADC4);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_0B4D0);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_0BB4C);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_0BDC4);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_0BE14);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_0C7DC);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_0C874);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_0C944);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_0CDD8);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_0CF3C);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_0D05C);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_0D3D8);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_0D65C);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_0D828);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_0E148);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_0E520);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_0E668);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_0ED10);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_0EF2C);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_0EF70);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_0F2FC);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_0F478);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_0F7C8);

/* Called when something hits the boss: notes a hit by the player */
void func_m068_10734(Object *hitter, Object *obj, s32 arg2) {
    M068State *s = obj->unk68;

    if (hitter->behaviorId == 1) {
        s->hitByPlayer = 1;
    }
}

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_10758);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_108C8);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_10A28);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_10B58);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_10DD4);

/* Pushes a piece's matrix: its own transform times its base matrix */
void func_m068_10F90(Gfx **dl, s32 arg1, Object *obj) {
    M068Piece *p = obj->unk68;
    MtxF local;
    MtxF out;

    func_80049DC0(obj, &local);
    func_80048C7C(p->base, &local, &out);
    mtx_model_push(dl, arg1, &out);
}

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_10FE8);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_110E0);

INCLUDE_ASM("asm/nonmatchings/modules/m068", data);

INCLUDE_ASM("asm/nonmatchings/modules/m068", rodata);

INCLUDE_ASM("asm/nonmatchings/modules/m068", bss);

