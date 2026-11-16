/* Runlink module 79: a floating object that turns slowly. */
#include "common.h"

typedef struct M079Object {
    u8 pad0[0x28];
    f32 unk28;
} M079Object;

extern void obj_collision_spheres_init(M079Object *, s32, s32, s32, s32, s32, s32, f32, f32, f32, f32, s32);

void func_m079_00000(M079Object *obj, s32 arg1, s32 arg2) {
    obj_collision_spheres_init(obj, -1, 1, 1, 0, 0, 0, 30.0f, 30.0f, -1.0f, 1.0f, 0x11);
}

void func_m079_0006C(M079Object *obj, s32 updateRate) {
    obj->unk28 += updateRate * 8.0f;
}
