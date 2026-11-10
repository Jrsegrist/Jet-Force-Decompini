/*
 * Runlink module 129: a pad that changes the player's form. The player stands
 * on it and presses the button: the pad takes over the camera (which circles
 * the player), dims the fog, raises a ring of beams of light (a module 64
 * object), flashes the screen, swaps the player's model (and the shadow's) for
 * the one the pad gives, flashes again and gives the camera back.
 */
#include "module.h"

typedef struct M129Setup {
    LevelObjectEntryCommon common;
    /* 0x0A */ u8 radius;
    /* 0x0B */ s8 minY; /* relative to the pad */
    /* 0x0C */ s8 maxY;
    /* 0x0D */ u8 kind; /* low four bits: the form */
} M129Setup;

typedef struct M129State {
    /* 0x00 */ f32 radiusSq;
    /* 0x04 */ f32 minY;
    /* 0x08 */ f32 maxY;
    /* 0x0C */ u8 kind;
    /* 0x0D */ u8 state;
    /* 0x0E */ u8 nextState; /* after the wait (state 6) */
    /* 0x0F */ u8 timer;
    /* 0x10 */ s32 sound;
    /* 0x14 */ Object *beams;
    /* 0x18 */ u8 spawnBeams;
    /* 0x19 */ u8 fogR; /* the fog before the pad dimmed it */
    /* 0x1A */ u8 fogG;
    /* 0x1B */ u8 fogB;
    /* 0x1C */ s16 fogNear;
    /* 0x1E */ s16 fogFar;
} M129State;

/* The pad's own state in the object */
typedef struct M129Active {
    /* 0x0 */ s32 time;
    /* 0x4 */ s32 active; /* the player is changing */
} M129Active;

/* The player's data (object->unk68) */
typedef struct M129Player {
    /* 0x000 */ u8 pad0;
    /* 0x001 */ s8 character;
    /* 0x002 */ u8 pad2[0x118 - 0x2];
    /* 0x118 */ f32 unk118;
    /* 0x11C */ u8 pad11C[0x189 - 0x11C];
    /* 0x189 */ u8 unk189;
    /* 0x18A */ u8 pad18A[0x19F - 0x18A];
    /* 0x19F */ u8 unk19F;
    /* 0x1A0 */ u8 pad1A0[0x568 - 0x1A0];
    /* 0x568 */ u8 unk568;
    /* 0x569 */ u8 pad569[0x594 - 0x569];
    /* 0x594 */ f32 lift; /* 0..1 */
    /* 0x598 */ u8 pad598[0x5C8 - 0x598];
    /* 0x5C8 */ s16 unk5C8;
} M129Player;

/* What func_80047C1C returns */
typedef struct M129Forms {
    /* 0x00 */ u8 pad0[4];
    /* 0x04 */ u16 unk4 : 11;
    /* 0x04 */ u16 form : 5;
    /* 0x06 */ u8 pad6[0x12 - 0x6];
    /* 0x12 */ s16 charges;
} M129Forms;

/* The level entry of a module 64 object (rotating beams) */
typedef struct M129BeamEntry {
    /* 0x0 */ s8 rot[3];
    /* 0x3 */ s8 rotSpeed[3];
    /* 0x6 */ u8 r;
    /* 0x7 */ u8 g;
    /* 0x8 */ u8 b;
    /* 0x9 */ u8 width;
    /* 0xA */ u8 endWidth;
    /* 0xB */ u8 length;
    /* 0xC */ u16 flags;
} M129BeamEntry;

typedef struct M129BeamSetup {
    LevelObjectEntryCommon common;
    /* 0x0A */ u8 count;
    /* 0x0B */ u8 alpha;
    /* 0x0C */ M129BeamEntry beams[8];
} M129BeamSetup;

typedef struct M129BeamState {
    /* 0x0 */ u8 count;
    /* 0x2 */ s16 alpha;
} M129BeamState;

/* What object->shadow points at here */
typedef struct M129Shadow {
    /* 0x00 */ u8 pad0[0x34];
    /* 0x34 */ void *unk34;
    /* 0x38 */ u8 pad38[0x60 - 0x38];
    /* 0x60 */ s32 *unk60;
    /* 0x64 */ s32 *unk64;
    /* 0x68 */ s32 *unk68;
} M129Shadow;

typedef struct M129ShadowData {
    /* 0x00 */ u8 pad0[0xC];
    /* 0x0C */ u8 flags;
    /* 0x0D */ u8 padD[0x1C - 0xD];
    /* 0x1C */ M129Shadow *shadow;
} M129ShadowData;

extern void func_8001BC2C(s32 player, s16 *near, s16 *far, s16 *arg3, u8 *r, u8 *g, u8 *b, u8 *arg7); /* DKR get_fog_settings */
extern void func_8001C3D0(s32 player, s32 r, s32 g, s32 b, s32 near, s32 far, f32 time); /* DKR slowly_change_fog */
extern Object *func_80005F14(void *entry, s32 arg1); /* DKR spawn_object */
extern u16 func_80043510(s32 player);                /* buttons pressed */
extern M129Forms *func_80047C1C(s32 player, s32 character);
extern void transition_begin(void *flash);              /* flashes the screen */
extern void func_80003614(s32 handle);               /* stops a positioned sound */
extern void func_80048518(s32);
extern void func_m077_00000(s32 id, s32 apart);
extern Object **func_80005E98(s32 *first, s32 *count); /* DKR objGetObjList */
extern ObjectTransform *func_80041DE0(void);         /* the camera */
extern f32 func_80097140(f32 x);                     /* sqrtf */
extern s32 func_80049C84(f32 dx, f32 dz);            /* the angle of a vector */
extern f32 func_80049BEC(f32 base, s32 exponent);
extern s16 angle_lerp(s16 from, s16 to, f32 rate);
extern s32 func_80009A04(Object *obj, f32 dx, f32 dy, f32 dz); /* DKR objMoveXYZ */
extern s32 *func_8003B9E8(s32 model, s32 arg1);      /* loads a model instance */
extern void free_3d_model(s32 *inst);                /* frees a model instance */
extern void func_8003D21C(s32 *inst, s32 model, Object *obj, s32 arg3, f32 arg4);
extern void func_8003DDD8(void *arg0, s32 model, s32 arg2, s32 arg3, s32 arg4, s32 arg5, s32 arg6);
extern void func_8004AA80(void *data); /* DKR mempool_free */

/* Screen flashes */
static u8 sFlashOut[8] = { 0x80, 0xFF, 0xFF, 0xFF, 0x00, 0x3C, 0x00, 0x00 };
static u8 sFlashIn[8] = { 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0x3C, 0xFF, 0xFF };

/* The models of each form, for the four characters (0, 4, 1, 5): the player's and the shadow's */
static s16 sFormModels[4][2][16] = {
    { { 0xDA, 0xF8, 0xF9, 0xDA, 0xDA, 0xDA, 0xDA, 0xDA, 0xDA, 0xDA, 0xDA, 0xDA, 0xDA, 0xDA, 0xDA, 0xDA },
      { 0x31A, 0xF8, 0xF9, 0x31A, 0x31A, 0x31A, 0x31A, 0x31A, 0x31A, 0x31A, 0x31A, 0x31A, 0x31A, 0x31A, 0x31A,
        0x31A } },
    { { 0xDB, 0xF8, 0xF9, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB },
      { 0xDB, 0xF8, 0xF9, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB } },
    { { 0xDC, 0xFA, 0xFB, 0xDC, 0xDC, 0xDC, 0xDC, 0xDC, 0xDC, 0xDC, 0xDC, 0xDC, 0xDC, 0xDC, 0xDC, 0xDC },
      { 0x31C, 0xFA, 0xFB, 0x31C, 0x31C, 0x31C, 0x31C, 0x31C, 0x31C, 0x31C, 0x31C, 0x31C, 0x31C, 0x31C, 0x31C,
        0x31C } },
    { { 0xDD, 0xFA, 0xFB, 0xDD, 0xDD, 0xDD, 0xDD, 0xDD, 0xDD, 0xDD, 0xDD, 0xDD, 0xDD, 0xDD, 0xDD, 0xDD },
      { 0xDD, 0xFA, 0xFB, 0xDD, 0xDD, 0xDD, 0xDD, 0xDD, 0xDD, 0xDD, 0xDD, 0xDD, 0xDD, 0xDD, 0xDD, 0xDD } },
};

/* The pad's model for each kind */
static u8 sKindModel[16] = { 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

/* The ring of beams raised over the pad */
static M129BeamSetup sBeamSetup = {
    { 0x2C2, 0, 0, 0, 0 },
    8,
    0,
    {
        { { 0x40, 0x00, 0x00 }, { 0x1E, 0x28, 0x00 }, 0xFF, 0x00, 0x00, 10, 8, 9, 0x61 },
        { { 0x00, 0x00, 0x00 }, { 0x14, 0x23, 0x00 }, 0xFF, 0x00, 0x80, 10, 8, 10, 0 },
        { { -0x40, 0x00, 0x00 }, { 0x28, -0x1E, 0x00 }, 0xFF, 0x00, 0xFF, 10, 8, 11, 0x61 },
        { { -0x80, 0x00, 0x00 }, { 0x1E, -0x19, 0x00 }, 0x80, 0x00, 0xFF, 10, 8, 12, 0 },
        { { 0x40, 0x00, 0x00 }, { -0x19, 0x23, 0x00 }, 0x00, 0x00, 0xFF, 10, 8, 12, 0x61 },
        { { 0x00, 0x00, 0x00 }, { -0x1E, 0x1E, 0x00 }, 0x80, 0x00, 0xFF, 10, 8, 11, 0 },
        { { -0x40, 0x00, 0x00 }, { -0x23, 0x19, 0x00 }, 0xFF, 0x00, 0xFF, 10, 8, 10, 0x61 },
        { { -0x80, 0x00, 0x00 }, { -0x28, 0x14, 0x00 }, 0xFF, 0x00, 0x80, 10, 8, 9, 0 },
    },
};

/* The camera while the pad has it */
static f32 sCamTargetX;
static f32 sCamTargetY;
static f32 sCamTargetZ;
static f32 sCamCentreX;
static f32 sCamCentreY;
static f32 sCamCentreZ;
static f32 sCamDist;
static f32 sCamHeight;
static s16 sCamAngle;
static s16 sCamAngleSpeed;
static s16 sCamEased;
static f32 sCamDistStart;
static f32 sCamHeightStart;
static s16 sCamAngleStart;

/* Dims the fog, keeping the old settings */
static void fog_dim(M129State *s, f32 time) {
    s16 unused1;
    u8 unused2;

    func_8001BC2C(0, &s->fogNear, &s->fogFar, &unused1, &s->fogR, &s->fogG, &s->fogB, &unused2);
    func_8001C3D0(0, 0, 0, 0, 0x3C0, 0x3DE, time);
}

/* Gives the fog back */
static void fog_restore(M129State *s, f32 time) {
    func_8001C3D0(0, s->fogR, s->fogG, s->fogB, s->fogNear, s->fogFar, time);
}

/*
 * Raises the ring of beams once, then fades it in while the pad is active and
 * out after. Declared as returning a value it never sets: the caller's
 * register choice after the call needs it.
 */
static s32 beams_update(Object *obj, M129State *s, s32 updateRate) {
    M129BeamState *beams;

    if (s->spawnBeams) {
        sBeamSetup.common.x = obj->trans.x_position;
        sBeamSetup.common.y = obj->trans.y_position + 62.5f;
        sBeamSetup.common.z = obj->trans.z_position;
        s->spawnBeams = 0;
        s->beams = func_80005F14(&sBeamSetup, 1);
    }
    if (s->beams != NULL) {
        beams = s->beams->unk68;
        if (OBJ_STATE(obj, M129Active)->active) {
            beams->alpha += updateRate * 2;
            if (beams->alpha >= 0x100) {
                beams->alpha = 0xFF;
            }
        } else {
            beams->alpha -= updateRate * 16;
            if (beams->alpha < 0) {
                beams->alpha = 0;
            }
        }
    }
}

static s16 *form_models(M129Player *pd) {
    switch (pd->character) {
        case 0:
            return sFormModels[0][0];
        case 4:
            return sFormModels[1][0];
        case 1:
            return sFormModels[2][0];
        case 5:
            return sFormModels[3][0];
    }
    return NULL;
}

static s16 *form_shadow_models(M129Player *pd) {
    switch (pd->character) {
        case 0:
            return sFormModels[0][1];
        case 4:
            return sFormModels[1][1];
        case 1:
            return sFormModels[2][1];
        case 5:
            return sFormModels[3][1];
    }
    return NULL;
}

void func_m129_002F4(Object *obj, M129Setup *setup) {
    M129State *s = obj->unk68;

    s->radiusSq = setup->radius;
    s->radiusSq = s->radiusSq * s->radiusSq;
    s->minY = obj->trans.y_position + setup->minY;
    s->maxY = obj->trans.y_position + setup->maxY;
    s->kind = setup->kind & 0xF;
    s->state = 0;
    s->nextState = 0;
    s->timer = 0;
    s->beams = NULL;
    s->spawnBeams = 1;
    obj->modelIndex = sKindModel[s->kind];
    func_80079D1C(obj, -1, 0, 8, 0, 0, 0, 32.0f, 32.0f, -1.0f, 1.0f, 1);
}

static void camera_start(Object *player, s32 angle, s32 eased);
static void player_set_form(Object *player, s32 model, s32 shadowModel);

void func_m129_003F4(Object *obj, s32 updateRate) {
    s32 buttons;
    s32 model;
    M129State *s;
    f32 dx;
    f32 dz;
    s32 pad0;
    s32 pad1;
    Object *player;
    M129Player *pd;
    M129Forms *forms;

    player = func_8000E4D0(0);
    if (player != NULL) {
        s = obj->unk68;
        pd = player->unk68;
        buttons = func_80043510(0);
        forms = func_80047C1C(0, pd->character);
        beams_update(obj, s, updateRate);
        switch (s->state) {
            case 0:
                if ((buttons & 0x8000) && pd->unk568 == 0 && player->interactObj->hitPoints >= 0x100 &&
                    (forms->charges > 0 || forms->form) && form_models(pd) != NULL) {
                    dx = player->trans.x_position - obj->trans.x_position;
                    dz = player->trans.z_position - obj->trans.z_position;
                    if (dx * dx + dz * dz < s->radiusSq && s->minY <= player->trans.y_position &&
                        player->trans.y_position <= s->maxY) {
                        pd->unk19F = 1;
                        OBJ_STATE(obj, M129Active)->time = 0;
                        OBJ_STATE(obj, M129Active)->active = 1;
                        camera_start(player, 0, 1);
                        fog_dim(s, 4.0f);
                        func_80003410(0x18, obj->trans.x_position, obj->trans.y_position, obj->trans.z_position, 1,
                                      &s->sound);
                        s->state = 6;
                        s->timer = 240;
                        s->nextState = 1;
                    }
                }
                break;
            case 1:
                transition_begin(sFlashIn);
                s->state = 6;
                s->timer = 75;
                s->nextState = 2;
                break;
            case 2:
                if (forms->form == s->kind || forms->charges <= 0) {
                    pd->unk189 = 0;
                    forms->form = 0;
                    func_m077_00000(0, 1);
                    func_m077_00000(1, 0);
                } else {
                    pd->unk189 = s->kind;
                    forms->form = s->kind;
                    forms->charges--;
                    func_m077_00000(0, 0);
                    func_m077_00000(1, 1);
                }
                model = form_models(pd)[forms->form];
                player_set_form(player, model, form_shadow_models(pd)[forms->form]);
                if (s->sound != 0) {
                    func_80003614(s->sound);
                }
                func_80003410(0xDB, obj->trans.x_position, obj->trans.y_position, obj->trans.z_position, 4, NULL);
                s->state = 3;
                break;
            case 3:
                transition_begin(sFlashOut);
                s->timer = 60;
                s->state = 6;
                s->nextState = 4;
                break;
            case 4:
                OBJ_STATE(obj, M129Active)->active = 0;
                s->timer = 60;
                s->state = 6;
                s->nextState = 5;
                fog_restore(s, 1.0f);
                break;
            case 5:
                pd->unk19F = 0;
                pd->lift = 0.0f;
                pd->unk118 = 0.0f;
                s->state = 0;
                break;
            case 6:
                if (updateRate < s->timer) {
                    s->timer -= updateRate;
                } else {
                    s->state = s->nextState;
                }
                break;
        }
        if (s->state != 0) {
            if (pd->unk5C8 < 10) {
                pd->unk5C8 = 10;
            }
            func_80048518(10);
        }
    }
}

/* Finds a pad's object (behaviour 0x7C) */
static Object *find_pad(void) {
    s32 first;
    s32 count;
    Object **objects;
    Object *found;

    objects = func_80005E98(&first, &count);
    found = NULL;
    while (first < count) {
        if (objects[first++]->behaviorId == 0x7C) {
            found = objects[first - 1];
        }
    }
    return found;
}

/* Starts the camera circling the player */
static void camera_start(Object *player, s32 angle, s32 eased) {
    f32 dx;
    f32 dz;
    ObjectTransform *cam;

    cam = func_80041DE0();
    sCamCentreX = player->trans.x_position;
    sCamCentreY = player->trans.y_position + 4.0f;
    sCamCentreZ = player->trans.z_position;
    sCamTargetX = player->trans.x_position;
    sCamTargetY = player->trans.y_position + 50.0f;
    sCamTargetZ = player->trans.z_position;
    dx = cam->x_position - sCamCentreX;
    dz = cam->z_position - sCamCentreZ;
    sCamDist = func_80097140(dx * dx + dz * dz);
    sCamHeight = cam->y_position - sCamCentreY;
    if (eased) {
        sCamAngle = 0x8000 - func_80049C84(dx, dz);
    } else {
        sCamAngle = angle;
    }
    sCamAngleSpeed = 0;
    sCamEased = eased;
    sCamDistStart = sCamDist;
    sCamHeightStart = sCamHeight;
    sCamAngleStart = angle;
}

void func_m129_00A2C(ObjectTransform *cam, s32 updateRate) {
    s16 pitch;
    s16 yaw;
    f32 dx;
    f32 dy;
    f32 dz;
    f32 rate;
    Object *pad;

    if (sCamEased) {
        rate = 1.0f - func_80049BEC(0.975f, updateRate);
    } else {
        rate = 1.0f;
    }
    pad = find_pad();
    if (pad != NULL && !OBJ_STATE(pad, M129Active)->active) {
        sCamDist += (sCamDistStart - sCamDist) * rate;
        sCamHeight += (sCamHeightStart - sCamHeight) * rate;
    } else {
        sCamDist += (120.0f - sCamDist) * rate;
        sCamHeight += (0.0f - sCamHeight) * rate;
    }
    sCamAngleSpeed += updateRate * 8;
    if (sCamAngleSpeed > 0xC0) {
        sCamAngleSpeed = 0xC0;
    }
    sCamAngle += sCamAngleSpeed * updateRate;
    cam->x_position = func_80049C34(sCamAngle) * sCamDist + sCamCentreX;
    cam->y_position = sCamCentreY + sCamHeight;
    cam->z_position = sCamCentreZ - func_80049C30(sCamAngle) * sCamDist;
    dx = cam->x_position - sCamTargetX;
    dy = cam->y_position - sCamTargetY;
    dz = cam->z_position - sCamTargetZ;
    yaw = 0x8000 - func_80049C84(dx, dz);
    pitch = func_80049C84(dy, func_80097140(dx * dx + dz * dz));
    if (sCamEased) {
        rate = 1.0f - func_80049BEC(0.85f, updateRate);
    } else {
        rate = 1.0f;
    }
    cam->rotation.y_rotation = angle_lerp(cam->rotation.y_rotation, yaw, rate);
    cam->rotation.x_rotation = angle_lerp(cam->rotation.x_rotation, pitch, rate);
    sCamEased = 1;
}

void func_m129_00CF8(Object *player, M129Player *pd, s32 updateRate) {
    f32 x;
    f32 y;
    f32 z;
    f32 f;
    f32 max;
    Object *pad;

    pad = find_pad();
    if (pad != NULL) {
        f = func_80049BEC(0.95f, updateRate);
        player->velocity.y *= f;
        f = 1.0f - f;
        if (OBJ_STATE(pad, M129Active)->active) {
            pd->lift += (1.0f - pd->lift) * f;
        } else {
            pd->lift += (0.0f - pd->lift) * f;
        }
        x = pad->trans.x_position;
        OBJ_STATE(pad, M129Active)->time += updateRate;
        y = (func_80049C34(OBJ_STATE(pad, M129Active)->time << 8) * 8.0f + 40.0f) * pd->lift + pad->trans.y_position;
        z = pad->trans.z_position;
        f = 1.0f - func_80049BEC(0.98f, updateRate);
        x = (x - player->trans.x_position) * f;
        y = (y - player->trans.y_position) * f;
        z = (z - player->trans.z_position) * f;
        max = updateRate * 0.1f;
        if (max < y) {
            y = max;
        }
        if (player->velocity.y < -0.3f) {
            player->velocity.y = -0.3f;
        }
        func_80009A04(player, x, y, z);
    }
}

/* Gives the player (and its shadow) new models */
static void player_set_form(Object *player, s32 model, s32 shadowModel) {
    s32 base;
    M129ShadowData *sd;
    M129Player *pd;
    M129Shadow *sh;
    s32 *inst;

    inst = func_8003B9E8(model, 1);
    if (inst != NULL) {
        free_3d_model(player->modelInstances[0]);
        player->modelInstances[0] = inst;
        pd = player->unk68;
        obj_anim_set(player, player->animationID, player->unk28);
        base = *inst;
        func_8003D21C(inst, base, player, *(s32 *) ((u8 *) pd + 0x48), 0.0f);
        ((u8 *) inst)[0xA] = 0;
        sd = (M129ShadowData *) player->shadow;
        if ((sd->flags & 8) && sd->shadow != NULL) {
            sh = sd->shadow;
            if (sh->unk34 != NULL) {
                func_8004AA80(sh->unk34);
            }
            if (sh->unk68 != NULL) {
                free_3d_model(sh->unk68);
                sh->unk68 = NULL;
            }
            if (model != shadowModel) {
                sh->unk60 = inst;
                sh->unk64 = sh->unk68 = func_8003B9E8(shadowModel, 1);
                func_8003DDD8(&sh->unk34, *sh->unk68, 0x400, 11, 0, 0xFF, 1);
            } else {
                sh->unk60 = inst;
                sh->unk64 = inst;
                func_8003DDD8(&sh->unk34, base, 0x400, 11, 0, 0xFF, 1);
            }
        }
    }
}
