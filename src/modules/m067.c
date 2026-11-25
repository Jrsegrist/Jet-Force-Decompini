/*
 * Runlink module 67: an enemy and its missiles (still mostly assembly).
 * Family: enemy behaviours (docs/runlink.md, "Module families").
 */
#include "module.h"
#include "gfx.h"

/* A missile's state (obj->unk68), as far as used here */
typedef struct M067Missile {
    /* 0x0 */ u8 pad0[4];
    /* 0x4 */ f32 thrust;
    /* 0x8 */ void *flame;
} M067Missile;

/* A jump in progress */
typedef struct M067Jump {
    /* 0x00 */ u8 pad0;
    /* 0x01 */ s8 kind;
    /* 0x02 */ s8 target;
    /* 0x03 */ u8 pad3[0x10 - 0x3];
    /* 0x10 */ Vec3f start;
    /* 0x1C */ s16 yRotation;
    /* 0x1E */ s16 xRotation;
    /* 0x20 */ Vec3f dest;
    /* 0x2C */ s16 yRotation2;
    /* 0x2E */ s16 xRotation2;
} M067Jump;

/* An entry of the jump table (size 8) */
typedef struct M067JumpKind {
    /* 0x0 */ s32 anim;
    /* 0x4 */ f32 speed; /* the animation's speed */
} M067JumpKind;

/* The set-up of a shot (a bolt or a rock) */
typedef struct M067ShotSetup {
    LevelObjectEntryCommon common;
    /* 0x0A */ u8 strength;
    /* 0x0B */ u8 time;
    /* 0x0C */ s16 yRotation;
    /* 0x0E */ s16 xRotation;
    /* 0x10 */ f32 scale;
    /* 0x14 */ f32 speed;
    /* 0x18 */ Object *owner;
} M067ShotSetup;

typedef struct M067Shot {
    /* 0x0 */ Object *owner;
    /* 0x4 */ s16 timer;
    /* 0x6 */ u8 strength;
    /* 0x7 */ u8 unk7;
} M067Shot;

/* A jump point's data (the unk68 of an object with behaviour 0x68) */
typedef struct M067JumpPoint {
    /* 0x00 */ u8 pad0[0x14];
    /* 0x14 */ s16 id;
} M067JumpPoint;

extern M067JumpKind rl_m067_04430[];
extern s8 rl_m067_04580[][8]; /* the jump kind between two jump points (-1: none) */
extern s16 func_80049D80(s32 angle, s32 target);

/* An entry of the enemy's event list (size 0x18): what happens once the previous entry's time has passed */
typedef struct M067Event {
    /* 0x00 */ s32 time;
    /* 0x04 */ u8 point;  /* the model point it happens at */
    /* 0x05 */ s8 part;   /* the part whose flags it sets (-1: none) */
    /* 0x06 */ u8 value;
    /* 0x07 */ u8 numDrops;
    /* 0x08 */ u8 numSparks;
    /* 0x09 */ u8 numPieces;
    /* 0x0A */ s16 burst; /* -1: none */
    /* 0x0C */ s32 colour1;
    /* 0x10 */ s32 colour2;
    /* 0x14 */ s16 pieceID;
    /* 0x16 */ s16 sound; /* -1: none */
} M067Event;

/* The enemy's state (obj->unk68), as far as used here */
typedef struct M067State {
    /* 0x00 */ s8 side;
    /* 0x01 */ s8 kind;
    /* 0x02 */ s8 target;
    /* 0x03 */ u8 unk3;
    /* 0x04 */ s32 unk4; /* from the pair's table (0025C) */
    /* 0x08 */ u8 unk8;
    /* 0x09 */ u8 unk9;
    /* 0x0A */ u8 unkA;
    /* 0x0B */ u8 unkB;
    /* 0x0C */ u8 unkC;
    /* 0x0D */ u8 unkD;
    /* 0x0E */ u8 unkE;
    /* 0x0F */ s8 unkF; /* a countdown; -1: stopped */
    /* 0x10 */ Vec3f start; /* the jump in progress (as in M067Jump) */
    /* 0x1C */ s16 yRotation;
    /* 0x1E */ s16 xRotation;
    /* 0x20 */ Vec3f dest; /* from start to the jump's end */
    /* 0x2C */ s16 yRotation2;
    /* 0x2E */ s16 xRotation2;
    /* 0x30 */ s32 headAngle; /* turns after the player */
    /* 0x34 */ M067Event *events;
    /* 0x38 */ s32 unk38; /* the time since the last event */
    /* 0x3C */ u8 unk3C[6];
    /* 0x42 */ s16 joints[47]; /* the model's joint commands, rebuilt every frame */
} M067State;

typedef struct M067Setup {
    LevelObjectEntryCommon common;
    /* 0x0A */ s8 side;
    /* 0x0B */ s8 yRotation; /* in 256ths of a turn */
} M067Setup;

/* The hit box numbers in the object's header */
typedef struct M067Header {
    /* 0x00 */ u8 pad0[0xB4];
    /* 0xB4 */ s16 hitX;
    /* 0xB6 */ s16 hitY;
    /* 0xB8 */ s16 hitZ;
    /* 0xBA */ u8 padBA[0xBC - 0xBA];
    /* 0xBC */ f32 hitRadius;
    /* 0xC0 */ f32 hitHeight;
} M067Header;

/* The level entry of the dust the enemy raises when it lands */
typedef struct M067DustEntry {
    /* 0x00 */ s16 objectID;
    /* 0x02 */ u8 pad2[0x4 - 0x2];
    /* 0x04 */ s16 x;
    /* 0x06 */ s16 y;
    /* 0x08 */ s16 z;
    /* 0x0A */ s16 yRotation;
    /* 0x0C */ s16 xRotation;
    /* 0x0E */ s16 unkE;
    /* 0x10 */ f32 scale;
} M067DustEntry;

/* What obj->interactObj points at here */
typedef struct M067HitShape {
    /* 0x00 */ u8 pad0[0x24];
    /* 0x24 */ f32 radius;
} M067HitShape;

extern void spawn_object_from_list(s16 kind, s16 x, s16 y, s16 z, s32 arg4, s32 arg5); /* a burst */
extern s32 func_80009A04(Object *obj, f32 dx, f32 dy, f32 dz); /* DKR objMoveXYZ */

/* The level's fog */
extern void func_8001BC2C(s32 arg0, s16 *fogNear, s16 *fogFar, s16 *arg3, u8 *r, u8 *g, u8 *b, u8 *arg7);

extern Object *rl_m067_04574[2]; /* the two enemies of a pair */
extern s32 rl_m067_0456C;
extern void rl_m067_0025C(void);
extern s32 runlink_module_loaded(s32 arg0);
extern void func_m039_00000(void);
extern f32 get_distance_to_active_camera(f32 x, f32 y, f32 z); /* distance to the camera */
extern void cam_shake_start(s32 arg0, f32 arg1, f32 arg2, f32 arg3, s32 strength); /* shakes the camera */
extern Object *func_80005F14(void *entry, s32 arg1); /* DKR spawn_object */

extern void func_80041CA0(Gfx **dl, s32 arg1, Object *obj, f32 scale, f32 arg4);
extern void func_80069BE8(Gfx **dl, void *flame, s32 arg2, u8 alpha);
extern void mtx_pop(Gfx **dl);
extern void effect_texture_pair_free(void *flame);

void func_m067_00000(UNUSED Object *obj) {
}

/*
 * Finds the jump point with the given id (its position and rotation go to the
 * pointers) and returns the jump kind from point `from` to it (2 with no `from`)
 */
static s32 func_m067_00008(s32 from, s32 to, f32 *x, f32 *y, f32 *z, s16 *yRotation, s16 *xRotation) {
    s32 first;
    s32 count;
    Object **objs;
    Object *o;
    M067JumpPoint *p;

    objs = func_80005E98(&first, &count);
    count -= first;
    objs = first + objs;
    while (count--) {
        o = *objs++;
        if (o->behaviorId == 0x68 && (p = o->unk68)->id == to) {
            *x = o->trans.x_position;
            *y = o->trans.y_position;
            *z = o->trans.z_position;
            *yRotation = o->trans.rotation.y_rotation;
            *xRotation = o->trans.rotation.x_rotation;
        }
    }
    if (from != -1) {
        return rl_m067_04580[from][to];
    }
    return 2;
}

/* Starts a jump to target (an index into the jump table); returns whether there was one */
s32 func_m067_00130(Object *obj, M067Jump *j, s32 target) {
    s32 kind;

    kind = func_m067_00008(j->target, target, &j->dest.x, &j->dest.y, &j->dest.z, &j->yRotation2, &j->xRotation2);
    if (kind != -1) {
        j->kind = kind;
        j->target = target;
        obj_anim_set(obj, rl_m067_04430[j->kind].anim, 0.0f);
        j->start.x = obj->trans.x_position;
        j->dest.x -= obj->trans.x_position;
        j->start.y = obj->trans.y_position;
        j->dest.y -= obj->trans.y_position;
        j->start.z = obj->trans.z_position;
        j->dest.z -= obj->trans.z_position;
        j->yRotation = obj->trans.rotation.y_rotation;
        j->yRotation2 = func_80049D80(obj->trans.rotation.y_rotation, j->yRotation2);
        j->xRotation = obj->trans.rotation.x_rotation;
        j->xRotation2 = func_80049D80(obj->trans.rotation.x_rotation, j->xRotation2);
        return TRUE;
    }
    return FALSE;
}

extern s32 rl_m067_046C0[]; /* .data (still assembly): the pair's values, two per step */

/* Gives both enemies of the pair their next values from the table and moves the table on */
void func_m067_0025C(void) {
    M067State *s;

    if (rl_m067_04574[0] != NULL) {
        s = rl_m067_04574[0]->unk68;
        s->unk4 = rl_m067_046C0[rl_m067_0456C];
        s->unk8 = 0;
        if (s->unkF == -1) {
            s->unkF = 12;
        }
    }
    if (rl_m067_04574[1] != NULL) {
        s = rl_m067_04574[1]->unk68;
        s->unk4 = rl_m067_046C0[rl_m067_0456C + 1];
        s->unk8 = 0;
        if (s->unkF == -1) {
            s->unkF = 12;
        }
    }
    rl_m067_0456C += 2;
}

/*
 * Runs one step of the enemy's step list: -2 to -18 set the enemy's kind and animation (some also set the
 * countdown, raise flags or play a sound); any other step starts a jump to that target
 */
static void func_m067_00300(Object *obj, M067State *s, s32 step) {
    switch (step) {
        case -2:
            s->kind = 0;
            s->unkF = 120;
            obj_anim_set(obj, 0, 0.0f);
            break;
        case -3:
            s->kind = 1;
            obj_anim_set(obj, 15, 0.0f);
            break;
        case -4:
            s->unkB |= 0xC0;
            s->kind = 10;
            obj_anim_set(obj, 10, 0.0f);
            break;
        case -5:
            s->unkB |= 0xC0;
            s->kind = 11;
            obj_anim_set(obj, 11, 0.0f);
            break;
        case -6:
            s->kind = 12;
            s->unkF = 120;
            obj_anim_set(obj, 2, 0.0f);
            break;
        case -7:
            s->kind = 13;
            obj_anim_set(obj, 8, 0.0f);
            break;
        case -8:
            s->kind = 14;
            obj_anim_set(obj, 9, 0.0f);
            break;
        case -9:
            s->unkC |= 0xC0;
            s->kind = 15;
            obj_anim_set(obj, 20, 0.0f);
            break;
        case -10:
            s->kind = 16;
            obj_anim_set(obj, 21, 0.0f);
            break;
        case -11:
            func_80003410(0x1B7, obj->trans.x_position, obj->trans.y_position, obj->trans.z_position, 4, NULL);
            s->kind = 17;
            obj_anim_set(obj, 22, 0.0f);
            break;
        case -12:
            func_80003410(0x1B8, obj->trans.x_position, obj->trans.y_position, obj->trans.z_position, 4, NULL);
            s->kind = 18;
            obj_anim_set(obj, 23, 0.0f);
            break;
        case -13:
            func_80003410(0x1B7, obj->trans.x_position, obj->trans.y_position, obj->trans.z_position, 4, NULL);
            s->kind = 19;
            obj_anim_set(obj, 24, 0.0f);
            break;
        case -18:
            s->kind = 0;
            s->unkF = -1;
            obj_anim_set(obj, 0, 0.0f);
            break;
        case -14:
            s->unkE |= 0xC0;
            s->kind = 20;
            obj_anim_set(obj, 13, 0.0f);
            break;
        case -15:
            s->unkE |= 0xC0;
            s->kind = 21;
            obj_anim_set(obj, 14, 0.0f);
            break;
        case -16:
            s->kind = 22;
            obj_anim_set(obj, 25, 0.0f);
            break;
        case -17:
            func_80003410(0x1B8, obj->trans.x_position, obj->trans.y_position, obj->trans.z_position, 4, NULL);
            s->kind = 23;
            obj_anim_set(obj, 7, 0.0f);
            break;
        default:
            func_m067_00130(obj, (M067Jump *) s, step);
            break;
    }
}

/*
 * Runs the next step of the enemy's step list (s->unk4): -1 goes back to the
 * start and on to the first -19, a -19 and a step equal to the target are
 * skipped
 */
void func_m067_0064C(Object *obj, M067State *s) {
    if (((s8 *) s->unk4)[s->unk8] == -1) {
        s->unk8 = 0;
        while (((s8 *) s->unk4)[s->unk8] != -19) {
            s->unk8++;
        }
    }
    if (((s8 *) s->unk4)[s->unk8] == -19) {
        s->unk8++;
    }
    if (((s8 *) s->unk4)[s->unk8] == s->target) {
        s->unk8++;
    }
    func_m067_00300(obj, s, ((s8 *) s->unk4)[s->unk8]);
    s->unk8++;
}

/* Landing: shakes the camera by how near it is, plays the thud and raises dust */
void func_m067_00728(Object *obj, UNUSED M067State *s) {
    f32 dist;
    s32 strength;
    M067DustEntry entry;
    Object *dust;

    dist = get_distance_to_active_camera(obj->trans.x_position, obj->trans.y_position, obj->trans.z_position);
    if (dist < 2000.0f) {
        if (dist < 800.0f) {
            strength = 20;
        } else {
            strength = (2000.0f - dist) * 20.0f / 1200.0f;
        }
        cam_shake_start(0, 0.0f, 0.0f, 2.0f, strength);
    }
    func_80003410(0x1B9, obj->trans.x_position, obj->trans.y_position, obj->trans.z_position, 4, NULL);
    entry.objectID = 0x13B;
    entry.x = obj->trans.x_position;
    entry.y = obj->trans.y_position;
    entry.z = obj->trans.z_position;
    entry.yRotation = 0;
    entry.xRotation = -0x4000;
    entry.unkE = 45;
    entry.scale = 1.0f;
    dust = func_80005F14(&entry, 1);
    if (dust != NULL) {
        dust->level_entry = NULL;
    }
}

/* The points of the enemy's model that its events happen at */
typedef struct M067Points {
    /* 0x00 */ u8 pad0[0x70];
    /* 0x70 */ Vec3f *points;
} M067Points;

/* The level entry of a piece an event throws out */
typedef struct M067DebrisEntry {
    /* 0x00 */ s16 objectID;
    /* 0x02 */ u8 pad2[0x4 - 0x2];
    /* 0x04 */ s16 x;
    /* 0x06 */ s16 y;
    /* 0x08 */ s16 z;
    /* 0x0A */ s16 yRotation;
    /* 0x0C */ s16 xRotation;
    /* 0x0E */ u8 unkE;
    /* 0x0F */ u8 unkF;
    /* 0x10 */ f32 unk10;
    /* 0x14 */ f32 unk14;
} M067DebrisEntry;

extern void func_m008_001CC(f32 x, f32 y, f32 z, f32 vx, f32 vy, f32 vz, s32 angle, s32 colour, f32 scale);

#ifdef NON_MATCHING
/*
 * Plays the enemy's event list (s->events): when an event's time has passed, the next event happens at one of
 * the model's points: a part's flag, drops, sparks, pieces, a burst and a sound. A zero time ends the list.
 * (Matches, staged: only the assembly func_m067_00EF4 calls it, so IDO would drop it.)
 */
static void func_m067_008C4(Object *obj, M067State *s, s32 updateRate) {
    Object *piece;
    Vec3s rot;
    f32 x;
    f32 y;
    f32 z;
    Vec3f vel;
    SparkParams spark;
    M067DebrisEntry debris;
    s32 n;
    M067Points *model;
    M067Event *ev;

    ev = s->events;
    if (ev == NULL) {
        return;
    }
    model = *(M067Points **) obj->modelInstances;
    s->unk38 += updateRate;
    while (s->unk38 >= ev->time) {
        s->unk38 -= ev->time;
        ev++;
        if (ev->time != 0) {
            x = model->points[ev->point].x;
            y = model->points[ev->point].y;
            z = model->points[ev->point].z;
            if (ev->part != -1) {
                s->unk3C[ev->part] = ev->value;
            }
            n = ev->numDrops;
            if (n != 0) {
                while (n--) {
                    rot.y_rotation = func_80048E2C(-0x8000, 0x7FFF);
                    rot.x_rotation = func_80048E2C(0, 0x3000);
                    vel.z = func_80048E2C(450, 600) * -0.01f;
                    func_8004944C(&rot, &vel);
                    func_m008_001CC(x, y, z, vel.x, vel.y, vel.z, 0xFF, 1, 1.0f);
                }
            }
            n = ev->numSparks;
            if (n != 0) {
                spark.x_position = x;
                spark.y_position = y;
                spark.z_position = z;
                spark.textureID = 20;
                spark.lifeSteps = 10;
                spark.alpha = 255;
                spark.colour1 = ev->colour1;
                spark.colour2 = ev->colour2;
                while (n--) {
                    spark.y_rotation = func_80048E2C(-0x8000, 0x7FFF);
                    spark.x_rotation = func_80048E2C(0, 0x3000);
                    spark.speed = func_80048E2C(70, 90) * 0.1f;
                    spark.sparkSize = func_80048E2C(25, 50) * 0.1f;
                    func_m010_0000C(&spark);
                }
            }
            n = ev->numPieces;
            if (n != 0) {
                debris.objectID = ev->pieceID;
                debris.unkE = 60;
                debris.unkF = 1;
                debris.unk14 = -0.3f;
                debris.x = x;
                debris.y = y;
                debris.z = z;
                while (n--) {
                    debris.yRotation = func_80048E2C(-0x8000, 0x7FFF);
                    debris.xRotation = func_80048E2C(0, 0x3000);
                    debris.unk10 = func_80048E2C(750, 900) * 0.01f;
                    piece = func_80005F14(&debris, 1);
                    if (piece != NULL) {
                        piece->level_entry = NULL;
                    }
                }
            }
            if (ev->burst != -1) {
                spawn_object_from_list(ev->burst, x, y, z, 100, 0);
            }
            if (ev->sound != -1) {
                func_80003410(ev->sound, x, y, z, 4, NULL);
            }
        } else {
            ev = NULL;
            break;
        }
    }
    s->events = ev;
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m067/ro", func_m067_008C4);
#endif

void func_m067_00D70(Object *obj, M067Setup *setup) {
    M067Header *header;
    M067State *s = obj->unk68;

    obj->trans.rotation.y_rotation = setup->yRotation << 8;
    s->side = setup->side;
    rl_m067_04574[s->side & 1] = obj;
    s->kind = 0;
    s->target = -1;
    s->unk3 = 0;
    if (rl_m067_04574[0] != NULL && rl_m067_04574[1] != NULL) {
        rl_m067_0456C = 0;
        rl_m067_0025C();
    }
    s->unkF = 60;
    s->unk9 = 0;
    s->unkA = 0;
    s->unkB = 0;
    s->unkC = 0;
    s->unkD = 0;
    s->unkE = 0;
    obj_anim_set(obj, rl_m067_04430[s->kind].anim, 0.0f);
    s->events = NULL;
    s->unk38 = 0;
    s->unk3C[0] = 255;
    s->unk3C[1] = 255;
    s->unk3C[2] = 255;
    s->unk3C[3] = 255;
    s->unk3C[4] = 255;
    s->unk3C[5] = 255;
    header = (M067Header *) obj->header;
    obj_collision_spheres_init(obj, 0xF00, 1, 1, header->hitX, header->hitY, header->hitZ, header->hitRadius,
                  header->hitHeight, -1.0f, 1.0f, 1);
    ((u8 *) obj->interactObj)[0x31] = 255;
    if (!runlink_module_loaded(0x27)) {
        func_m039_00000();
    }
}

/* A missile's set-up */
typedef struct M067MissileSetup {
    LevelObjectEntryCommon common;
    /* 0x0A */ s16 yRotation;
    /* 0x0C */ s16 xRotation;
    /* 0x0E */ s16 zRotation;
    /* 0x10 */ u8 scale;
    /* 0x11 */ s8 speedY;
    /* 0x12 */ u16 unk12;
    /* 0x14 */ s16 unk14;
    /* 0x16 */ s16 unk16;
    /* 0x18 */ s16 unk18;
    /* 0x1A */ s16 unk1A;
    /* 0x1C */ Object *target;
} M067MissileSetup;

/* One of the enemy's hit spheres (size 0x34) */
typedef struct M067Sphere {
    /* 0x00 */ u8 pad0[0x30];
    /* 0x30 */ union {
        u8 flags;
        struct {
            u8 unk80 : 1;
            u8 hidden : 1; /* 0x40 */
            u8 active : 1; /* 0x20 */
            u8 unk1F : 5;
        } bits;
    } f;
    /* 0x31 */ u8 pad31[0x34 - 0x31];
} M067Sphere;

/* What obj->interactObj points at here */
typedef struct M067Interaction {
    /* 0x00 */ u8 pad0[0x6];
    /* 0x06 */ s16 hitPoints;
    /* 0x08 */ u8 pad8[0xC - 0x8];
    /* 0x0C */ f32 x;
    /* 0x10 */ f32 y;
    /* 0x14 */ f32 z;
    /* 0x18 */ u8 pad18[0x31 - 0x18];
    /* 0x31 */ u8 hitSphere; /* the sphere hit (0xFF: none) */
    /* 0x32 */ u8 pad32[0x40 - 0x32];
    /* 0x40 */ M067Sphere *spheres;
} M067Interaction;

/* The enemy's model instance, as used here */
typedef struct M067Instance {
    /* 0x00 */ void *model;
    /* 0x04 */ u8 pad4[0xA - 0x4];
    /* 0x0A */ s8 unkA;
    /* 0x0B */ s8 unkB;
    /* 0x0C */ u8 padC[0x10 - 0xC];
    /* 0x10 */ void *unk10[2];
    /* 0x18 */ u8 pad18[0x68 - 0x18];
    /* 0x68 */ Vec3f *points;  /* where the shots start */
    /* 0x6C */ u8 pad6C[0x70 - 0x6C];
    /* 0x70 */ Vec3f *points2;
} M067Instance;

/* The player's state, as used here */
typedef struct M067PlayerState {
    /* 0x000 */ u8 pad0[0x5C8];
    /* 0x5C8 */ s16 unk5C8;
} M067PlayerState;

extern u8 rl_m067_044F0[][2];   /* each hit sphere's part ([1]; 0xFE: none, 0xFF ends the table) */
extern s32 rl_m067_04570;       /* the music has been started */
extern M067Event rl_m067_04700[]; /* the event lists of the four deaths */
extern M067Event rl_m067_047C0[];
extern M067Event rl_m067_048C8[];
extern M067Event rl_m067_049A0[];
extern f32 rl_m067_04C70[];     /* the speeds of a rock burst */

extern s32 obj_anim_frame_advance(Object *obj, f32 speed, f32 updateRate);
extern void func_8003D21C(void *inst, void *model, Object *obj, f32 arg3, f32 arg4);
extern void func_800221F0(Object *obj, void *inst, void *shading, void *arg3);
extern s32 func_80049C84(f32 x, f32 z);         /* the angle of a direction */
extern f32 func_80097140(f32 x);                /* sqrtf */
extern void chain_object_unlink_from_players(Object *obj);
extern void music_volume_set(u8 volume);
extern void music_play(u8 seqID);
extern void music_volume_fade(f32 volume, u8 arg1);
extern s32 pending_level_character(void);
extern s32 menu_mode_get(void);
extern void level_goto(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5);

/*
 * One jump animation step: between the animation's times `from` and `to` the enemy moves from the jump's
 * start to its end on an arc `height` high and turns to the end's rotation; it lands at `to`
 */
#define M067_JUMP(from, to, height)                                                                       \
    done = obj_anim_frame_advance(obj, 0.0085f, updateRate);                                              \
    if (done) {                                                                                           \
        obj->unk28 = 1.0f;                                                                                \
    }                                                                                                     \
    f = (obj->unk28 - (from)) / ((to) - (from));                                                          \
    if (f > 0.0f) {                                                                                       \
        if (f > 1.0f) {                                                                                   \
            f = 1.0f;                                                                                     \
        }                                                                                                 \
        dx = s->dest.x * f + s->start.x - obj->trans.x_position;                                          \
        dy = func_80049C34(32768.0f * f) * (height) + (s->dest.y * f + s->start.y - obj->trans.y_position); \
        dz = s->dest.z * f + s->start.z - obj->trans.z_position;                                          \
        obj->trans.rotation.y_rotation = (s32) (s->yRotation2 * f) + s->yRotation;                        \
        obj->trans.rotation.x_rotation = (s32) (s->xRotation2 * f) + s->xRotation;                        \
        func_80009A04(obj, dx, dy, dz);                                                                   \
    }                                                                                                     \
    if (prev < (to) && obj->unk28 >= (to)) {                                                              \
        func_m067_00728(obj, s);                                                                          \
    }                                                                                                     \
    if (done) {                                                                                           \
        func_m067_0064C(obj, s);                                                                          \
    }

/* Where an object's interaction keeps its position */
#define M067_TARGET(obj) ((M067Interaction *) (obj)->interactObj)

#ifdef NON_MATCHING
/*
 * The enemy's control: turns its head after the player, updates its parts' flags and the hit flashes, plays
 * its events and builds its joint commands, then runs its animation by its kind (jumps between the jump
 * points, attacks, being hit and dying), draws the model's points and fires what the animation asked for
 * (a missile, bolts or rocks).
 * NON_MATCHING: ROM length, frame and stack slots (1855 words), 24 aligned diffs, all between the parts loop
 * and the 008C4 call: the parts loop's table pointer and flags value (a0/a2, ROM a1/a0), the temporaries from
 * its `| 0x40` on one t-register behind the ROM's, and the decay loop's result register. Matching it would let
 * 008C4 go to C too.
 */
void func_m067_00EF4(Object *obj, s32 updateRate) {
    s32 i;
    s32 done;
    s32 angle;
    M067State *s;
    s16 *cur;
    f32 dx;
    f32 dy;
    f32 dz;
    f32 f;
    f32 prev;
    M067MissileSetup missile;
    M067ShotSetup bolt;
    M067ShotSetup rock;
    M067Sphere *sphere;
    u8 (*parts)[2];
    M067Instance *inst;
    M067PlayerState *playerState;
    s32 part;
    s32 flags;
    Object *shot;
    Object *player;

    s = obj->unk68;
    cur = s->joints;
    player = func_8000E4D0(0);
    inst = *(M067Instance **) obj->modelInstances;
    if (player == NULL || inst == NULL) {
        return;
    }
    playerState = player->unk68;
    dx = obj->trans.x_position - player->trans.x_position;
    dz = obj->trans.z_position - player->trans.z_position;
    angle = func_80049C84(dx, dz) - obj->trans.rotation.y_rotation;
    i = updateRate;
    while (i--) {
        s->headAngle += func_80049D80(s->headAngle, angle) >> 3;
    }
    *cur++ = 8;
    *cur++ = s->headAngle;
    *cur++ = 200;
    *cur++ = s->headAngle >> 2;
    *cur++ = 206;
    *cur++ = s->headAngle >> 2;
    *cur++ = 212;
    *cur++ = s->headAngle >> 2;
    *cur++ = 218;
    *cur++ = s->headAngle >> 2;
    sphere = ((M067Interaction *) obj->interactObj)->spheres;
    parts = rl_m067_044F0;
    while ((*parts)[1] != 0xFF) {
        sphere->f.bits.active = 1;
        sphere->f.bits.hidden = 0;
        part = (*parts)[1];
        if (part != 0xFE) {
            i = (&s->unk9)[part]; /* i (free here) holds the part's flags */
            if (i & 1) {
                sphere->f.flags |= 0x40;
            } else if (i & 2) {
                sphere->f.bits.active = 0;
            }
        }
        sphere++;
        parts++;
    }
    i = 6;
    while (i--) {
        flags = (&s->unk9)[i];
        if (flags & 0x30) {
            flags -= 0x10;
        }
        if (flags & 0xC0) {
            flags -= 0x40;
        }
        (&s->unk9)[i] = flags & ~4;
    }
    flags = ((M067Interaction *) obj->interactObj)->hitSphere;
    if (flags != 0xFF) {
        i = rl_m067_044F0[flags][1];
        if (i != 0xFE) {
            if ((&s->unk9)[i] & 2) {
                (&s->unk9)[i] |= 4;
            }
        }
        ((M067Interaction *) obj->interactObj)->hitSphere = 0xFF;
    }
    i = 6;
    while (i--) {
        (&s->unk9)[i] &= ~2;
    }
    func_m067_008C4(obj, s, updateRate);
    i = 6;
    while (i--) {
        if (s->unk3C[i] != 0xFF) {
            *cur++ = (s->unk3C[i] * 6) | 0x4000;
            *cur++ = 0x20;
            *cur++ = (s->unk3C[i] * 6 + 2) | 0x4000;
            *cur++ = 0x20;
            *cur++ = (s->unk3C[i] * 6 + 4) | 0x4000;
            *cur++ = 0x20;
        }
    }
    *cur = 0x1000;
    if (s->side == 0) {
        func_m067_00000(obj);
    }
    prev = obj->unk28;
    switch (s->kind) {
        case 0:
        case 12:
            obj_anim_frame_advance(obj, rl_m067_04430[s->kind].speed, updateRate);
            if (s->unkF != -1) {
                s->unkF -= updateRate;
                if (s->unkF <= 0) {
                    s->unkF = 0;
                    func_m067_0064C(obj, s);
                }
            } else {
                Object *other = rl_m067_04574[(s->side ^ 1) & 1];

                if (other != NULL && ((M067State *) other->unk68)->unkF == -1) {
                    s->unkF = 15;
                    ((M067State *) other->unk68)->unkF = 15;
                }
            }
            break;
        case 1:
            done = obj_anim_frame_advance(obj, 0.00375f, updateRate);
            if (prev < 0.227f && obj->unk28 >= 0.227f) {
                s->unk3 = 1;
            }
            if (prev < 0.548f && obj->unk28 >= 0.548f) {
                s->unk3 = 1;
            }
            if (done) {
                func_m067_0064C(obj, s);
            }
            break;
        case 10:
        case 11:
            if (s->kind == 10) {
                done = obj_anim_frame_advance(obj, 0.007f, updateRate);
                if (prev < 0.518f) {
                    s->unkB |= 2;
                    if (obj->unk28 >= 0.518f) {
                        s->unk3 = 10;
                    }
                }
            } else {
                done = obj_anim_frame_advance(obj, 0.005f, updateRate);
                if (prev < 0.64f) {
                    s->unkB |= 2;
                    if (obj->unk28 >= 0.64f) {
                        s->unk3 = 11;
                    }
                }
            }
            if (obj->interactObj->hitPoints <= 0) {
                s->events = rl_m067_04700;
                s->unk38 = 0;
                func_m067_00300(obj, s, -11);
            } else if (done) {
                func_m067_0064C(obj, s);
            }
            break;
        case 2:
            M067_JUMP(0.345f, 0.72f, 400.0f);
            break;
        case 3:
            M067_JUMP(0.345f, 0.72f, 400.0f);
            break;
        case 4:
        case 6:
        case 8:
            M067_JUMP(0.42f, 0.812f, 250.0f);
            break;
        case 5:
        case 7:
        case 9:
            M067_JUMP(0.402f, 0.777f, 250.0f);
            break;
        case 13:
        case 14:
            if (obj_anim_frame_advance(obj, rl_m067_04430[s->kind].speed, updateRate)) {
                func_m067_0064C(obj, s);
            }
            break;
        case 15:
            done = obj_anim_frame_advance(obj, 0.00667f, updateRate);
            if (prev < 0.63f) {
                s->unkC |= 2;
                if (obj->unk28 >= 0.63f) {
                    s->unk3 = 15;
                }
            }
            if (obj->interactObj->hitPoints <= 0) {
                s->events = rl_m067_047C0;
                s->unk38 = 0;
                func_m067_00300(obj, s, -12);
            } else if (done) {
                func_m067_0064C(obj, s);
            }
            break;
        case 16:
            done = obj_anim_frame_advance(obj, 0.00834f, updateRate);
            if (prev < 0.615f) {
                s->unkA |= 2;
                if (obj->unk28 >= 0.615f) {
                    s->unk3 = 16;
                }
            }
            if (obj->interactObj->hitPoints <= 0) {
                if (player->interactObj->hitPoints < 0x100) {
                    obj->interactObj->hitPoints = 0x100;
                } else if (rl_m067_04574[0] != NULL && rl_m067_04574[1] != NULL) {
                    s->events = rl_m067_049A0;
                    s->unk38 = 0;
                    func_m067_00300(obj, s, -17);
                } else {
                    playerState->unk5C8 = 60;
                    music_volume_fade(0.5f, 0);
                    level_goto(0x189, pending_level_character(), 0, menu_mode_get(), 0, 0);
                }
            } else if (done) {
                func_m067_0064C(obj, s);
            }
            break;
        case 17:
            if (obj_anim_frame_advance(obj, 0.00417f, updateRate)) {
                func_m067_0025C();
                s->unkB |= 1;
                chain_object_unlink_from_players(obj);
                obj->interactObj->hitPoints = 0xF00;
                func_m067_0064C(obj, s);
            }
            break;
        case 18:
            if (obj_anim_frame_advance(obj, 0.00417f, updateRate)) {
                func_m067_0025C();
                s->unkC |= 1;
                chain_object_unlink_from_players(obj);
                obj->interactObj->hitPoints = 0x1E00;
                func_m067_0064C(obj, s);
            }
            break;
        case 19:
            if (obj_anim_frame_advance(obj, 0.00417f, updateRate)) {
                func_m067_0025C();
                s->unkE |= 1;
                chain_object_unlink_from_players(obj);
                obj->interactObj->hitPoints = 0x3200;
                func_m067_0064C(obj, s);
            }
            break;
        case 20:
        case 21:
            if (s->kind == 20) {
                done = obj_anim_frame_advance(obj, 0.008f, updateRate);
                if (prev < 0.586f) {
                    s->unkE |= 2;
                    if (obj->unk28 >= 0.586f) {
                        s->unk3 = 20;
                    }
                }
            } else {
                done = obj_anim_frame_advance(obj, 0.008f, updateRate);
                if (prev < 0.586f) {
                    s->unkE |= 2;
                    if (obj->unk28 >= 0.586f) {
                        s->unk3 = 21;
                    }
                }
            }
            if (obj->interactObj->hitPoints <= 0) {
                s->events = rl_m067_048C8;
                s->unk38 = 0;
                func_m067_00300(obj, s, -13);
            } else if (done) {
                func_m067_0064C(obj, s);
            }
            break;
        case 22:
            done = obj_anim_frame_advance(obj, 0.00667f, updateRate);
            if (prev < 0.475f && obj->unk28 >= 0.475f) {
                func_80001248(0x1B2, 0);
            }
            if (done) {
                obj->unk28 = 1.0f;
                if (rl_m067_04570 == 0) {
                    music_volume_set(0x70);
                    music_play(0x35);
                    music_volume_fade(10.0f, 0x50);
                    rl_m067_04570 = 1;
                }
                func_m067_0064C(obj, s);
            }
            break;
        case 23:
            if (obj_anim_frame_advance(obj, 0.00413f, updateRate)) {
                rl_m067_04574[s->side] = NULL;
                if (rl_m067_04574[0] == NULL && rl_m067_04574[1] == NULL) {
                    level_goto(0x189, pending_level_character(), 0, menu_mode_get(), 1, 0);
                } else {
                    func_m067_0025C();
                }
                func_800079B8(obj);
                return;
            }
            break;
    }
    if (inst != NULL && inst->unkA != 0) {
        func_8003D21C(inst, inst->model, obj, 1.0f, 0.0f);
        func_800221F0(obj, inst, obj->shading, inst->unk10[inst->unkB]);
        inst->unkA = 0;
    }
    switch (s->unk3) {
        case 1:
            missile.common.objectID = 0x2D6;
            missile.xRotation = 0;
            missile.zRotation = 0;
            missile.scale = 5;
            missile.speedY = -75;
            missile.unk12 = 0x113;
            missile.unk14 = 0x16C;
            missile.unk16 = 0x16C;
            missile.unk18 = 0x888;
            missile.unk1A = 40;
            missile.target = func_8000E4D0(0);
            if (missile.target != NULL) {
                missile.common.x = inst->points[1].x;
                missile.common.y = inst->points[1].y;
                missile.common.z = inst->points[1].z;
                missile.yRotation = obj->trans.rotation.y_rotation + 0x4000;
                shot = func_80005F14(&missile, 1);
                if (shot != NULL) {
                    shot->level_entry = NULL;
                }
                missile.common.x = inst->points[2].x;
                missile.common.y = inst->points[2].y;
                missile.common.z = inst->points[2].z;
                missile.yRotation = obj->trans.rotation.y_rotation - 0x4000;
                shot = func_80005F14(&missile, 1);
                if (shot != NULL) {
                    shot->level_entry = NULL;
                }
            }
            func_80003410(0xF3, obj->trans.x_position, obj->trans.y_position, obj->trans.z_position, 4, NULL);
            break;
        case 10:
        case 11:
            dx = M067_TARGET(player)->x - inst->points[8].x;
            dy = M067_TARGET(player)->y - inst->points[8].y;
            dz = M067_TARGET(player)->z - inst->points[8].z;
            bolt.common.objectID = 0x2D9;
            bolt.common.x = inst->points[8].x;
            bolt.common.y = inst->points[8].y;
            bolt.common.z = inst->points[8].z;
            bolt.strength = 2;
            bolt.time = 20;
            bolt.yRotation = func_80049C84(dx, dz) + 0x8000;
            bolt.xRotation = func_80049C84(dy, func_80097140(dx * dx + dz * dz));
            bolt.owner = obj;
            bolt.scale = 0.25f;
            bolt.speed = 16.0f;
            shot = func_80005F14(&bolt, 1);
            if (shot != NULL) {
                shot->level_entry = NULL;
                func_80003410(0x1BB, shot->trans.x_position, shot->trans.y_position, shot->trans.z_position, 4, NULL);
            }
            break;
        case 15:
            dx = M067_TARGET(player)->x - inst->points[12].x;
            dy = M067_TARGET(player)->y - inst->points[12].y;
            dz = M067_TARGET(player)->z - inst->points[12].z;
            rock.common.objectID = 0x2E7;
            rock.common.x = inst->points[12].x;
            rock.common.y = inst->points[12].y;
            rock.common.z = inst->points[12].z;
            rock.strength = 3;
            rock.time = 20;
            rock.yRotation = func_80049C84(dx, dz) + 0x8000;
            rock.xRotation = func_80049C84(dy, func_80097140(dx * dx + dz * dz));
            rock.owner = obj;
            rock.scale = 1.0f;
            rock.speed = 20.0f;
            shot = func_80005F14(&rock, 1);
            if (shot != NULL) {
                shot->level_entry = NULL;
                func_80003410(0x1BC, shot->trans.x_position, shot->trans.y_position, shot->trans.z_position, 4, NULL);
            }
            break;
        case 16:
            dx = M067_TARGET(player)->x - inst->points2[19].x;
            dy = M067_TARGET(player)->y - inst->points2[19].y;
            dz = M067_TARGET(player)->z - inst->points2[19].z;
            rock.common.objectID = 0x2E7;
            rock.common.x = inst->points2[19].x;
            rock.common.y = inst->points2[19].y;
            rock.common.z = inst->points2[19].z;
            rock.strength = 2;
            rock.time = 25;
            rock.yRotation = func_80049C84(dx, dz) + 0x7800;
            rock.xRotation = func_80049C84(dy, func_80097140(dx * dx + dz * dz));
            rock.owner = obj;
            rock.scale = 0.5f;
            i = 5;
            while (i--) {
                rock.speed = rl_m067_04C70[i] * 16.0f;
                shot = func_80005F14(&rock, 1);
                if (shot != NULL) {
                    shot->level_entry = NULL;
                }
                rock.yRotation += 0x400;
            }
            func_80003410(0xC4, shot->trans.x_position, shot->trans.y_position, shot->trans.z_position, 4, NULL);
            break;
        case 20:
        case 21:
            dx = M067_TARGET(player)->x - inst->points[0].x;
            dy = M067_TARGET(player)->y - inst->points[0].y;
            dz = M067_TARGET(player)->z - inst->points[0].z;
            bolt.common.objectID = 0x2D9;
            bolt.common.x = inst->points[0].x;
            bolt.common.y = inst->points[0].y;
            bolt.common.z = inst->points[0].z;
            bolt.strength = 4;
            bolt.time = 20;
            bolt.yRotation = func_80049C84(dx, dz) + 0x8000;
            bolt.xRotation = func_80049C84(dy, func_80097140(dx * dx + dz * dz));
            bolt.owner = obj;
            bolt.scale = 0.35f;
            bolt.speed = 20.0f;
            shot = func_80005F14(&bolt, 1);
            if (shot != NULL) {
                shot->level_entry = NULL;
                func_80003410(0xC5, shot->trans.x_position, shot->trans.y_position, shot->trans.z_position, 4, NULL);
            }
            break;
    }
    s->unk3 = 0;
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m067/ro", func_m067_00EF4);
#endif

/* Sets the fog for drawing one of the enemy's parts: a flash colour, or the level's fog */
void func_m067_02BF0(Gfx **dl, Object *obj, s32 part) {
    s16 fogNear;
    s16 fogFar;
    s16 unused;
    u8 r;
    u8 g;
    u8 b;
    u8 unused2;
    s32 flags;
    M067State *s;

    if (part >= 0 && part < 6) {
        s = obj->unk68;
        flags = (&s->unk9)[part];
        if (flags & 0x40) {
            gDPSetFogColor((*dl)++, 255, 255, 255, 255);
            gSPFogFactor((*dl)++, 0x500, 0x500);
        } else if (flags & 0x10) {
            gDPSetFogColor((*dl)++, 0x32, 0xFF, 0x32, 0xFF);
            gSPFogFactor((*dl)++, 0x500, 0x500);
        } else if (flags & 4) {
            gDPSetFogColor((*dl)++, 0xFF, 0x32, 0x32, 0xFF);
            gSPFogFactor((*dl)++, 0x500, 0x500);
        } else {
            func_8001BC2C(0, &fogNear, &fogFar, &unused, &r, &g, &b, &unused2);
            gDPSetFogColor((*dl)++, r, g, b, 255);
            gSPFogPosition((*dl)++, fogNear, fogFar);
        }
    }
}

/* A missile's whole state (obj->unk68) */
typedef struct M067MissileState {
    /* 0x00 */ u8 unk0;
    /* 0x01 */ u8 unk1;
    /* 0x02 */ u8 unk2;
    /* 0x03 */ u8 unk3;
    /* 0x04 */ f32 thrust;
    /* 0x08 */ void *flame;
    /* 0x0C */ f32 unkC;
    /* 0x10 */ f32 unk10;
    /* 0x14 */ s16 unk14;
    /* 0x16 */ s16 unk16;
    /* 0x18 */ s16 unk18;
    /* 0x1A */ s16 unk1A;
    /* 0x1C */ s16 unk1C;
    /* 0x1E */ s16 unk1E;
    /* 0x20 */ Object *target;
} M067MissileState;

/* A model vertex (size 0xA) */
typedef struct M067Vertex {
    /* 0x0 */ s16 x;
    /* 0x2 */ s16 y;
    /* 0x4 */ s16 z;
    /* 0x6 */ u8 pad6[4];
} M067Vertex;

/* The missile's model */
typedef struct M067Model {
    /* 0x00 */ u8 pad0[0x1C];
    /* 0x1C */ M067Vertex *vertices;
    /* 0x20 */ u8 pad20[0x30 - 0x20];
    /* 0x30 */ u16 *flameVertex; /* the vertex the flame comes from */
} M067Model;

extern void *effect_mesh_create(s16 x, s16 y, s16 z, s16 arg3, s16 arg4, f32 arg5, f32 arg6, s32 arg7, s32 arg8,
                                s32 arg9);

void func_m067_02E38(Object *obj, M067MissileSetup *setup) {
    f32 k;
    M067Model *model;
    M067Vertex *v;
    M067MissileState *m;

    k = 0.1f;
    obj->trans.rotation.y_rotation = setup->yRotation;
    obj->trans.rotation.x_rotation = setup->xRotation;
    m = obj->unk68;
    obj->trans.rotation.z_rotation = setup->zRotation;
    obj->trans.scale = setup->scale * k;
    obj->velocity.x = 0.0f;
    obj->velocity.y = setup->speedY * k;
    obj->velocity.z = 0.0f;
    m->unkC = 0.0f;
    m->unk10 = setup->unk12 * k;
    m->unk1E = 0;
    m->unk14 = setup->unk14;
    m->unk16 = setup->unk16;
    m->unk18 = setup->unk18;
    m->unk1A = 0;
    m->unk1C = 0;
    m->unk1E = 0;
    m->target = setup->target;
    {
        M067Model **inst = obj->modelInstances[0];

        model = *inst;
    }
    v = &model->vertices[*model->flameVertex];
    m->thrust = 0.0f;
    m->unk2 = 0;
    m->unk3 = 0;
    m->flame = effect_mesh_create(v->x, v->y, v->z, -0x8000, 0, 20.0f, 200.0f, 8, 16, -1);
    m->unk0 = 0;
    m->unk1 = setup->unk1A * 6;
}

/* The query that func_8007C2E8 takes, as used here (m069's M069Ray) */
typedef struct M067Query {
    /* 0x00 */ u8 pad0[0xC];
    /* 0x0C */ f32 to[3];
    /* 0x18 */ f32 from[3];
    /* 0x24 */ f32 unk24;
    /* 0x28 */ f32 unk28;
    /* 0x2C */ u8 pad2C[0x54 - 0x2C];
} M067Query;

/* What obj->0x78 points at: a glow whose brightness the flame sets */
typedef struct M067Glow {
    /* 0x00 */ u8 pad0[0x3B];
    /* 0x3B */ u8 alpha;
} M067Glow;

/* Where the target's interaction keeps its position, as this shot reads it */
typedef struct M067TargetInteraction {
    /* 0x00 */ u8 pad0[0x6];
    /* 0x06 */ s16 hitPoints;
    /* 0x08 */ u8 pad8[0xC - 0x8];
    /* 0x0C */ f32 x;
    /* 0x10 */ f32 y;
    /* 0x14 */ f32 z;
} M067TargetInteraction;

extern s32 func_80015510(f32 x, f32 y, f32 z); /* the level segment at a position */
extern s32 func_8007C2E8(Object *obj, M067Query *query, s32 count, Object **objs, Object **hit, s32 arg5);
extern f32 func_80097140(f32 x);                /* sqrtf */
extern s32 func_80049C84(f32 x, f32 z);         /* the angle of a direction */
extern f32 func_80049BEC(f32 base, s32 exponent);
extern void effect_mesh_set_uvs(void *mesh, s16 angle);
extern void effect_mesh_update(void *mesh, s16 depth, s32 brightness);
extern void func_8005F5AC(Object *obj, s32 updateRate);

/*
 * A homing shot's control: it drifts to a stop, fires up its flame, then steers after its target (turning,
 * pitching and rolling towards it), until it hits the level, hits the target (0x200 damage) or runs out of
 * time, and then explodes. The flame's size and the shot's glow follow the thrust.
 */
void func_m067_02FDC(Object *obj, s32 updateRate) {
    M067MissileState *m;
    s32 i;
    f32 dx;
    f32 dz;
    f32 dy;
    f32 dist;
    s32 d;
    f32 tx;
    f32 ty;
    f32 tz;
    M067Query query;
    f32 k;
    HitResult hit;
    Object *hitObj;
    M067TargetInteraction *t;
    Object *list[1];
    s32 v; /* the state switched on, the loops' dead counter copies, then each steering limit */
    s32 alpha;
    s32 step;

    m = obj->unk68;
    obj->particleEmittersEnabled = 0;
    query.from[0] = obj->trans.x_position;
    query.from[1] = obj->trans.y_position;
    query.from[2] = obj->trans.z_position;
    switch (v = m->unk0) {
        case 0:
            i = updateRate;
            while (v = i, i--, v) {
                obj->trans.x_position += obj->velocity.x;
                obj->trans.y_position += obj->velocity.y;
                obj->trans.z_position += obj->velocity.z;
                obj->velocity.x *= 0.925f;
                obj->velocity.y *= 0.925f;
                obj->velocity.z *= 0.925f;
            }
            obj->segmentID = func_80015510(obj->trans.x_position, obj->trans.y_position, obj->trans.z_position);
            if (-0.1f < obj->velocity.y && obj->velocity.y < 0.1f) {
                m->unk0 = 1;
                func_80003410(0xBA, obj->trans.x_position, obj->trans.y_position, obj->trans.z_position, 4, NULL);
            }
            break;
        case 1:
            obj->particleEmittersEnabled |= 1;
            m->thrust += updateRate * 0.2f;
            if (2.0f < m->thrust) {
                m->unk0 = 2;
                m->thrust = 2.0f;
            }
            break;
        case 2:
            obj->particleEmittersEnabled |= 6;
            t = (M067TargetInteraction *) m->target->interactObj;
            tx = t->x;
            ty = t->y;
            tz = t->z;
            i = updateRate;
            while (v = i, i--, v) {
                dx = obj->trans.x_position - tx;
                dy = obj->trans.y_position - ty;
                dz = obj->trans.z_position - tz;
                dist = func_80097140(dx * dx + dz * dz);
                d = func_80049D80(obj->trans.rotation.y_rotation, func_80049C84(dx, dz));
                v = m->unk14 * m->unkC;
                if (d < 0 && d > -0x6000) {
                    m->unk1A += ((-v - m->unk1A) * 5) >> 6;
                } else if (d > 0 && d < 0x6000) {
                    m->unk1A += ((v - m->unk1A) * 5) >> 6;
                } else {
                    m->unk1A -= m->unk1A >> 3;
                }
                obj->trans.rotation.y_rotation += m->unk1A;
                d = func_80049D80(obj->trans.rotation.x_rotation, -func_80049C84(dy, dist));
                v = m->unk16 * m->unkC;
                if (d < 0 && d > -0x6000) {
                    m->unk1C += (-v - m->unk1C) >> 3;
                } else if (d > 0 && d < 0x6000) {
                    m->unk1C += (v - m->unk1C) >> 3;
                } else {
                    m->unk1C -= m->unk1C >> 3;
                }
                obj->trans.rotation.x_rotation += m->unk1C;
                m->unkC += (1.0f - m->unkC) * 0.125f;
                m->unk1E += (m->unk18 - m->unk1E) >> 4;
                obj->trans.rotation.z_rotation += m->unk1E;
                obj->velocity.z = -m->unkC * m->unk10;
                func_8004944C(&obj->trans.rotation, &obj->velocity);
                func_80009A04(obj, obj->velocity.x, obj->velocity.y, obj->velocity.z);
            }
            m->unk2 += updateRate << 4;
            k = func_80049BEC(0.9f, updateRate);
            m->thrust += ((1.0f + 0.25f * func_80049C34(m->unk2 << 8)) - m->thrust) * (1.0f - k);
            if (func_80018808(obj, (Vec3f *) query.from, (Vec3f *) &obj->trans.x_position, &hit, 0, 0)) {
                func_80009A04(obj, hit.pos.x - obj->trans.x_position, hit.pos.y - obj->trans.y_position,
                              hit.pos.z - obj->trans.z_position);
                m->unk0 = 3;
            }
            query.to[0] = obj->trans.x_position;
            query.to[1] = obj->trans.y_position;
            query.to[2] = obj->trans.z_position;
            query.unk24 = obj->trans.scale * 32.0f;
            query.unk28 = obj->trans.scale * 32.0f;
            list[0] = m->target;
            if (func_8007C2E8(obj, &query, 1, list, &hitObj, 0)) {
                ((M067TargetInteraction *) m->target->interactObj)->hitPoints -= 0x200;
                m->unk0 = 3;
            }
            if (updateRate < m->unk1) {
                m->unk1 -= updateRate;
            } else {
                m->unk0 = 3;
            }
            break;
        case 3:
            func_80003410(0x96, obj->trans.x_position, obj->trans.y_position, obj->trans.z_position, 4, NULL);
            spawn_object_from_list(3, obj->trans.x_position, obj->trans.y_position, obj->trans.z_position, 100, 0);
            func_800079B8(obj);
            break;
    }
    if (m->flame != NULL && 0.0f < m->thrust) {
        step = updateRate << 4;
        if (1.25f < m->thrust) {
            alpha = (m->thrust - 1.25f) * 128.0f;
        } else {
            alpha = 0;
        }
        if (*(M067Glow ***) obj->pad78 != NULL && **(M067Glow ***) obj->pad78 != NULL) {
            (**(M067Glow ***) obj->pad78)->alpha = alpha;
        }
        m->unk3 += step;
        effect_mesh_set_uvs(m->flame, m->unk3 << 8);
        effect_mesh_update(m->flame, m->thrust * 200.0f, 0xFF);
    }
    func_8005F5AC(obj, updateRate);
}

/* Draws a missile's flame */
void func_m067_03808(Gfx **dl, s32 arg1, Object *obj) {
    M067Missile *m = obj->unk68;

    if (m->flame != NULL && m->thrust > 0.0f) {
        func_80041CA0(dl, arg1, obj, 1.0f, 0.0f);
        func_80069BE8(dl, m->flame, 6, 255);
        mtx_pop(dl);
    }
}

/* Frees a missile's flame */
void func_m067_03888(Object *obj) {
    M067Missile *m = obj->unk68;

    if (m->flame != NULL) {
        effect_texture_pair_free(m->flame);
    }
}

void func_m067_038C0(Object *obj, M067ShotSetup *setup) {
    M067Shot *s = obj->unk68;

    obj->trans.rotation.y_rotation = setup->yRotation;
    obj->trans.rotation.x_rotation = setup->xRotation;
    obj->velocity.z = -setup->speed;
    func_8004944C(&obj->trans.rotation, &obj->velocity);
    obj->trans.scale = setup->scale;
    s->timer = setup->time * 6;
    s->owner = setup->owner;
    s->strength = setup->strength;
    s->unk7 = 0;
    obj_collision_spheres_init(obj, s->strength << 8, 1, 4, 0, 0, 0, 80.0f, 80.0f, 100.0f, 1.0f, 0x11);
}

/* A bolt: spins and flies until its time is up, and bursts when it hits something */
void func_m067_0399C(Object *obj, s32 updateRate) {
    f32 rate;
    f32 radius;
    Vec3f from;
    HitResult hit;
    M067Shot *s = obj->unk68;

    rate = updateRate;
    if (obj->interactObj->hitPoints <= 0 || s->unk7) {
        spawn_object_from_list(3, obj->trans.x_position, obj->trans.y_position, obj->trans.z_position, 100, 0);
        func_800079B8(obj);
        return;
    }
    s->timer -= updateRate;
    if (s->timer < 0) {
        func_800079B8(obj);
        return;
    }
    obj->trans.rotation.z_rotation += updateRate << 12;
    from.x = obj->trans.x_position;
    from.y = obj->trans.y_position;
    from.z = obj->trans.z_position;
    func_80009A04(obj, obj->velocity.x * rate, obj->velocity.y * rate, obj->velocity.z * rate);
    radius = ((M067HitShape *) obj->interactObj)->radius;
    func_8001CE10(1, &from, (Vec3f *) &obj->trans.x_position, &radius, 0xCE002000, 0, 0);
    if (func_80016208(obj, &from, (Vec3f *) &obj->trans.x_position, radius, 1, &hit)) {
        s->unk7 = 1;
    }
}

extern void func_80055C00(Gfx **dl, void *tex, s32 flags, s32 offset); /* sets a texture */
extern void func_80041DC0(Gfx **dl);
extern Vertex rl_m067_04CC8[];   /* the bolt's vertices (.data) */
extern Triangle rl_m067_04C88[]; /* its triangles (.data) */

/* Draws a bolt: five copies along its path, growing and fading */
void func_m067_03B7C(Gfx **dl, s32 arg1, Object *obj) {
    M067Shot *s;
    s32 kind;
    s32 i;
    f32 scale;
    f32 savedScale;
    s32 savedRot;
    f32 savedX;
    f32 savedY;
    f32 savedZ;
    s32 b;
    s32 g;
    s32 a;

    s = obj->unk68;
    if (s->strength == 2) {
        kind = 0;
    } else {
        kind = 1;
    }
    savedRot = obj->trans.rotation.z_rotation;
    savedX = obj->trans.x_position;
    savedY = obj->trans.y_position;
    savedZ = obj->trans.z_position;
    savedScale = obj->trans.scale;
    obj->trans.x_position -= 4.0f * obj->velocity.x;
    obj->trans.y_position -= 4.0f * obj->velocity.y;
    obj->trans.z_position -= 4.0f * obj->velocity.z;
    scale = 0.5f;
    b = 0xFF;
    g = 0;
    a = 0x33;
    i = 5;
    func_80055C00(dl, obj->modelInstances[kind], 0x207, 0);
    while (i--) {
        obj->trans.scale = scale * savedScale;
        func_80041CA0(dl, arg1, obj, 1.0f, 0.0f);
        if (kind == 0) {
            gDPSetPrimColor((*dl)++, 0, 0, 255, 0, b, a);
            gDPSetEnvColor((*dl)++, 255, 255, 255, 255);
        } else {
            gDPSetPrimColor((*dl)++, 0, 0, 0, b, g, a);
            gDPSetEnvColor((*dl)++, 255, 255, 255, 255);
        }
        gSPVertexJFG((*dl)++, OS_K0_TO_PHYSICAL(rl_m067_04CC8), 6, 0);
        gSPPolygon((*dl)++, OS_K0_TO_PHYSICAL(rl_m067_04C88), 4, 1);
        gDPPipeSync((*dl)++);
        func_80041DC0(dl);
        obj->trans.x_position += obj->velocity.x;
        obj->trans.y_position += obj->velocity.y;
        obj->trans.z_position += obj->velocity.z;
        obj->trans.rotation.z_rotation += 0x800;
        scale += 0.125f;
        b -= 0x3D;
        g += 0x3D;
        a += 0x33;
    }
    gDPSetPrimColor((*dl)++, 0, 0, 255, 255, 255, 255);
    obj->trans.rotation.z_rotation = savedRot;
    obj->trans.x_position = savedX;
    obj->trans.y_position = savedY;
    obj->trans.z_position = savedZ;
    obj->trans.scale = savedScale;
}

void func_m067_03F08(Object *obj, M067ShotSetup *setup) {
    M067Shot *s;

    obj->trans.rotation.y_rotation = setup->yRotation;
    obj->trans.rotation.x_rotation = setup->xRotation;
    obj->trans.rotation.z_rotation = 0;
    s = obj->unk68;
    obj->trans.scale *= setup->scale;
    obj->velocity.z = -setup->speed;
    func_8004944C(&obj->trans.rotation, &obj->velocity);
    s->owner = setup->owner;
    s->timer = setup->time * 6;
    s->strength = setup->strength;
    s->unk7 = 0;
    obj_collision_spheres_init(obj, s->strength << 8, 1, 4, 0, 0, 0, 10.0f, 10.0f, 100.0f, 1.0f, 0x11);
}

/* The level entry of a piece a rock breaks into */
typedef struct M067PieceEntry {
    /* 0x00 */ s16 objectID;
    /* 0x02 */ u8 pad2[0x4 - 0x2];
    /* 0x04 */ s16 x;
    /* 0x06 */ s16 y;
    /* 0x08 */ s16 z;
    /* 0x0A */ s16 yRotation;
    /* 0x0C */ s16 xRotation;
    /* 0x0E */ s16 unkE;
    /* 0x10 */ f32 unk10;
    /* 0x14 */ f32 unk14;
    /* 0x18 */ f32 scale;
    /* 0x1C */ u8 unk1C;
    /* 0x1D */ u8 pad1D[0x20 - 0x1D];
    /* 0x20 */ s32 unk20;
} M067PieceEntry;

extern void func_8005F5A0(f32 scale);
extern void func_8005F5AC(Object *obj, s32 updateRate);

/* A rock: flies and spins until its time is up; when it hits something it breaks into pieces */
void func_m067_03FF0(Object *obj, s32 updateRate) {
    s32 step;
    M067Shot *s;
    f32 radius;
    s32 n;
    Vec3f from;
    HitResult hit;
    M067PieceEntry entry;
    Object *piece;
    f32 rate;

    s = obj->unk68;
    rate = updateRate;
    if (obj->interactObj->hitPoints <= 0) {
        spawn_object_from_list(3, obj->trans.x_position, obj->trans.y_position, obj->trans.z_position, 100, 0);
        func_80003410(0x96, obj->trans.x_position, obj->trans.y_position, obj->trans.z_position, 4, NULL);
        func_800079B8(obj);
        return;
    }
    n = obj->interactObj->hitPoints >> 8;
    if (s->unk7) {
        if (n > 0) {
            step = 0x10000 / n;
        }
        entry.objectID = 0x138;
        entry.x = obj->trans.x_position;
        entry.y = obj->trans.y_position;
        entry.z = obj->trans.z_position;
        entry.unkE = 0x3C;
        entry.unk1C = 100;
        entry.unk20 = 0;
        entry.scale = 0.25f;
        while (n--) {
            entry.yRotation = func_80048E2C(-0x800, 0x800) + step * n;
            entry.xRotation = func_80048E2C(0x2800, 0x3800);
            entry.unk10 = func_80048E2C(50, 75) * 0.01f;
            entry.unk14 = func_80048E2C(50, 75) * 0.1f;
            piece = func_80005F14(&entry, 1);
            if (piece != NULL) {
                piece->level_entry = NULL;
            }
        }
        func_80003410(0x195, obj->trans.x_position, obj->trans.y_position, obj->trans.z_position, 4, NULL);
        func_800079B8(obj);
        return;
    }
    s->timer -= updateRate;
    if (s->timer < 0 || obj->interactObj->hitPoints <= 0) {
        func_800079B8(obj);
        return;
    }
    obj->trans.rotation.z_rotation += updateRate << 10;
    from.x = obj->trans.x_position;
    from.y = obj->trans.y_position;
    from.z = obj->trans.z_position;
    func_80009A04(obj, obj->velocity.x * rate, obj->velocity.y * rate, obj->velocity.z * rate);
    radius = ((M067HitShape *) obj->interactObj)->radius;
    func_8001CE10(1, &from, (Vec3f *) &obj->trans.x_position, &radius, 0xCE002000, 0, 0);
    if (func_80016208(obj, &from, (Vec3f *) &obj->trans.x_position, radius, 1, &hit)) {
        s->unk7 = 1;
    }
    obj->particleEmittersEnabled = 1;
    func_8005F5A0(obj->trans.scale / obj->header->scale);
    func_8005F5AC(obj, updateRate);
}

INCLUDE_ASM("asm/nonmatchings/modules/m067", data);

/* (.rodata: C, and the assembly functions' own from ro/) */

