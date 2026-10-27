/* Runlink module 94: a trigger that runs one action once its game flag is set. */
#include "module.h"

typedef struct M094Setup {
    LevelObjectEntryCommon common;
    /* 0x0A */ u8 flag;
    /* 0x0B */ u8 arg;
    /* 0x0C */ u8 action;
    /* 0x0D */ s8 level;
} M094Setup;

typedef struct M094Trigger {
    /* 0x0 */ s16 flag; /* -1 once done */
    /* 0x2 */ s16 level;
    /* 0x4 */ union {
        s16 h;
        u8 b[2];
    } arg;
    /* 0x6 */ u16 action;
} M094Trigger;

extern s32 func_800468B0(void);

void func_m094_00000(Object *obj, M094Setup *setup) {
    M094Trigger *t = OBJ_STATE(obj, M094Trigger);

    t->flag = setup->flag;
    t->level = setup->level;
    t->arg.h = setup->arg;
    t->action = setup->action;
}

void func_m094_00030(Object *obj, s32 updateRate) {
    M094Trigger *t;

    if (OBJ_STATE(obj, M094Trigger)->flag >= 0) {
        t = OBJ_STATE(obj, M094Trigger);
        if (func_800468B0() == t->level && game_flag_get(t->flag) != 0) {
            switch (t->action) {
                case 0:
                    func_80079BB8();
                    break;
                case 1:
                    func_80079BA8();
                    break;
                case 2:
                    func_800767EC(t->arg.b[1]);
                    break;
                case 3:
                    func_8007688C(t->arg.b[1]);
                    break;
                case 4:
                    func_80076928(t->arg.b[1]);
                    break;
                case 5:
                    func_800772C4(t->arg.h);
                    break;
            }
            t->flag = -1;
        }
    }
}
