/*
 * Runlink module 30: builds the collision data of an object's models. For
 * the model the object's header names (or every model it has), it allocates
 * one 32 KiB block, fills it in, then shrinks the block to what was used and
 * keeps it in the model instance or in the model itself.
 */
#include "module.h"

typedef struct M030Header {
    /* 0x00 */ u8 pad0[0x1F];
    /* 0x1F */ s8 numberOfModelIds;
    /* 0x20 */ u8 pad20[0x71 - 0x20];
    /* 0x71 */ s8 modelIndex; /* the model to set up, or -1 for all of them */
} M030Header;

/* The collision data: a header followed by its tables in one block */
typedef struct M030Hit {
    /* 0x00 */ void *unk0;
    /* 0x04 */ void *unk4;
    /* 0x08 */ void *unk8;
    /* 0x0C */ void *unkC;
    /* 0x10 */ s16 unk10;
    /* 0x12 */ s16 unk12;
} M030Hit;

typedef struct M030Model {
    /* 0x00 */ u8 pad0[0x14];
    /* 0x14 */ s16 unk14;
    /* 0x16 */ u8 pad16[0x4E - 0x16];
    /* 0x4E */ s8 unk4E;
    /* 0x4F */ u8 pad4F[0x60 - 0x4F];
    /* 0x60 */ M030Hit *hit;
} M030Model;

typedef struct M030ModelInstance {
    /* 0x00 */ M030Model *model;
    /* 0x04 */ u8 pad4[0xC - 0x4];
    /* 0x0C */ M030Hit *hit;
} M030ModelInstance;

extern void *func_8004A60C(s32 size, s32 colour); /* allocates */
extern void func_8004AA80(void *data);            /* frees */
extern void *func_8004A828(s32 size, void *address, s32 colour); /* allocates at an address */
extern u8 *func_8004AFE0(u8 *address);            /* aligns */
extern s32 mempool_free_timer_get(s32 size);
extern void func_8004AA08(s32 state);

extern void rl_m030_00288(M030ModelInstance *inst, M030Model *model, M030Hit *hit);
extern void rl_m030_00C14(M030ModelInstance *inst, M030Model *model, M030Hit *hit);
extern void rl_m030_01508(M030ModelInstance *inst, M030Model *model, M030Hit *hit);

#ifdef NON_MATCHING
static void model_hit_build(M030ModelInstance *inst, M030Model *model, M030Hit **slot);
#else
extern void rl_m030_000F4(M030ModelInstance *inst, M030Model *model, M030Hit **slot);
#define model_hit_build rl_m030_000F4
#endif

void func_m030_00000(Object *obj, s32 flags) {
    M030ModelInstance *inst;
    M030Model *model;
    s32 i;

    if (((M030Header *) obj->header)->modelIndex >= 0) {
        inst = ((M030ModelInstance **) obj->modelInstances)[((M030Header *) obj->header)->modelIndex];
        model = inst->model;
        if (flags & 4) {
            model_hit_build(inst, model, &inst->hit);
        } else {
            model_hit_build(inst, model, &model->hit);
        }
        return;
    }
    for (i = 0; i < ((M030Header *) obj->header)->numberOfModelIds; i++) {
        inst = ((M030ModelInstance **) obj->modelInstances)[i];
        model = inst->model;
        if (flags & 4) {
            model_hit_build(inst, model, &inst->hit);
        } else {
            model_hit_build(inst, model, &model->hit);
        }
    }
}

#ifdef NON_MATCHING
/* Register choice: the ROM keeps ptr in a3 (as end), this gives v1 */
static void model_hit_build(M030ModelInstance *inst, M030Model *model, M030Hit **slot) {
    M030Hit *block;
    s32 n;
    u8 *end;
    s32 p;
    M030Hit *hit;
    u8 *ptr;
    s32 state;

    if (model->unk4E == 0 && *slot == NULL) {
        block = func_8004A60C(0x8000, 0xFF0000FF);
        if (block != NULL) {
            p = (s32) block;
            n = 0x2000;
            while (n != 0) {
                n--;
                *(u32 *) p = 0;
                p += 4;
            }
            *slot = block;
            block->unk0 = (u8 *) block + 0x14;
            ptr = (u8 *) block + model->unk14 * 14;
            ptr = func_8004AFE0(ptr + 0x14);
            (*slot)->unk4 = ptr;
            ptr += model->unk14 * 4;
            (*slot)->unk8 = ptr;
            end = func_8004AFE0(ptr + model->unk14);
            rl_m030_00C14(inst, model, *slot);
            hit = *slot;
            hit->unkC = end;
            p = hit->unk10 + hit->unk12;
            end += p * 16;
            if (end - (u8 *) block > 0x8000) {
                func_8004AA80(*slot);
                *slot = NULL;
                return;
            }
            state = mempool_free_timer_get(end - (u8 *) block);
            func_8004AA08(0);
            func_8004AA80(block);
            func_8004A828(end - (u8 *) block, block, 0xFF0000FF);
            func_8004AA08(state);
            rl_m030_00288(inst, model, *slot);
            rl_m030_01508(inst, model, *slot);
        }
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m030", func_m030_000F4);
#endif

INCLUDE_ASM("asm/nonmatchings/modules/m030", func_m030_00288);

INCLUDE_ASM("asm/nonmatchings/modules/m030", func_m030_00C14);

INCLUDE_ASM("asm/nonmatchings/modules/m030", func_m030_01134);

INCLUDE_ASM("asm/nonmatchings/modules/m030", func_m030_012F0);

INCLUDE_ASM("asm/nonmatchings/modules/m030", func_m030_01508);

INCLUDE_ASM("asm/nonmatchings/modules/m030", func_m030_018D0);
