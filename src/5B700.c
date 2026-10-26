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

typedef struct ParticleEmitter ParticleEmitter;

typedef struct ParticleBehaviour {
    /* 0x00 */ s32 flags;
    /* 0x04 */ Vec3f emitterPos;
} ParticleBehaviour;

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

INCLUDE_ASM("asm/nonmatchings/5B700", func_8005BC44);

INCLUDE_ASM("asm/nonmatchings/5B700", func_8005BD30);

INCLUDE_ASM("asm/nonmatchings/5B700", func_8005BDB8);

/* DKR include/level.h LevelHeader, reduced (JFG offsets) */
typedef struct LevelHeader {
    /* 0x00 */ u8 pad0[0xA2];
    /* 0xA2 */ u8 unkA2; // JFG: bit 0 = no rain fog
} LevelHeader;

extern s32 gLightningFrequency;
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

extern s32 D_800A5D28;
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

INCLUDE_ASM("asm/nonmatchings/5B700", func_8005CAD0);

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

typedef struct ParticleEmitterUnk23 {
    u8 pad0[0x23];
    u8 unk23;
} ParticleEmitterUnk23;

/* JFG: emitter_init without the behaviour's position, plus a trigger byte. */
void func_8005ED54(ParticleEmitter *emitter, s32 behaviourID, s32 particleID, s32 trigger) {
    if (behaviourID < gParticleBehavioursAssetTableCount) {
        emitter_init_with_pos(emitter, behaviourID, particleID, 0, 0, 0);
        ((ParticleEmitterUnk23 *) emitter)->unk23 = trigger;
    }
}

INCLUDE_ASM("asm/nonmatchings/5B700", func_8005EDA4);

INCLUDE_ASM("asm/nonmatchings/5B700", func_8005EE78);

INCLUDE_ASM("asm/nonmatchings/5B700", func_8005EEFC);

INCLUDE_ASM("asm/nonmatchings/5B700", func_8005F048);

INCLUDE_ASM("asm/nonmatchings/5B700", func_8005F294);

INCLUDE_ASM("asm/nonmatchings/5B700", func_8005F438);

INCLUDE_ASM("asm/nonmatchings/5B700", func_8005F4F4);

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

INCLUDE_ASM("asm/nonmatchings/5B700", func_80062A00);

INCLUDE_ASM("asm/nonmatchings/5B700", func_80062A90);

INCLUDE_ASM("asm/nonmatchings/5B700", func_80062D5C);

INCLUDE_ASM("asm/nonmatchings/5B700", func_80062E0C);

INCLUDE_ASM("asm/nonmatchings/5B700", func_80062F0C);

INCLUDE_ASM("asm/nonmatchings/5B700", func_80063030);

INCLUDE_ASM("asm/nonmatchings/5B700", func_80063190);

INCLUDE_ASM("asm/nonmatchings/5B700", func_80063244);

INCLUDE_ASM("asm/nonmatchings/5B700", func_80063358);

INCLUDE_ASM("asm/nonmatchings/5B700", func_80063730);

INCLUDE_ASM("asm/nonmatchings/5B700", func_8006382C);
