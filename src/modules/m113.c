/*
 * Runlink module 113: a rocket projectile. Fired with a speed, a direction and a kind:
 * kind 1 homes in on a target object (or one of its parts), turning at most
 * 600 angle units a frame towards it; the others fly straight while
 * corkscrewing around their path at a growing radius. It explodes when it
 * hits something, runs out of time or is shot down.
 */
#include "module.h"

typedef struct M113Setup {
    /* 0x00 */ u8 pad0[0xA];
    /* 0x0A */ u8 strength;
    /* 0x0B */ u8 lifetime; /* in seconds / 10 */
    /* 0x0C */ f32 speed;
    /* 0x10 */ Object *owner;
    /* 0x14 */ Object *target;
    /* 0x18 */ s16 angle;
    /* 0x1A */ s16 angleSpeed;
    /* 0x1C */ s16 radius;
    /* 0x1E */ s16 y_rotation;
    /* 0x20 */ s16 x_rotation;
    /* 0x22 */ s16 radiusSpeed;
    /* 0x24 */ u8 kind;
    /* 0x25 */ u8 targetPart;
    /* 0x26 */ s8 unk26;
} M113Setup;

typedef struct M113Rocket {
    /* 0x00 */ Vec3f hitPos;
    /* 0x0C */ s32 hitFlags;
    /* 0x10 */ Vec3f pathPos; /* the centre of the corkscrew */
    /* 0x1C */ f32 speed;     /* negated */
    /* 0x20 */ Object *owner;
    /* 0x24 */ Object *target;
    /* 0x28 */ s16 timer;
    /* 0x2A */ s16 hitYaw;
    /* 0x2C */ s16 hitPitch;
    /* 0x2E */ s16 angle;
    /* 0x30 */ s16 angleSpeed;
    /* 0x32 */ s16 radius;
    /* 0x34 */ s16 radiusSpeed;
    /* 0x36 */ u8 hit;
    /* 0x37 */ u8 kind;
    /* 0x38 */ u8 targetPart; /* 0xFF: the target itself */
    /* 0x39 */ s8 unk39;
} M113Rocket;

typedef struct M113State {
    /* 0x0 */ s32 health; /* at the start */
    /* 0x4 */ f32 *light;
} M113State;

typedef struct M113Part {
    /* 0x00 */ u8 pad0[0x18];
    /* 0x18 */ Vec3f pos;
    /* 0x24 */ u8 pad24[0x34 - 0x24];
} M113Part;

typedef struct M113Interaction {
    /* 0x00 */ u8 pad0[0x18];
    /* 0x18 */ Vec3f pos; /* (for the rocket itself: its previous position) */
    /* 0x24 */ u8 pad24[0x32 - 0x24];
    /* 0x32 */ s16 numParts;
    /* 0x34 */ u8 pad34[0x40 - 0x34];
    /* 0x40 */ M113Part *parts;
} M113Interaction;

/* The level entry of the scorch mark it can leave */
typedef struct M113Spawn {
    /* 0x00 */ s16 objectID;
    /* 0x02 */ u8 size;
    /* 0x03 */ u8 pad3;
    /* 0x04 */ s16 x;
    /* 0x06 */ s16 y;
    /* 0x08 */ s16 z;
    /* 0x0A */ s16 unkA;
    /* 0x0C */ f32 unkC;
} M113Spawn;

/* The level entry of the blast a homing rocket leaves */
typedef struct M113Blast {
    /* 0x00 */ s16 objectID;
    /* 0x02 */ u8 pad2[2];
    /* 0x04 */ s16 x;
    /* 0x06 */ s16 y;
    /* 0x08 */ s16 z;
    /* 0x0A */ s16 yaw;
    /* 0x0C */ s16 pitch;
    /* 0x0E */ u8 padE[2];
    /* 0x10 */ u8 hitSomething;
    /* 0x11 */ u8 unk11;
    /* 0x12 */ u8 unk12;
    /* 0x13 */ u8 hitFloor;
    /* 0x14 */ Object *owner;
} M113Blast;

typedef struct M113Level {
    /* 0x00 */ u8 pad0[0xE2];
    /* 0xE2 */ u8 unkE2;
} M113Level;

extern f32 sqrtf(f32);
extern s32 func_80049C84(f32, f32); /* the angle of a vector */
extern s32 func_80049D80(s32 from, s32 to); /* the signed difference of two angles */
extern void func_800496B0(s32 arg0, Object *obj, Vec3f *in, Vec3f *out);
extern void func_800492DC(Object *obj, Vec3f *vec);
extern void obj_list_insert_after(Object *obj, Object *target);
extern f32 *func_80013E0C(f32 x, f32 y, f32 z, f32 arg3, f32 arg4, s32 arg5, s32 arg6, s32 arg7); /* adds a light */
extern void track_light_off(f32 *light);
extern void track_light_set_position(f32 *light, f32 x, f32 y, f32 z);
extern s32 func_80009A04(Object *obj, f32 dx, f32 dy, f32 dz); /* DKR objMoveXYZ */
extern Object *func_80005F14(void *entry, s32 arg1); /* DKR spawn_object */
extern M113Level *func_8004470C();                   /* DKR levelGetLevel */
extern void func_80029BA4(f32 x, f32 y, f32 z, f32 arg3, f32 arg4, f32 arg5, s32 arg6, Object *owner);
extern void spawn_object_from_list(s32, s16, s16, s16, s32, Object *);
extern void func_80052734(s32, s16, s16, s16, u8, u8, u8);
extern void func_8005F5AC(Object *obj, s32 updateRate);

#ifdef NON_MATCHING
s32 D_m113_00CC0 = 113; /* the module's number */
static u8 sBlastKinds[2] = { 5, 1 };
#else
/* func_m113_002C8 is still asm and refers to the data (and its .rodata) by its own labels */
INCLUDE_ASM("asm/nonmatchings/modules/m113", data);
INCLUDE_ASM("asm/nonmatchings/modules/m113", rodata);
#endif

void func_m113_00000(Object *obj, M113Setup *setup) {
    M113Rocket *m;
    s32 unused;
    Vec3f offset;

    obj->velocity.x = 0.0f;
    obj->velocity.y = 0.0f;
    obj->velocity.z = -setup->speed;
    obj->trans.rotation.y_rotation = setup->y_rotation;
    m = obj->unk68;
    obj->trans.rotation.x_rotation = setup->x_rotation;
    obj->trans.rotation.z_rotation = 0;
    func_8004944C(&obj->trans.rotation, &obj->velocity);
    m->hit = 0;
    m->hitFlags = 0;
    m->speed = -setup->speed;
    m->kind = setup->kind;
    m->target = setup->target;
    m->targetPart = setup->targetPart;
    m->pathPos.x = obj->trans.x_position;
    m->pathPos.y = obj->trans.y_position;
    m->pathPos.z = obj->trans.z_position;
    m->unk39 = setup->unk26;
    if (m->kind != 1) {
        m->radius = setup->radius;
        m->radiusSpeed = setup->radiusSpeed;
        m->angle = setup->angle;
        m->angleSpeed = setup->angleSpeed;
        offset.x = func_80049C34(m->angle) * m->radius;
        offset.y = func_80049C30(m->angle) * m->radius;
        offset.z = 0.0f;
        func_800496B0(1, obj, &offset, &offset);
        obj->trans.x_position += offset.x;
        obj->trans.y_position += offset.y;
        obj->trans.z_position += offset.z;
    } else {
        m->radius = 0;
        m->radiusSpeed = 0;
        m->angle = 0;
        m->angleSpeed = 0;
        obj_list_insert_after(obj, m->target);
    }
    m->hitPos.x = obj->trans.x_position;
    m->hitPos.y = obj->trans.y_position;
    m->hitPos.z = obj->trans.z_position;
    m->timer = func_80005064(setup->lifetime * 6);
    m->owner = setup->owner;
    func_80079D1C(obj, (setup->strength + 1) << 8, 1, 4, 0, 0, 0, 2.0f, 2.0f, -1.0f, 1.0f, 0x11);
    OBJ_STATE(obj, M113State)->health = obj->interactObj->hitPoints;
    if (m->radius == 0.0f) {
        OBJ_STATE(obj, M113State)->light = func_80013E0C(obj->trans.x_position, obj->trans.y_position,
                                                         obj->trans.z_position, 500.0f, 300.0f, 0xFF, 0xE0, 0);
    } else {
        OBJ_STATE(obj, M113State)->light = NULL;
    }
}

#ifdef NON_MATCHING
/* Same size; differs in where `rate` is spilled (0x58, ROM 0x60), the register of the hit normal's z
   (ROM keeps it in a caller-saved register spilled to dz's slot) and the register of the light pointer. */
void func_m113_002C8(Object *obj, s32 updateRate) {
    M113Rocket *m;
    HitResult hit;
    Object *spawned;
    Vec3f offset;
    Object *owner;
    Object *target;
    s16 step;
    s16 yaw;
    s16 pitch;
    M113Interaction *ia;
    Vec3f *unused;
    f32 dx;
    f32 dz;
    f32 dy;
    M113Spawn entry;
    f32 dist;
    M113Blast blast;
    s32 i;
    f32 rate;

    runlink_unload_timer_set(D_m113_00CC0, 2, 2);
    m = obj->unk68;
    rate = updateRate;
    m->timer -= updateRate;
    if (m->timer > 0 && obj->interactObj->unk6 > 0 && m->hit == 0 &&
        obj->interactObj->unk6 == OBJ_STATE(obj, M113State)->health) {
        if (m->hit == 0) {
            if (m->kind == 1) {
                target = m->target;
                if (target != NULL && target->interactObj == NULL) {
                    m->target = NULL;
                    target = NULL;
                }
                if (target != NULL) {
                    if (m->targetPart != 0xFF &&
                        m->targetPart >= ((M113Interaction *) target->interactObj)->numParts) {
                        m->targetPart = 0xFF;
                    }
                    for (i = 0; i < updateRate; i++) {
                        ia = (M113Interaction *) m->target->interactObj;
                        if (m->targetPart == 0xFF) {
                            dx = ia->pos.x;
                            dy = ia->pos.y;
                            dz = ia->pos.z;
                        } else {
                            dx = ia->parts[m->targetPart].pos.x;
                            dy = ia->parts[m->targetPart].pos.y;
                            dz = ia->parts[m->targetPart].pos.z;
                        }
                        dx = obj->trans.x_position - dx;
                        dz = obj->trans.z_position - dz;
                        dy = obj->trans.y_position - dy;
                        dist = sqrtf(dx * dx + dz * dz);
                        if (dz == 0.0f) {
                            yaw = 0;
                        } else {
                            yaw = func_80049C84(dx, dz);
                        }
                        if (dist == 0.0f) {
                            pitch = 0;
                        } else {
                            pitch = -func_80049C84(dy, dist);
                        }
                        step = func_80049D80(obj->trans.rotation.y_rotation, yaw) >> 3;
                        if (step > 600) {
                            step = 600;
                        }
                        if (step < -600) {
                            step = -600;
                        }
                        obj->trans.rotation.y_rotation += step;
                        step = func_80049D80(obj->trans.rotation.x_rotation, pitch) >> 3;
                        if (step > 600) {
                            step = 600;
                        }
                        if (step < -600) {
                            step = -600;
                        }
                        obj->trans.rotation.x_rotation += step;
                    }
                }
                obj->velocity.x = 0.0f;
                obj->velocity.y = 0.0f;
                obj->velocity.z = m->speed;
                func_8004944C(&obj->trans.rotation, &obj->velocity);
                func_80009A04(obj, obj->velocity.x * updateRate, obj->velocity.y * updateRate, obj->velocity.z * updateRate);
            } else {
                m->pathPos.x += obj->velocity.x * rate;
                m->pathPos.y += obj->velocity.y * rate;
                m->pathPos.z += obj->velocity.z * rate;
                m->radius += m->radiusSpeed * updateRate;
                if (m->radius == 0) {
                    obj->trans.rotation.z_rotation += updateRate << 10;
                    offset.x = 0.0f;
                    offset.y = 0.0f;
                    offset.z = 0.0f;
                } else {
                    m->angle += m->angleSpeed * updateRate;
                    if (m->radius > 50) {
                        m->radius = 50;
                    }
                    offset.x = func_80049C34(m->angle) * m->radius;
                    offset.y = func_80049C30(m->angle) * m->radius;
                    offset.z = 0.0f;
                    func_800492DC(obj, &offset);
                }
                offset.x += m->pathPos.x;
                offset.y += m->pathPos.y;
                offset.z += m->pathPos.z;
                func_80009A04(obj, offset.x - obj->trans.x_position, offset.y - obj->trans.y_position,
                              offset.z - obj->trans.z_position);
            }
            m->hit = func_80018808(obj, &((M113Interaction *) obj->interactObj)->pos,
                                   (Vec3f *) &obj->trans.x_position, &hit, 0, 0);
            if (m->hit != 0) {
                if (hit.obj == NULL) {
                    m->hit |= 2;
                }
                m->hitPos.x = hit.pos.x;
                m->hitPos.y = hit.pos.y;
                m->hitPos.z = hit.pos.z;
                m->hitFlags = hit.flags;
                func_80009A04(obj, m->hitPos.x - obj->trans.x_position, m->hitPos.y - obj->trans.y_position,
                              m->hitPos.z - obj->trans.z_position);
                dx = hit.normal.x;
                dist = hit.normal.z;
                dy = hit.normal.y;
                m->hitYaw = func_80049C84(dx, dist) + 0x8000;
                m->hitPitch = func_80049C84(dy, sqrtf(dx * dx + dist * dist));
            }
            if (OBJ_STATE(obj, M113State)->light != NULL) {
                track_light_set_position(OBJ_STATE(obj, M113State)->light, obj->trans.x_position, obj->trans.y_position,
                              obj->trans.z_position);
            }
        }
    } else {
        if ((m->hitFlags & 0x2000) && !(m->hitFlags & 0x8000)) {
            func_80003410(0xD, obj->trans.x_position, obj->trans.y_position, obj->trans.z_position, 4, NULL);
            entry.objectID = 0xC0;
            entry.size = 0x10;
            entry.x = m->hitPos.x;
            entry.y = m->hitPos.y;
            entry.z = m->hitPos.z;
            entry.unkA = 0x100;
            entry.unkC = 20.0f;
            spawned = func_80005F14(&entry, 1);
            if (spawned != NULL) {
                spawned->level_entry = NULL;
            }
        } else {
            func_80003410(0x96, obj->trans.x_position, obj->trans.y_position, obj->trans.z_position, 4, NULL);
            owner = m->owner;
            if (owner != NULL && owner->behaviorId != 1) {
                owner = NULL;
            }
            if (m->kind == 1) {
                blast.objectID = 0x13C;
                blast.x = obj->trans.x_position;
                blast.y = obj->trans.y_position;
                blast.z = obj->trans.z_position;
                blast.yaw = m->hitYaw;
                blast.pitch = m->hitPitch;
                blast.hitSomething = m->hit & 1;
                blast.unk12 = func_8004470C()->unkE2;
                blast.unk11 = 1;
                blast.hitFloor = m->hit & 2;
                blast.owner = owner;
                spawned = func_80005F14(&blast, 1);
                if (spawned != NULL) {
                    spawned->level_entry = NULL;
                }
                func_80029BA4(obj->trans.x_position, obj->trans.y_position, obj->trans.z_position, 120.0f, 120.0f,
                              0.34f, sBlastKinds[m->kind], owner);
            } else {
                spawn_object_from_list(2, obj->trans.x_position, obj->trans.y_position, obj->trans.z_position,
                              sBlastKinds[m->kind], owner);
            }
            func_80052734(0, obj->trans.x_position, obj->trans.y_position, obj->trans.z_position, 5, 1, 0x96);
        }
        if (OBJ_STATE(obj, M113State)->light != NULL) {
            track_light_off(OBJ_STATE(obj, M113State)->light);
        }
        func_800079B8(obj);
        m = NULL;
    }
    if (m != NULL) {
        if (m->radius == 0) {
            obj->particleEmittersEnabled = 1;
        } else {
            obj->particleEmittersEnabled = 3;
        }
        func_8005F5AC(obj, updateRate);
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m113", func_m113_002C8);
#endif
