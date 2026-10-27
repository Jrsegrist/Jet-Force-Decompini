/*
 * Weather and particles. Functions in C here are from the Diddy Kong Racing
 * decompilation (CC0 1.0), src/weather.c and src/particles.c; see CREDITS.md.
 */
#include "common.h"

/* DKR src/weather.h, src/particles.h and include/structs.h (reduced) */
typedef enum WeatherType {
    WEATHER_SNOW,
    WEATHER_RAIN
} WeatherType;

typedef struct Vec3f {
    f32 x, y, z;
} Vec3f;

typedef struct Vec3s {
    s16 y_rotation, x_rotation, z_rotation;
} Vec3s;

/* JFG: DKR's flags, size 0x28 */
typedef struct ParticleBehaviour {
    /* 0x00 */ s32 flags;
    /* 0x04 */ Vec3f emitterPos;
    /* 0x10 */ u8 pad10[4];
    /* 0x14 */ Vec3s sourceRotation;
    /* 0x1A */ u8 pad1A[0x22 - 0x1A];
    /* 0x22 */ Vec3s emissionDirection;
} ParticleBehaviour;

#define PARTICLE_POINT 0x400
#define PARTICLE_LINE 0x4000
#define PARTICLE_ACTIVE 0x8000

/* JFG layout, size 0x24 */
typedef struct ParticleEmitter {
    /* 0x00 */ ParticleBehaviour *behaviour;
    /* 0x04 */ s32 flags;
    /* 0x08 */ u8 pad8[2];
    /* 0x0A */ s16 descriptorID;
    /* 0x0C */ s16 colourIndex;
    /* 0x0E */ Vec3s sourceRotation;
    /* 0x14 */ Vec3s emissionDirection;
    /* 0x1A */ s16 posX, posY, posZ;
    /* 0x20 */ s16 unk20;
    /* 0x22 */ u8 index; // into the line or point particle buffers, 0xFF if none
    /* 0x23 */ s8 unk23;
} ParticleEmitter;

/* JFG: the free-emitter pool, size 0x28 */
typedef struct PoolEmitter {
    /* 0x00 */ ParticleEmitter emitter;
    /* 0x24 */ u8 active;
    /* 0x25 */ u8 pad25[3];
} PoolEmitter;

typedef struct ParticleObjHeader {
    /* 0x00 */ u8 pad0[0x1C];
    /* 0x1C */ s16 unk1C;
    /* 0x1E */ u8 pad1E[0x22 - 0x1E];
    /* 0x22 */ s8 emitterCount;
} ParticleObjHeader;

typedef struct ParticleObjModel {
    /* 0x00 */ u8 pad0[0x30];
    /* 0x30 */ u8 emitterCount;
} ParticleObjModel;

typedef struct ParticleObj {
    /* 0x00 */ u8 pad0[0x1A];
    /* 0x1A */ s16 activeEmitters;
    /* 0x1C */ u8 pad1C[0x40 - 0x1C];
    /* 0x40 */ ParticleObjHeader *header;
    /* 0x44 */ u8 pad44[0x68 - 0x44];
    /* 0x68 */ struct {
        u8 pad0[0x20];
        ParticleObjModel *model;
    } *unk68;
    /* 0x6C */ u8 pad6C[4];
    /* 0x70 */ ParticleEmitter *emitters;
} ParticleObj;

/* JFG: a point emitter's particles, size 0xC0 */
typedef struct PointParticleSet {
    /* 0x00 */ s32 points[15];
    /* 0x3C */ s32 count;
    /* 0x40 */ u8 pad40[0xA4 - 0x40];
    /* 0xA4 */ u8 unkA4; // set when the emitter lets go of it
    /* 0xA5 */ u8 padA5[0xC0 - 0xA5];
} PointParticleSet;

/* JFG: a line emitter's data, size 0x148 */
typedef struct LineParticleSet {
    /* 0x000 */ u8 pad0[0x124];
    /* 0x124 */ u8 unk124; // set when the emitter lets go of it
    /* 0x125 */ u8 pad125[0x148 - 0x125];
} LineParticleSet;

extern PointParticleSet *D_800A5D88;
extern LineParticleSet *D_800A5D84;

typedef struct WeatherData {
    s32 intensity;
    s32 intensityStep;
    s32 intensityTarget;
    s32 velX;
    s32 velXStep;
    s32 velXTarget;
    s32 velY;
    s32 velYStep;
    s32 velYTarget;
    s32 velZ;
    s32 velZStep;
    s32 velZTarget;
    s32 opacity;
    s32 opacityStep;
    s32 opacityTarget;
    s32 shiftTime;
} WeatherData;

typedef struct SnowLimits {
    s16 near;
    s16 far;
    s32 current;
} SnowLimits;

void rain_set(s32 lightningFrequency, s32 opacity, f32 time);
void free_particle_buffers(void);
void free_particle_vertices_triangles(void);
void particle_free_dummy(void);
void emitter_init_with_pos(ParticleEmitter *emitter, s32 behaviourID, s32 particleID, s16 posX, s16 posY, s16 posZ);

extern ParticleBehaviour **gParticleBehavioursAssetTable;
extern SnowLimits gSnowPlane; // Uses negative values, so all distances will be checked in reverse.
extern WeatherData gWeather;
extern s32 gParticleBehavioursAssetTableCount;
extern s32 gWeatherType;

typedef struct Vertex Vertex;
typedef struct Triangle Triangle;
typedef struct ObjectSegment ObjectSegment;

ObjectSegment *cam_get_active_camera(void);
MtxF *get_camera_matrix(void);
void rain_update(s32 updateRate);
void snow_update(s32 updateRate);
void snow_vertices(void);
void snow_render(void);

extern Gfx *gCurrWeatherDisplayList;
extern Mtx *gCurrWeatherMatrix;
extern Vertex *gCurrWeatherVertexList;
extern Triangle *gCurrWeatherTriList;
extern ObjectSegment *gWeatherCamera;
extern MtxF *gWeatherCameraMatrix;
extern s32 gSnowDensity;
extern s32 gSnowParticleCount;
extern Vertex *gSnowVerts;
extern Vertex *gSnowVertexData[2];
extern s8 gSnowVertexFlip;

typedef struct TextureHeader TextureHeader;

typedef struct SnowGfxData {
    /* 0x00 */ void *pos;
    /* 0x04 */ s32 unk4;
    /* 0x08 */ TextureHeader *texture;
} SnowGfxData;

void mempool_free(void *data);
void tex_free(TextureHeader *tex);
void free_rain_memory(void);

extern void *gSnowTriangles;
extern void *gSnowPhysics;
extern SnowGfxData gSnowGfx;
extern void *gSnowTriIndices;

extern s32 gSnowVertOffset;
extern s32 gSnowTriCount;
extern s32 *D_800A5A0C; // DKR: gWeatherAssetTable
extern s8 gWeatherAssetTableLength;
s32 *asset_table_load(u32 assetIndex);

/**
 * Zero out all the weather data.
 * Load the weather table into memory.
 * JFG: no lens flare.
 * Official Name: initWeather
 */
void weather_init(void) {
    gSnowGfx.pos = NULL;
    gSnowGfx.unk4 = 0;
    gSnowPhysics = NULL;
    gSnowDensity = 0;
    gSnowVertOffset = 6;
    gSnowVertOffset <<= 2;
    gSnowTriCount = gSnowVertOffset >> 1; // One triangle per two verts
    gSnowVertexData[0] = 0;
    gSnowVertexData[1] = 0;
    gSnowTriangles = 0;
    gSnowPlane.near = -1;
    gSnowPlane.far = -512;
    if (D_800A5A0C == NULL) {
        D_800A5A0C = asset_table_load(0x1B);
        gWeatherAssetTableLength = 0;
        while (D_800A5A0C[gWeatherAssetTableLength] != -1) {
            gWeatherAssetTableLength++;
        }
    }
    gSnowVertexFlip = 0;
}

/**
 * Set the near and far culling distances for the weather effects.
 * Official Name: setWeatherLimits
 */
void weather_clip_planes(s16 near, s16 far) {
    if (gSnowPlane.far < gSnowPlane.near) {
        gSnowPlane.near = near;
        gSnowPlane.far = far;
    } else {
        gSnowPlane.near = far;
        gSnowPlane.far = near;
    }
}

/**
 * Free the weather assets from memory.
 * If it's raining, then free those too.
 * JFG: no lens flare to reset.
 * Official Name: freeWeather
 */
void weather_free(void) {
    if (gSnowTriangles != NULL) {
        mempool_free(gSnowTriangles);
        gSnowTriangles = NULL;
    }
    if (gSnowVertexData[0] != NULL) {
        mempool_free(gSnowVertexData[0]);
        gSnowVertexData[0] = NULL;
    }
    if (gSnowVertexData[1] != NULL) {
        mempool_free(gSnowVertexData[1]);
        gSnowVertexData[1] = NULL;
    }
    if (gSnowPhysics != NULL) {
        mempool_free(gSnowPhysics);
        gSnowPhysics = NULL;
    }
    if (gSnowGfx.pos != NULL) {
        mempool_free(gSnowGfx.pos);
        gSnowGfx.pos = NULL;
    }
    if (gSnowGfx.texture != NULL) {
        tex_free(gSnowGfx.texture);
        gSnowGfx.texture = NULL;
    }
    if (gSnowTriIndices != NULL) {
        mempool_free(gSnowTriIndices);
        gSnowTriIndices = NULL;
    }
    if (gWeatherType != WEATHER_SNOW) {
        free_rain_memory();
    }
}

INCLUDE_ASM("asm/nonmatchings/5B700", func_8005AD90);

/**
 * Change the current properties of the weather.
 * Will start a timer that means it will gradually shift from the current to the new.
 * Official name: changeWeather
 */
void weather_set(s32 velX, s32 velY, s32 velZ, s32 intensity, s32 opacity, s32 time) {
    if (time > 0 && (velX != gWeather.velXTarget || velY != gWeather.velYTarget || velZ != gWeather.velZTarget ||
                     intensity != gWeather.intensity || opacity != gWeather.opacity)) {
        gWeather.velXTarget = velX;
        gWeather.velYTarget = velY;
        gWeather.velZTarget = velZ;
        gWeather.velXStep = (velX - gWeather.velX) / time;
        gWeather.velYStep = (velY - gWeather.velY) / time;
        gWeather.velZStep = (velZ - gWeather.velZ) / time;
        if (gWeatherType == WEATHER_SNOW) {
            gWeather.intensityStep = (intensity - gWeather.intensity) / time;
            gWeather.opacityStep = (opacity - gWeather.opacity) / time;
            gWeather.intensityTarget = intensity;
            gWeather.opacityTarget = opacity;
            gWeather.shiftTime = time;
        } else {
            gWeather.intensity = intensity;
            gWeather.opacity = opacity;
            gWeather.shiftTime = 0;
            rain_set(intensity + 1, opacity + 1, time / 60.0f);
        }
    }
}

/**
 * The root function for handling all weather.
 * Decide whether to perform rain or snow logic, execute it, then set it to render right after.
 * Official name: doWeather
 */
void weather_update(Gfx **currDisplayList, Mtx **currHudMat, Vertex **currHudVerts, Triangle **currHudTris,
                    s32 updateRate) {
    UNUSED s32 unused;
    gCurrWeatherDisplayList = *currDisplayList;
    gCurrWeatherMatrix = *currHudMat;
    gCurrWeatherVertexList = *currHudVerts;
    gCurrWeatherTriList = *currHudTris;
    gWeatherCamera = cam_get_active_camera();
    gWeatherCameraMatrix = get_camera_matrix();
    if (gWeatherType != WEATHER_SNOW) {
        rain_update(updateRate);
    } else {
        if (gWeather.shiftTime > 0) {
            if (updateRate < gWeather.shiftTime) {
                gWeather.intensity += gWeather.intensityStep * updateRate;
                gWeather.velX += gWeather.velXStep * updateRate;
                gWeather.velY += gWeather.velYStep * updateRate;
                gWeather.velZ += gWeather.velZStep * updateRate;
                gWeather.opacity += gWeather.opacityStep * updateRate;
                gWeather.shiftTime -= updateRate;
            } else {
                gWeather.intensity = gWeather.intensityTarget;
                gWeather.velX = gWeather.velXTarget;
                gWeather.velY = gWeather.velYTarget;
                gWeather.velZ = gWeather.velZTarget;
                gWeather.opacity = gWeather.opacityTarget;
                gWeather.shiftTime = 0;
            }
        }
        gSnowParticleCount = (gSnowDensity * gWeather.intensity) >> (unused = 16);
        gSnowPlane.current =
            (gSnowPlane.near + ((gSnowPlane.far - gSnowPlane.near) * gWeather.opacity)) >> (unused = 16);

        snow_update(updateRate);
        if (gSnowParticleCount > 0 && gSnowPlane.current < gSnowPlane.near) {
            gSnowVerts = gSnowVertexData[gSnowVertexFlip];
            snow_vertices();
            snow_render();
            gSnowVertexFlip = 1 - gSnowVertexFlip;
        }
    }
    *currDisplayList = gCurrWeatherDisplayList;
    *currHudMat = gCurrWeatherMatrix;
    *currHudVerts = gCurrWeatherVertexList;
    *currHudTris = gCurrWeatherTriList;
}

INCLUDE_ASM("asm/nonmatchings/5B700", func_8005B62C);

INCLUDE_ASM("asm/nonmatchings/5B700", func_8005B928);

/* DKR's rain state (gLightningFrequencyStep etc. in DKR) */
typedef struct AudioPoint AudioPoint;
typedef struct SpriteAsset SpriteAsset;
extern s32 gLightningFrequency;
extern s32 D_800A5D20;
extern s32 D_800A5D24;
extern s32 D_800A5D28;
extern s32 D_800A5D2C;
extern s32 D_800A5D30;
extern s32 D_800A5D34;
extern s32 D_800A5D38;
extern s32 D_800A5D3C;
extern s32 D_800A5D40;
extern SpriteAsset *D_800A5D44;
extern TextureHeader *D_800A5D48;
extern s32 D_800A5D4C;
extern AudioPoint *gWeatherSoundMask;
extern s32 D_801008B4;
void func_80054530();
SpriteAsset *tex_load_sprite(s32 spriteID, s32 arg1);
TextureHeader *load_texture(s32 assetIndex);
void sprite_free(SpriteAsset *sprite);
void audspat_point_stop(AudioPoint *point);

/**
 * Set all rain related values to the default.
 * JFG: a module call sets up the rain effect; the textures are fixed asset numbers.
 * DKR: rain_init.
 */
void func_8005BC44(s32 arg0, s32 intensity, s32 opacity) {
    gLightningFrequency = intensity;
    D_800A5D20 = 0;
    D_800A5D24 = gLightningFrequency;
    D_800A5D28 = opacity;
    D_800A5D2C = 0;
    D_800A5D30 = D_800A5D28;
    D_800A5D34 = 0;
    D_800A5D38 = 0;
    D_800A5D3C = 0;
    D_800A5D40 = 0;
    D_800A5D4C = 0;
    func_80054530(arg0, 700, 700, 700, 0x2080E002, 0xA0E0FF04, 550);
    D_800A5D44 = tex_load_sprite(0x4A, 0);
    D_800A5D48 = load_texture(0x1AC);
    gWeatherType = WEATHER_RAIN;
    D_801008B4 = 0;
}

/**
 * Free the rain overlay and splash asset data from memory.
 * Clear the soundmask too, stopping any ongoing audio.
 * JFG: also tells the rain module.
 */
void free_rain_memory(void) {
    if (D_800A5D44 != NULL) {
        sprite_free(D_800A5D44);
        D_800A5D44 = NULL;
    }
    if (D_800A5D48 != NULL) {
        tex_free(D_800A5D48);
        D_800A5D48 = NULL;
    }
    if (gWeatherSoundMask != NULL) {
        audspat_point_stop(gWeatherSoundMask);
        gWeatherSoundMask = NULL;
    }
    func_80054530();
    gWeatherType = WEATHER_SNOW;
}

INCLUDE_ASM("asm/nonmatchings/5B700", func_8005BDB8);

/* DKR include/level.h LevelHeader, reduced (JFG offsets) */
typedef struct LevelHeader {
    /* 0x00 */ u8 pad0[0xA2];
    /* 0xA2 */ u8 unkA2; // JFG: bit 0 = no rain fog
} LevelHeader;

s32 get_current_viewport(void);
LevelHeader *level_header(void);
void set_fog(s32 fogIdx, s16 near, s16 far, s16 unk, u8 red, u8 green, u8 blue, s8 unk2);

/**
 * If it's currently raining, update the fog levels to match the intensity of the storm.
 * JFG: skipped on levels flagged in the header; set_fog takes more arguments.
 * Official Name: rainSetFog
 */
void rain_fog(void) {
    s32 a, b;
    if (gWeatherType != WEATHER_SNOW && get_current_viewport() == 0 && !(level_header()->unkA2 & 1)) {
        a = ((gLightningFrequency * -38) >> 16) + 1018;
        b = ((gLightningFrequency * -20) >> 16) + 1023;
        set_fog(0, a, b, a, 28, 15, 36, 0);
    }
}

extern u32 D_800A5D1C;

/* JFG: the rain density, 0.0 to 1.0. */
f32 func_8005BFC4(void) {
    f32 density = (f32) ((s32) ((D_800A5D28 >> 2) * D_800A5D1C) >> 14) / 65536.0f;

    if (density < 0.0f) {
        density = 0.0f;
    }
    if (density > 1.0f) {
        density = 1.0f;
    }
    return density;
}

INCLUDE_ASM("asm/nonmatchings/5B700", func_8005C040);

INCLUDE_ASM("asm/nonmatchings/5B700", func_8005C188);

INCLUDE_ASM("asm/nonmatchings/5B700", func_8005C9B8);

struct ObjectSegment {
    /* 0x00 */ s16 y_rotation;
    /* 0x02 */ u8 pad2[0xC - 2];
    /* 0x0C */ Vec3f pos;
};

f32 func_80049C30(s32 angle); // cosine of a 16-bit angle
f32 func_80049C34(s32 angle); // sine of a 16-bit angle
void audspat_point_set_position(AudioPoint *audioPoint, f32 x, f32 y, f32 z);
void audspat_play_sound_at_position(u16 soundId, f32 x, f32 y, f32 z, u8 flags, AudioPoint **handlePtr);

/**
 * Emit looping rain sound, using the camera position and direction as reference.
 * DKR: rain_sound.
 */
void func_8005CAD0(UNUSED s32 updateRate) {
    f32 xPos;
    f32 yPos;
    f32 zPos;
    f32 length;
    f32 sine;
    f32 cosine;

    length = 1152.0f - (f32) (gLightningFrequency >> 6);
    sine = func_80049C34(-0x8000 - gWeatherCamera->y_rotation);
    cosine = func_80049C30(-0x8000 - gWeatherCamera->y_rotation);
    xPos = gWeatherCamera->pos.x - length * sine;
    yPos = gWeatherCamera->pos.y;
    zPos = gWeatherCamera->pos.z - length * cosine;
    if (gWeatherSoundMask != NULL) {
        audspat_point_set_position(gWeatherSoundMask, xPos, yPos, zPos);
    } else {
        audspat_play_sound_at_position(4, xPos, yPos, zPos, 1, &gWeatherSoundMask);
    }
}

extern s32 D_801008B8;
extern s32 D_801008BC;
extern s32 D_801008C0;
extern s32 D_801008C4;
extern TextureHeader *D_800A59D0;
extern s32 *D_800A5A0C;
TextureHeader *load_texture(s32 assetIndex);

void func_8005CBBC(void) {
    D_801008B8 = 0;
    D_801008BC = 0;
    D_801008C0 = 0;
    D_801008C4 = 0;
    D_800A59D0 = load_texture(*D_800A5A0C);
}

INCLUDE_ASM("asm/nonmatchings/5B700", func_8005CC0C);

/**
 * Deallocate all memory buffers used by particles.
 */
void reset_particles(void) {
    free_particle_buffers();
    free_particle_vertices_triangles();
    particle_free_dummy();
}

INCLUDE_ASM("asm/nonmatchings/5B700", func_8005CE90);

INCLUDE_ASM("asm/nonmatchings/5B700", func_8005CFD0);

extern void *D_800A5D8C[2];

void func_8005D19C(void) {
    if (D_800A5D8C[0] != NULL) {
        mempool_free(D_800A5D8C[0]);
        D_800A5D8C[0] = NULL;
        D_800A5D8C[1] = NULL;
    }
}

INCLUDE_ASM("asm/nonmatchings/5B700", func_8005D1DC);

INCLUDE_ASM("asm/nonmatchings/5B700", func_8005D2E8);

INCLUDE_ASM("asm/nonmatchings/5B700", func_8005D660);

INCLUDE_ASM("asm/nonmatchings/5B700", func_8005D93C);

/**
 * Initializes the emitter with the specified particle type and behaviour.
 */
void emitter_init(ParticleEmitter *emitter, s32 behaviourID, s32 particleID) {
    ParticleBehaviour *behaviour;

    if (behaviourID < gParticleBehavioursAssetTableCount) {
        behaviour = gParticleBehavioursAssetTable[behaviourID];
        emitter_init_with_pos(emitter, behaviourID, particleID, behaviour->emitterPos.x, behaviour->emitterPos.y,
                              behaviour->emitterPos.z);
    }
}

/* JFG: emitter_init without the behaviour's position, plus a trigger byte. */
void func_8005ED54(ParticleEmitter *emitter, s32 behaviourID, s32 particleID, s32 trigger) {
    if (behaviourID < gParticleBehavioursAssetTableCount) {
        emitter_init_with_pos(emitter, behaviourID, particleID, 0, 0, 0);
        emitter->unk23 = trigger;
    }
}

INCLUDE_ASM("asm/nonmatchings/5B700", func_8005EDA4);

extern PoolEmitter *D_800A5DAC;
extern s32 D_800A5DB0;

/* JFG: take a free emitter from the pool and initialise it; NULL if none is free. */
ParticleEmitter *func_8005EE78(s32 behaviourID, s32 particleID) {
    s32 i;
    PoolEmitter *entry = D_800A5DAC;

    for (i = 0; i < D_800A5DB0; i++, entry++) {
        if (!entry->active) {
            entry->active = TRUE;
            entry->emitter.colourIndex = 0;
            emitter_init_with_pos(&entry->emitter, behaviourID, particleID, 0, 0, 0);
            return &entry->emitter;
        }
    }
    return NULL;
}

INCLUDE_ASM("asm/nonmatchings/5B700", func_8005EEFC);

INCLUDE_ASM("asm/nonmatchings/5B700", func_8005F048);

INCLUDE_ASM("asm/nonmatchings/5B700", func_8005F294);

/* JFG: switch off an object's emitter and release its line/point buffer. */
void func_8005F438(ParticleObj *obj, s32 emitterIndex) {
    ParticleEmitter *emitter = &obj->emitters[emitterIndex];
    s32 flags;
    LineParticleSet *line;
    PointParticleSet *point;

    emitter->flags &= ~PARTICLE_ACTIVE;
    obj->activeEmitters--;
    flags = obj->emitters[emitterIndex].flags;
    if (flags & PARTICLE_LINE) {
        line = &D_800A5D84[emitter->index];
        emitter->index = 0xFF;
        line->unk124 = TRUE;
    } else if (flags & PARTICLE_POINT) {
        point = &D_800A5D88[emitter->index];
        emitter->index = 0xFF;
        point->unkA4 = TRUE;
    }
}

/* JFG: switch off every active emitter of an object. */
void func_8005F4F4(ParticleObj *obj) {
    s32 i;
    s32 count;
    ParticleObjHeader *header = obj->header;
    ParticleObjModel *model;

    if (header->unk1C == 0x27) {
        model = obj->unk68->model;
        count = model->emitterCount;
    } else {
        count = header->emitterCount;
    }
    for (i = 0; i < count; i++) {
        if (obj->emitters[i].flags & PARTICLE_ACTIVE) {
            func_8005F438(obj, i);
        }
    }
}

extern f32 D_800A5DDC;

void func_8005F5A0(f32 arg0) {
    D_800A5DDC = arg0;
}

INCLUDE_ASM("asm/nonmatchings/5B700", func_8005F5AC);

INCLUDE_ASM("asm/nonmatchings/5B700", func_8005F76C);

INCLUDE_ASM("asm/nonmatchings/5B700", func_8005F960);

INCLUDE_ASM("asm/nonmatchings/5B700", func_8005FE4C);

INCLUDE_ASM("asm/nonmatchings/5B700", func_800602C0);

INCLUDE_ASM("asm/nonmatchings/5B700", func_80060434);

INCLUDE_ASM("asm/nonmatchings/5B700", func_80060EC4);

INCLUDE_ASM("asm/nonmatchings/5B700", func_800610CC);

INCLUDE_ASM("asm/nonmatchings/5B700", func_80061228);

INCLUDE_ASM("asm/nonmatchings/5B700", func_80061698);

INCLUDE_ASM("asm/nonmatchings/5B700", func_80061C30);

INCLUDE_ASM("asm/nonmatchings/5B700", func_80061FC8);

INCLUDE_ASM("asm/nonmatchings/5B700", func_80062178);

extern s32 D_800A5D80;
void func_80062178(Gfx **dList, Mtx **mtx, PointParticleSet *set);

/* JFG: render the flagged point particle sets. */
void func_80062A00(Gfx **dList, Mtx **mtx) {
    s32 i;
    PointParticleSet *item;

    if (D_800A5D88 != NULL) {
        item = D_800A5D88;
        for (i = 0; i < D_800A5D80; i++, item++) {
            if (item->unkA4) {
                func_80062178(dList, mtx, item);
            }
        }
    }
}

INCLUDE_ASM("asm/nonmatchings/5B700", func_80062A90);

typedef struct ParticleDesc {
    /* 0x00 */ u8 pad0[4];
    /* 0x04 */ u32 flags; // top 4 bits: kind (4 = point)
} ParticleDesc;
extern ParticleDesc **D_800A5D9C;
s32 func_80060434(void *arg0, ParticleEmitter *emitter, s32 arg2);

/* JFG: add a new particle to the emitter's point set (at most 15). */
void func_80062D5C(void *arg0, ParticleEmitter *emitter) {
    ParticleDesc *desc = D_800A5D9C[emitter->descriptorID];
    PointParticleSet *set;
    s32 particle;

    if (emitter->index != 0xFF) {
        if ((desc->flags >> 28) == 4) {
            set = &D_800A5D88[emitter->index];
            if (set->count != 15) {
                particle = func_80060434(arg0, emitter, -1);
                if (particle != 0) {
                    set->points[set->count] = particle;
                    set->count++;
                }
            }
        }
    }
}

INCLUDE_ASM("asm/nonmatchings/5B700", func_80062E0C);

INCLUDE_ASM("asm/nonmatchings/5B700", func_80062F0C);

INCLUDE_ASM("asm/nonmatchings/5B700", func_80063030);

/* DKR include/structs.h Particle, reduced (JFG offsets) */
typedef struct Particle {
    /* 0x00 */ s16 y_rotation;
    /* 0x02 */ s16 x_rotation;
    /* 0x04 */ s16 z_rotation;
    /* 0x06 */ u8 pad6[2];
    /* 0x08 */ f32 scale;
    /* 0x0C */ Vec3f position;
    /* 0x18 */ f32 scaleVelocity;
    /* 0x1C */ Vec3f velocity;
    /* 0x28 */ u8 pad28[0x40 - 0x28];
    /* 0x40 */ f32 gravity;
    /* 0x44 */ u8 pad44[0x5E - 0x44];
    /* 0x5E */ s16 angVelY;
    /* 0x60 */ s16 angVelX;
    /* 0x62 */ s16 angVelZ;
} Particle;

extern s32 gParticleUpdateRate;

/**
 * Apply translation, rotation and scale based off the velocities of the particle.
 * DKR: move_particle_basic.
 */
void func_80063190(Particle *particle) {
    s32 i = 0;

    while (i++ < gParticleUpdateRate) {
        particle->position.x += particle->velocity.x;
        particle->position.y += particle->velocity.y;
        particle->velocity.y -= particle->gravity;
        particle->position.z += particle->velocity.z;
        particle->scale += particle->scaleVelocity;
        particle->y_rotation += particle->angVelY;
        particle->x_rotation += particle->angVelX;
        particle->z_rotation += particle->angVelZ;
    }
}

INCLUDE_ASM("asm/nonmatchings/5B700", func_80063244);

INCLUDE_ASM("asm/nonmatchings/5B700", func_80063358);

INCLUDE_ASM("asm/nonmatchings/5B700", func_80063730);

/* Size: 0x78 bytes */
typedef struct WeatherParticle {
    /* 0x00 */ u8 pad0[0x2C];
    /* 0x2C */ s16 unk2C;
    /* 0x2E */ u8 unk2E;
    /* 0x2F */ u8 pad2F;
    /* 0x30 */ Vec3f pos;
    /* 0x3C */ u8 pad3C[0x48 - 0x3C];
    /* 0x48 */ s32 unk48;
    /* 0x4C */ u8 pad4C[0x78 - 0x4C];
} WeatherParticle;

typedef struct WeatherParticleList {
    /* 0x00 */ u8 pad0[0x14];
    /* 0x14 */ WeatherParticle *particles;
    /* 0x18 */ s32 count;
} WeatherParticleList;

typedef struct ObjPos {
    /* 0x00 */ u8 pad0[0xC];
    /* 0x0C */ Vec3f pos;
} ObjPos;

extern WeatherParticleList *D_801008E0[5];

/* JFG: move the particles of all five lists by the object's position. */
void func_8006382C(ObjPos *obj) {
    s32 i;
    s32 j;
    WeatherParticleList *list;
    WeatherParticle *p;

    for (i = 0; i < 5; i++) {
        list = D_801008E0[i];
        p = list->particles;
        for (j = 0; j < list->count; j++, p++) {
            if (p->unk2C != 0x80 && p->unk2E == 3) {
                p->pos.x += obj->pos.x;
                p->pos.y += obj->pos.y;
                p->pos.z += obj->pos.z;
                p->unk48 = 0;
            }
        }
    }
}
