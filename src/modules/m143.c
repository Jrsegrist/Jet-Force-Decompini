/*
 * Runlink module 143: an enemy kind that flies in a linked group.
 */
#include "module.h"

/* What each member's obj->unk68 points at */
typedef struct M143Member {
    /* 0x00 */ u8 pad0[0x28];
    /* 0x28 */ Object *next;
    /* 0x2C */ u8 pad2C[0xA6 - 0x2C];
    /* 0xA6 */ s16 health;
} M143Member;

/* The group */
typedef struct M143Group {
    /* 0x00 */ u8 count;
    /* 0x01 */ u8 pad1[0x58 - 0x1];
    /* 0x58 */ Object *first;
    /* 0x5C */ u8 pad5C[0x84 - 0x5C];
    /* 0x84 */ s32 sound;
} M143Group;

/* A vertex of the ring */
typedef struct M143Vtx {
    /* 0x00 */ s16 x;
    /* 0x02 */ s16 y;
    /* 0x04 */ s16 z;
    /* 0x06 */ u8 pad6[4];
} M143Vtx;

/* What a ring's obj->unk68 points at */
typedef struct M143Ring {
    /* 0x000 */ u8 pad0[0x300];
    /* 0x300 */ M143Vtx verts[2 * 48]; /* two buffers of 48 */
    /* 0x6C0 */ u8 pad6C0[0x6EC - 0x6C0];
    /* 0x6EC */ s32 radius;
    /* 0x6F0 */ u8 pad6F0[0x6F8 - 0x6F0];
    /* 0x6F8 */ u8 buffer;
    /* 0x6F9 */ u8 hit;
} M143Ring;

extern void sound_stop(s32 handle);
extern Object *func_m003_053C4(void); /* the closest player */
extern f32 func_80097140(f32 x);     /* sqrtf */
extern void func_800013FC(s32 sound, s32 handle, s32 volume);

extern s32 rl_m143_02F74; /* a sound playing */

INCLUDE_ASM("asm/nonmatchings/modules/m143", func_m143_00000);

INCLUDE_ASM("asm/nonmatchings/modules/m143", func_m143_0007C);

INCLUDE_ASM("asm/nonmatchings/modules/m143", func_m143_00280);

INCLUDE_ASM("asm/nonmatchings/modules/m143", func_m143_00548);

/* A ring that grows outwards along the ground and hurts a player it passes */
void func_m143_008C0(Object *obj, s32 speed) {
    M143Ring *ring = obj->unk68;
    M143Vtx *v = &ring->verts[ring->buffer * 48];
    M143Vtx *w = v + 1;
    Object *player = func_m003_053C4();
    s32 angle;
    f32 dx;
    f32 dz;
    f32 d;
    s32 volume;

    for (angle = 0; angle != 0x10A9A; angle += 0xAAA) {
        w->x = func_80049C34((s16) angle) * (f32) (ring->radius + 10);
        w->z = func_80049C30((s16) angle) * (f32) (ring->radius + 10);
        v->x = func_80049C34((s16) angle) * (f32) (ring->radius - 10);
        v->z = func_80049C30((s16) angle) * (f32) (ring->radius - 10);
        w->y = 0;
        v->y = 50;
        w += 2;
        v += 2;
    }
    if (!ring->hit) {
        dx = player->trans.x_position - obj->trans.x_position;
        dz = player->trans.z_position - obj->trans.z_position;
        d = func_80097140(dx * dx + dz * dz) - (f32) ring->radius;
        if (d < 0.0f) {
            d = -d;
        }
        if (d < 30.0f && player->trans.y_position - obj->trans.y_position < 50.0f) {
            ((s16 *) player->interactObj)[3] -= 0x500;
            ring->hit = 1;
        }
    }
    ring->buffer ^= 1;
    ring->radius += speed * 5;
    if (ring->radius > 1300) {
        func_800079B8(obj);
    }
    if (ring->radius >= 434 && ring->radius < 473 && rl_m143_02F74 == 0) {
        func_80001248(0x20F, (s32) &rl_m143_02F74);
    }
    if (rl_m143_02F74 != 0) {
        volume = ring->radius * 80 / 1300 + 47;
        if (volume > 127) {
            volume = 127;
        }
        if (volume < 0) {
            volume = 47;
        }
        func_800013FC(0x20F, rl_m143_02F74, volume & 0xFF);
    }
}

INCLUDE_ASM("asm/nonmatchings/modules/m143", func_m143_00C2C);

INCLUDE_ASM("asm/nonmatchings/modules/m143", func_m143_01940);

INCLUDE_ASM("asm/nonmatchings/modules/m143", func_m143_02394);

/* Now and then one living member of the group, picked at random, makes its sound */
void func_m143_02450(M143Group *group) {
    Object *o;
    Object *pick;
    s32 r;
    s32 i;
    M143Member *m;

    if (group->sound == 0 && func_80048E2C(0, 100) >= 96) {
        o = group->first;
        pick = o;
        r = func_80048E2C(0, group->count);
        for (i = 0; o != NULL; i++) {
            if (i == r) {
                pick = o;
            }
            m = o->unk68;
            o = m->next;
        }
        if (pick != NULL && (m = pick->unk68, m->health > 0)) {
            func_80003410(0x1E3, pick->trans.x_position, pick->trans.y_position, pick->trans.z_position, 4,
                          &group->sound);
        }
    }
}

INCLUDE_ASM("asm/nonmatchings/modules/m143", func_m143_02520);

void func_m143_02B88(void) {
    if (rl_m143_02F74 != 0) {
        sound_stop(rl_m143_02F74);
    }
}
#pragma weak __rl_fini = func_m143_02B88

INCLUDE_ASM("asm/nonmatchings/modules/m143", data);

INCLUDE_ASM("asm/nonmatchings/modules/m143", rodata);

INCLUDE_ASM("asm/nonmatchings/modules/m143", bss);
