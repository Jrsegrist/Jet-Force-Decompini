/* Runlink module 100: a pad that tops up the players standing on it, a limited number of times. */
#include "module.h"

typedef struct M100Setup {
    LevelObjectEntryCommon common;
    /* 0x0A */ u8 radius;
    /* 0x0B */ s8 minY;
    /* 0x0C */ s8 maxY;
    /* 0x0D */ u8 amount; /* in seconds */
    /* 0x0E */ s8 uses;   /* 0: unlimited */
} M100Setup;

typedef struct M100Pad {
    /* 0x00 */ f32 radius;
    /* 0x04 */ f32 minY;
    /* 0x08 */ f32 maxY;
    /* 0x0C */ Object *last; /* the object topped up last frame */
    /* 0x10 */ s16 uses;     /* -1: unlimited */
    /* 0x12 */ s16 amount;
    /* 0x14 */ s32 unk14;
    /* 0x18 */ s32 unk18;
    /* 0x1C */ u8 unk1C;
} M100Pad;

typedef struct M100Player {
    /* 0x000 */ u8 pad0;
    /* 0x001 */ s8 flags;
    /* 0x002 */ u8 pad2[0x58C - 0x2];
    /* 0x58C */ s16 fuel;
    /* 0x58E */ u8 pad58E[0x592 - 0x58E];
    /* 0x592 */ s16 fuelTimer;
} M100Player;

extern void func_800541B0(s32, s32, s32);
extern s32 func_80005064(s32);
extern s32 func_8007E150(f32, f32, f32, f32, s32, Object **);

s32 D_m100_002F0 = 100; /* the module's number */

void func_m100_00000(Object *obj, M100Setup *setup) {
    M100Pad *pad = obj->unk68;

    pad->radius = setup->radius;
    pad->minY = obj->trans.y_position + setup->minY;
    pad->maxY = obj->trans.y_position + setup->maxY;
    pad->last = NULL;
    pad->uses = setup->uses;
    if (pad->uses == 0) {
        pad->uses = -1;
    }
    pad->amount = setup->amount * 60;
    pad->unk14 = 0;
    pad->unk18 = 0;
    pad->unk1C = 0;
    func_80079D1C(obj, -1, 0, 8, 0, 0, 0, 32.0f, 32.0f, -1.0f, 1.0f, 1);
    *(s32 *) obj->state = 0;
}

void func_m100_00110(Object *obj, s32 updateRate) {
    s32 i;
    s32 n;
    s32 add;
    f32 y;
    Object *list[12];
    Object *last;
    M100Pad *pad;
    M100Player *player;

    pad = obj->unk68;
    func_800541B0(D_m100_002F0, 2, 2);
    last = pad->last;
    pad->last = NULL;
    add = func_80005064(updateRate * 6);
    if (pad->uses != 0) {
        n = func_8007E150(obj->trans.x_position, obj->trans.y_position, obj->trans.z_position, 300.0f, 1, list);
        if (n != 0) {
            for (i = 0; i < n; i++) {
                if (pad->uses < 0 || last != list[i]) {
                    y = list[i]->trans.y_position;
                    if ((list[i]->trans.z_position - obj->trans.z_position) *
                                (list[i]->trans.z_position - obj->trans.z_position) +
                                (list[i]->trans.x_position - obj->trans.x_position) *
                                    (list[i]->trans.x_position - obj->trans.x_position) <
                            pad->radius * pad->radius &&
                        y < pad->maxY && pad->minY < y) {
                        player = list[i]->unk68;
                        if (player->fuel < pad->amount && (player->flags & 4)) {
                            player->fuel += add;
                            player->fuelTimer = 360;
                            if (pad->amount < player->fuel) {
                                player->fuel = pad->amount;
                            }
                        }
                        pad->last = list[i];
                        if (pad->uses > 0) {
                            pad->uses--;
                        }
                    }
                }
            }
        }
    }
}
