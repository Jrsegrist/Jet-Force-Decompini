/*
 * Runlink module 118: objects for the robot sections: a target that runs an
 * action when it is shot, a collectable that the robot picks up, and a
 * checkpoint plane that the robot crosses.
 */
#include "module.h"

typedef struct M118PlayerData {
    /* 0x000 */ u8 pad0[0x10D];
    /* 0x10D */ u8 inRobot; /* 1 while the player controls the robot */
    /* 0x10E */ u8 pad10E[0x50C - 0x10E];
    /* 0x50C */ Object *robot;
} M118PlayerData;

/* The robot's own data (its object's unk68) */
typedef struct M118Robot {
    /* 0x0 */ f32 x;
    /* 0x4 */ f32 y;
    /* 0x8 */ f32 z;
} M118Robot;

typedef struct M118TargetSetup {
    LevelObjectEntryCommon common;
    /* 0x0A */ u8 message;
    /* 0x0B */ u8 hitbox;
    /* 0x0C */ u8 sparks;
    /* 0x0D */ u8 swapModel;
    /* 0x0E */ u8 y_rotation;
    /* 0x0F */ u8 x_rotation;
    /* 0x10 */ u8 action;
    /* 0x11 */ u8 arg;
} M118TargetSetup;

typedef struct M118Target {
    /* 0x0 */ u8 message;
    /* 0x1 */ u8 hitbox;
    /* 0x2 */ u8 sparks;
    /* 0x3 */ u8 swapModel;
    /* 0x4 */ u8 action;
    /* 0x5 */ u8 arg;
    /* 0x6 */ s16 unk6;
} M118Target;

typedef struct M118CollectableSetup {
    LevelObjectEntryCommon common;
    /* 0x0A */ u8 message;
    /* 0x0B */ u8 radius;
} M118CollectableSetup;

typedef struct M118Collectable {
    /* 0x0 */ f32 radius;
    /* 0x4 */ s16 message;
    /* 0x6 */ s16 fade; /* 0xFF until collected, then counts down */
} M118Collectable;

typedef struct M118CheckSetup {
    LevelObjectEntryCommon common;
    /* 0x0A */ u8 message;
    /* 0x0B */ u8 last; /* 0xFF: the last check */
    /* 0x0C */ u8 size;
    /* 0x0D */ u8 y_rotation;
    /* 0x0E */ u8 x_rotation;
} M118CheckSetup;

/* The plane nx * x + ny * y + nz * z + d = 0, within radius of the object */
typedef struct M118Check {
    /* 0x00 */ f32 nx;
    /* 0x04 */ f32 ny;
    /* 0x08 */ f32 nz;
    /* 0x0C */ f32 d;
    /* 0x10 */ f32 radius;
    /* 0x14 */ s16 message;
    /* 0x16 */ s16 last;
} M118Check;

extern u8 D_800A3208;

extern s32 check_fadeout_transition(void);
extern void func_8006B9CC(void *);
extern void func_m022_02430(s32 message, s32 arg1);
extern void func_m022_023FC(M118PlayerData *player);

static s32 D_m118_00900[4] = { 0x40FFFFFF, 0x00060000, 0, 0 };

/* Finds the robot (the object with behaviour 2) (module m118, 0x00000). */
static Object *robot_find(void) {
    s32 i;
    s32 count;
    Object *robot = NULL;
    Object **list;

    list = func_80005E98(&i, &count);
    for (; i < count; i++) {
        if (list[i]->behaviorId == 2) {
            robot = list[i];
        }
    }
    return robot;
}

void func_m118_0007C(Object *obj, M118TargetSetup *setup) {
    M118Target *t = OBJ_STATE(obj, M118Target);

    t->message = setup->message;
    t->hitbox = setup->hitbox;
    t->sparks = setup->sparks;
    t->swapModel = setup->swapModel;
    t->action = setup->action;
    t->arg = setup->arg;
    t->unk6 = 0xFF;
    obj->trans.rotation.y_rotation = setup->y_rotation << 8;
    obj->trans.rotation.x_rotation = setup->x_rotation << 8;
    obj->modelIndex = 0;
    func_80079D1C(obj, t->hitbox << 8, 0, 8, 0, 0, 0, 20.0f, 20.0f, -1.0f, 1.0f, 0x11);
}

void func_m118_00140(Object *obj, s32 updateRate) {
    s32 n;
    SparkParams spark;
    ObjectInteraction *hit;
    Object *player;
    M118PlayerData *pd;
    M118Target *t;

    hit = obj->interactObj;
    if (hit != NULL && obj->unk5C != NULL && hit->hitPoints > 0) {
        player = func_8000E4D0(0);
        t = OBJ_STATE(obj, M118Target);
        if (player != NULL) {
            pd = player->unk68;
            if (pd != NULL && pd->inRobot == 1) {
                hit->hitPoints -= obj->unk5C->damage;
            }
        }
        if (hit->hitPoints <= 0) {
            if (t->swapModel) {
                obj->modelIndex = 1;
            }
            n = t->sparks;
            if (n != 0) {
                spark.x_position = obj->trans.x_position;
                spark.y_position = obj->trans.y_position;
                spark.z_position = obj->trans.z_position;
                spark.unk18 = 20;
                spark.unk1A = 10;
                spark.unk1B = 0xFF;
                spark.unk1C = 0xFFFF80FF;
                spark.unk20 = 0xFF0000FF;
                while (n--) {
                    spark.y_rotation = func_80048E2C(-0x8000, 0x7FFF);
                    spark.x_rotation = func_80048E2C(-0x3000, 0x3C00);
                    spark.speed = func_80048E2C(70, 90) * 0.1f;
                    spark.unk14 = func_80048E2C(25, 50) * 0.1f;
                    func_m010_0000C(&spark);
                }
            }
            switch (t->action) {
                case 0:
                    func_80079BA8();
                    break;
                case 1:
                    func_80079BB8();
                    break;
                case 2:
                    func_800767EC(t->arg);
                    break;
                case 3:
                    func_8007688C(t->arg);
                    break;
                case 4:
                    func_80076928(t->arg);
                    break;
                case 5:
                    func_800772C4(t->arg);
                    break;
            }
            hit->hitPoints = 0;
            func_80001248(0x22, 0);
            func_m022_02430(t->message, 1);
        }
    }
}

void func_m118_003DC(Object *obj, M118CollectableSetup *setup) {
    M118Collectable *c = OBJ_STATE(obj, M118Collectable);

    c->radius = setup->radius;
    c->message = setup->message;
    c->fade = 0xFF;
}

void func_m118_00414(Object *obj, s32 updateRate) {
    M118Collectable *c;
    f32 dx;
    f32 dy;
    f32 dz;
    f32 radiusSq;
    Object *robot;
    Object *player;
    M118PlayerData *pd;
    M118Robot *rd;
    f32 distSq;

    c = OBJ_STATE(obj, M118Collectable);
    if (c->fade == 0xFF) {
        if (obj->objectID == 0x2C5) {
            func_8001138C(obj, 0.01f, updateRate);
        } else {
            obj->trans.rotation.y_rotation += updateRate << 8;
        }
        robot = NULL;
        player = func_8000E4D0(0);
        if (player != NULL) {
            pd = player->unk68;
            if (pd != NULL && pd->inRobot == 1) {
                robot = robot_find();
            }
        }
        if (robot != NULL) {
            rd = robot->unk68;
            radiusSq = c->radius;
            radiusSq *= radiusSq;
            dx = obj->trans.x_position - rd->x;
            dy = obj->trans.y_position - rd->y;
            dz = obj->trans.z_position - rd->z;
            if (dx * dx + dy * dy + dz * dz < radiusSq) {
                if (check_fadeout_transition() == 0) {
                    func_8006B9CC(D_m118_00900);
                }
                func_80001248(0x8A, 0);
                func_m022_02430(c->message, 0);
                c->fade -= updateRate << 4;
            }
        }
    } else {
        if (c->fade > 0) {
            if (obj->objectID == 0x2C5) {
                func_8001138C(obj, 0.01f, updateRate);
            } else {
                obj->trans.rotation.y_rotation += updateRate << 8;
            }
            c->fade -= updateRate << 4;
            if (c->fade < 0) {
                c->fade = 0;
            }
        }
    }
}

void func_m118_0063C(Object *obj, M118CheckSetup *setup) {
    M118Check *c = obj->unk68;
    f32 s;

    obj->trans.scale = setup->size * 0.04f;
    obj->trans.rotation.y_rotation = setup->y_rotation << 10;
    obj->trans.rotation.x_rotation = setup->x_rotation << 10;
    obj->trans.rotation.z_rotation = 0;
    s = func_80049C34(obj->trans.rotation.y_rotation);
    c->nx = func_80049C30(obj->trans.rotation.x_rotation) * s;
    c->ny = -func_80049C34(obj->trans.rotation.x_rotation);
    s = func_80049C30(obj->trans.rotation.y_rotation);
    c->nz = func_80049C30(obj->trans.rotation.x_rotation) * s;
    c->d = -(c->nx * obj->trans.x_position + c->ny * obj->trans.y_position + c->nz * obj->trans.z_position);
    c->radius = setup->size * 4;
    c->message = setup->message;
    c->last = setup->last;
}

void func_m118_00794(Object *obj, s32 updateRate) {
    Object *player;
    Object *robot;
    f32 dx;
    f32 dy;
    f32 dz;
    f32 distSq;
    f32 radiusSq;
    f32 side;
    M118PlayerData *pd;
    M118Check *c;
    f32 x;
    f32 y;
    f32 z;

    c = obj->unk68;
    if (D_800A3208 != 0 && c->last != 0) {
        player = func_8000E4D0(0);
        if (player != NULL) {
            pd = player->unk68;
            robot = pd->robot;
            if (robot != NULL) {
                x = robot->trans.x_position;
                dx = x - obj->trans.x_position;
                y = robot->trans.y_position;
                dy = y - obj->trans.y_position;
                z = robot->trans.z_position;
                dz = z - obj->trans.z_position;
                radiusSq = c->radius * c->radius;
                if (dx * dx + dy * dy + dz * dz < radiusSq &&
                    (side = c->nx * x + c->ny * y + c->nz * z + c->d) < 0.0f) {
                    func_m022_02430(c->message, 2);
                    if (c->last == 0xFF && pd->inRobot == 1) {
                        func_m022_023FC(pd);
                    }
                }
            }
        }
    }
}
