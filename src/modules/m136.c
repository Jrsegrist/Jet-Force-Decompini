/*
 * Runlink module 136: the init functions of level set-up objects. Each
 * copies its level entry into the object (or its state at obj->unk68) and
 * registers it with whatever runs it: exits (a plane through the object),
 * fixed cameras, doors and door switches, fog, sky, sound points and lines,
 * scrolling textures, lights, lens flares, spline camera nodes, static hit
 * objects, position overrides, AI points and junctions, and troops.
 *
 * NON_MATCHING: the exit, level, door, lens flare, hit object, override,
 * troop and junction inits and the troop's leader search are written but
 * differ (mostly register numbering and scheduling); they and the .rodata
 * (whose users are among them) stay assembly in the default build.
 */
#include "module.h"
#include "gfx.h"

/* Common to the level entries here: the object ID and position */
typedef struct M136Entry {
    /* 0x00 */ s16 objectID;
    /* 0x02 */ s16 unk2;
    /* 0x04 */ s16 x;
    /* 0x06 */ s16 y;
    /* 0x08 */ s16 z;
    /* 0x0A */ u8 b[0x40 - 0xA]; /* bytes 0x0A.. */
} M136Entry;

#define OBJ_REACH(obj) (*(f32 *) ((u8 *) (obj) + 0x34))
#define TROOP_KIND16(t) (*(u16 *) &(t)->kind)
#define ENTRY_U8(e, off) (((u8 *) (e))[off])
#define ENTRY_S8(e, off) (((s8 *) (e))[off])
#define ENTRY_S16(e, off) (*(s16 *) ((u8 *) (e) + (off)))
#define ENTRY_U16(e, off) (*(u16 *) ((u8 *) (e) + (off)))

/* An exit: a plane through the object (obj->unk68) */
typedef struct M136Exit {
    /* 0x00 */ Vec3f normal;
    /* 0x0C */ f32 d;
    /* 0x10 */ s32 width;
    /* 0x14 */ s16 unk14;
    /* 0x16 */ s16 unk16;
} M136Exit;

/* A door (obj->unk68) */
typedef struct M136Door {
    /* 0x00 */ f32 baseY;
    /* 0x04 */ f32 heightScale;
    /* 0x08 */ u8 pad8[0x10 - 0x8];
    /* 0x10 */ s32 unk10;
    /* 0x14 */ f32 unk14;
    /* 0x18 */ f32 unk18;
    /* 0x1C */ f32 unk1C;
    /* 0x20 */ f32 unk20;
    /* 0x24 */ f32 unk24;
    /* 0x28 */ s32 unk28;
    /* 0x2C */ s16 unk2C;
    /* 0x2E */ s16 unk2E;
    /* 0x30 */ s16 unk30;
    /* 0x32 */ s16 speed;
    /* 0x34 */ s16 unk34;
    /* 0x36 */ s16 unk36;
    /* 0x38 */ s16 sound[3];
    /* 0x3E */ s16 unk3E;
    /* 0x40 */ u16 unk40;
    /* 0x42 */ u8 type;
    /* 0x43 */ u8 unk43;
    /* 0x44 */ u8 unk44;
    /* 0x45 */ u8 unk45;
    /* 0x46 */ u8 anim[4];
    /* 0x4A */ u8 unk4A;
    /* 0x4B */ u8 unk4B;
} M136Door;

/* A model as the door and the hit object read it */
typedef struct M136Model {
    /* 0x00 */ u8 pad0[0x12];
    /* 0x12 */ s16 numVerts;
    /* 0x14 */ u8 pad14[0x1C - 0x14];
    /* 0x1C */ Vertex *verts;
    /* 0x20 */ u8 pad20[0x4E - 0x20];
    /* 0x4E */ s8 unk4E;
} M136Model;

typedef struct M136ModelInstance {
    /* 0x00 */ M136Model *model;
    /* 0x04 */ Vertex *verts;
    /* 0x08 */ u8 pad8[0x8C - 0x8];
    /* 0x8C */ s16 bounds[6];
} M136ModelInstance;

/* What these objects' headers hold */
typedef struct M136Header {
    /* 0x00 */ f32 scale;
    /* 0x04 */ u8 pad4[0x1F - 0x4];
    /* 0x1F */ s8 numberOfModelIds;
    /* 0x20 */ u8 pad20[0x54 - 0x20];
    /* 0x54 */ f32 shadowScale;
} M136Header;

/* A sound point or line (obj->unk68) */
typedef struct M136Sound {
    /* 0x00 */ u16 sound;
    /* 0x02 */ u16 unk2;
    /* 0x04 */ u8 unk4;
    /* 0x05 */ u8 unk5;
    /* 0x06 */ u8 unk6;
    /* 0x07 */ u8 pad7;
    /* 0x08 */ s32 handle;
    /* 0x0C */ u8 unkC;
    /* 0x0D */ u8 unkD;
} M136Sound;

typedef struct M136SoundLine {
    /* 0x00 */ u8 sound;
    /* 0x01 */ u8 pad1;
    /* 0x02 */ u16 unk2;
    /* 0x04 */ u16 unk4;
    /* 0x06 */ u8 pad6[2];
    /* 0x08 */ s32 handle;
    /* 0x0C */ u8 unkC;
    /* 0x0D */ u8 unkD;
    /* 0x0E */ u8 unkE;
    /* 0x0F */ u8 unkF;
    /* 0x10 */ u8 unk10;
    /* 0x11 */ u8 unk11;
    /* 0x12 */ u8 unk12;
} M136SoundLine;

/* A scrolling texture (obj->unk68) */
typedef struct M136Scroll {
    /* 0x00 */ s16 texture;
    /* 0x02 */ s16 pad2;
    /* 0x04 */ s16 speedU;
    /* 0x06 */ s16 speedV;
    /* 0x08 */ s16 u;
    /* 0x0A */ s16 v;
} M136Scroll;

/* A lens flare's description (corona_add) */
typedef struct M136Flare {
    /* 0x00 */ u8 flags;
    /* 0x01 */ u8 unk1;
    /* 0x02 */ s16 unk2;
    /* 0x04 */ s16 unk4;
    /* 0x06 */ s16 unk6;
    /* 0x08 */ f32 x;
    /* 0x0C */ f32 y;
    /* 0x10 */ f32 z;
    /* 0x14 */ u8 r;
    /* 0x15 */ u8 g;
    /* 0x16 */ u8 b;
    /* 0x17 */ u8 a;
    /* 0x18 */ s16 unk18;
    /* 0x1A */ s16 unk1A;
    /* 0x1C */ s16 unk1C;
    /* 0x1E */ s8 unk1E;
    /* 0x1F */ s8 unk1F;
} M136Flare;

/* A spline camera node (obj->unk68) */
typedef struct M136Node {
    /* 0x00 */ u8 spline;
    /* 0x01 */ u8 index;
    /* 0x02 */ s16 unk2;
    /* 0x04 */ f32 unk4;
    /* 0x08 */ Vec3f pos;
    /* 0x14 */ Vec3f unk14;
    /* 0x20 */ Vec3f unk20;
    /* 0x2C */ u8 unk2C;
    /* 0x2D */ u8 unk2D;
    /* 0x2E */ u8 unk2E;
    /* 0x2F */ u8 pad2F[0x38 - 0x2F];
    /* 0x38 */ u8 unk38;
    /* 0x39 */ u8 unk39;
    /* 0x3A */ u8 unk3A;
    /* 0x3B */ u8 pad3B[0x44 - 0x3B];
    /* 0x44 */ u8 unk44;
    /* 0x45 */ u8 unk45;
    /* 0x46 */ u8 unk46;
} M136Node;

/* A position override (obj->unk68) */
typedef struct M136Override {
    /* 0x00 */ f32 unk0;
    /* 0x04 */ f32 unk4;
    /* 0x08 */ f32 unk8;
    /* 0x0C */ f32 unkC;
    /* 0x10 */ f32 unk10;
    /* 0x14 */ f32 unk14;
    /* 0x18 */ u32 unk18;
    /* 0x1C */ f32 unk1C;
    /* 0x20 */ f32 unk20;
    /* 0x24 */ Vec3f pos;
} M136Override;

/* An AI point (obj->unk68) */
typedef struct M136AIPoint {
    /* 0x00 */ u8 unk0;
    /* 0x01 */ u8 unk1;
    /* 0x02 */ u8 unk2;
    /* 0x03 */ u8 unk3;
    /* 0x04 */ u8 unk4;
    /* 0x05 */ u8 unk5;
    /* 0x06 */ u8 unk6;
    /* 0x07 */ u8 pad7;
    /* 0x08 */ u8 unk8;
    /* 0x09 */ u8 unk9;
    /* 0x0A */ u8 unkA;
} M136AIPoint;

/* An AI junction (obj->unk68) */
typedef struct M136Junction {
    /* 0x00 */ u8 pad0[0x10];
    /* 0x10 */ u8 flags;
    /* 0x11 */ u8 id;
} M136Junction;

/* A troop member (obj->unk68) */
typedef struct M136Troop {
    /* 0x00 */ u8 unk0;
    /* 0x01 */ u8 unk1;
    /* 0x02 */ u8 unk2;
    /* 0x03 */ u8 unk3[9];
    /* 0x0C */ u8 unkC[4];
    /* 0x10 */ s8 unk10[3];
    /* 0x13 */ u8 unk13[2];
    /* 0x15 */ s8 unk15[3];
    /* 0x18 */ u8 unk18[2];
    /* 0x1A */ u8 kind; /* bits 4-7: leader 4; bits 0-3 */
    /* 0x1B */ s8 unk1B[3];
    /* 0x1E */ u8 unk1E;
    /* 0x1F */ u8 unk1F[3];
    /* 0x22 */ u8 unk22[2];
    /* 0x24 */ s8 unk24[2];
    /* 0x26 */ u8 unk26[2];
    /* 0x28 */ u16 unk28;
    /* 0x2A */ s16 unk2A;
    /* 0x2C */ u16 unk2C;
    /* 0x2E */ s16 unk2E;
    /* 0x30 */ u8 pad30[3];
    /* 0x33 */ u8 unk33;
    /* 0x34 */ u8 unk34;
    /* 0x35 */ u8 unk35;
    /* 0x36 */ u8 unk36;
    /* 0x37 */ u8 unk37;
    /* 0x38 */ u8 unk38;
    /* 0x39 */ u8 unk39;
    /* 0x3A */ s16 unk3A;
    /* 0x3C */ s16 unk3C;
    /* 0x3E */ u8 pad3E[2];
    /* 0x40 */ s16 home[3];
    /* 0x46 */ u8 pad46[2];
    /* 0x48 */ Vec3f pos;
    /* 0x54 */ s32 unk54;
    /* 0x58 */ s32 unk58;
    /* 0x5C */ Object *leader;
    /* 0x60 */ u8 unk60;
    /* 0x61 */ u8 unk61;
    /* 0x62 */ u8 unk62;
    /* 0x63 */ u8 unk63;
    /* 0x64 */ u8 unk64;
    /* 0x65 */ u8 unk65;
    /* 0x66 */ s16 unk66[3];
    /* 0x6C */ f32 unk6C;
    /* 0x70 */ f32 unk70;
    /* 0x74 */ u8 unk74;
    /* 0x75 */ u8 unk75;
    /* 0x76 */ u8 unk76;
    /* 0x77 */ u8 unk77;
    /* 0x78 */ s8 unk78;
    /* 0x79 */ u8 pad79;
    /* 0x7A */ s8 unk7A;
    /* 0x7B */ u8 unk7B;
    /* 0x7C */ u8 pad7C[0x92 - 0x7C];
    /* 0x92 */ u16 unk92;
} M136Troop;

/* An entry of D_800FEA9C */
typedef struct M136Area {
    /* 0x00 */ u8 pad0[0x17];
    /* 0x17 */ u8 flags;
    /* 0x18 */ u8 pad18[0x34 - 0x18];
} M136Area;

extern void view_override_set();
extern s32 func_80001598(s32 sound);
extern void func_800036C8(u16 sound, f32 x, f32 y, f32 z, s32 arg4, s32 arg5, s32 arg6, s32 arg7, s32 arg8, s32 arg9,
                          s32 arg10, s32 arg11, s32 *handle);
extern void func_m044_0070C(s32 sound, s32 arg1, f32 x, f32 y, f32 z, s32 arg5, s32 arg6, s32 arg7, s32 arg8,
                            s32 arg9, s32 arg10, s32 arg11, s32 arg12);
extern void *func_8001B650();
extern s32 light_add_from_level_object_entry();
extern s32 corona_add(Object *obj, M136Flare *flare, M136Entry *setup);
extern void func_m011_000D4(s32 spline);
extern void func_8002F014();
extern Object **func_80050F44(s32 *count);
extern f32 point_distance_squared(f32 x1, f32 y1, f32 z1, f32 x2, f32 y2, f32 z2);
extern s32 func_m154_00130(f32 x, f32 y, f32 z);
extern M136Area *D_800FEA9C;

s32 D_m136_01DB0 = 136;
/* The door sounds (three) for each list of door objects */
static s16 sDoorSounds0[] = { 0x0009, 0x000A, 0x000B, 0x001F, 0x00C9, 0x004C, 0x00AB, 0x00DD, 0x00CA, 0x0140,
                              0x00CB, 0x00CC, 0x00CD, 0x00DA, 0x01AD, 0x00F9, 0x01C9, 0x019D, 0x018D, 0x021F,
                              0x02C4, -1 };
static s16 sDoorSounds1[] = { 0x01C0, 0x01C1, 0x01C2, 0x020E, -1 };
static s16 sDoorSounds2[] = { 0x01FC, 0x01FD, 0x01FE, 0x02C8, -1 };
static s16 sDoorSounds3[] = { 0x01FC, 0x0205, 0x0206, 0x0057, 0x0129, 0x031C, 0x027B, -1 };
static s16 sDoorSounds4[] = { 0x01FC, 0x01FD, 0x01FE, 0x014B, 0x0199, -1 };
static s16 sDoorSounds5[] = { 0x0000, 0x0000, 0x0000, 0x01A1, -1 };
static s16 sDoorSounds6[] = { 0x0000, 0x022C, 0x0000, 0x01AC, 0x02C0, -1 };
static s16 sDoorSounds7[] = { 0x01F7, 0x0000, 0x0000, 0x02BF, -1 };
static s16 sDoorSounds8[] = { 0x021E, 0x0000, 0x0000, 0x0075, -1 };
static s16 sDoorSounds9[] = { 0x0000, 0x01E0, 0x0000, 0x01AB, 0x02EF, 0x001A, 0x0146, 0x026D, 0x0017, 0x002F,
                              0x028E, 0x0334, 0x0096, -1 };
static s16 *sDoorSounds[] = {
    sDoorSounds0, sDoorSounds1, sDoorSounds2, sDoorSounds3, sDoorSounds4,
    sDoorSounds5, sDoorSounds6, sDoorSounds7, sDoorSounds8, sDoorSounds9, NULL,
};

#ifdef NON_MATCHING
void func_m136_00000(Object *obj, M136Entry *setup) {
    M136Exit *exit;
    f32 sinYaw;

    if (ENTRY_U8(setup, 0x12) < 5) {
        obj->trans.scale = 0.05f;
    } else {
        obj->trans.scale = ENTRY_U8(setup, 0x12) * 0.01f;
    }
    exit = obj->unk68;
    obj->trans.rotation.y_rotation = ENTRY_U8(setup, 0x13) << 10;
    obj->trans.rotation.x_rotation = ENTRY_U8(setup, 0x1B) << 10;
    sinYaw = func_80049C34(obj->trans.rotation.y_rotation);
    exit->normal.x = func_80049C30(obj->trans.rotation.x_rotation) * sinYaw;
    exit->normal.y = -func_80049C34(obj->trans.rotation.x_rotation);
    sinYaw = func_80049C30(obj->trans.rotation.y_rotation);
    exit->normal.z = func_80049C30(obj->trans.rotation.x_rotation) * sinYaw;
    exit->d = -(exit->normal.x * obj->trans.x_position + exit->normal.y * obj->trans.y_position +
                exit->normal.z * obj->trans.z_position);
    exit->width = ENTRY_U8(setup, 0x12);
    exit->unk14 = ENTRY_S8(setup, 0x18);
    exit->unk16 = ENTRY_S8(setup, 0x15);
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m136", func_m136_00000);
#endif

void func_m136_00158(Object *obj, M136Entry *setup) {
    obj->trans.rotation.y_rotation = ENTRY_S16(setup, 0xE);
    obj->trans.rotation.x_rotation = ENTRY_S16(setup, 0xC);
    obj->trans.rotation.z_rotation = ENTRY_S16(setup, 0xA);
    view_override_set();
}

#ifdef NON_MATCHING
void func_m136_00194(Object *obj, M136Entry *setup) {
    runlink_unload_timer_set(D_m136_01DB0, 2, 2);
    *(s32 *) &obj->state[0] = ENTRY_U8(setup, 0xA);
    *(s32 *) &obj->state[4] = ENTRY_U8(setup, 0xB);
    obj->trans.rotation.y_rotation = ENTRY_U8(setup, 0xC) << 10;
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m136", func_m136_00194);
#endif

#ifdef NON_MATCHING
void func_m136_001FC(Object *obj, M136Entry *setup) {
    M136Door *door = obj->unk68;
    M136ModelInstance *inst;
    M136Model *model;
    Vertex *in;
    Vertex *out;
    s16 **list;
    s16 *ids;
    s16 min;
    s16 max;
    s32 i;
    s32 j;
    s32 found;
    f32 size;
    f32 maxReach;
    f32 f;
    f32 unit;
    f32 zero;

    door->baseY = obj->trans.y_position;
    door->unk36 = ENTRY_U8(setup, 0xE) << 1;
    door->unk40 = ENTRY_U8(setup, 0x10);
    door->type = ENTRY_U8(setup, 0x11);
    door->unk43 = ENTRY_U8(setup, 0x12);
    door->unk2E = ENTRY_S16(setup, 0xA);
    door->unk30 = ENTRY_S16(setup, 0x18);
    door->unk2C = ENTRY_S16(setup, 0xC);
    door->unk44 = ENTRY_U8(setup, 0x14);
    door->unk45 = ENTRY_U8(setup, 0x15);
    door->heightScale = ENTRY_U8(setup, 0x16) * 0.015625f;
    door->unk4A = 0;
    if (ENTRY_U8(setup, 0x17) != 0) {
        door->unk28 = ENTRY_U8(setup, 0x17);
    } else {
        door->unk28 = 0xFF;
    }
    if (door->type == 6) {
        door->type = 4;
        door->unk40 *= 10;
    }
    zero = 0.0f;
    switch (door->type) {
        case 4:
            if (door->unk2C < 0) {
                door->speed = -0x80;
            } else {
                door->speed = 0x80;
            }
            break;
        case 5:
            unit = 0.01f;
            door->anim[0] = ENTRY_U8(setup, 0x1E);
            door->unk14 = ENTRY_U8(setup, 0x1A) * unit;
            if (ENTRY_U8(setup, 0x1E) == 0xFF) {
                door->anim[0] = ENTRY_U8(setup, 0x1F);
                door->unk14 = -(f32) ENTRY_U8(setup, 0x1A) * unit;
            } else {
                door->anim[0] = ENTRY_U8(setup, 0x1E);
                door->unk14 = ENTRY_U8(setup, 0x1A) * unit;
            }
            if (ENTRY_U8(setup, 0x1F) == 0xFF) {
                door->anim[1] = ENTRY_U8(setup, 0x1E);
                door->unk18 = -(f32) ENTRY_U8(setup, 0x1B) * unit;
            } else {
                door->anim[1] = ENTRY_U8(setup, 0x1F);
                door->unk18 = ENTRY_U8(setup, 0x1B) * unit;
            }
            door->anim[2] = ENTRY_U8(setup, 0x20);
            door->unk1C = ENTRY_U8(setup, 0x1C) * unit;
            door->anim[3] = ENTRY_U8(setup, 0x21);
            door->unk20 = ENTRY_U8(setup, 0x1D) * unit;
            if (door->anim[3] == 0xFF) {
                func_8001149C(obj, door->anim[1], zero);
                door->unk24 = door->unk18;
            } else {
                func_8001149C(obj, door->anim[3], zero);
                door->unk24 = door->unk20;
            }
            if (door->anim[2] != 0xFF) {
                obj->modelIndex = 1;
            } else {
                obj->modelIndex = ENTRY_U8(setup, 0xF);
            }
            break;
        default:
            if (door->unk2C < 0) {
                door->speed = -0x100;
            } else {
                door->speed = 0x100;
            }
            break;
    }
    door->unk10 = 0;
    door->unk34 = 0;
    if (door->unk43 == 0) {
        door->unk3E = 0;
    } else {
        door->unk3E = 8;
    }
    door->unk4B = ENTRY_U8(setup, 0x22);
    obj->trans.rotation.y_rotation = door->unk2E & 0xFFF0;
    obj->trans.rotation.x_rotation = door->unk30 & 0xFFF0;
    maxReach = zero;
    if (door->type != 5) {
        obj->modelIndex = ENTRY_U8(setup, 0xF);
    }
    size = ENTRY_U8(setup, 0x13);
    if (size < 10.0f) {
        size = 10.0f;
    }
    size *= 0.015625f;
    obj->trans.scale = ((M136Header *) obj->header)->scale * size;
    if (obj->modelIndex >= ((M136Header *) obj->header)->numberOfModelIds) {
        obj->modelIndex = 0;
    }
    for (i = 0; i < ((M136Header *) obj->header)->numberOfModelIds; i++) {
        inst = ((M136ModelInstance **) obj->modelInstances)[i];
        model = inst->model;
        if (model->unk4E == 0) {
            in = model->verts;
            out = inst->verts;
            min = 0x7D00;
            max = -0x7D00;
            for (j = 0; j < model->numVerts; j++) {
                out->y = in->y * door->heightScale;
                if (max < out->y) {
                    max = out->y;
                }
                if (out->y < min) {
                    min = out->y;
                }
                in++;
                out++;
            }
            inst->bounds[1] = min;
            inst->bounds[4] = max;
            for (j = 0; j < 6; j++) {
                f = inst->bounds[j];
                if (f < zero) {
                    f = -f;
                }
                if (maxReach < f) {
                    maxReach = f;
                }
            }
            OBJ_REACH(obj) = obj->trans.scale * maxReach;
        }
    }
    door->sound[0] = 0;
    door->sound[1] = 0;
    door->sound[2] = 0;
    found = FALSE;
    for (list = sDoorSounds; *list != NULL && !found; list++) {
        ids = *list;
        if (ids[3] != -1 && !found) {
            for (j = 3; ids[j] != -1 && !found; j++) {
                if (setup->objectID == ids[j]) {
                    found = TRUE;
                    door->sound[0] = ids[0];
                    door->sound[1] = ids[1];
                    door->sound[2] = ids[2];
                }
            }
        }
    }
    func_80079D1C(obj, -1, 0, 8, 0, 0, 0, obj->trans.scale * 32.0f, obj->trans.scale * 32.0f, -1.0f, 1.0f, 1);
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m136", func_m136_001FC);
#endif

typedef struct M136Switch {
    /* 0x00 */ f32 unk0;
    /* 0x04 */ s32 unk4;
    /* 0x08 */ u8 unk8;
    /* 0x09 */ u8 unk9;
    /* 0x0A */ u8 unkA;
    /* 0x0B */ u8 unkB;
    /* 0x0C */ u8 unkC;
} M136Switch;

void func_m136_009FC(Object *obj, M136Entry *setup) {
    M136Switch *sw = obj->unk68;
    f32 size;

    sw->unk4 = 0;
    sw->unk8 = 0;
    sw->unk0 = 0.0f;
    sw->unk9 = ENTRY_U8(setup, 0xE);
    sw->unkA = 0xFF;
    sw->unkB = 0;
    sw->unkC = ENTRY_U8(setup, 0xF);
    size = ENTRY_U8(setup, 0xD);
    if (size < 10.0f) {
        size = 10.0f;
    }
    size /= 64;
    obj->trans.scale = ((M136Header *) obj->header)->scale * size;
    obj->trans.rotation.y_rotation = ENTRY_U8(setup, 0xA) << 8;
    obj->trans.rotation.x_rotation = ENTRY_U8(setup, 0xB) << 8;
    obj->trans.rotation.z_rotation = ENTRY_U8(setup, 0xC) << 8;
    func_80079D1C(obj, -1, 0, 8, 0, 0, 0, 32.0f, 32.0f, -1.0f, 1.0f, 1);
}

void func_m136_00B1C(Object *obj, M136Entry *setup) {
    f32 d = ENTRY_U8(setup, 0xA) * 8.0f;

    d *= d;
    *(f32 *) &obj->state[0] = d;
}

void func_m136_00B5C(Object *obj, M136Entry *setup) {
    *(s32 *) &obj->state[0] = ENTRY_U8(setup, 0xA);
    *(s32 *) &obj->state[4] = ENTRY_U8(setup, 0xB);
}

void func_m136_00B74(Object *obj, M136Entry *setup) {
    M136Sound *snd = obj->unk68;

    snd->sound = ENTRY_U16(setup, 0xA);
    snd->unk2 = ENTRY_U16(setup, 0xC);
    snd->unkC = ENTRY_U8(setup, 0x11);
    snd->unk6 = ENTRY_U8(setup, 0x10);
    snd->unk4 = ENTRY_U8(setup, 0xE);
    snd->unk5 = ENTRY_U8(setup, 0xF);
    snd->unkD = ENTRY_U8(setup, 0x12);
    snd->handle = 0;
    if (func_80001598(snd->sound) != 0) {
        func_800036C8(snd->sound, setup->x, setup->y, setup->z, 9, snd->unk5, snd->unk4, snd->unk2, snd->unkC,
                      snd->unk6, snd->unkD, 0, &snd->handle);
    } else {
        func_800036C8(snd->sound, setup->x, setup->y, setup->z, 10, snd->unk5, snd->unk4, snd->unk2, snd->unkC,
                      snd->unk6, snd->unkD, 0, &snd->handle);
    }
    func_800079B8(obj);
}

void func_m136_00D08(Object *obj, M136Entry *setup) {
    M136SoundLine *snd = obj->unk68;

    snd->sound = ENTRY_U8(setup, 0xA);
    snd->unk2 = ENTRY_U16(setup, 0xC);
    snd->unkC = ENTRY_U8(setup, 0xE);
    snd->unkD = ENTRY_U8(setup, 0xF);
    snd->handle = 0;
    snd->unk4 = ENTRY_U16(setup, 0x10);
    snd->unk11 = ENTRY_U8(setup, 0x14);
    snd->unk10 = ENTRY_U8(setup, 0x13);
    snd->unkE = ENTRY_U8(setup, 0xB);
    snd->unkF = ENTRY_U8(setup, 0x12);
    snd->unk12 = ENTRY_U8(setup, 0x15);
    func_m044_0070C(snd->sound, snd->unk2, setup->x, setup->y, setup->z, snd->unkF, snd->unkE, snd->unk10, snd->unk12,
                    snd->unk4, snd->unk11, snd->unkC, snd->unkD);
    func_800079B8(obj);
}

void func_m136_00E30(Object *obj, M136Entry *setup, s32 arg2) {
    M136Scroll *scroll = obj->unk68;
    s16 *textures;

    textures = func_8001B650();
    scroll->texture = ENTRY_S16(setup, 0xA);
    if (scroll->texture < 0) {
        scroll->texture = 0;
    }
    if (scroll->texture >= textures[0xC]) {
        scroll->texture = textures[0xC] - 1;
    }
    scroll->speedU = ENTRY_S8(setup, 0xC);
    scroll->speedV = ENTRY_S8(setup, 0xD);
    if (arg2 == 0) {
        scroll->u = 0;
        scroll->v = 0;
    }
}

void func_m136_00ED4(Object *obj, M136Entry *setup, s32 arg2) {
    obj->unk68 = (void *) light_add_from_level_object_entry(obj, setup);
}

#ifdef NON_MATCHING
void func_m136_00F04(Object *obj, M136Entry *setup) {
    M136Flare flare;

    flare.flags = 0x21;
    flare.unk18 = -1;
    if (ENTRY_U8(setup, 0xE) != 0) {
        flare.flags = 0x23;
    }
    if (ENTRY_U8(setup, 0xF) != 0) {
        flare.flags |= 4;
    }
    if (ENTRY_U8(setup, 0x10) != 0) {
        flare.unk18 = 0x61;
    }
    flare.unk1 = 0x50;
    flare.unk2 = ENTRY_S16(setup, 0xC);
    flare.unk6 = 0;
    flare.x = 0.0f;
    flare.y = 0.0f;
    flare.z = 0.0f;
    flare.unk4 = ENTRY_S16(setup, 0xA);
    flare.r = ENTRY_U8(setup, 0x12);
    flare.g = ENTRY_U8(setup, 0x13);
    flare.b = ENTRY_U8(setup, 0x14);
    flare.unk1A = 100;
    flare.unk1C = 100;
    flare.unk1E = -1;
    flare.unk1F = 1;
    flare.a = ENTRY_U8(setup, 0x15);
    *(s32 *) &obj->state[0] = corona_add(obj, &flare, setup);
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m136", func_m136_00F04);
#endif

void func_m136_00FF0(Object *obj, M136Entry *setup, s32 arg2) {
    obj->trans.scale = ENTRY_S16(setup, 0xA);
    obj->trans.scale /= 40.0f;
}

void func_m136_01020(Object *obj, M136Entry *setup, s32 arg2) {
}

void func_m136_01030(Object *obj, M136Entry *setup, s32 arg2) {
    M136Node *node = obj->unk68;

    node->spline = ENTRY_U8(setup, 0xA);
    node->index = ENTRY_U8(setup, 0xB);
    node->unk4 = ENTRY_U8(setup, 0x24);
    node->unk2 = ENTRY_U8(setup, 0xC) * 4;
    node->unk2C = ENTRY_U8(setup, 0xF);
    node->unk2D = ENTRY_U8(setup, 0xD) & 1;
    node->unk2E = ENTRY_U8(setup, 0xE) & 1;
    node->unk38 = ENTRY_U8(setup, 0x12);
    node->unk39 = ENTRY_U8(setup, 0xD) & 2;
    node->unk3A = ENTRY_U8(setup, 0xE) & 2;
    node->unk44 = ENTRY_U8(setup, 0x15);
    node->unk45 = ENTRY_U8(setup, 0xD) & 4;
    node->unk46 = ENTRY_U8(setup, 0xE) & 4;
    node->pos.x = setup->x;
    node->pos.y = setup->y;
    node->pos.z = setup->z;
    node->unk14.x = ENTRY_S16(setup, 0x18);
    node->unk14.y = ENTRY_S16(setup, 0x1A);
    node->unk14.z = ENTRY_S16(setup, 0x1C);
    node->unk20.x = ENTRY_U16(setup, 0x1E);
    node->unk20.y = ENTRY_U16(setup, 0x20);
    node->unk20.z = ENTRY_U16(setup, 0x22);
    func_m011_000D4(node->spline);
}

#ifdef NON_MATCHING
void func_m136_01224(Object *obj, M136Entry *setup) {
    M136ModelInstance *inst;
    s16 *bounds;
    f32 size;
    f32 maxReach;
    f32 f;
    s32 j;

    size = (s32) ENTRY_U8(setup, 0xB);
    if (size < 10.0f) {
        size = 10.0f;
    }
    size *= 0.015625f;
    obj->trans.scale = ((M136Header *) obj->header)->scale * size;
    if (obj->shadow != NULL) {
        *obj->shadow = ((M136Header *) obj->header)->shadowScale * size;
    }
    obj->modelIndex = ENTRY_U8(setup, 0xA);
    obj->trans.rotation.y_rotation = ENTRY_U8(setup, 0xC) << 8;
    func_80079D1C(obj, -1, 0, 8, 0, 0, 0, 32.0f, 32.0f, -1.0f, 1.0f, 1);
    if (obj->modelIndex >= ((M136Header *) obj->header)->numberOfModelIds) {
        obj->modelIndex = 0;
    }
    inst = ((M136ModelInstance **) obj->modelInstances)[obj->modelIndex];
    bounds = inst->bounds;
    maxReach = 0.0f;
    for (j = 0; j < 6; j++) {
        f = bounds[j];
        if (f < 0.0f) {
            f = -f;
        }
        if (maxReach < f) {
            maxReach = f;
        }
    }
    OBJ_REACH(obj) = obj->trans.scale * maxReach;
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m136", func_m136_01224);
#endif

#ifdef NON_MATCHING
void func_m136_014C0(Object *obj, M136Entry *setup, s32 arg2) {
    M136Override *ov = obj->unk68;

    ov->unk0 = ENTRY_U16(setup, 0xA);
    ov->unk4 = setup->y + ENTRY_S16(setup, 0x1A);
    ov->unk8 = setup->y - ENTRY_S16(setup, 0x1A);
    ov->unkC = ENTRY_U16(setup, 0xC);
    ov->unk10 = setup->y + ENTRY_S16(setup, 0x1C);
    ov->unk14 = setup->y - ENTRY_S16(setup, 0x1C);
    ov->unk18 = (f32) ENTRY_U16(setup, 0xE);
    ov->unk1C = ENTRY_S16(setup, 0x10);
    ov->unk20 = ENTRY_S16(setup, 0x12);
    ov->pos.x = obj->trans.x_position + ENTRY_S16(setup, 0x14);
    ov->pos.y = obj->trans.y_position + ENTRY_S16(setup, 0x16);
    ov->pos.z = obj->trans.z_position + ENTRY_S16(setup, 0x18);
    if (arg2 == 0) {
        func_8002F014();
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m136", func_m136_014C0);
#endif

void func_m136_016D0(Object *obj, M136Entry *setup) {
    M136AIPoint *point = obj->unk68;

    point->unk0 = ENTRY_U8(setup, 0xA);
    point->unk1 = ENTRY_U8(setup, 0xB);
    point->unk2 = ENTRY_U8(setup, 0xC);
    point->unk3 = ENTRY_U8(setup, 0xD);
    point->unk4 = ENTRY_U8(setup, 0xE);
    point->unk6 = ENTRY_U8(setup, 0xF);
    point->unk5 = 0;
    point->unk8 = ENTRY_U8(setup, 0x10);
    point->unk9 = ENTRY_U8(setup, 0x11);
    point->unkA = ENTRY_U8(setup, 0x12);
}

#ifdef NON_MATCHING
/* The nearest other troop object that is not a leader */
static Object *find_leader(Object *obj) {
    Object **objects;
    Object *o;
    Object *nearest;
    s32 count;
    f32 d;
    f32 best;

    best = 5000000000.0f;
    objects = func_80050F44(&count);
    nearest = NULL;
    while (count--) {
        o = objects[count];
        if (obj != o && (TROOP_KIND16((M136Troop *) o->unk68) >> 12) != 4) {
            d = point_distance_squared(o->trans.x_position, o->trans.y_position, o->trans.z_position, obj->trans.x_position,
                              obj->trans.y_position, obj->trans.z_position);
            if (d < best) {
                best = d;
                nearest = o;
            }
        }
    }
    return nearest;
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m136", func_m136_01740);
#endif

#ifdef NON_MATCHING
void func_m136_01850(Object *obj, M136Entry *setup) {
    M136Troop *troop = obj->unk68;
    M136Area *area;

    troop->unk64 = ENTRY_U8(setup, 0x2D);
    if ((TROOP_KIND16(troop) >> 12) == 4) {
        if (troop->leader == NULL) {
            troop->leader = find_leader(obj);
        }
        ENTRY_U8(setup, 0xB) = 0;
    }
    troop->unk2C = ENTRY_U16(setup, 0x36);
    troop->unk1 = ENTRY_U8(setup, 0xB);
    troop->unk3[0] = ENTRY_U8(setup, 0xC);
    troop->unk3[1] = ENTRY_U8(setup, 0xD);
    troop->unk3[2] = ENTRY_U8(setup, 0xE);
    troop->unk3[3] = ENTRY_U8(setup, 0xF);
    troop->unk3[4] = ENTRY_U8(setup, 0x10);
    troop->unk3[5] = ENTRY_U8(setup, 0x11);
    troop->unk3[6] = ENTRY_U8(setup, 0x12);
    troop->unk3[7] = ENTRY_U8(setup, 0x13);
    troop->unk3[8] = ENTRY_U8(setup, 0x14);
    troop->unk2 = ENTRY_U8(setup, 0x15);
    troop->unkC[0] = ENTRY_U8(setup, 0x16);
    troop->unkC[1] = ENTRY_U8(setup, 0x17);
    troop->unkC[2] = ENTRY_U8(setup, 0x18);
    troop->unkC[3] = ENTRY_U8(setup, 0x19);
    troop->unk10[0] = ENTRY_S8(setup, 0x1A);
    troop->unk10[1] = ENTRY_S8(setup, 0x1B);
    troop->unk10[2] = ENTRY_S8(setup, 0x1C);
    troop->unk13[0] = ENTRY_U8(setup, 0x1D);
    troop->unk13[1] = ENTRY_U8(setup, 0x1E);
    troop->unk15[0] = ENTRY_S8(setup, 0x1F);
    troop->unk15[1] = ENTRY_S8(setup, 0x20);
    troop->unk15[2] = ENTRY_S8(setup, 0x21);
    troop->unk18[0] = ENTRY_U8(setup, 0x22);
    troop->unk18[1] = ENTRY_U8(setup, 0x23);
    troop->kind = (ENTRY_U8(setup, 0x29) << 4) | (troop->kind & 0xFF0F);
    troop->kind = (ENTRY_U8(setup, 0x24) & 0xF) | (troop->kind & 0xF0);
    troop->unk1B[0] = ENTRY_S8(setup, 0x25);
    troop->unk1B[1] = ENTRY_S8(setup, 0x26);
    troop->unk1B[2] = ENTRY_S8(setup, 0x27);
    troop->unk1E = ENTRY_U8(setup, 0x28);
    troop->unk76 = ENTRY_U8(setup, 0x35);
    troop->unk1F[0] = ENTRY_U8(setup, 0x2E);
    troop->unk1F[1] = ENTRY_U8(setup, 0x2F);
    troop->unk1F[2] = ENTRY_U8(setup, 0x30);
    troop->unk22[0] = ENTRY_U8(setup, 0x32);
    troop->unk22[1] = ENTRY_U8(setup, 0x33);
    troop->unk2A = ENTRY_U8(setup, 0x34) << 8;
    troop->unk24[0] = ENTRY_S8(setup, 0x38);
    troop->unk24[1] = ENTRY_S8(setup, 0x39);
    troop->unk28 = ENTRY_U16(setup, 0x3A) & 0xFFF7;
    troop->unk26[0] = ENTRY_U8(setup, 0x3C);
    troop->unk26[1] = ENTRY_U8(setup, 0x3D);
    troop->unk34 = (ENTRY_U8(setup, 0xA) & 3) | (troop->unk34 & 0xFFFC);
    troop->unk60 = ENTRY_U8(setup, 0x2A);
    troop->unk33 = 0;
    troop->unk34 &= 3;
    troop->unk35 = 0;
    troop->unk2E = 1;
    troop->unk37 = 0;
    troop->unk38 = 1;
    troop->unk39 = 1;
    troop->unk3A = 0;
    troop->unk3C = 0;
    troop->unk54 = 0;
    troop->unk61 = ENTRY_U8(setup, 0x2B);
    troop->pos.x = obj->trans.x_position;
    troop->pos.y = obj->trans.y_position;
    troop->pos.z = obj->trans.z_position;
    troop->unk66[0] = 0;
    troop->unk66[1] = 0;
    troop->unk66[2] = 0;
    troop->unk6C = 1.0f;
    troop->unk70 = 0.0f;
    troop->unk7B = ENTRY_U8(setup, 0x3E);
    troop->unk74 = 0;
    troop->unk62 = ENTRY_U8(setup, 0x2C);
    troop->unk77 = 0xFF;
    troop->unk36 = 0;
    troop->unk78 = -1;
    troop->unk75 = ENTRY_U8(setup, 0x31);
    troop->unk7A = ENTRY_S8(setup, 0x3F);
    troop->home[0] = obj->trans.x_position;
    troop->home[1] = obj->trans.y_position;
    troop->unk65 = 0;
    troop->home[2] = obj->trans.z_position;
    troop->unk92 = ENTRY_U16(setup, 0x40);
    if (troop->unk58 == 0) {
        troop->unk63 = 0;
        troop->unk0 = 0;
    }
    obj->trans.rotation.y_rotation = 0xC000 - (troop->unk1B[0] << 8);
    if ((troop->kind & 0xF) == 6) {
        area = &D_800FEA9C[func_m154_00130(obj->trans.x_position, obj->trans.y_position, obj->trans.z_position)];
        if (area != NULL && (area->flags >> 6) != 0) {
            troop->unk33 = 0x1F;
        }
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m136", func_m136_01850);
#endif

#ifdef NON_MATCHING
void func_m136_01CA4(Object *obj, M136Entry *setup) {
    M136Junction *junction = obj->unk68;
    M136Junction *other;
    Object **objects;
    s32 first;
    s32 count;
    s32 i;
    s32 taken;
    u8 id;

    id = 0;
    objects = func_80005E98(&first, &count);
    if (junction != NULL && setup != NULL) {
        junction->flags &= 0xFFFE;
        if (ENTRY_U8(setup, 0xA) != 0) {
            junction->id = ENTRY_U8(setup, 0xA);
        } else {
            do {
                id++;
                taken = FALSE;
                for (i = first; i < count; i++) {
                    if (objects[i]->behaviorId == 0x1B && objects[i] != obj) {
                        other = objects[i]->unk68;
                        if (id == other->id) {
                            taken = TRUE;
                            break;
                        }
                    }
                }
            } while (taken);
            ENTRY_U8(setup, 0xA) = id;
            junction->id = id;
        }
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m136", func_m136_01CA4);

INCLUDE_ASM("asm/nonmatchings/modules/m136", rodata);
#endif
