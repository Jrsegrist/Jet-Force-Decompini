/* Runlink module 85: cut-scene cameras that other objects switch to for a while. */
#include "module.h"

typedef struct M085Setup {
    LevelObjectEntryCommon common;
    /* 0x0A */ s16 z_rotation;
    /* 0x0C */ s16 x_rotation;
    /* 0x0E */ s16 y_rotation;
    /* 0x10 */ s16 duration;
    /* 0x12 */ u8 id;
} M085Setup;

typedef struct M085Camera {
    /* 0x00 */ f32 x; /* the camera's own place, saved while the cut is on */
    /* 0x04 */ f32 y;
    /* 0x08 */ f32 z;
    /* 0x0C */ s16 z_rotation;
    /* 0x0E */ s16 x_rotation;
    /* 0x10 */ s16 y_rotation;
    /* 0x12 */ s16 timer;
    /* 0x14 */ s16 duration;
    /* 0x16 */ u8 saved;
    /* 0x17 */ u8 id;
} M085Camera;

typedef struct M085PlayerData {
    /* 0x000 */ s8 cameraIndex;
    /* 0x001 */ u8 pad1[0x5C0 - 0x1];
    /* 0x5C0 */ Object *cutCamera;
} M085PlayerData;

/* The game's cameras (0x4C bytes each) */
typedef struct M085View {
    /* 0x00 */ ObjectTransform trans;
    /* 0x18 */ u8 pad18[0x4C - 0x18];
} M085View;

extern s32 func_80005064(s32);
extern Object **func_80005E98(s32 *first, s32 *count); /* DKR objGetObjList */
extern void func_80043404(s32);
extern void func_80043418(s32);
extern void func_80048518(s32);
extern void func_80000450(s32);
extern M085View *func_80041E0C(void);
extern s32 func_80053370(s32);
extern void func_m006_015E0(s32);
extern void func_m005_00C04(void);

void func_m085_00000(Object *obj, M085Setup *setup) {
    M085Camera *c = obj->unk68;

    c->duration = func_80005064(setup->duration);
    c->id = setup->id;
    obj->trans.rotation.z_rotation = setup->z_rotation;
    obj->trans.rotation.x_rotation = setup->x_rotation;
    obj->trans.rotation.y_rotation = setup->y_rotation;
}

/* Starts cut-scene camera `id` */
void func_m085_00074(s32 id) {
    Object **list;
    Object *o;
    s32 i;
    Object *player;
    M085Camera *c;
    s32 first;
    s32 count;
    M085PlayerData *pd;

    list = func_80005E98(&first, &count);
    for (i = first; i < count; i++) {
        o = list[i];
        if (o->behaviorId == 100) {
            c = o->unk68;
            if (c->id == id) {
                player = func_8000E4D0(0);
                pd = player->unk68;
                pd->cutCamera = o;
                func_80043404(0);
                func_80048518(10);
                c->saved = 0;
                c->timer = c->duration;
                func_80000450(2);
                func_m006_015E0(1);
                return;
            }
        }
    }
}

void func_m085_00150(Object *obj, s32 updateRate) {
    M085Camera *c = obj->unk68;
    Object *player;
    M085View *view;
    M085PlayerData *pd;

    if (c->timer != 0) {
        if (updateRate < c->timer) {
            c->timer -= updateRate;
        } else {
            c->timer = 0;
            player = func_8000E4D0(0);
            pd = player->unk68;
            pd->cutCamera = NULL;
            view = &func_80041E0C()[pd->cameraIndex];
            view->trans.x_position = c->x;
            view->trans.y_position = c->y;
            view->trans.z_position = c->z;
            view->trans.rotation.y_rotation = c->y_rotation;
            view->trans.rotation.x_rotation = c->x_rotation;
            view->trans.rotation.z_rotation = c->z_rotation;
            func_80000450(0);
            func_m006_015E0(0);
            func_80043418(pd->cameraIndex);
            if (func_80053370(5) != 0) {
                func_m005_00C04();
            }
        }
    }
}

/* Moves view to the cut-scene camera obj (saving the view first) */
void func_m085_00250(M085View *view, Object *obj, s32 updateRate) {
    M085Camera *c = obj->unk68;

    if (c->saved == 0) {
        c->x = view->trans.x_position;
        c->y = view->trans.y_position;
        c->z = view->trans.z_position;
        c->y_rotation = view->trans.rotation.y_rotation;
        c->x_rotation = view->trans.rotation.x_rotation;
        c->z_rotation = view->trans.rotation.z_rotation;
        c->saved = 1;
    }
    view->trans.x_position = obj->trans.x_position;
    view->trans.y_position = obj->trans.y_position;
    view->trans.z_position = obj->trans.z_position;
    view->trans.rotation.y_rotation = 0x8000 - obj->trans.rotation.y_rotation;
    view->trans.rotation.x_rotation = -obj->trans.rotation.x_rotation;
    view->trans.rotation.z_rotation = obj->trans.rotation.z_rotation;
    if (func_80053370(5) != 0) {
        func_m005_00C04();
    }
}
