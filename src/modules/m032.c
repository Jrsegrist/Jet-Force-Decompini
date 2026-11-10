/*
 * Runlink module 32: characters that talk to the player (lines picked from a
 * table by the character's object ID), and a bobbing object that turns,
 * moves its light with it and fades out once it is told to.
 */
#include "module.h"

/* One spoken line: a sound and how many variants follow it */
typedef struct M032Line {
    /* 0x0 */ s16 sound;
    /* 0x2 */ u8 pad2;
    /* 0x3 */ s8 variants;
} M032Line;

/* What a character says, by object ID. Size 0x38 */
typedef struct M032Lines {
    /* 0x00 */ s16 objectID; /* -1 ends the table */
    /* 0x02 */ u8 pad2[0x8 - 0x2];
    /* 0x08 */ M032Line *lines;
    /* 0x0C */ u8 padC[0x38 - 0xC];
} M032Lines;

/* A speaker's state (obj->unk68) */
typedef struct M032Speaker {
    /* 0x00 */ u8 kind;
    /* 0x01 */ u8 pad1[0x2C - 0x1];
    /* 0x2C */ s32 sound;
} M032Speaker;

typedef struct M032BobSetup {
    LevelObjectEntryCommon common;
    /* 0x0A */ u8 height;
    /* 0x0B */ u8 rising;
    /* 0x0C */ f32 scale;
} M032BobSetup;

/* The bobbing object's data (obj->unk68) */
typedef struct M032Bob {
    /* 0x0 */ f32 baseY;
    /* 0x4 */ f32 height;
    /* 0x8 */ s16 phase;
    /* 0xA */ s16 rising;
} M032Bob;

typedef struct M032BobState {
    /* 0x0 */ s32 fading;
    /* 0x4 */ s32 alpha;
} M032BobState;

/* What obj->unk70 points at here: an attachment point */
typedef struct M032Attach {
    /* 0x00 */ u8 pad0[0xE];
    /* 0x0E */ s16 yRotation;
    /* 0x10 */ s16 xRotation;
    /* 0x12 */ u8 pad12[0x1A - 0x12];
    /* 0x1A */ s16 x;
    /* 0x1C */ s16 y;
    /* 0x1E */ s16 z;
} M032Attach;

/* What *obj->light points at here */
typedef struct M032Light {
    /* 0x00 */ u8 pad0[0x8];
    /* 0x08 */ Vec3f pos;
} M032Light;

extern void func_80003614(s32 handle); /* stops a positioned sound */
extern s32 func_80009A04(Object *obj, f32 dx, f32 dy, f32 dz); /* DKR objMoveXYZ */
extern void func_8005F5AC(Object *obj, s32 updateRate);
extern void light_intensity_set(M032Light *light, s32 intensity);

extern M032Lines rl_m032_02F24[]; /* the lines of each character */
extern Object *rl_m032_031CC;     /* the character the player is talking to */

void func_m032_0241C(Object *obj, s16 sound, s8 variants);

INCLUDE_ASM("asm/nonmatchings/modules/m032", func_m032_00000);

INCLUDE_ASM("asm/nonmatchings/modules/m032", func_m032_002E8);

/* The byte of the object at 0x32: its bit in the settings' flags from 8 on */
typedef struct M032FlagObj {
    /* 0x00 */ u8 pad0[0x32];
    /* 0x32 */ u8 flag;
} M032FlagObj;

extern u8 *get_settings(void);

/* Whether the object's settings flag is still clear (called from 00000 and 002E8) */
s32 func_m032_01D2C(Object *obj) {
    s32 byte = ((M032FlagObj *) obj)->flag >> 3;
    s32 bit = 0x80 >> (((M032FlagObj *) obj)->flag & 7);

    if (get_settings()[byte + 8] & bit) {
        return FALSE;
    }
    return TRUE;
}

/* Sets the object's settings flag (called from 002E8) */
void func_m032_01D84(Object *obj) {
    s32 byte = ((M032FlagObj *) obj)->flag >> 3;
    s32 bit = 0x80 >> (((M032FlagObj *) obj)->flag & 7);

    get_settings()[byte + 8] |= bit;
}

INCLUDE_ASM("asm/nonmatchings/modules/m032", func_m032_01DD8);

#ifdef NON_MATCHING
/* 14 diffs, registers only: the ROM has the two differences in v0/a3 and the step in v1 */
/* Turns `angle` towards `target` the shorter way, an eighth of the way (at most 364) for each of `steps` */
s16 func_m032_02068(s16 angle, s16 target, s16 steps) {
    s32 d;
    s32 back;
    s32 step;

    while (steps--) {
        d = (u16) (target - angle);
        back = (u16) (angle - target);
        if (back < d) {
            step = -back >> 3;
            if (step < -364) {
                step = -364;
            }
        } else {
            step = d >> 3;
            if (step > 364) {
                step = 364;
            }
        }
        angle += step;
    }
    return angle;
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m032", func_m032_02068);
#endif

void func_m032_02110(Object *obj, M032BobSetup *setup) {
    M032Bob *b = obj->unk68;

    b->baseY = obj->trans.y_position;
    b->height = setup->height;
    b->phase = 0;
    b->rising = setup->rising;
    if (b->rising != 0) {
        b->phase = -0x3000;
    }
    if (obj->unk70 != NULL) {
        b->baseY -= ((M032Attach *) obj->unk70)->y;
    }
    obj->trans.scale = obj->header->scale * setup->scale;
    OBJ_STATE(obj, M032BobState)->fading = 0;
    OBJ_STATE(obj, M032BobState)->alpha = 255;
}

void func_m032_021B8(Object *obj, s32 updateRate) {
    f32 height;
    Vec3s rot;
    void **model;
    M032Light *light;
    M032Attach *attach;
    M032Bob *b;

    obj->trans.rotation.y_rotation += updateRate << 8;
    b = obj->unk68;
    b->phase += updateRate << 7;
    model = obj->modelInstances[0];
    if (model != NULL && *model != NULL && ((s8 *) *model)[0x4E] != 0) {
        func_8001138C(obj, 0.1667f, updateRate);
    }
    height = b->height;
    if (b->rising != 0) {
        if (b->phase >= 0) {
            b->rising = 0;
        } else {
            height = b->rising;
        }
    }
    height = func_80049C34(b->phase) * height + b->baseY - obj->trans.y_position;
    func_80009A04(obj, 0.0f, height, 0.0f);
    obj->particleEmittersEnabled = 1;
    func_8005F5AC(obj, updateRate);
    if (OBJ_STATE(obj, M032BobState)->fading) {
        obj->trans.flags |= 4;
        OBJ_STATE(obj, M032BobState)->alpha -= updateRate * 4;
    }
    if (OBJ_STATE(obj, M032BobState)->alpha <= 0) {
        func_800079B8(obj);
        return;
    }
    if (obj->light != NULL && (light = *(M032Light **) obj->light) != NULL) {
        light->pos.x = 0.0f;
        light->pos.y = 0.0f;
        light->pos.z = -16.0f;
        attach = obj->unk70;
        if (attach != NULL) {
            rot.y_rotation = attach->yRotation - obj->trans.rotation.y_rotation;
            rot.x_rotation = attach->xRotation;
            rot.z_rotation = 0;
            func_8004944C(&rot, &light->pos);
            light->pos.x += attach->x;
            light->pos.y += attach->y;
            light->pos.z += attach->z;
        }
        light_intensity_set(light, ((u8 *) obj->state)[7]);
    }
}

/* Plays one of a line's variants at the speaker, stopping the last one */
void func_m032_0241C(Object *obj, s16 sound, s8 variants) {
    M032Speaker *s = obj->unk68;

    if (s->sound != 0) {
        func_80003614(s->sound);
    }
    if (variants >= 2) {
        sound += func_80048E2C(0, variants - 1);
    }
    func_80003410(sound, obj->trans.x_position, obj->trans.y_position, obj->trans.z_position, 4, &s->sound);
}

#ifdef NON_MATCHING
/* Says the character's line number `line`. Register choice: line - 1 in t9, the ROM's in t0 */
void func_m032_024C4(Object *obj, s32 line) {
    M032Lines *l = rl_m032_02F24;
    M032Line *e;

    while (l->objectID != obj->objectID && l->objectID >= 0) {
        l++;
    }
    e = &l->lines[line - 1];
    func_m032_0241C(obj, e->sound, e->variants);
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m032", func_m032_024C4);
#endif

/* The character being talked to answers */
void func_m032_02540(void) {
    Object *obj = rl_m032_031CC;
    M032Lines *l;
    M032Speaker *s;

    if (obj != NULL) {
        l = rl_m032_02F24;
        s = obj->unk68;
        while (l->objectID != obj->objectID && l->objectID >= 0) {
            l++;
        }
        if (s->kind == 8) {
            func_m032_0241C(obj, l->lines[5].sound, l->lines[5].variants);
        } else {
            func_m032_0241C(obj, l->lines[6].sound, l->lines[6].variants);
        }
    }
}

/* Where a character walks to, as 025F0 uses it */
typedef struct M032Walk {
    /* 0x00 */ u8 state;
    /* 0x01 */ u8 pad1;
    /* 0x02 */ s16 yaw;
    /* 0x04 */ f32 x;
    /* 0x08 */ f32 y;
    /* 0x0C */ f32 z;
    /* 0x10 */ u8 pad10[0x28 - 0x10];
    /* 0x28 */ f32 speed;
    /* 0x2C */ u8 pad2C[0x33 - 0x2C];
    /* 0x33 */ u8 unk33;
} M032Walk;

/* An animation and its speed */
typedef struct M032Anim {
    /* 0x00 */ u8 pad0[0xE];
    /* 0x0E */ s16 anim;
    /* 0x10 */ u8 pad10[0x14 - 0x10];
    /* 0x14 */ f32 speed;
} M032Anim;

extern s32 func_80049C84(f32 x, f32 z); /* the angle of a direction */

/* Starts walking to the walk's point with animation `anim` (called from 002E8) */
void func_m032_025F0(Object *obj, M032Walk *walk, M032Anim *anim) {
    walk->state = 1;
    walk->unk33 = 0;
    walk->yaw = func_80049C84(walk->x - obj->trans.x_position, walk->z - obj->trans.z_position);
    walk->speed = anim->speed;
    obj_anim_set(obj, anim->anim, obj->unk28);
}

INCLUDE_ASM("asm/nonmatchings/modules/m032", data);

INCLUDE_ASM("asm/nonmatchings/modules/m032", rodata);
