/*
 * Runlink module 114: shooting targets that appear after a delay and stay up
 * for a while. The first kind just shows and hides; the second kind (a
 * pop-up target) swings up into view, swings down again when its time is
 * up, and falls flat (with a sound) when it has been shot.
 */
#include "module.h"

typedef struct M114Setup {
    LevelObjectEntryCommon common;
    /* 0x0A */ s16 delay;    /* hundredths of a second */
    /* 0x0C */ s16 duration; /* hundredths of a second */
    /* 0x0E */ u8 angle1;
    /* 0x0F */ u8 angle2;
    /* 0x10 */ u8 health;
    /* 0x11 */ u8 points;   /* tens */
    /* 0x12 */ s16 scale;   /* twentieths */
} M114Setup;

typedef struct M114Target {
    /* 0x0 */ s16 delay;
    /* 0x2 */ s16 duration;
    /* 0x4 */ s16 raise; /* 0: down, 0x100: up */
    /* 0x6 */ s16 health;
} M114Target;

extern void func_80001248(s32 sound, s32 arg1);

void func_m114_00000(Object *obj, M114Setup *setup) {
    M114Target *t;

    obj->trans.rotation.y_rotation = setup->angle1 << 8;
    obj->trans.rotation.z_rotation = setup->angle2 << 8;
    t = obj->unk68;
    t->delay = setup->delay * 60 / 100;
    t->duration = setup->duration * 60 / 100;
    t->raise = 0;
    t->health = setup->health << 8;
    OBJ_STATE(obj, s32)[0] = setup->points * 10;
    obj->interactObj->unk6 = t->health;
}

void func_m114_000E4(Object *obj, s32 updateRate) {
    M114Target *t = obj->unk68;

    if (t->delay != 0) {
        obj->interactObj->unk6 = t->health;
    }
    if (obj->interactObj->unk6 <= 0) {
        obj->trans.flags |= 0x400;
        return;
    }
    if (updateRate < t->delay) {
        t->delay -= updateRate;
        obj->trans.flags |= 0x400;
    } else if (t->delay > 0) {
        updateRate -= t->delay;
        t->delay = 0;
    }
    if (t->delay == 0) {
        if (updateRate < t->duration) {
            t->duration -= updateRate;
            obj->trans.flags &= ~0x400;
            return;
        }
        t->duration = 0;
        obj->trans.flags |= 0x400;
    }
}

void func_m114_001D4(Object *obj, M114Setup *setup, s32 arg2) {
    M114Target *t;

    obj->trans.rotation.y_rotation = setup->angle1 << 8;
    obj->trans.rotation.x_rotation = setup->angle2 << 8;
    obj->trans.rotation.z_rotation = -0x4000;
    obj->trans.scale = setup->scale * obj->header->scale * 0.05f;
    t = obj->unk68;
    t->delay = setup->delay * 60 / 100;
    t->duration = setup->duration * 60 / 100;
    t->raise = 0;
    t->health = setup->health << 8;
    OBJ_STATE(obj, s32)[0] = setup->points * 10;
    func_80079D1C(obj, t->health, 0, 8, 0, 0, 0, 20.0f, 20.0f, -1.0f, 1.0f, 0x11);
    obj->interactObj->flags &= ~1;
}

void func_m114_00370(Object *obj, s32 updateRate) {
    M114Target *t = obj->unk68;

    if (t->delay != 0) {
        obj->interactObj->unk6 = t->health;
    } else if (obj->interactObj->unk6 > 0) {
        obj->interactObj->unk6 -= obj->unk5C->damage;
        if (obj->interactObj->unk6 <= 0) {
            func_80001248(0x13, 0);
        }
    }
    if (obj->interactObj->unk6 <= 0) {
        if (t->raise > 0) {
            t->raise -= updateRate << 5;
            if (t->raise < 0) {
                t->raise = 0;
            }
            obj->trans.rotation.z_rotation = (t->raise << 6) + 0xC000;
        } else {
            obj->trans.flags |= 0x400;
        }
        obj->interactObj->flags &= ~1;
        return;
    }
    if (updateRate < t->delay) {
        t->delay -= updateRate;
        obj->trans.flags |= 0x400;
    } else if (t->delay > 0) {
        updateRate -= t->delay;
        t->delay = 0;
    }
    if (t->delay == 0) {
        if (updateRate < t->duration) {
            if (t->raise < 0x100) {
                t->raise += updateRate * 16;
                if (t->raise > 0x100) {
                    t->raise = 0x100;
                }
                obj->trans.rotation.z_rotation = (t->raise << 6) + 0xC000;
            } else {
                t->duration -= updateRate;
            }
            obj->trans.flags &= ~0x400;
            obj->interactObj->flags |= 1;
        } else if (t->raise > 0) {
            t->raise -= updateRate * 16;
            if (t->raise < 0) {
                t->raise = 0;
            }
            obj->trans.rotation.z_rotation = (t->raise << 6) + 0xC000;
        } else {
            t->duration = 0;
            obj->trans.flags |= 0x400;
            obj->interactObj->flags &= ~1;
        }
    }
}
