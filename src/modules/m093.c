/* Runlink module 93: a target that sets a game flag when it is shot. */
#include "module.h"

typedef struct M093Setup {
    LevelObjectEntryCommon common;
    /* 0x0A */ u8 flag;
    /* 0x0B */ u8 unkB;
    /* 0x0C */ s16 y_rotation;
    /* 0x0E */ s16 x_rotation;
} M093Setup;

typedef struct M093Target {
    /* 0x0 */ s32 flag;
} M093Target;

#ifdef NON_MATCHING
/* the message shown for each flag, or -1 */
static s16 D_m093_002B0[96] = {
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    0x131, 0x131, 0x131, 0x131, 0x131, 0x131, 0x131, 0x131,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
};
#else
INCLUDE_ASM("asm/nonmatchings/modules/m093", data);
#endif

void func_m093_00000(Object *obj, M093Setup *setup) {
    M093Target *t = OBJ_STATE(obj, M093Target);

    obj->trans.rotation.y_rotation = setup->y_rotation;
    obj->trans.rotation.x_rotation = setup->x_rotation;
    t->flag = setup->flag;
    if (func_80047ED0(t->flag) != 0) {
        obj->modelIndex = 1;
    } else {
        obj->modelIndex = 0;
    }
    func_80079D1C(obj, setup->unkB << 8, 0, 8, 0, 0, 0, 20.0f, 20.0f, -1.0f, 1.0f, 0x11);
}

#ifdef NON_MATCHING
/* Equivalent; the original loads the two 1s into separate registers (frame layout matches) */
void func_m093_000BC(Object *obj, s32 updateRate) {
    ObjectInteraction *hit;
    SparkParams spark;
    ObjectUnk5C *coll;
    Object *player;
    s32 i;

    if (obj->modelIndex == 0 && (hit = obj->interactObj) != NULL && (coll = obj->unk5C) != NULL) {
        hit->unk6 -= coll->damage;
        if (hit->unk6 <= 0) {
            obj->modelIndex = 1;
            obj->particleEmittersEnabled = 1;
            obj_spawn_particle(obj, 2);
            spark.x_position = obj->trans.x_position;
            spark.y_position = obj->trans.y_position;
            spark.z_position = obj->trans.z_position;
            spark.unk18 = 20;
            spark.unk1A = 10;
            spark.unk1B = 0xFF;
            spark.unk1C = 0xFFFF80FF;
            spark.unk20 = 0xFF0000FF;
            i = 12;
            while (i--) {
                spark.y_rotation = func_80048E2C(-0x8000, 0x7FFF);
                spark.x_rotation = func_80048E2C(-0x3000, 0x3C00);
                spark.speed = func_80048E2C(70, 90) * 0.1f;
                spark.unk14 = func_80048E2C(25, 50) * 0.1f;
                func_m010_0000C(&spark);
            }
            hit->unk6 = 0;
            func_80003410(0x1F0, obj->trans.x_position, obj->trans.y_position, obj->trans.z_position, 4, NULL);
            func_80047EFC(OBJ_STATE(obj, M093Target)->flag, 1);
            if (D_m093_002B0[OBJ_STATE(obj, M093Target)->flag] != -1) {
                player = func_8000E4D0(0);
                if (player != NULL) {
                    func_m006_011E0(player->unk68, D_m093_002B0[OBJ_STATE(obj, M093Target)->flag]);
                }
            }
        }
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m093", func_m093_000BC);

INCLUDE_ASM("asm/nonmatchings/modules/m093", rodata);
#endif
