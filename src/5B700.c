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

INCLUDE_ASM("asm/nonmatchings/5B700", func_8005AB00);

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

INCLUDE_ASM("asm/nonmatchings/5B700", func_8005AC58);

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

INCLUDE_ASM("asm/nonmatchings/5B700", func_8005B3CC);

INCLUDE_ASM("asm/nonmatchings/5B700", func_8005B62C);

INCLUDE_ASM("asm/nonmatchings/5B700", func_8005B928);

INCLUDE_ASM("asm/nonmatchings/5B700", func_8005BC44);

INCLUDE_ASM("asm/nonmatchings/5B700", func_8005BD30);

INCLUDE_ASM("asm/nonmatchings/5B700", func_8005BDB8);

INCLUDE_ASM("asm/nonmatchings/5B700", func_8005BEF8);

INCLUDE_ASM("asm/nonmatchings/5B700", func_8005BFC4);

INCLUDE_ASM("asm/nonmatchings/5B700", func_8005C040);

INCLUDE_ASM("asm/nonmatchings/5B700", func_8005C188);

INCLUDE_ASM("asm/nonmatchings/5B700", func_8005C9B8);

INCLUDE_ASM("asm/nonmatchings/5B700", func_8005CAD0);

INCLUDE_ASM("asm/nonmatchings/5B700", func_8005CBBC);

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

INCLUDE_ASM("asm/nonmatchings/5B700", func_8005D19C);

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

INCLUDE_ASM("asm/nonmatchings/5B700", func_8005ED54);

INCLUDE_ASM("asm/nonmatchings/5B700", func_8005EDA4);

INCLUDE_ASM("asm/nonmatchings/5B700", func_8005EE78);

INCLUDE_ASM("asm/nonmatchings/5B700", func_8005EEFC);

INCLUDE_ASM("asm/nonmatchings/5B700", func_8005F048);

INCLUDE_ASM("asm/nonmatchings/5B700", func_8005F294);

INCLUDE_ASM("asm/nonmatchings/5B700", func_8005F438);

INCLUDE_ASM("asm/nonmatchings/5B700", func_8005F4F4);

INCLUDE_ASM("asm/nonmatchings/5B700", func_8005F5A0);

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
