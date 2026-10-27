/*
 * Runlink module 117: a cloud of twenty glowing motes that swirl inside a
 * box. The cloud brightens while something is inside its radius (with a
 * sound when that begins and ends) and fades otherwise; it keeps a light
 * object for its glow, and renders the motes as billboards, back to front.
 */
#include "module.h"

/* The cloud's level entry */
typedef struct M117Setup {
    /* 0x00 */ LevelObjectEntryCommon common;
    /* 0x0A */ u8 colour; /* index into the colour table (& 3) */
    /* 0x0B */ u8 sizeX;
    /* 0x0C */ u8 sizeY;
    /* 0x0D */ u8 sizeZ;
    /* 0x0E */ u8 yaw;
    /* 0x0F */ u8 pitch;
    /* 0x10 */ u8 unk10;
    /* 0x11 */ u8 unk11;
    /* 0x12 */ u8 radius;
    /* 0x13 */ u8 moteRadius;
    /* 0x14 */ u8 lightUnk14;
    /* 0x15 */ u8 lightUnk15;
    /* 0x16 */ u8 lightUnk16;
    /* 0x17 */ u8 lightUnk17;
} M117Setup;

/* One mote */
typedef struct M117Mote {
    /* 0x0 */ s16 angle;
    /* 0x2 */ s16 phase;
    /* 0x4 */ s16 radius;
    /* 0x6 */ s16 phaseSpeed;
    /* 0x8 */ f32 scale;
    /* 0xC */ s8 spin;
    /* 0xD */ s8 spinSpeed;
    /* 0xE */ s8 roll;
    /* 0xF */ s8 rollSpeed;
} M117Mote;

/* The cloud (obj->unk68) */
typedef struct M117Cloud {
    /* 0x000 */ s16 sizeX;
    /* 0x002 */ s16 sizeY;
    /* 0x004 */ s16 sizeZ;
    /* 0x006 */ s16 unk6;
    /* 0x008 */ s16 unk8;
    /* 0x00A */ s16 radius;
    /* 0x00C */ u8 red;
    /* 0x00D */ u8 green;
    /* 0x00E */ u8 blue;
    /* 0x00F */ u8 brightness;
    /* 0x010 */ Object *light;
    /* 0x014 */ M117Mote motes[20];
    /* 0x154 */ u8 soundOn;
} M117Cloud;

/* The level entry of the light object it spawns (object 0xC3) */
typedef struct M117LightEntry {
    /* 0x00 */ s16 objectID;
    /* 0x02 */ s16 unk2;
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
    /* 0x16 */ u8 red;
    /* 0x17 */ u8 green;
    /* 0x18 */ u8 blue;
    /* 0x19 */ u8 unk19;
    /* 0x1A */ s16 unk1A;
    /* 0x1C */ u8 unk1C;
    /* 0x1D */ u8 unk1D;
    /* 0x1E */ u8 unk1E;
    /* 0x1F */ u8 unk1F;
    /* 0x20 */ s16 unk20;
    /* 0x22 */ s16 unk22;
} M117LightEntry;

/* What the light object's unk68 points at */
typedef struct M117LightData {
    /* 0x00 */ u8 pad0[0xB8];
    /* 0xB8 */ f32 unkB8;
    /* 0xBC */ f32 unkBC;
} M117LightData;

/* A billboard to render: the first fields of an Object (transform ... unk28) */
typedef struct M117Billboard {
    /* 0x00 */ ObjectTransform trans;
    /* 0x18 */ s16 animFrame;
    /* 0x1A */ s16 unk1A;
    /* 0x1C */ Vec3f velocity;
    /* 0x28 */ f32 unk28;
} M117Billboard;

/* What obj->header points at here (the billboard texture at 0xAC) */
typedef struct M117Header {
    /* 0x00 */ u8 pad0[0xAC];
    /* 0xAC */ s32 *texture;
} M117Header;

#define OBJ_LIGHT(obj) (((s32 **) (obj)->pad70)[(0x74 - 0x70) / 4])

extern Object *func_80005F14(M117LightEntry *entry, s32 count);
extern void func_8002130C(s32 light, u8 red, u8 green, u8 blue); /* sets a light's colour */
extern void func_80021358(s32 light, u8 intensity);             /* sets a light's intensity */
extern s32 func_8007E150(f32 x, f32 y, f32 z, f32 radius, s32 arg4, void *list);
extern void func_80049DC0(void *trans, MtxF *out);
extern void func_80048B60(MtxF *mtx, f32 x, f32 y, f32 z, f32 *ox, f32 *oy, f32 *oz); /* DKR mathMtxXFMF */
extern f32 func_800420F0(f32 x, f32 y, f32 z); /* depth of a point from the camera */
extern void func_8005600C(u8 a, u8 b, u8 c, u8 d, u8 e, u8 f);
extern void func_80056064(void);
extern void func_80041174(s32 arg0, s32 arg1, s32 arg2, M117Billboard *board, s32 texture, s32 arg5, s32 alpha);

static u8 D_m117_00B80[4][4] = {
    { 0x08, 0xFF, 0x20, 0x00 },
    { 0xFF, 0x20, 0x08, 0x00 },
    { 0x20, 0x08, 0xFF, 0x00 },
    { 0x20, 0x20, 0x20, 0x00 },
};

/*
 * NON_MATCHING: only func_m117_00154 matches. The static light spawner (the
 * original keeps the level entry in a3; the order of the entry stores differs),
 * func_m117_003F8 (the f20/f22 registers of base and range are swapped) and
 * func_m117_0070C (a first draft) do not. The module's .rodata floats are used by
 * all of them, so the default build takes every function and the .rodata from
 * assembly; only the .data is C.
 */
#ifdef NON_MATCHING
static void spawnLight(Object *obj) {
    M117Cloud *cloud;
    M117Setup *setup;
    M117LightEntry entry;
    Object *light;
    s32 unused[2]; /* never used, but takes stack space */

    setup = (M117Setup *) obj->level_entry;
    if (setup != NULL) {
        cloud = obj->unk68;
        entry.objectID = 0xC3;
        entry.x = obj->trans.x_position;
        entry.y = obj->trans.y_position;
        entry.z = obj->trans.z_position;
        entry.unkA = setup->lightUnk14;
        entry.unkE = 0;
        entry.unk10 = 0;
        entry.unkC = setup->lightUnk15;
        entry.unk14 = 10;
        entry.unk15 = 50;
        entry.unk12 = setup->lightUnk17;
        entry.red = cloud->red;
        entry.green = cloud->green;
        entry.unk19 = 0x80;
        entry.blue = cloud->blue;
        entry.unk1C = 100;
        entry.unk1D = 100;
        entry.unk1E = 6;
        entry.unk1F = 0;
        entry.unk1A = setup->lightUnk16;
        entry.unk20 = obj->trans.rotation.x_rotation + 0x4000;
        entry.unk22 = obj->trans.rotation.y_rotation;
        light = func_80005F14(&entry, 1);
        cloud->light = light;
        if (light != NULL) {
            light->level_entry = NULL;
        }
    }
}

void func_m117_00154(Object *obj, M117Setup *setup) {
    M117Cloud *cloud;
    M117Mote *mote;
    s32 i;
    s32 moteRadius;

    obj->trans.rotation.y_rotation = setup->yaw << 8;
    cloud = obj->unk68;
    obj->trans.rotation.x_rotation = setup->pitch << 8;
    cloud->sizeX = setup->sizeX * 10;
    cloud->sizeY = setup->sizeY * 10;
    cloud->sizeZ = setup->sizeZ * 10;
    cloud->unk6 = setup->unk10 << 5;
    cloud->unk8 = setup->unk11 << 5;
    cloud->radius = setup->radius * 16;
    cloud->red = D_m117_00B80[setup->colour & 3][0];
    cloud->green = D_m117_00B80[setup->colour & 3][1];
    cloud->blue = D_m117_00B80[setup->colour & 3][2];
    cloud->brightness = 0;
    moteRadius = setup->moteRadius;
    mote = cloud->motes;
    i = 20;
    while (i--) {
        mote->angle = func_80048E2C(0, 0x10000);
        mote->phase = func_80048E2C(0, 0x10000);
        mote->radius = func_80048E2C(moteRadius, moteRadius * 2);
        mote->phaseSpeed = func_80048E2C(182, 364);
        mote->scale = func_80048E2C(75, 100) * 0.01f;
        mote->spin = func_80048E2C(-128, 127);
        mote->spinSpeed = func_80048E2C(4, 12);
        mote->roll = func_80048E2C(-128, 127);
        mote->rollSpeed = func_80048E2C(1, 8);
        mote++;
    }
    if (OBJ_LIGHT(obj) != NULL && *OBJ_LIGHT(obj) != 0) {
        func_8002130C(*OBJ_LIGHT(obj), cloud->red, cloud->green, cloud->blue);
    }
    func_80079D1C(obj, -1, 0, 8, 0, 0, 0, obj->trans.scale * 96.0f, obj->trans.scale * 96.0f, -1.0f, 1.0f, 1);
}

void func_m117_003F8(Object *obj, s32 updateRate) {
    M117Cloud *cloud;
    M117Mote *mote;
    M117LightData *data;
    s32 old;
    s32 inside;
    s32 brightness;
    s32 i;
    s32 j;
    f32 base;
    f32 range;
    Object *list[11];
    f32 swell;
    f32 r;

    cloud = obj->unk68;
    inside = func_8007E150(obj->trans.x_position, obj->trans.y_position, obj->trans.z_position, cloud->radius, 0,
                           list);
    brightness = cloud->brightness;
    old = brightness;
    if (inside) {
        brightness += updateRate * 4;
        if (brightness >= 256) {
            brightness = 255;
        }
        if (!cloud->soundOn) {
            func_80003410(0x210, obj->trans.x_position, obj->trans.y_position, obj->trans.z_position, 4, NULL);
            cloud->soundOn = 1;
        }
    } else {
        brightness -= updateRate * 8;
        if (brightness < 0) {
            brightness = 0;
        }
        if (cloud->soundOn) {
            func_80003410(0x211, obj->trans.x_position, obj->trans.y_position, obj->trans.z_position, 4, NULL);
            cloud->soundOn = 0;
        }
    }
    cloud->brightness = brightness;
    if (brightness != 0) {
        range = ((f32) cloud->sizeX - cloud->sizeZ) * 0.5f;
        base = cloud->sizeZ;
        mote = cloud->motes;
        i = 20;
        while (i--) {
            j = updateRate;
            while (j--) {
                mote->phase += mote->phaseSpeed;
                swell = func_80049C30(mote->phase) + 1.0f;
                r = (range * swell + base) * mote->scale;
                mote->spin += mote->spinSpeed;
                mote->roll += mote->rollSpeed;
                mote->angle += (s32) ((f32) (mote->radius << 16) * (3.0f - swell) / (6.283186f * r));
            }
            mote++;
        }
    }
    if (cloud->light == NULL) {
        spawnLight(obj);
    }
    if (cloud->light != NULL) {
        data = cloud->light->unk68;
        data->unkB8 = brightness / 255.0f;
        data->unkBC = brightness / 255.0f;
    }
    if (old != brightness && OBJ_LIGHT(obj) != NULL && *OBJ_LIGHT(obj) != 0) {
        func_80021358(*OBJ_LIGHT(obj), brightness);
    }
}

void func_m117_0070C(s32 arg0, s32 arg1, s32 arg2, Object *obj) {
    M117Cloud *cloud;
    M117Mote *mote;
    s32 *texture;
    s32 tex;
    s32 i;
    s32 j;
    u8 *ord;
    f32 *dep;
    Vec3f *pos;
    f32 y;
    f32 scale;
    f32 base;
    f32 range;
    f32 height;
    f32 fade;
    f32 swell;
    f32 r;
    f32 x;
    f32 tmp;
    u8 t;
    Vec3f positions[20];
    M117Billboard board;
    f32 depth[20];
    MtxF mtx;
    u8 order[20];

    cloud = obj->unk68;
    if (cloud->brightness != 0 && obj->header != NULL &&
        (texture = ((M117Header *) obj->header)->texture) != NULL) {
        y = obj->trans.y_position;
        tex = *texture;
        scale = obj->trans.scale;
        obj->trans.y_position = y + 8.0f;
        obj->trans.scale = 0.1f;
        func_80049DC0(obj, &mtx);
        obj->trans.scale = scale;
        obj->trans.y_position = y;
        base = cloud->sizeZ;
        fade = cloud->brightness / 255.0f;
        height = (cloud->sizeY * 2) * 0.5f;
        range = (cloud->sizeX - base) * 0.5f;
        pos = positions;
        mote = cloud->motes;
        dep = depth;
        ord = order;
        for (i = 0; i < 20; i++) {
            swell = func_80049C30(mote->phase) + 1.0f;
            r = (range * swell + base) * mote->scale;
            x = func_80049C34(mote->angle) * r * fade;
            func_80048B60(&mtx, x, (height * swell + 0.0f) * fade, func_80049C30(mote->angle) * r * fade, &pos->x,
                          &pos->y, &pos->z);
            *dep = func_800420F0(pos->x, pos->y, pos->z);
            *ord = i;
            dep++;
            ord++;
            mote++;
            pos++;
        }
        for (i = 19; i != 0; i--) {
            for (j = 0; j < i; j++) {
                if (depth[j + 1] < depth[j]) {
                    tmp = depth[j];
                    depth[j] = depth[j + 1];
                    depth[j + 1] = tmp;
                    t = order[j];
                    order[j] = order[j + 1];
                    order[j + 1] = t;
                }
            }
        }
        board.trans.rotation.y_rotation = 0;
        board.trans.rotation.x_rotation = 0;
        board.unk28 = 0.0f;
        func_8005600C(255, 255, 255, cloud->red, cloud->green, cloud->blue);
        for (ord = order; ord != order + 20; ord++) {
            mote = &cloud->motes[*ord];
            board.trans.x_position = positions[*ord].x;
            board.trans.y_position = positions[*ord].y;
            board.trans.z_position = positions[*ord].z;
            board.trans.rotation.z_rotation = mote->roll << 8;
            board.trans.scale = func_80049C34(mote->spin << 8) * 0.125f + 0.55f;
            func_80041174(arg0, arg1, arg2, &board, tex, 7, cloud->brightness);
        }
        func_80056064();
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m117", func_m117_00000);

INCLUDE_ASM("asm/nonmatchings/modules/m117", func_m117_00154);

INCLUDE_ASM("asm/nonmatchings/modules/m117", func_m117_003F8);

INCLUDE_ASM("asm/nonmatchings/modules/m117", func_m117_0070C);

INCLUDE_ASM("asm/nonmatchings/modules/m117", rodata);
#endif
