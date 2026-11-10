#include "common.h"

typedef struct SpawnEntry {
    /* 0x00 */ u8 pad0[4];
    /* 0x04 */ s16 x;
    /* 0x06 */ s16 y;
    /* 0x08 */ s16 z;
    /* 0x0A */ u8 padA[0x46];
    /* 0x50 */ u8 unk50;
    /* 0x54 */ s32 unk54;
} SpawnEntry;
extern SpawnEntry **D_800F6B04;
extern s32 D_800F6B08;
void *spawn_object(SpawnEntry *entry, s32 arg1);

/* JFG: spawn entry index at (x, y, z) (0x800285B0). */
void spawn_object_from_list(s16 index, s16 x, s16 y, s16 z, s32 arg4, s32 arg5) {
    SpawnEntry *entry;

    if (index >= D_800F6B08) {
        return;
    }
    entry = D_800F6B04[index];
    entry->x = x;
    entry->y = y;
    entry->z = z;
    entry->unk50 = arg4;
    entry->unk54 = arg5;
    spawn_object(entry, 1);
}

/* Size: 0x24 bytes */
typedef struct Emitter {
    u8 pad0[0x24];
} Emitter;
typedef struct EmitterObject {
    /* 0x00 */ u8 pad0[0x70];
    /* 0x70 */ Emitter *emitters;
} EmitterObject;
typedef struct EmitterHeader {
    /* 0x00 */ u8 pad0[0xC];
    /* 0x0C */ s16 *emitterIDs;
    /* 0x10 */ u8 pad10[0x20];
    /* 0x30 */ u8 emitterCount;
} EmitterHeader;
void emitter_init(Emitter *emitter, s32 behaviourID, s32 particleID);

/* JFG: set up the object's emitters (in the given buffer); returns the bytes used (0x8002863C). */
s32 obj_init_emitter(EmitterObject *obj, Emitter *emitters, EmitterHeader *header) {
    s32 i;

    obj->emitters = emitters;
    for (i = 0; i < header->emitterCount; i++) {
        emitter_init(&obj->emitters[i], (u8) (header->emitterIDs[i] >> 8), (u8) header->emitterIDs[i]);
    }
    return (header->emitterCount * sizeof(Emitter) + 3) & ~3;
}

/* JFG: an object header's light (217C0.c ObjectHeaderLight). Size: 0x18 bytes */
typedef struct LightHeader29 {
    /* 0x00 */ u8 unk0;
    /* 0x01 */ u8 unk1;
    /* 0x02 */ u8 unk2;
    /* 0x03 */ s8 unk3;
    /* 0x04 */ u16 unk4;
    /* 0x06 */ s16 homeX;
    /* 0x08 */ s16 homeY;
    /* 0x0A */ s16 homeZ;
    /* 0x0C */ u16 radius;
    /* 0x0E */ u16 unkE;
    /* 0x10 */ u8 colourR;
    /* 0x11 */ u8 colourG;
    /* 0x12 */ u8 colourB;
    /* 0x13 */ u8 intensity;
    /* 0x14 */ s16 colourCycleID;
    /* 0x16 */ u8 unk16;
    /* 0x17 */ u8 unk17;
} LightHeader29;

/* JFG: an object header's corona (245D0.c CoronaHeader). Size: 0x20 bytes */
typedef struct CoronaHeader29 {
    /* 0x00 */ u8 active;
    /* 0x01 */ u8 size;
    /* 0x02 */ s16 rotation[3];
    /* 0x08 */ f32 offset[3];
    /* 0x14 */ u8 colour[4];
    /* 0x18 */ s16 spriteID;
    /* 0x1A */ s16 unk1A;
    /* 0x1C */ s16 unk1C;
    /* 0x1E */ s8 attachPoint;
    /* 0x1F */ u8 visible;
} CoronaHeader29;

/* JFG: an object entry an explosion spawns (0x22 bytes) */
typedef struct ExplosionSpawn {
    /* 0x00 */ u8 pad0[4];
    /* 0x04 */ s16 x;
    /* 0x06 */ s16 y;
    /* 0x08 */ s16 z;
    /* 0x0A */ u8 padA[0x22 - 0xA];
} ExplosionSpawn;

/* JFG: an object entry an explosion spawns once, at its time (0x1C bytes) */
typedef struct ExplosionEvent {
    /* 0x00 */ u8 pad0[4];
    /* 0x04 */ s16 x;
    /* 0x06 */ s16 y;
    /* 0x08 */ s16 z;
    /* 0x0A */ s16 time;
    /* 0x0C */ u8 padC[0x1C - 0xC];
} ExplosionEvent;

/* JFG: the level entry of a light object (also the explosions) */
typedef struct LevelObjectEntry_LightObj {
    /* 0x00 */ u8 pad0[0x10];
    /* 0x10 */ s16 *triggerStart; // per particle trigger: the time span it runs
    /* 0x14 */ s16 *triggerEnd;
    /* 0x18 */ ExplosionSpawn *spawns; // repeated spawns: the time span and the interval
    /* 0x1C */ s16 *spawnEnd;
    /* 0x20 */ u8 *spawnStart;
    /* 0x24 */ u8 *spawnInterval;
    /* 0x28 */ ExplosionEvent *events;
    /* 0x2C */ u8 pad2C[4];
    /* 0x30 */ u8 numTriggers;
    /* 0x31 */ u8 numFlags; // the number of spawns
    /* 0x32 */ u8 numEvents;
    /* 0x33 */ u8 pad33;
    /* 0x34 */ s16 endTime;
    /* 0x36 */ s16 lightRadius;
    /* 0x38 */ s16 lightColourCycle;
    /* 0x3A */ s16 coronaSize;
    /* 0x3C */ s16 coronaColourCycle;
    /* 0x3E */ s16 shakeTimes[3]; // the camera shake: the rate after each of these times
    /* 0x44 */ s16 shakeRates[3];
    /* 0x4A */ s16 zoneSize;
    /* 0x4C */ u8 zoneValue;
    /* 0x4D */ u8 pad4D;
    /* 0x4E */ s16 zoneTime;
    /* 0x50 */ s8 zoneUnk1F;
    /* 0x51 */ u8 pad51[3];
    /* 0x54 */ s32 zoneUnk20;
} LevelObjectEntry_LightObj;

/* JFG: a colour cycle (0x10 bytes) */
typedef struct ColourCycle29 {
    /* 0x00 */ s32 index;
    /* 0x04 */ s32 unk4;
    /* 0x08 */ u8 colour[4]; // set by the colour cycle
    /* 0x0C */ s32 *table;   // table[0]: the number of steps
} ColourCycle29;

/* JFG: the state of a light object */
typedef struct LightObjState {
    /* 0x00 */ s16 unk0;
    /* 0x02 */ u8 pad2[2];
    /* 0x04 */ f32 unk4;
    /* 0x08 */ void *light;
    /* 0x0C */ void *corona;
    /* 0x10 */ ColourCycle29 cycle;
    /* 0x20 */ LevelObjectEntry_LightObj *entry;
    /* 0x24 */ u8 *flags;
    /* 0x28 */ u8 pad28[8];
    /* 0x30 */ u8 flagBuffer[1];
} LightObjState;

/* JFG: an object, reduced to what obj_init_light_object uses */
typedef struct LightObj {
    /* 0x00 */ u8 pad0[6];
    /* 0x06 */ s16 flags;
    /* 0x08 */ u8 pad8[4];
    /* 0x0C */ f32 x;
    /* 0x10 */ f32 y;
    /* 0x14 */ f32 z;
    /* 0x18 */ u8 pad18[0x3C - 0x18];
    /* 0x3C */ void *unk3C;
    /* 0x40 */ u8 pad40[0x68 - 0x40];
    /* 0x68 */ LightObjState *state;
    /* 0x6C */ u8 pad6C[0x84 - 0x6C];
    /* 0x84 */ s32 triggers; // the particle triggers to run
} LightObj;

extern f32 D_800ABB3C;
void colour_cycle_init(void *cycle, s32 assetID);
void *light_add_from_object_header(LightObj *obj, LightHeader29 *header);
void *corona_add(LightObj *owner, CoronaHeader29 *header);
void func_80029BA4(f32 x, f32 y, f32 z, f32 size, f32 value, f32 time, s32 arg6, s32 arg7);

/* JFG: init a light object from its level entry: a light, a corona (with a colour cycle) and a zone,
   each if the entry has one. */
void obj_init_light_object(LightObj *obj, LevelObjectEntry_LightObj *entry) {
    LightObjState *state;
    s32 i;
    LightHeader29 lightHeader;
    CoronaHeader29 coronaHeader;

    state = obj->state;
    state->corona = NULL;
    state->light = NULL;
    state->flags = state->flagBuffer;
    state->unk0 = 0;
    state->entry = entry;
    state->unk4 = 0;
    obj->flags |= 0x400;
    obj->unk3C = NULL;
    if (entry->shakeTimes[0] == 0) {
        state->unk4 = entry->shakeTimes[0] * D_800ABB3C;
    }
    for (i = 0; i < entry->numFlags; i++) {
        state->flags[i] = 0;
    }
    if (entry->lightRadius) {
        lightHeader.unk0 = 1;
        lightHeader.unk1 = 3;
        lightHeader.unk2 = 0x21;
        lightHeader.unk3 = -1;
        lightHeader.unk4 = 0;
        lightHeader.homeX = 0;
        lightHeader.homeY = 0;
        lightHeader.homeZ = 0;
        lightHeader.intensity = 255;
        lightHeader.radius = entry->lightRadius;
        lightHeader.unkE = entry->lightRadius;
        lightHeader.colourCycleID = entry->lightColourCycle;
        lightHeader.unk16 = 0;
        lightHeader.unk17 = 0;
        state->light = light_add_from_object_header(obj, &lightHeader);
    }
    if (entry->coronaSize) {
        colour_cycle_init(&state->cycle, entry->coronaColourCycle);
        coronaHeader.active = 1;
        coronaHeader.spriteID = -1;
        coronaHeader.size = 0;
        coronaHeader.rotation[0] = 0;
        coronaHeader.rotation[1] = 0;
        coronaHeader.rotation[2] = 0;
        coronaHeader.offset[0] = 0.0f;
        coronaHeader.offset[1] = 0.0f;
        coronaHeader.offset[2] = 0.0f;
        coronaHeader.colour[0] = state->cycle.colour[0];
        coronaHeader.colour[1] = state->cycle.colour[1];
        coronaHeader.colour[2] = state->cycle.colour[2];
        coronaHeader.colour[3] = state->cycle.colour[3];
        coronaHeader.unk1A = entry->coronaSize;
        coronaHeader.unk1C = entry->coronaSize;
        coronaHeader.attachPoint = -1;
        state->corona = corona_add(obj, &coronaHeader);
    }
    if (entry->zoneSize && entry->zoneTime && entry->zoneValue) {
        func_80029BA4(obj->x, obj->y, obj->z, entry->zoneSize, entry->zoneValue, entry->zoneTime / 60.0f,
                      entry->zoneUnk1F, entry->zoneUnk20);
    }
}

/* JFG: what an explosion updates in its corona and light */
typedef struct ExplosionCorona {
    /* 0x00 */ u8 pad0[0x38];
    /* 0x38 */ u8 colour[4];
} ExplosionCorona;
typedef struct ExplosionLight {
    /* 0x00 */ u8 pad0[0x48];
    /* 0x48 */ ColourCycle29 cycle;
} ExplosionLight;

extern f32 D_800ABB40;
void obj_spawn_particle(LightObj *obj, s32 updateRate);
void update_colour_cycle(void *cycle, s32 updateRate);
void corona_free(void *corona);
void light_remove(void *light);
void cam_shake_on(void);
void cam_shake_off(void);
void cam_shake_near_point(f32 x, f32 y, f32 z, f32 radius, f32 strength);
s32 rand_range(s32 min, s32 max);
void free_object(void *obj);

/* JFG: an explosion's update: the particle triggers, repeated and one-off spawns at its position, the camera
 * shake by the time, the corona and the light fading by their colour cycles; at the end time everything goes */
void obj_explosion_update(LightObj *obj, s32 updateRate) {
    LightObjState *state;
    s16 prev;
    UNUSED s32 pad;
    ColourCycle29 *lightCycle;
    LevelObjectEntry_LightObj *entry;
    s32 i;
    ExplosionEvent *event;
    ExplosionSpawn *spawn;
    ExplosionCorona *corona;
    ExplosionLight *light;
    ColourCycle29 *cycle;

    state = obj->state;
    entry = state->entry;
    prev = state->unk0;
    state->unk0 += updateRate;
    obj->triggers = 0;
    for (i = 0; i < entry->numTriggers; i++) {
        if (state->unk0 >= entry->triggerStart[i] && state->unk0 < entry->triggerEnd[i]) {
            obj->triggers |= 1 << i;
        }
    }
    obj_spawn_particle(obj, updateRate);
    for (i = 0; i < entry->numFlags; i++) {
        if (state->unk0 >= entry->spawnStart[i] && state->unk0 < entry->spawnEnd[i]) {
            if (state->flags[i] < updateRate) {
                spawn = &entry->spawns[i];
                spawn->x = obj->x;
                spawn->y = obj->y;
                spawn->z = obj->z;
                state->flags[i] = entry->spawnInterval[i] + state->flags[i] - updateRate;
                spawn_object((SpawnEntry *) spawn, 1);
            } else {
                state->flags[i] -= updateRate;
            }
        }
    }
    event = entry->events;
    for (i = 0; i < entry->numEvents; i++, event++) {
        if (event->time >= prev && state->unk0 >= event->time) {
            event->x = obj->x;
            event->y = obj->y;
            event->z = obj->z;
            spawn_object((SpawnEntry *) event, 1);
        }
    }
    for (i = 0; entry->shakeTimes[i] < state->unk0 && i != 2; i++) {
    }
    state->unk4 += (f32) updateRate * (f32) entry->shakeRates[i] * D_800ABB40;
    corona = state->corona;
    cycle = &state->cycle;
    if (corona != NULL) {
        update_colour_cycle(cycle, updateRate);
        if (cycle->index < *cycle->table - 1) {
            corona->colour[0] = cycle->colour[0];
            corona->colour[1] = cycle->colour[1];
            corona->colour[2] = cycle->colour[2];
            corona->colour[3] = cycle->colour[3];
        } else {
            corona_free(state->corona);
            state->corona = NULL;
        }
    }
    if (state->light != NULL) {
        update_colour_cycle(&state->cycle, updateRate);
        cycle = &((ExplosionLight *) state->light)->cycle;
        // fakematch: the copy sets the order of the two loads' registers
        if (*cycle->table == (lightCycle = cycle)->index + 1) {
            light_remove(state->light);
            state->light = NULL;
        }
    }
    if (state->unk0 >= entry->endTime) {
        cam_shake_on();
        if (state->light != NULL) {
            light_remove(state->light);
        }
        if (state->corona != NULL) {
            corona_free(state->corona);
        }
        free_object(obj);
    } else if (state->unk4 > 2.0f) {
        cam_shake_off();
        cam_shake_near_point(obj->x, obj->y, obj->z, 1000.0f, rand_range(0, state->unk4));
    }
}

/* JFG: the level entry of object 0x73 (0x22 bytes) */
typedef struct LevelObjectEntry73 {
    /* 0x00 */ s16 objectID;
    /* 0x02 */ u8 size;
    /* 0x03 */ u8 pad3;
    /* 0x04 */ s16 x;
    /* 0x06 */ s16 y;
    /* 0x08 */ s16 z;
    /* 0x0A */ u8 padA[2];
    /* 0x0C */ s16 unkC;
    /* 0x0E */ s16 unkE;
    /* 0x10 */ s16 unk10;
    /* 0x12 */ s16 unk12;
    /* 0x14 */ u8 pad14[4];
    /* 0x18 */ s16 unk18;
    /* 0x1A */ u8 pad1A[2];
    /* 0x1C */ s16 unk1C;
    /* 0x1E */ u8 pad1E[4];
} LevelObjectEntry73;

extern f32 D_800ABB44;

#ifdef NON_MATCHING
/* NON_MATCHING: the target loads D_800ABB44 before moving `a` to $f12 (the first two instructions swap) */
/* JFG: spawn an object 0x73 at (x, y, z); the four values are scaled by D_800ABB44. */
void func_80028E7C(s16 x, s16 y, s16 z, f32 a, f32 b, f32 c, f32 d, s16 arg7, u8 arg8) {
    LevelObjectEntry73 entry;

    entry.size = sizeof(LevelObjectEntry73);
    entry.objectID = 0x73;
    entry.unkC = a * D_800ABB44;
    entry.unkE = b * D_800ABB44;
    entry.unk10 = c * D_800ABB44;
    entry.unk12 = d * D_800ABB44;
    entry.x = x;
    entry.unk18 = arg7;
    entry.unk1C = arg8;
    entry.y = y;
    entry.z = z;
    spawn_object((SpawnEntry *) &entry, 1);
}
#else
INCLUDE_ASM("asm/nonmatchings/291B0", func_80028E7C);
#endif

/* JFG: a 10 byte coloured vertex */
typedef struct ColVertex {
    /* 0x00 */ s16 x;
    /* 0x02 */ s16 y;
    /* 0x04 */ s16 z;
    /* 0x06 */ u8 r;
    /* 0x07 */ u8 g;
    /* 0x08 */ u8 b;
    /* 0x09 */ u8 a;
} ColVertex;

extern u8 D_800A11B0;
extern f32 D_800F6B10[20];
extern f32 D_800ABB48;
f32 func_80049C34(s32 angle); /* sine of a 16-bit angle */
f32 func_80049C30(s32 angle); /* cosine of a 16-bit angle */
void *load_texture(s32 id);
void colour_cycle_init(void *cycle, s32 assetID);

/* JFG: init: (once) the sine/cosine table of 10 angles, then the 40 vertices (white, y 0), the sizes from the
 * entry, the texture and two colour cycles. */
void obj_init_col_vertices(u8 *obj, u8 *entry) {
    u8 *state;
    s32 i;
    f32 scale;

    if (D_800A11B0) {
        f32 *table = D_800F6B10;
        s32 angle;

        for (i = 0, angle = 0; i < 10; i++, angle += 0x1999) {
            *table++ = func_80049C34(angle);
            *table++ = func_80049C30(angle);
        }
        D_800A11B0 = 0;
    }
    state = *(u8 **) (obj + 0x68);
    for (i = 0; i < 40; i++) {
        ((ColVertex *) state)[i].y = 0;
        ((ColVertex *) state)[i].r = 255;
        ((ColVertex *) state)[i].g = 255;
        ((ColVertex *) state)[i].b = 255;
        ((ColVertex *) state)[i].a = 255;
    }
    scale = D_800ABB48;
    *(s16 *) (obj + 2) = *(s16 *) (entry + 0xA);
    *(f32 *) (state + 0x198) = *(s16 *) (entry + 0xE) * scale;
    *(f32 *) (state + 0x194) = *(s16 *) (entry + 0xC) * scale;
    *(f32 *) (state + 0x19C) = *(s16 *) (entry + 0x10) * scale;
    *(f32 *) (state + 0x1A0) = *(s16 *) (entry + 0x12) * scale;
    *(f32 *) (state + 0x1A4) = *(s16 *) (entry + 0x14) * scale;
    *(f32 *) (state + 0x1A8) = *(s16 *) (entry + 0x16) * scale;
    *(s16 *) (state + 0x1D8) = *(s16 *) (entry + 0x18);
    *(u16 *) (state + 0x1D6) = *(u16 *) (entry + 0x1C);
    *(void **) (state + 0x190) = load_texture(*(s16 *) (entry + 0x1A));
    state[0x1DA] = 0;
    *(s16 *) (state + 0x1D4) = entry[0x20] << 8;
    *(s32 *) (state + 0x1CC) = 0;
    *(s32 *) (state + 0x1D0) = 0;
    colour_cycle_init(state + 0x1AC, entry[0x1E]);
    colour_cycle_init(state + 0x1BC, entry[0x1F]);
    *(s32 *) (obj + 0x3C) = 0;
}

INCLUDE_ASM("asm/nonmatchings/291B0", func_800291BC);

void *cam_get_active_camera(void);
void mtx_cam_push(Gfx **dList, Mtx **mtx, void *trans, f32 scaleY, f32 offsetY);
void func_80055C00(Gfx **dList, void *model, s32 flags, s32 alpha);
void func_80041DC0(Gfx **dList);
extern u8 D_F2D58[];
extern u8 D_F2E58[];

/* JFG: a 10 byte vertex of the 0x68 block's two buffers */
typedef struct Vertex10 {
    /* 0x00 */ u8 pad0[10];
} Vertex10;

#ifdef NON_MATCHING
/* NON_MATCHING: same code and frame; the target's temporaries are numbered one higher from the first command
   on (and the colour bytes load in another order) */
/* JFG: draw the object (its 0x68 block): face the camera when 0x2 is set, colours from the block, the model at
 * 0x190, then its two 20-vertex buffers (the current one at 0x1DA) with the textures at D_F2D58/D_F2E58. */
void func_80029688(Gfx **dList, Mtx **mtx, u8 *obj) {
    Gfx *gfx;
    Mtx *mats;
    u8 *state;
    void *cam;

    gfx = *dList;
    mats = *mtx;
    gDPPipeSync(gfx++);
    if (*(s16 *) (obj + 2) != 0) {
        cam = cam_get_active_camera();
        *(s16 *) obj = -*(s16 *) cam;
    }
    state = *(u8 **) (obj + 0x68);
    if (*(s16 *) (*(u8 **) (state + 0x190) + 6) & 0x40) {
        gDPSetEnvColor(gfx++, state[0x1B4], state[0x1B5], state[0x1B6], *(s32 *) (state + 0x1D0));
    } else {
        gDPSetEnvColor(gfx++, state[0x1B4], state[0x1B5], state[0x1B6], 255);
    }
    gDPSetPrimColor(gfx++, 0, 0, state[0x1C4], state[0x1C5], state[0x1C6], *(u16 *) (state + 0x1D4) >> 8);
    mtx_cam_push(&gfx, &mats, obj, 1.0f, 0.0f);
    func_80055C00(&gfx, *(void **) (state + 0x190), 0x20E, *(s32 *) (state + 0x1D0) << 8);
    {
        Gfx *_g = gfx++;
        _g->words.w0 =
            ((((OS_K0_TO_PHYSICAL(&((Vertex10(*)[20]) state)[state[0x1DA]]) & 6) | 0xA0) & 0xFF) << 16) | 0x040000D0;
        _g->words.w1 = OS_K0_TO_PHYSICAL(&((Vertex10(*)[20]) state)[state[0x1DA]]);
    }
    {
        Gfx *_g = gfx++;
        _g->words.w1 = (u32) D_F2D58;
        _g->words.w0 = 0x05F10100;
    }
    {
        Gfx *_g = gfx++;
        _g->words.w1 = (u32) D_F2E58;
        _g->words.w0 = 0x05310040;
    }
    func_80041DC0(&gfx);
    gDPSetEnvColor(gfx++, 255, 255, 255, 255);
    gDPSetPrimColor(gfx++, 0, 0, 255, 255, 255, 255);
    *dList = gfx;
    *mtx = mats;
}
#else
INCLUDE_ASM("asm/nonmatchings/291B0", func_80029688);
#endif

/* A sprite object with two colour cycles, set up from its level entry */
typedef struct ChromaSpriteState {
    /* 0x00 */ s16 x;
    /* 0x02 */ s16 y;
    /* 0x04 */ s16 z;
    /* 0x06 */ s16 unk6;
    /* 0x08 */ u8 unk8;
    /* 0x09 */ u8 unk9;
    /* 0x0A */ u8 padA[6];
    /* 0x10 */ u8 colourCycle1[0x10];
    /* 0x20 */ u8 colourCycle2[0x10];
    /* 0x30 */ void *entry;
} ChromaSpriteState;
typedef struct ChromaSpriteObject {
    /* 0x00 */ u8 pad0[4];
    /* 0x04 */ s16 unk4;
    /* 0x06 */ u8 pad6[2];
    /* 0x08 */ f32 scale;
    /* 0x0C */ u8 padC[0x28 - 0xC];
    /* 0x28 */ f32 animFrame;
    /* 0x2C */ u8 pad2C[0x39 - 0x2C];
    /* 0x39 */ u8 opacity;
    /* 0x3A */ u8 pad3A[2];
    /* 0x3C */ s32 unk3C;
    /* 0x40 */ u8 pad40[0x28];
    /* 0x68 */ ChromaSpriteState *data;
    /* 0x6C */ void **sprite;
} ChromaSpriteObject;
typedef struct ChromaSpriteEntry {
    /* 0x00 */ u8 pad0[4];
    /* 0x04 */ s16 x;
    /* 0x06 */ s16 y;
    /* 0x08 */ s16 z;
    /* 0x0A */ u8 padA[2];
    /* 0x0C */ s16 lifetime;
    /* 0x0E */ s16 spriteID;
    /* 0x10 */ u8 pad10[2];
    /* 0x12 */ u16 scale;
    /* 0x14 */ u8 fadeOut;
    /* 0x15 */ u8 fadeIn;
    /* 0x16 */ u8 unk16;
    /* 0x17 */ u8 colourCycle1;
    /* 0x18 */ u8 colourCycle2;
    /* 0x19 */ u8 pad19;
    /* 0x1A */ u16 spin;
} ChromaSpriteEntry;
void sprite_free(void *sprite);
void *tex_load_sprite(s32 spriteID, s32 arg1);
void colour_cycle_init(void *cycle, s32 assetID);
extern f32 D_800ABB4C;

/* JFG: (re)set a sprite object from its entry. */
void func_8002993C(ChromaSpriteObject *obj, ChromaSpriteEntry *entry) {
    ChromaSpriteState *data = obj->data;

    sprite_free(*obj->sprite);
    *obj->sprite = tex_load_sprite(entry->spriteID, 0);
    obj->scale = entry->scale * D_800ABB4C;
    data->x = entry->x;
    data->y = entry->y;
    data->z = entry->z;
    data->unk8 = 0;
    data->unk6 = 0;
    data->unk9 = entry->unk16;
    data->entry = entry;
    obj->unk3C = 0;
    colour_cycle_init(data->colourCycle1, entry->colourCycle1);
    colour_cycle_init(data->colourCycle2, entry->colourCycle2);
}

void free_object(void *obj);
void update_colour_cycle(void *cycle, s32 updateRate);
void sprite_anim_advance(void *sprite, s32 *flags, s32 arg2, f32 *frame, s32 updateRate); // animate a sprite

/* JFG: the sprite object's loop: fade in, then after its lifetime fade out and free it; spin and animate. */
void obj_loop_chroma_sprite(ChromaSpriteObject *obj, s32 updateRate) {
    ChromaSpriteState *data;
    ChromaSpriteEntry *entry;
    void *sprite;
    s32 flags;
    u16 opacity;
    s32 fade;

    flags = 1;
    data = obj->data;
    sprite = *obj->sprite;
    entry = (ChromaSpriteEntry *) data->entry;
    if (sprite == NULL) {
        free_object(obj);
        return;
    }
    data->unk6 += updateRate;
    if (data->unk6 >= entry->lifetime) {
        fade = updateRate;
        fade *= entry->fadeOut;
        if (fade < data->unk8) {
            data->unk8 -= fade;
        } else {
            free_object(obj);
            return;
        }
    } else if (data->unk8 < 0xFF) {
        opacity = data->unk8 + updateRate * entry->fadeIn;
        if (opacity >= 0x100) {
            opacity = 0xFF;
        }
        data->unk8 = opacity;
    }
    obj->unk4 += updateRate * entry->spin;
    obj->opacity = data->unk8;
    update_colour_cycle(data->colourCycle1, updateRate);
    update_colour_cycle(data->colourCycle2, updateRate);
    sprite_anim_advance(sprite, &flags, data->unk9, &obj->animFrame, updateRate);
}

/* A sphere (centre and radius) in a fixed table of 16. Size: 0x24 bytes */
typedef struct Zone {
    /* 0x00 */ f32 x;
    /* 0x04 */ f32 y;
    /* 0x08 */ f32 z;
    /* 0x0C */ f32 size;
    /* 0x10 */ f32 unk10;
    /* 0x14 */ f32 value;
    /* 0x18 */ f32 unk18;
    /* 0x1C */ s16 unk1C; // frames left (0: unused)
    /* 0x1E */ u8 unk1E;
    /* 0x1F */ u8 unk1F;
    /* 0x20 */ s32 unk20;
} Zone;
extern Zone D_800F6B60[16];
extern s32 D_800A11B4;

/* JFG: mark all 16 zones unused and zero D_800A11B4 (the zone test is skipped while it is zero). */
void func_80029B78(void) {
    Zone *zone = D_800F6B60;
    s32 i = 16;
    while (i--) {
        zone->unk1C = 0;
        zone++;
    }
    D_800A11B4 = 0;
}

#ifdef NON_MATCHING
/* NON_MATCHING: the target keeps the `i != 0` guards of the constant-count loops (and other registers) */
/* JFG: add a zone: the first unused one, or (all 16 in use) the one with the smallest value. */
void func_80029BA4(f32 x, f32 y, f32 z, f32 size, f32 value, f32 time, s32 arg6, s32 arg7) {
    Zone *zone = D_800F6B60;
    Zone *other;
    s32 i;

    if (D_800A11B4 < 16) {
        i = 15;
        while (i-- && zone->unk1C != 0) {
            zone++;
        }
        D_800A11B4++;
    } else {
        other = &D_800F6B60[1];
        i = 15;
        while (i--) {
            if (other->value < zone->value) {
                zone = other;
            }
            other++;
        }
    }
    zone->x = x;
    zone->y = y;
    zone->z = z;
    zone->size = size;
    zone->unk10 = 16384.0f / size;
    zone->unk1E = 1;
    zone->value = value;
    zone->unk1C = time * 60.0f;
    zone->unk18 = value / zone->unk1C;
    zone->unk1F = arg6;
    zone->unk20 = arg7;
}
#else
INCLUDE_ASM("asm/nonmatchings/291B0", func_80029BA4);
#endif

INCLUDE_ASM("asm/nonmatchings/291B0", func_80029CFC);

/* JFG: TRUE if a sphere of the given radius at the point touches a zone in use (the zone's own size is left out with ignoreSize). */
s32 zone_sphere_touches(f32 x, f32 y, f32 z, f32 radius, s32 ignoreSize) {
    Zone *zone;
    s32 i;
    s32 hit = FALSE;
    f32 dx, dy, dz;
    f32 r;

    if (D_800A11B4 != 0) {
        zone = D_800F6B60;
        i = 16;
        while (i--) {
            if (zone->unk1C != 0) {
                dx = zone->x - x;
                dy = zone->y - y;
                dz = zone->z - z;
                r = radius;
                if (!ignoreSize) {
                    r += zone->size;
                }
                r *= r;
                if (dx * dx + dy * dy + dz * dz < r) {
                    i = 0;
                    hit = TRUE;
                }
            }
            zone++;
        }
    }
    return hit;
}

f32 func_80049C30(s32 angle); // cosine of a 16-bit angle
f32 sqrtf(f32 x);
extern f32 D_800ABD08;

typedef struct WaveObject {
    /* 0x00 */ u8 pad0[0x10];
    /* 0x10 */ f32 unk10; // angle speed
    /* 0x14 */ f32 unk14; // amplitude
} WaveObject;

/* JFG: scale (x, y, z) to a length of at least 1 times the object's wave value at time t; y is kept at least
 * 1.5 away from 0. */
void wave_vector_scale(WaveObject *obj, f32 t, f32 x, f32 y, f32 z, f32 *ox, f32 *oy, f32 *oz) {
    f32 scale;
    f32 k;

    scale = x * x + y * y + z * z;
    if (scale != 0.0f) {
        scale = 1.0f / sqrtf(scale);
        k = func_80049C30(obj->unk10 * t) * (D_800ABD08 * obj->unk14);
        if (k < 1.0f) {
            k = 1.0f;
        }
        scale *= k;
    }
    *ox = x * scale;
    *oy = y * scale;
    *oz = z * scale;
    if (*oy >= 0.0f) {
        if (*oy < 1.5f) {
            *oy = 1.5f;
        }
    } else if (*oy > -1.5f) {
        *oy = -1.5f;
    }
}

/* JFG: a collision sphere of an object (0x34 bytes) */
typedef struct HitSphere {
    /* 0x00 */ u8 pad0[0xC];
    /* 0x0C */ f32 x;
    /* 0x10 */ f32 y;
    /* 0x14 */ f32 z;
    /* 0x18 */ u8 pad18[0xC];
    /* 0x24 */ f32 radius;
    /* 0x28 */ u8 pad28[0xC];
} HitSphere;

typedef struct SphereObject {
    /* 0x00 */ u8 pad0[0xC];
    /* 0x0C */ f32 x;
    /* 0x10 */ f32 y;
    /* 0x14 */ f32 z;
    /* 0x18 */ u8 pad18[0xC];
    /* 0x24 */ f32 radius;
    /* 0x28 */ u8 pad28[0x32 - 0x28];
    /* 0x32 */ s16 numSpheres;
    /* 0x34 */ u8 pad34[0xC];
    /* 0x40 */ HitSphere *spheres;
} SphereObject;

f32 sqrtf(f32 x);

/* JFG: the distance from pos to the nearest surface of the object's spheres (the object itself when it has
 * none), at most maxDist and at least 0. */
f32 sphere_distance_min(f32 *pos, SphereObject *obj, f32 maxDist) {
    s32 n;
    HitSphere *sphere;
    f32 x, y, z, r;

    if (obj->numSpheres == 0) {
        n = 1;
        sphere = NULL;
    } else {
        sphere = obj->spheres;
        n = obj->numSpheres;
    }
    while (n--) {
        if (sphere == NULL) {
            x = obj->x;
            y = obj->y;
            z = obj->z;
            r = obj->radius;
        } else {
            x = sphere->x;
            y = sphere->y;
            z = sphere->z;
            r = sphere->radius;
            sphere++;
        }
        x -= pos[0];
        y -= pos[1];
        z -= pos[2];
        r = sqrtf(x * x + y * y + z * z) - r;
        if (r < maxDist) {
            maxDist = r;
        }
    }
    if (maxDist < 0.0f) {
        maxDist = 0.0f;
    }
    return maxDist;
}

/* JFG: a hit part of an object (0x34 bytes; HitSphere viewed for damage) */
typedef struct HitPart {
    /* 0x00 */ u8 pad0[0x2C];
    /* 0x2C */ s16 damage;
    /* 0x2E */ u8 pad2E;
    /* 0x2F */ u8 percent;
    /* 0x30 */ u32 unk31 : 1;
    /* 0x30 */ u32 unk30 : 1;
    /* 0x30 */ u32 unk29 : 1;
    /* 0x30 */ u32 unk0 : 29;
} HitPart;

typedef struct DamageObject {
    /* 0x00 */ u8 pad0[0x6];
    /* 0x06 */ s16 health;
    /* 0x08 */ u32 unk31 : 1;
    /* 0x08 */ u32 unk30 : 1;
    /* 0x08 */ u32 unk0 : 30;
    /* 0x0C */ u8 padC[0x2C - 0xC];
    /* 0x2C */ s16 damage;
    /* 0x2E */ u8 pad2E;
    /* 0x2F */ u8 percent;
    /* 0x30 */ u8 pad30;
    /* 0x31 */ s8 lastPart;
    /* 0x32 */ s16 numParts;
    /* 0x34 */ u8 pad34[0xC];
    /* 0x40 */ HitPart *parts;
} DamageObject;

/* JFG: applies damage to the object, or shared out over its hit parts (scaled by each one's percent);
 * returns the damage dealt. */
s32 obj_apply_damage(DamageObject *obj, s32 damage) {
    s32 total;
    s32 n;
    s32 share;
    s32 dealt;
    HitPart *part;

    if (damage <= 0) {
        return 0;
    }
    total = 0;
    if (obj->numParts == 0) {
        if (!obj->unk30) {
            if (obj->percent != 100) {
                damage = (obj->percent * damage * 164) >> 14;
            }
            total = damage;
            obj->damage += damage;
            obj->health -= damage;
        }
    } else {
        part = obj->parts;
        share = damage / obj->numParts;
        n = obj->numParts;
        while (n--) {
            if (n == 0) {
                share = damage;
            }
            if (!part->unk29 && !part->unk30) {
                dealt = share;
                if (part->percent != 100) {
                    dealt = (part->percent * share * 164) >> 14;
                }
                total += dealt;
                obj->lastPart = part - obj->parts;
                damage -= share;
                part->damage += dealt;
                obj->health -= dealt;
            }
            part++;
        }
        if (total != 0) {
            obj->health -= damage;
            total += damage;
        }
    }
    return total;
}

#ifdef NON_MATCHING
/* NON_MATCHING: same code and frame, but register choice: the target keeps the model in $t0 (spilled to 0x24) and
   the sphere in $a1, mine gives the model $a1 and the sphere $a2 (so it homes a1 at entry); 41 words */
f32 func_80049C34(s32 angle); // sine of a 16-bit angle

/* JFG: an object model with a bounding box */
typedef struct BoxModel {
    /* 0x00 */ u8 pad0[0x8C];
    /* 0x8C */ s16 bounds[6]; // min x, y, z, max x, y, z
} BoxModel;

typedef struct BoxObject {
    /* 0x00 */ s16 yaw;
    /* 0x02 */ u8 pad2[6];
    /* 0x08 */ f32 scale;
    /* 0x0C */ f32 x;
    /* 0x10 */ f32 y;
    /* 0x14 */ f32 z;
    /* 0x18 */ u8 pad18[0x3A - 0x18];
    /* 0x3A */ s8 modelIndex;
    /* 0x3B */ u8 pad3B[0x6C - 0x3B];
    /* 0x6C */ BoxModel **models;
} BoxObject;

/* JFG: whether a sphere (x, y, z, radius) touches the object's current model's bounding box (turned by its yaw,
 * scaled); the squared distance outside the box goes to distSq (0 inside). */
s32 func_8002AC3C(BoxObject *obj, f32 *sphere, f32 *distSq) {
    s32 outside;
    f32 lx;
    f32 ly;
    f32 lz;
    f32 dx;
    f32 dy;
    f32 dz;
    f32 lo;
    f32 hi;
    f32 d;
    f32 scale;
    f32 s;
    f32 c;
    s16 *bounds;
    BoxModel *model;

    if (obj->models[obj->modelIndex] == NULL) {
        return 0;
    }
    model = obj->models[obj->modelIndex];
    scale = obj->scale;
    outside = 0;
    s = func_80049C34(-obj->yaw);
    c = func_80049C30(-obj->yaw);
    dx = sphere[0] - obj->x;
    dz = sphere[2] - obj->z;
    bounds = model->bounds;
    lx = dx * c + dz * s;
    ly = sphere[1] - obj->y;
    lz = dz * c - dx * s;
    dx = 0.0f;
    lo = bounds[0] * scale;
    hi = bounds[3] * scale;
    if (lx < lo) {
        dx = lo - lx;
        outside = 1;
    } else if (hi < lx) {
        dx = lx - hi;
        outside = 1;
    }
    dy = 0.0f;
    lo = bounds[1] * scale;
    hi = bounds[4] * scale;
    if (ly < lo) {
        dy = lo - ly;
        outside = 1;
    } else if (hi < ly) {
        dy = ly - hi;
        outside = 1;
    }
    dz = 0.0f;
    lo = bounds[2] * scale;
    hi = bounds[5] * scale;
    if (lz < lo) {
        dz = lo - lz;
        outside = 1;
    } else if (hi < lz) {
        dz = lz - hi;
        outside = 1;
    }
    if (!outside) {
        *distSq = 0.0f;
        return 1;
    }
    d = dx * dx + dy * dy + dz * dz;
    *distSq = d;
    if (d < sphere[3] * sphere[3]) {
        return 1;
    }
    return 0;
}
#else
INCLUDE_ASM("asm/nonmatchings/291B0", func_8002AC3C);
#endif
