/*
 * Runlink module 109: an energy shot drawn as a textured quad. It flies
 * along its launch direction with a coloured light; an enemy's shot plays a
 * fly-by sound when it passes a player. When it hits something it can bounce
 * off (reflected about the surface normal), split into smaller shots fanned
 * around the normal, or stop and play the surface's hit sound and particles.
 */
#include "module.h"
#include "gfx.h"

/* The level entry; split shots are spawned from one of these */
typedef struct M109Setup {
    /* 0x00 */ s16 objectID;
    /* 0x02 */ u8 size;
    /* 0x03 */ u8 unk3;
    /* 0x04 */ s16 x;
    /* 0x06 */ s16 y;
    /* 0x08 */ s16 z;
    /* 0x0A */ u8 strength;
    /* 0x0B */ u8 lifetime; /* in seconds / 10 */
    /* 0x0C */ s16 yRotation;
    /* 0x0E */ s16 xRotation;
    /* 0x10 */ u8 splitCount;
    /* 0x11 */ u8 bounces;
    /* 0x12 */ s8 unk12;
    /* 0x13 */ s8 colour; /* -1: by strength */
    /* 0x14 */ f32 speed;
    /* 0x18 */ Object *shooter;
    /* 0x1C */ s32 *hitParticles; /* per surface */
    /* 0x20 */ u16 *hitSounds;    /* per surface */
} M109Setup;

typedef struct M109Shot {
    /* 0x00 */ Triangle tris[2];
    /* 0x20 */ u8 pad20[0x5C - 0x20];
    /* 0x5C */ s32 hitFlags;
    /* 0x60 */ u8 hit;
    /* 0x61 */ u8 strength;
    /* 0x62 */ u8 splitCount;
    /* 0x63 */ u8 bounces;
    /* 0x64 */ f32 speed;
    /* 0x68 */ u8 pad68[2];
    /* 0x6A */ s16 timer;
    /* 0x6C */ f32 scale;
    /* 0x70 */ Object *shooter;
    /* 0x74 */ s32 *hitParticles;
    /* 0x78 */ u16 *hitSounds;
    /* 0x7C */ s8 unk7C;
    /* 0x7D */ s8 flybyPlayed;
    /* 0x80 */ f32 distance; /* flown */
} M109Shot;

typedef struct M109State {
    /* 0x0 */ s32 light;
    /* 0x4 */ s32 lightColour; /* RGBx */
} M109State;

typedef struct M109Interaction {
    /* 0x00 */ u8 pad0[0xC];
    /* 0x0C */ Vec3f oldPos;
} M109Interaction;

typedef struct M109Hit {
    /* 0x00 */ u8 pad0[0x4];
    /* 0x04 */ Vec3f pos;
    /* 0x10 */ Vec3f normal;
    /* 0x1C */ u8 pad1C[0x24 - 0x1C];
    /* 0x24 */ s32 flags;
    /* 0x28 */ s32 unk28;
} M109Hit;

/* A model's texture size */
typedef struct M109Texture {
    /* 0x0 */ u8 width;
    /* 0x1 */ u8 height;
} M109Texture;

/* What func_8004470C returns */
typedef struct M109Env {
    /* 0x00 */ u8 pad0[0xE2];
    /* 0xE2 */ u8 surface; /* of the last hit */
} M109Env;

/* The object's f32 at 0x34 */
#define OBJ_F34(obj) (*(f32 *) &(obj)->pad2C[0x34 - 0x2C])

extern void runlink_unload_timer_set(s32, s32, s32);
extern f32 *func_80013E0C(f32 x, f32 y, f32 z, f32 arg3, f32 arg4, s32 r, s32 g, s32 b); /* adds a light */
extern void track_light_off(f32 *light);
extern s32 func_80009A04(Object *obj, f32 dx, f32 dy, f32 dz); /* DKR objMoveXYZ */
extern void enemy_module_notify(Object *obj, s16 x, s16 y, s16 z, s32, s32, s32);
extern void func_8005F5AC(Object *obj, s32 updateRate);
extern Object **func_8000E464(s32 *count); /* the players */
extern s32 func_80049C84(f32, f32);         /* the angle of a vector */
extern f32 sqrtf(f32);
extern Object *func_80005F14(M109Setup *entry, s32 arg1); /* DKR spawn_object */
extern M109Env *func_8004470C(void);
extern void func_80003614(s32 sound);                      /* stops a sound */
extern void func_800035E8(s32 sound, f32 x, f32 y, f32 z); /* moves a sound */

/*
 * NON_MATCHING: func_m109_00B44 (fini), .data and .rodata match. In the init
 * the original keeps the hit box size in its stack slot (0x4C) rather than a
 * register, which shifts the temporaries (a1/a2/a3); in the control function
 * the fly-by loop has s0/s1 swapped and the bounce's spilled temporaries sit
 * at different offsets (0x54/0x5C, 0x40/0x44). The default build uses the
 * assembly for those two and for .rodata.
 */
s32 D_m109_00B90 = 109; /* the module's number */
static s32 sLightColours[4] = { 0x0040FF00, 0x00FF4000, 0xFF400000, 0xFFE00000 };
static s32 sStrengthColours[4] = { 0x0080FF00, 0xC040FF00, 0xFF401000, 0x40FF0000 };
static f32 sStrengthScales[4] = { 4.0f, 5.5f, 7.0f, 8.5f };
/* the split shots' directions: n shots use the four entries from n * 4 */
static s16 sSplitYRotations[16] = {
    0, 0, 0, 0, -0x1000, 0x1000, 0, 0, -0x1000, 0, 0x1000, 0, -0x1000, 0, 0x1000, 0,
};
static s16 sSplitXRotations[16] = {
    0, 0, 0, 0, 0, 0, 0, 0, -0x1000, 0x1000, -0x1000, 0, 0, 0x1000, 0, -0x1000,
};
static s32 sHitSound = 0;
static s32 sFlybySound = 0;
static Object *sFlybyShot = NULL;

#ifdef NON_MATCHING
void func_m109_00000(Object *obj, M109Setup *setup) {
    s32 pad0;
    s32 pad1;
    s32 hitSize;
    s32 pad2;
    M109Shot *b;
    M109Texture *tex;
    s32 w;
    s32 h;

    obj->velocity.x = 0.0f;
    obj->velocity.y = 0.0f;
    b = obj->unk68;
    obj->velocity.z = -setup->speed;
    obj->trans.rotation.y_rotation = setup->yRotation;
    obj->trans.rotation.x_rotation = setup->xRotation;
    obj->trans.rotation.z_rotation = 0;
    func_8004944C(&obj->trans.rotation, &obj->velocity);
    obj->modelIndex = setup->strength >> 2;
    if (obj->modelIndex >= obj->header->numberOfModelIds) {
        obj->modelIndex = obj->header->numberOfModelIds - 1;
    }
    tex = (M109Texture *) obj->modelInstances[obj->modelIndex];
    w = tex->width - 1;
    h = tex->height - 1;
    b->tris[0].flags = 0x40;
    b->tris[0].vi0 = 0;
    b->tris[0].uv[0][0] = w << 5;
    b->tris[0].uv[0][1] = 0;
    b->tris[0].vi1 = 1;
    b->tris[0].uv[1][0] = w << 5;
    b->tris[0].uv[1][1] = h << 5;
    b->tris[0].vi2 = 2;
    b->tris[0].uv[2][0] = 0;
    b->tris[0].uv[2][1] = 0;
    b->tris[1].flags = 0x40;
    b->tris[1].vi0 = 1;
    b->tris[1].uv[0][0] = w << 5;
    b->tris[1].uv[0][1] = h << 5;
    b->tris[1].vi1 = 2;
    b->tris[1].uv[1][0] = 0;
    b->tris[1].uv[1][1] = 0;
    b->tris[1].vi2 = 3;
    b->tris[1].uv[2][0] = 0;
    b->tris[1].uv[2][1] = h << 5;
    b->hit = 0;
    b->strength = setup->strength;
    b->splitCount = setup->splitCount;
    b->bounces = setup->bounces;
    b->speed = setup->speed;
    b->timer = setup->lifetime * 6;
    b->shooter = setup->shooter;
    b->hitParticles = setup->hitParticles;
    b->hitSounds = setup->hitSounds;
    b->unk7C = setup->unk12;
    b->flybyPlayed = 0;
    if (b->splitCount >= 5) {
        b->splitCount = 4;
    }
    if (setup->strength == 0) {
        b->scale = 2.5f;
        hitSize = 0x40;
        OBJ_STATE(obj, M109State)->lightColour = 0xFFC01000;
    } else {
        b->scale = sStrengthScales[(setup->strength - 1) & 3];
        hitSize = setup->strength << 8;
        OBJ_STATE(obj, M109State)->lightColour = sStrengthColours[(setup->strength - 1) & 3];
    }
    func_80079D1C(obj, hitSize, 1, 4, 0, 0, 0, 4.0f, 4.0f, 100.0f, 1.0f, 0x11);
    OBJ_STATE(obj, M109State)->light = 0;
    if (setup->colour >= 0 && setup->colour < 4) {
        OBJ_STATE(obj, M109State)->lightColour = sLightColours[setup->colour];
    }
}

void func_m109_00298(Object *obj, s32 updateRate) {
    M109Hit hit;
    s32 pad0;
    s32 pad1;
    s32 remaining;
    s32 done;
    s32 yRot;
    s32 xRot;
    s32 pad;
    s32 count;
    s32 pad2;
    f32 fx; /* also the new direction after a bounce */
    f32 fy; /* also the step */
    f32 fz;
    s32 pad3;
    M109Setup entry;
    M109Shot *b;
    Object **list; /* the players, then a spawned shot */
    s32 i;         /* also the surface hit */
    f32 dot;
    s32 j;

    runlink_unload_timer_set(D_m109_00B90, 500, 2);
    b = obj->unk68;
    fy = updateRate;
    done = 1;
    if (OBJ_STATE(obj, M109State)->light != 0) {
        track_light_off((f32 *) OBJ_STATE(obj, M109State)->light);
        OBJ_STATE(obj, M109State)->light = 0;
    }
    b->timer -= updateRate;
    if (b->timer > 0 && obj->interactObj->hitPoints > 0) {
        if (b->hit == 0) {
            func_80009A04(obj, obj->velocity.x * fy, obj->velocity.y * fy, obj->velocity.z * fy);
            b->distance += b->speed * fy;
            if (b->shooter->behaviorId == 1) {
                enemy_module_notify(obj, obj->trans.x_position, obj->trans.y_position, obj->trans.z_position, 4, 1, 0);
            } else if (b->shooter->behaviorId == 0x18 && b->flybyPlayed == 0 && sFlybySound == 0 &&
                       b->distance > 900.0f) {
                list = func_8000E464(&count);
                for (i = 0; i < count; i++) {
                    fx = obj->trans.x_position - list[i]->trans.x_position;
                    fy = obj->trans.y_position - list[i]->trans.y_position;
                    fz = obj->trans.z_position - list[i]->trans.z_position;
                    if (fx * fx + fy * fy + fz * fz < 20000.0f) {
                        func_80003410(func_80048E2C(0x219, 0x21B), obj->trans.x_position, obj->trans.y_position,
                                      obj->trans.z_position, 4, &sFlybySound);
                        b->flybyPlayed = 1;
                        sFlybyShot = obj;
                    }
                }
            }
            if (b->hit = func_80018808(obj, &((M109Interaction *) obj->interactObj)->oldPos,
                                       (Vec3f *) &obj->trans.x_position, (HitResult *) &hit, 0, 0)) {
                b->hitFlags = hit.flags;
                func_80009A04(obj, hit.pos.x - obj->trans.x_position, hit.pos.y - obj->trans.y_position,
                              hit.pos.z - obj->trans.z_position);
                if (b->bounces != 0) {
                    if (!(b->hitFlags & 0xA000)) {
                        dot = (hit.normal.z * obj->velocity.z +
                               (hit.normal.x * obj->velocity.x + hit.normal.y * obj->velocity.y)) *
                              -2.0f;
                        fx = hit.normal.x * dot + obj->velocity.x;
                        fy = hit.normal.y * dot + obj->velocity.y;
                        fz = hit.normal.z * dot + obj->velocity.z;
                        obj->trans.rotation.y_rotation = func_80049C84(-fx, -fz);
                        obj->trans.rotation.x_rotation = func_80049C84(fy, sqrtf(fx * fx + fz * fz));
                        obj->velocity.z = -b->speed;
                        func_8004944C(&obj->trans.rotation, &obj->velocity);
                        b->hit = 0;
                    }
                } else if (b->splitCount != 0) {
                    entry.objectID = 0x50;
                    entry.size = 0x24;
                    entry.unk3 = 0;
                    entry.x = obj->trans.x_position;
                    entry.y = obj->trans.y_position;
                    entry.z = obj->trans.z_position;
                    entry.strength = b->strength;
                    entry.lifetime = b->timer / 6;
                    entry.splitCount = b->splitCount;
                    entry.bounces = b->bounces;
                    entry.speed = b->speed;
                    entry.shooter = b->shooter;
                    entry.hitParticles = b->hitParticles;
                    entry.hitSounds = b->hitSounds;
                    if (entry.lifetime <= 0) {
                        entry.lifetime = 1;
                    }
                    yRot = func_80049C84(-hit.normal.x, -hit.normal.z);
                    xRot = func_80049C84(hit.normal.y,
                                         sqrtf(hit.normal.z * hit.normal.z + hit.normal.x * hit.normal.x));
                    remaining = b->splitCount;
                    if (entry.strength >= 2) {
                        entry.strength >>= 1;
                    }
                    j = remaining * 4;
                    entry.splitCount--;
                    while (remaining--) {
                        entry.yRotation = sSplitYRotations[j] + yRot;
                        entry.xRotation = sSplitXRotations[j] + xRot;
                        j++;
                        list = (Object **) func_80005F14(&entry, 1);
                        if (list != NULL) {
                            ((Object *) list)->level_entry = NULL;
                        }
                    }
                }
                OBJ_STATE(obj, M109State)->light = (s32) func_80013E0C(
                    obj->trans.x_position, obj->trans.y_position, obj->trans.z_position, 200.0f, 100.0f,
                    (OBJ_STATE(obj, M109State)->lightColour >> 24) & 0xFF,
                    (OBJ_STATE(obj, M109State)->lightColour >> 16) & 0xFF,
                    (OBJ_STATE(obj, M109State)->lightColour >> 8) & 0xFF);
            }
            fx = obj->trans.x_position - ((M109Interaction *) obj->interactObj)->oldPos.x;
            fy = obj->trans.y_position - ((M109Interaction *) obj->interactObj)->oldPos.y;
            fz = obj->trans.z_position - ((M109Interaction *) obj->interactObj)->oldPos.z;
            OBJ_F34(obj) = sqrtf(fx * fx + fy * fy + fz * fz);
            done = 0;
        } else {
            i = 2;
            if (b->hitFlags & 0x2000) {
                obj->trans.rotation.x_rotation = 0;
            } else {
                i = func_8004470C()->surface;
            }
            if (i != 0xFF) {
                if (b->hitSounds != NULL) {
                    if (sHitSound != 0) {
                        func_80003614(sHitSound);
                    }
                    func_80003410(b->hitSounds[i], obj->trans.x_position, obj->trans.y_position,
                                  obj->trans.z_position, 4, &sHitSound);
                    enemy_module_notify(NULL, obj->trans.x_position, obj->trans.y_position, obj->trans.z_position, 1, 1,
                                  b->hitSounds[i]);
                }
                if (b->hitParticles != NULL && updateRate < 5) {
                    obj->trans.rotation.x_rotation = 0;
                    obj->particleEmittersEnabled = b->hitParticles[i];
                    func_8005F5AC(obj, 2);
                }
            }
        }
    }
    if (done) {
        func_800079B8(obj);
    } else if (sFlybySound != 0 && obj == sFlybyShot) {
        func_800035E8(sFlybySound, obj->trans.x_position, obj->trans.y_position, obj->trans.z_position);
    }
}

#else
INCLUDE_ASM("asm/nonmatchings/modules/m109", func_m109_00000);

INCLUDE_ASM("asm/nonmatchings/modules/m109", func_m109_00298);

INCLUDE_ASM("asm/nonmatchings/modules/m109", rodata);

#endif

void func_m109_00B44(void) {
    if (sHitSound != 0) {
        func_80003614(sHitSound);
    }
    if (sFlybySound != 0) {
        func_80003614(sFlybySound);
    }
    sFlybyShot = NULL;
}
#pragma weak __rl_fini = func_m109_00B44
