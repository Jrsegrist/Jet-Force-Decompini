/*
 * Runlink module 82: an object that opens when the player comes to it, with a
 * camera that swings round to look at it while it does (m075's cabinet camera
 * works the same way). Opening it gives the player the weapon or item its kind
 * names.
 */
#include "module.h"

/* What init gets */
typedef struct M082Setup {
    /* 0x00 */ LevelObjectEntryCommon common;
    /* 0x0A */ u8 kind;
    /* 0x0B */ u8 scale;
    /* 0x0C */ s16 yaw;
    /* 0x0E */ s16 camHeight;
    /* 0x10 */ u16 camDist;
    /* 0x12 */ s16 camYaw;
} M082Setup;

/* Kept in obj->unk68 */
typedef struct M082State {
    /* 0x00 */ s32 timer;
    /* 0x04 */ u8 phase; /* 0 shut, 1 turning the player, 2 opening, 3 open, 4 shutting, 5 shut again */
    /* 0x05 */ u8 kind;
    /* 0x06 */ s16 camYaw;
    /* 0x08 */ f32 camDist;
    /* 0x0C */ f32 camHeight;
    /* 0x10 */ Vec3f offset; /* where the player must stand, from the object */
    /* 0x1C */ f32 radius;
    /* 0x20 */ f32 height;
} M082State;

/* Kept in the object at 0x88: the glow spawned while it opens */
typedef struct M082Glow {
    /* 0x00 */ Object *obj;
    /* 0x04 */ s32 timer;
} M082Glow;

/* What the glow keeps in its unk68 */
typedef struct M082GlowState {
    /* 0x00 */ u8 pad0[0xA8];
    /* 0xA8 */ f32 unkA8;
    /* 0xAC */ u8 padAC[0xB4 - 0xAC];
    /* 0xB4 */ f32 unkB4;
    /* 0xB8 */ u8 padB8[0xCB - 0xB8];
    /* 0xCB */ s8 unkCB;
} M082GlowState;

/* The player object's unk68 */
typedef struct M082Player {
    /* 0x000 */ u8 pad0[1];
    /* 0x001 */ s8 character;
    /* 0x002 */ u8 pad2[0x11C - 0x2];
    /* 0x11C */ s16 yaw;
    /* 0x11E */ u8 pad11E[0x1FA - 0x11E];
    /* 0x1FA */ u8 unk1FA;
    /* 0x1FB */ u8 pad1FB[0x568 - 0x1FB];
    /* 0x568 */ u8 unk568;
    /* 0x569 */ u8 pad569[0x5C8 - 0x569];
    /* 0x5C8 */ s16 unk5C8;
} M082Player;

/* What func_80047C1C returns: the character's weapons */
typedef struct M082Weapons {
    /* 0x00 */ u8 pad0[0xA];
    /* 0x0A */ u16 owned; /* bit per weapon */
    /* 0x0C */ u8 padC[0x14 - 0xC];
    /* 0x14 */ s16 ammo[16];
    /* 0x34 */ s16 maxAmmo[16];
    /* 0x54 */ u8 unk54[0x6F - 0x54];
    /* 0x6F */ u8 count;
} M082Weapons;

/* What weapon_info_get returns */
typedef struct M082WeaponInfo {
    /* 0x00 */ u8 pad0[3];
    /* 0x03 */ u8 unk3;
    /* 0x04 */ u8 pad4[0x24 - 0x4];
    /* 0x24 */ s16 maxAmmo;
    /* 0x26 */ u8 pad26[2];
    /* 0x28 */ s16 ammo;
} M082WeaponInfo;

/* The level entry of the glow (object 0xC3) that 005D4 spawns (size 0x24) */
typedef struct M082GlowEntry {
    /* 0x00 */ s16 objectID;
    /* 0x02 */ u8 size;
    /* 0x03 */ u8 unk3;
    /* 0x04 */ s16 x;
    /* 0x06 */ s16 y;
    /* 0x08 */ s16 z;
    /* 0x0A */ s16 unkA;
    /* 0x0C */ s16 unkC;
    /* 0x0E */ s16 unkE;
    /* 0x10 */ s16 unk10;
    /* 0x12 */ s16 unk12;
    /* 0x14 */ u8 unk14;
    /* 0x15 */ u8 unk15;
    /* 0x16 */ u8 unk16;
    /* 0x17 */ u8 unk17;
    /* 0x18 */ u8 unk18;
    /* 0x19 */ u8 unk19;
    /* 0x1A */ s16 unk1A;
    /* 0x1C */ u8 unk1C;
    /* 0x1D */ u8 unk1D;
    /* 0x1E */ u8 unk1E;
    /* 0x1F */ u8 unk1F;
    /* 0x20 */ s16 unk20;
    /* 0x22 */ s16 unk22;
} M082GlowEntry;

extern ObjectTransform *func_80041DE0(void); /* the camera */
extern f32 func_80097140(f32 x);             /* sqrtf */
extern s32 func_80049C84(f32 x, f32 z);      /* the angle of a vector */
extern f32 func_80049BEC(f32 rate, s32 updateRate);
extern s32 func_80049D80(s32 from, s32 to); /* the difference between two angles */
extern s16 angle_lerp(s16 angle, s16 target, f32 rate); /* turns an angle towards another */
extern s32 func_80043510(s32 player);                   /* the buttons pressed */
extern void func_80048518(s32 arg0);
extern void func_80001170(s32 sound);
extern void func_8003B6C8(Object *player, M082Player *racer);
extern s32 func_8003B740(Object *player, M082Player *racer);
extern void func_8003D21C(void *inst, void *model, Object *obj, f32 arg3, f32 arg4);
extern void func_800212D0(s32 light);
extern void func_800212E4(s32 light);
extern Object *func_80005F14(void *entry, s32 arg1); /* DKR spawn_object */
extern s32 func_800468CC(void);
extern M082Weapons *func_80047C1C(s32 player, s32 character);
extern M082WeaponInfo *weapon_info_get(s32 weapon);
extern s32 character_has_item(s32 player, s32 character, s32 item);
extern void character_item_give(s32 player, s32 character, s32 item);
extern int player_chars_have_item(s32 item);
extern void func_m007_00148(s32 hint, s32 arg1);
extern void func_m007_00D6C(void);
extern void func_m007_02824(void);

/* The module's .data (still assembly): the thing spawned last, then the views of the kinds */
struct M082Linked;
extern struct M082Linked *rl_m082_017A0;
extern Vec3f rl_m082_017A4[6]; /* x: distance, y: radius, z: height */

/* The camera */
static f32 sCentreX;
static f32 sCentreY;
static f32 sCentreZ;
static f32 sCamDist;
static f32 sCamHeight;
static s16 sCamYaw;
static f32 sTargetDist;
static f32 sTargetHeight;
static s16 sTargetYaw;

/* Starts the camera's swing from where it is to the object's view */
void func_m082_00000(Object *obj) {
    f32 dx;
    f32 dz;
    M082State *state = obj->unk68;
    ObjectTransform *camera;

    camera = func_80041DE0();
    sCentreX = obj->trans.x_position;
    sCentreY = obj->trans.y_position + (480.0f * obj->trans.scale + 8.0f);
    sCentreZ = obj->trans.z_position;
    dx = camera->x_position - obj->trans.x_position;
    dz = camera->z_position - obj->trans.z_position;
    sCamDist = func_80097140(dx * dx + dz * dz);
    sCamHeight = camera->y_position - sCentreY;
    sCamYaw = 0x8000 - func_80049C84(dx, dz);
    sTargetDist = state->camDist;
    sTargetHeight = state->camHeight;
    sTargetYaw = state->camYaw - obj->trans.rotation.y_rotation;
}

/* The camera while the object is open */
void func_m082_00114(ObjectTransform *camera, s32 updateRate) {
    s32 turn;
    s32 i;
    s16 yaw;
    s16 pitch;
    f32 x;
    f32 z;
    f32 rate;

    rate = 1.0f - func_80049BEC(0.975f, updateRate);
    sCamDist += (sTargetDist - sCamDist) * rate;
    sCamHeight += (sTargetHeight - sCamHeight) * rate;
    i = updateRate;
    while (i--) {
        turn = func_80049D80(sCamYaw, sTargetYaw) >> 4;
        if (turn < -0x200) {
            sCamYaw -= 0x200;
        } else if (turn > 0x200) {
            sCamYaw += 0x200;
        } else {
            sCamYaw += turn;
        }
    }
    x = func_80049C34(sCamYaw) * sCamDist;
    z = -func_80049C30(sCamYaw) * sCamDist;
    camera->x_position = sCentreX + x;
    camera->y_position = sCentreY + sCamHeight;
    camera->z_position = sCentreZ + z;
    yaw = 0x8000 - func_80049C84(x, z);
    pitch = func_80049C84(sCamHeight, sCamDist);
    rate = 1.0f - func_80049BEC(0.9f, updateRate);
    camera->rotation.y_rotation = angle_lerp(camera->rotation.y_rotation, yaw, rate);
    camera->rotation.x_rotation = angle_lerp(camera->rotation.x_rotation, pitch, rate);
}

/* Sets where the player must stand to open the object, from the view of its kind */
static void func_m082_00340(Object *obj, M082State *state, u32 kind) {
    f32 dist;
    Vec3f *view;

    switch (kind) {
        case 0:
            view = &rl_m082_017A4[0];
            break;
        case 1:
            view = &rl_m082_017A4[1];
            break;
        case 2:
            view = &rl_m082_017A4[2];
            break;
        case 4:
            view = &rl_m082_017A4[3];
            break;
        case 5:
            view = &rl_m082_017A4[4];
            break;
        case 6:
            view = &rl_m082_017A4[5];
            break;
        default:
            view = NULL;
            break;
    }
    if (view != NULL) {
        dist = view->x + -210.0f * obj->trans.scale;
        state->radius = view->y;
        state->height = view->z;
        state->offset.x = func_80049C34(obj->trans.rotation.y_rotation) * dist;
        state->offset.y = 0.0f;
        state->offset.z = func_80049C30(obj->trans.rotation.y_rotation) * dist;
    } else {
        state->offset.x = 0.0f;
        state->offset.y = 0.0f;
        state->offset.z = 0.0f;
        state->radius = 8.0f;
        state->height = 4.0f;
    }
}

static s32 func_m082_013C0(u32 kind);

void func_m082_00478(Object *obj, M082Setup *setup) {
    M082State *state = obj->unk68;

    state->timer = 0;
    obj->trans.scale = 0.01f * (setup->scale * obj->header->scale);
    obj->trans.rotation.y_rotation = setup->yaw;
    func_m082_00340(obj, state, 0);
    state->kind = setup->kind;
    if (func_m082_013C0(state->kind)) {
        state->phase = 5;
    } else {
        state->phase = 0;
    }
    state->camHeight = setup->camHeight;
    state->camDist = setup->camDist;
    state->camYaw = setup->camYaw;
    obj_anim_set(obj, 0, 0.0f);
    func_80079D1C(obj, -1, 0, 8, 0, 0, 0, 4.0f, 4.0f, -1.0f, 1.0f, 1);
}

/* 00E6C's view of a model: its batches (kind in the low byte of unk4) and its textures */
typedef struct M082Tex {
    /* 0x00 */ u8 pad0[0x16];
    /* 0x16 */ s16 unk16;
} M082Tex;
typedef struct M082TexEntry {
    /* 0x0 */ M082Tex *tex;
    /* 0x4 */ u8 pad4[4];
} M082TexEntry;
typedef struct M082Model {
    /* 0x00 */ u8 pad0[0x18];
    /* 0x18 */ M082TexEntry *textures;
    /* 0x1C */ u8 pad1C[0x2C - 0x1C];
    /* 0x2C */ u8 count;
} M082Model;
typedef struct M082Anim {
    /* 0x0 */ s16 unk0;
    /* 0x2 */ s16 unk2;
    /* 0x4 */ s32 unk4;
} M082Anim;
typedef struct M082Inst {
    /* 0x00 */ M082Model *model;
    /* 0x04 */ u8 pad4[0xA - 0x4];
    /* 0x0A */ s8 dirty;
    /* 0x0B */ s8 buffer;
    /* 0x0C */ u8 padC[0x74 - 0xC];
    /* 0x74 */ M082Anim *anims;
    /* 0x78 */ s16 *out[2];
} M082Inst;

static void func_m082_00E6C(M082Model *model, M082Inst *inst, M082State *state, UNUSED s32 arg3);
static s32 func_m082_00F8C(u32 kind, f32 x, f32 y, f32 z, f32 h);
static void func_m082_01778(void);

/* The object's control */
void func_m082_005D4(Object *obj, s32 updateRate) {
    f32 dx;
    f32 dy;
    f32 dz;
    UNUSED s32 pad;
    f32 s;
    f32 prevFrame;
    M082Inst *inst;
    Object *player;
    M082Player *racer;
    s16 turn;
    M082State *state;
    s32 light;
    Object *glow;
    M082GlowState *glowState;
    M082GlowEntry entry;

    player = func_8000E4D0(0);
    if (player != NULL) {
        racer = player->unk68;
        state = obj->unk68;
        switch (state->phase) {
            case 0:
                if ((func_80043510(0) & 0x8000) && racer->unk568 == 0 && player->interactObj->hitPoints >= 0x100) {
                    func_m082_00340(obj, state, racer->character);
                    dx = (obj->trans.x_position + state->offset.x) - player->trans.x_position;
                    dy = player->trans.y_position - (obj->trans.y_position + state->offset.y);
                    dz = (obj->trans.z_position + state->offset.z) - player->trans.z_position;
                    turn = player->trans.rotation.y_rotation - obj->trans.rotation.y_rotation;
                    if (dx * dx + dz * dz < state->radius * state->radius && dy >= -4.0f && dy <= state->height &&
                        (turn < -0x6000 || turn > 0x6000)) {
                        state->phase = 1;
                        func_m082_00000(obj);
                        func_80048518(10);
                        racer->unk1FA = 1;
                        func_8003B6C8(player, racer);
                    }
                }
                break;
            case 1:
                func_80048518(10);
                dx = player->trans.x_position - obj->trans.x_position;
                dz = player->trans.z_position - obj->trans.z_position;
                turn = func_80049D80(racer->yaw, func_80049C84(dx, dz));
                if (turn < -(updateRate << 6)) {
                    turn = -(updateRate << 6);
                }
                if (turn > (updateRate << 6)) {
                    turn = updateRate << 6;
                }
                racer->yaw += turn;
                if (!func_8003B740(player, racer)) {
                    state->phase = 2;
                }
                if (state->phase == 2) {
                    func_80001248(0xF2, 0);
                    entry.objectID = 0xC3;
                    entry.x = obj->trans.x_position;
                    entry.y = obj->trans.y_position + 230.0f * obj->trans.scale;
                    entry.z = obj->trans.z_position;
                    entry.unkA = obj->trans.scale * 400.0f;
                    entry.unkC = obj->trans.scale * 240.0f;
                    entry.unkE = 0;
                    entry.unk10 = 0;
                    entry.unk12 = 0x20;
                    entry.unk14 = 10;
                    entry.unk15 = 50;
                    entry.unk16 = 0;
                    entry.unk17 = 0x55;
                    entry.unk18 = 0xFF;
                    entry.unk19 = 0;
                    entry.unk1A = 0;
                    entry.unk1C = 100;
                    entry.unk1D = 100;
                    entry.unk1E = 6;
                    entry.unk1F = 0;
                    entry.unk20 = 0x4000;
                    entry.unk22 = obj->trans.rotation.y_rotation;
                    glow = func_80005F14(&entry, 1);
                    if (glow != NULL) {
                        OBJ_STATE(obj, M082Glow)->obj = glow;
                        OBJ_STATE(obj, M082Glow)->timer = 0;
                        glow->level_entry = NULL;
                    }
                }
                break;
            case 2:
                func_80048518(10);
                prevFrame = obj->unk28;
                if (func_8001138C(obj, 0.008333334f, updateRate)) {
                    state->phase = 3;
                    if (func_m082_00F8C(state->kind, obj->trans.x_position,
                                        obj->trans.y_position + 480.0f * obj->trans.scale, obj->trans.z_position,
                                        480.0f * obj->trans.scale)) {
                        func_80001170(15);
                    } else {
                        OBJ_STATE(obj, M082Glow)->timer = 0x80;
                    }
                }
                if (prevFrame < 0.42f && 0.42f <= obj->unk28) {
                    func_80001248(0x200, 0);
                } else if (prevFrame < 0.56f && 0.56f <= obj->unk28) {
                    func_80001248(0x201, 0);
                } else if (prevFrame < 0.8f && 0.8f <= obj->unk28) {
                    func_80001248(0x202, 0);
                }
                break;
            case 3:
                func_80048518(10);
                glow = OBJ_STATE(obj, M082Glow)->obj;
                if (glow != NULL) {
                    if ((OBJ_STATE(obj, M082Glow)->timer += updateRate) > 0x80) {
                        func_800079B8(glow);
                        OBJ_STATE(obj, M082Glow)->obj = NULL;
                    } else {
                        glowState = glow->unk68;
                        s = func_80049C34(OBJ_STATE(obj, M082Glow)->timer << 8);
                        glowState->unkCB = 255.0f * s;
                        glowState->unkB4 = obj->trans.scale * 5000.0f * s;
                        glowState->unkA8 = (OBJ_STATE(obj, M082Glow)->timer << 1) * 10 + 320;
                        *OBJ_STATE(glow, s32) = 1;
                    }
                }
                if (func_80043510(0) & 0xD000) {
                    state->phase = 4;
                    func_m082_01778();
                    racer->unk1FA = 2;
                    func_m007_00D6C();
                    func_80001248(0xF4, 0);
                    if (glow != NULL && OBJ_STATE(obj, M082Glow)->obj != NULL) {
                        func_800079B8(glow);
                        OBJ_STATE(obj, M082Glow)->obj = NULL;
                    }
                }
                break;
            case 4:
                prevFrame = obj->unk28;
                if (func_8001138C(obj, -0.016666668f, updateRate)) {
                    state->phase = 5;
                }
                if (obj->unk28 <= 0.56f && 0.56f < prevFrame) {
                    func_80001248(0x200, 0);
                }
                break;
            case 5:
                break;
        }
        if ((state->phase == 1 || state->phase == 2 || state->phase == 3) && racer->unk5C8 < 10) {
            racer->unk5C8 = 10;
        }
        inst = obj->modelInstances[obj->modelIndex];
        if (inst != NULL && inst->dirty) {
            func_8003D21C(inst, inst->model, obj, 1.0f, 0.0f);
            func_m082_00E6C(inst->model, inst, state, updateRate);
            inst->dirty = FALSE;
        }
        if (obj->light != NULL && (light = *obj->light) != 0) {
            state->timer += updateRate;
            if (state->phase == 4 || state->phase == 5 || (state->timer & 0x20)) {
                func_800212D0(light);
            } else {
                func_800212E4(light);
            }
        }
    }
}

/* Writes each batch's scaled values into the instance's current buffer (kinds 3, 4 and 8 open or shut by state) */
static void func_m082_00E6C(M082Model *model, M082Inst *inst, M082State *state, UNUSED s32 arg3) {
    M082Anim *a;
    s16 *out;
    s32 i;
    s32 kind;
    M082Tex *tex;

    a = inst->anims;
    out = inst->out[inst->buffer];
    for (i = 0; i < model->count; i++, a++) {
        kind = a->unk4 & 0xFF;
        tex = model->textures[kind].tex;
        if (kind == 3 || kind == 4 || kind == 8) {
            if (state->phase == 4 || state->phase == 5 || (state->timer & 0x20)) {
                a->unk0 = 0;
            } else {
                a->unk0 = 0x100;
            }
        }
        *out++ = (a->unk0 >> 8) * tex->unk16;
        if (a->unk2 >= 0) {
            *out++ = (a->unk2 >> 8) * tex->unk16;
        }
    }
}

static void func_m082_0165C(s16 id, f32 x, f32 y, f32 z, f32 a, f32 b, f32 c);

/* Gives the player the weapon or item of the object's kind, and spawns its pickup (returns TRUE if it did) */
static s32 func_m082_00F8C(u32 kind, f32 x, f32 y, f32 z, f32 h) {
    s32 weapon;
    s32 id;
    s32 hint;
    Object *player;
    s32 isWeapon;
    s32 isItem;
    M082Player *racer;
    M082WeaponInfo *info;
    M082Weapons *weapons;

    player = func_8000E4D0(0);
    if (player == NULL) {
        return FALSE;
    }
    racer = player->unk68;
    weapons = func_80047C1C(0, racer->character);
    isItem = FALSE;
    isWeapon = FALSE;
    switch (kind) {
        case 0:
            weapon = 3;
            id = 0x1A9;
            hint = 0x16;
            isWeapon = TRUE;
            break;
        case 1:
            weapon = 1;
            id = 0x1D3;
            hint = 0x17;
            isWeapon = TRUE;
            break;
        case 2:
        case 3:
            weapon = 2;
            id = 0x1E1;
            hint = 0x19;
            isWeapon = TRUE;
            break;
        case 4:
            weapon = 9;
            id = 0x24E;
            hint = 0x1A;
            isWeapon = TRUE;
            break;
        case 5:
            weapon = 8;
            id = 0x272;
            hint = 0x1B;
            isWeapon = TRUE;
            break;
        case 6:
            weapon = 3;
            id = 0x270;
            hint = 0x1C;
            isItem = TRUE;
            break;
        case 7:
            weapon = 0x10;
            id = 0x18A;
            hint = 0x1D;
            isItem = TRUE;
            break;
        case 8:
        case 10:
            weapon = 14;
            id = 0x2F4;
            hint = 0x1E;
            isWeapon = TRUE;
            break;
        case 9:
            weapon = 13;
            id = 0x2F5;
            hint = 0x1F;
            isWeapon = TRUE;
            break;
        case 11:
            weapon = 12;
            id = 0x2F6;
            hint = 0x20;
            isWeapon = TRUE;
            break;
        case 12:
            weapon = 11;
            id = 0x317;
            hint = 0x21;
            isWeapon = TRUE;
            break;
        case 13:
            weapon = 10;
            id = 0x316;
            hint = 0x22;
            isWeapon = TRUE;
            break;
        case 14:
            weapon = 7;
            id = 0x2F1;
            hint = 0x23;
            isWeapon = TRUE;
            break;
        case 15:
            weapon = 6;
            id = 0x2F2;
            hint = 0x24;
            isWeapon = TRUE;
            break;
        case 16:
            weapon = 4;
            id = 0x2F3;
            hint = 0x25;
            isWeapon = TRUE;
            break;
        case 17:
            weapon = 0x16;
            id = 0x2C1;
            hint = 0x26;
            isItem = TRUE;
            break;
        case 18:
            weapon = 9;
            id = 0x2F0;
            hint = 0x27;
            isItem = TRUE;
            break;
        case 19:
            weapon = 1;
            id = 0x17D;
            hint = 0x28;
            isItem = TRUE;
            break;
        case 20:
            weapon = 5;
            id = 0x271;
            hint = 0x29;
            isWeapon = TRUE;
            break;
    }
    if (isWeapon) {
        if (!(weapons->owned & (1 << weapon))) {
            info = weapon_info_get(weapon);
            weapons->owned |= 1 << weapon;
            weapons->count++;
            weapons->maxAmmo[weapon] = info->maxAmmo;
            weapons->ammo[weapon] = info->ammo;
            weapons->unk54[weapon] = info->unk3;
            func_m082_0165C(id, x, y, z, 8.0f, h, 1.0f);
            func_m007_00148(hint, 0);
            return TRUE;
        }
    }
    if (isItem && !character_has_item(0, racer->character, weapon)) {
        character_item_give(0, racer->character, weapon);
        func_m082_0165C(id, x, y, z, 8.0f, h, 1.0f);
        func_m007_00148(hint, 0);
        return TRUE;
    }
    func_m007_00148(0x2A, 0);
    return FALSE;
}

/* Whether the player already has the weapon or item of a kind */
static s32 func_m082_013C0(u32 kind) {
    M082Weapons *weapons = func_80047C1C(0, func_800468CC());

    switch (kind) {
        case 0:
            if (weapons->owned & (1 << 3)) {
                return TRUE;
            }
            break;
        case 1:
            if (weapons->owned & (1 << 1)) {
                return TRUE;
            }
            break;
        case 2:
        case 3:
            if (weapons->owned & (1 << 2)) {
                return TRUE;
            }
            break;
        case 4:
            if (weapons->owned & (1 << 9)) {
                return TRUE;
            }
            break;
        case 5:
            if (weapons->owned & (1 << 8)) {
                return TRUE;
            }
            break;
        case 6:
            if (character_has_item(0, func_800468CC(), 3)) {
                return TRUE;
            }
            break;
        case 7:
            if (player_chars_have_item(0x10) || player_chars_have_item(0x11)) {
                return TRUE;
            }
            break;
        case 8:
        case 10:
            if (weapons->owned & (1 << 14)) {
                return TRUE;
            }
            break;
        case 9:
            if (weapons->owned & (1 << 13)) {
                return TRUE;
            }
            break;
        case 11:
            if (weapons->owned & (1 << 12)) {
                return TRUE;
            }
            break;
        case 12:
            if (weapons->owned & (1 << 11)) {
                return TRUE;
            }
            break;
        case 13:
            if (weapons->owned & (1 << 10)) {
                return TRUE;
            }
            break;
        case 14:
            if (weapons->owned & (1 << 7)) {
                return TRUE;
            }
            break;
        case 15:
            if (weapons->owned & (1 << 6)) {
                return TRUE;
            }
            break;
        case 16:
            if (weapons->owned & (1 << 4)) {
                return TRUE;
            }
            break;
        case 17:
            if (player_chars_have_item(0x16)) {
                return TRUE;
            }
            break;
        case 18:
            if (character_has_item(0, func_800468CC(), 9)) {
                return TRUE;
            }
            break;
        case 19:
            if (character_has_item(0, func_800468CC(), 1)) {
                return TRUE;
            }
            break;
        case 20:
            if (weapons->owned & (1 << 5)) {
                return TRUE;
            }
            break;
    }
    return FALSE;
}

/* The level entry 0165C spawns (size 0x10) */
typedef struct M082Entry {
    /* 0x00 */ s16 objectID;
    /* 0x02 */ u8 size;
    /* 0x03 */ u8 unk3;
    /* 0x04 */ s16 x;
    /* 0x06 */ s16 y;
    /* 0x08 */ s16 z;
    /* 0x0A */ s8 unkA;
    /* 0x0B */ s8 unkB;
    /* 0x0C */ f32 unkC;
} M082Entry;

/* Spawns object `id` at (x, y, z) and keeps it (the pickup 00F8C gives) */
static void func_m082_0165C(s16 id, f32 x, f32 y, f32 z, f32 a, f32 b, f32 c) {
    M082Entry entry;

    func_m007_02824();
    entry.objectID = id;
    entry.size = sizeof(M082Entry);
    entry.unk3 = 0;
    entry.x = x;
    entry.y = y;
    entry.z = z;
    entry.unkA = a;
    entry.unkB = b;
    entry.unkC = c;
    rl_m082_017A0 = (struct M082Linked *) func_80005F14(&entry, 1);
    if (rl_m082_017A0 != NULL) {
        ((Object *) rl_m082_017A0)->level_entry = NULL;
    }
}

/* What the module's first .data word points at, as far as 01778 uses it */
typedef struct M082Linked {
    /* 0x00 */ u8 pad0[0x88];
    /* 0x88 */ s32 unk88;
} M082Linked;

/* Sets the linked thing's flag at 0x88 and forgets it (when the object shuts) */
static void func_m082_01778(void) {
    if (rl_m082_017A0 != NULL) {
        rl_m082_017A0->unk88 = 1;
        rl_m082_017A0 = NULL;
    }
}

INCLUDE_ASM("asm/nonmatchings/modules/m082", data);
