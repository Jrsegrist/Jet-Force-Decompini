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
extern s32 *gWeatherAssetTable;
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
    if (gWeatherAssetTable == NULL) {
        gWeatherAssetTable = asset_table_load(0x1B);
        gWeatherAssetTableLength = 0;
        while (gWeatherAssetTable[gWeatherAssetTableLength] != -1) {
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
extern s32 gLightningFrequencyStep;
extern s32 gLightningFrequencyTarget;
extern s32 gRainOpacity;
extern s32 gRainOpacityStep;
extern s32 gRainOpacityTarget;
extern s32 gRainHiddenTimer;
extern s32 gLightningTimer;
extern s32 gThunderTimer;
extern s32 gRainSplashDelay;
extern SpriteAsset *gRainSplashGfx;
extern TextureHeader *D_800A5D48;
extern s32 gRainVertexFlip;
extern AudioPoint *gWeatherSoundMask;
extern s32 D_801008B4;
void func_80054530();
SpriteAsset *tex_load_sprite(s32 spriteID, s32 arg1);
TextureHeader *load_texture(s32 assetIndex);
void sprite_free(SpriteAsset *sprite);
void audspat_point_stop(AudioPoint *point);

/* Sets up rain: lightning/opacity values, timers, the splash sprite and texture (0x8005BC44). */
/**
 * Set all rain related values to the default.
 * JFG: a module call sets up the rain effect; the textures are fixed asset numbers.
 * DKR: rain_init.
 */
void rain_init(s32 arg0, s32 intensity, s32 opacity) {
    gLightningFrequency = intensity;
    gLightningFrequencyStep = 0;
    gLightningFrequencyTarget = gLightningFrequency;
    gRainOpacity = opacity;
    gRainOpacityStep = 0;
    gRainOpacityTarget = gRainOpacity;
    gRainHiddenTimer = 0;
    gLightningTimer = 0;
    gThunderTimer = 0;
    gRainSplashDelay = 0;
    gRainVertexFlip = 0;
    func_80054530(arg0, 700, 700, 700, 0x2080E002, 0xA0E0FF04, 550);
    gRainSplashGfx = tex_load_sprite(0x4A, 0);
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
    if (gRainSplashGfx != NULL) {
        sprite_free(gRainSplashGfx);
        gRainSplashGfx = NULL;
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
    f32 density = (f32) ((s32) ((gRainOpacity >> 2) * D_800A5D1C) >> 14) / 65536.0f;

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

extern u8 gThunderTransition[];
extern s32 D_800A6784;
s32 check_fadeout_transition(void);
void func_8006B9CC(void *transition); // DKR: transition_begin
void sound_play(u16 soundID, s32 *handlePtr);
s32 rand_range(s32 min, s32 max);

/* Counts down to thunder and lightning flashes (0x8005C9B8). */
/**
 * On a randomly set timer, based on the weather intensity, count down and make the sound of thunder.
 * Set a second timer, that then counts down to zero and creates a flash of lightning.
 * DKR: rain_lightning.
 */
void rain_lightning(s32 updateRate) {
    s32 delay;

    if (gThunderTimer > 0) {
        gThunderTimer -= updateRate;
        if (gThunderTimer <= 0) {
            if (gRainOpacity > 0x8000 && check_fadeout_transition() == 0) {
                func_8006B9CC(gThunderTransition);
                D_800A6784 = 0;
            }
            sound_play(6, NULL);
            gThunderTimer = 0;
        }
    } else if (gLightningFrequency > 0xC000) {
        if (gLightningTimer > 0) {
            gLightningTimer -= updateRate;
        } else {
            delay = (gLightningFrequency * 600 - 0x1C20000) >> 14;
            sound_play(7, NULL);
            gThunderTimer = delay + 60;
            gLightningTimer = rand_range(1200, 1500) - delay;
        }
    }
}

struct ObjectSegment {
    /* 0x00 */ s16 y_rotation;
    /* 0x02 */ u8 pad2[0xC - 2];
    /* 0x0C */ Vec3f pos;
};

f32 func_80049C30(s32 angle); // cosine of a 16-bit angle
f32 func_80049C34(s32 angle); // sine of a 16-bit angle
void audspat_point_set_position(AudioPoint *audioPoint, f32 x, f32 y, f32 z);
void audspat_play_sound_at_position(u16 soundId, f32 x, f32 y, f32 z, u8 flags, AudioPoint **handlePtr);

/* Keeps the looping rain sound in front of the camera (0x8005CAD0). */
/**
 * Emit looping rain sound, using the camera position and direction as reference.
 * DKR: rain_sound.
 */
void rain_sound(UNUSED s32 updateRate) {
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
extern s32 *gWeatherAssetTable;
TextureHeader *load_texture(s32 assetIndex);

void func_8005CBBC(void) {
    D_801008B8 = 0;
    D_801008BC = 0;
    D_801008C0 = 0;
    D_801008C4 = 0;
    D_800A59D0 = load_texture(*gWeatherAssetTable);
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

extern Vertex *D_800A5D8C[2];

/* Frees the particle vertex buffers (0x8005D19C). */
void free_particle_vertex_buffers(void) {
    if (D_800A5D8C[0] != NULL) {
        mempool_free(D_800A5D8C[0]);
        D_800A5D8C[0] = NULL;
        D_800A5D8C[1] = NULL;
    }
}

/* Size: 0x78 bytes */
typedef struct WeatherParticle {
    /* 0x00 */ u8 pad0[0x2C];
    /* 0x2C */ s16 kind; // 0/1 texture, 2 sprite, 3 model
    /* 0x2E */ u8 unk2E;
    /* 0x2F */ u8 pad2F;
    /* 0x30 */ Vec3f pos;
    /* 0x3C */ u8 pad3C[0x48 - 0x3C];
    /* 0x48 */ s32 unk48;
    /* 0x4C */ void *asset;
    /* 0x50 */ u8 pad50[0x78 - 0x50];
} WeatherParticle;

typedef struct WeatherParticleList {
    /* 0x00 */ u8 pad0[0x14];
    /* 0x14 */ WeatherParticle *particles;
    /* 0x18 */ s32 count;
} WeatherParticleList;

void func_8003C7C4(void *model);

/* JFG: free a particle list and the assets of its particles (0x8005D1DC). */
void free_particle_list(WeatherParticleList **listPtr) {
    WeatherParticleList *list = *listPtr;
    WeatherParticle *p;
    s32 i;

    if (list != NULL) {
        p = list->particles;
        for (i = 0; i < list->count; i++, p++) {
            switch (p->kind) {
                case 2:
                    sprite_free(p->asset);
                    break;
                case 0:
                case 1:
                    tex_free(p->asset);
                    break;
                case 3:
                    func_8003C7C4(p->asset);
                    break;
            }
        }
        mempool_free(list);
        *listPtr = NULL;
    }
}

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

/* JFG: emitter_init without the behaviour's position, plus a trigger byte (0x8005ED54). */
void emitter_init_at_origin(ParticleEmitter *emitter, s32 behaviourID, s32 particleID, s32 trigger) {
    if (behaviourID < gParticleBehavioursAssetTableCount) {
        emitter_init_with_pos(emitter, behaviourID, particleID, 0, 0, 0);
        emitter->unk23 = trigger;
    }
}

/**
 * Initializes the emitter with the specified particle type, behaviour and position.
 * JFG: line and point emitters only get their flags; no buffers are allocated here.
 */
void emitter_init_with_pos(ParticleEmitter *emitter, s32 behaviourID, s32 particleID, s16 posX, s16 posY, s16 posZ) {
    ParticleBehaviour *behaviour;

    behaviour = gParticleBehavioursAssetTable[behaviourID];
    emitter->descriptorID = particleID;
    emitter->colourIndex = 0;
    emitter->posX = posX;
    emitter->behaviour = behaviour;
    emitter->posY = posY;
    emitter->posZ = posZ;
    emitter->unk20 = 0;
    emitter->unk23 = -1;
    if (behaviour->flags & PARTICLE_LINE) {
        emitter->flags = PARTICLE_LINE;
        emitter->index = 0xFF;
    } else if (behaviour->flags & PARTICLE_POINT) {
        emitter->flags = PARTICLE_POINT;
        emitter->index = 0xFF;
    } else {
        emitter->flags = 0;
        emitter->sourceRotation.y_rotation = behaviour->sourceRotation.y_rotation;
        emitter->sourceRotation.x_rotation = behaviour->sourceRotation.x_rotation;
        emitter->sourceRotation.z_rotation = behaviour->sourceRotation.z_rotation;
        emitter->emissionDirection.y_rotation = behaviour->emissionDirection.y_rotation;
        emitter->emissionDirection.x_rotation = behaviour->emissionDirection.x_rotation;
        emitter->emissionDirection.z_rotation = behaviour->emissionDirection.z_rotation;
        emitter->index = 0;
    }
}

extern PoolEmitter *D_800A5DAC;
extern s32 D_800A5DB0;

/* JFG: take a free emitter from the pool and initialise it; NULL if none is free (0x8005EE78). */
ParticleEmitter *emitter_alloc_from_pool(s32 behaviourID, s32 particleID) {
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

/* JFG: switch off an object's emitter and release its line/point buffer (0x8005F438). */
void obj_disable_emitter(ParticleObj *obj, s32 emitterIndex) {
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

/* JFG: switch off every active emitter of an object (0x8005F4F4). */
void obj_disable_all_emitters(ParticleObj *obj) {
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
            obj_disable_emitter(obj, i);
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
void func_80062178(Gfx **dList, Vertex **vtx, PointParticleSet *set);

/* JFG: render the flagged point particle sets. */
void func_80062A00(Gfx **dList, Vertex **vtx) {
    s32 i;
    PointParticleSet *item;

    if (D_800A5D88 != NULL) {
        item = D_800A5D88;
        for (i = 0; i < D_800A5D80; i++, item++) {
            if (item->unkA4) {
                func_80062178(dList, vtx, item);
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

typedef struct ObjPos {
    /* 0x00 */ u8 pad0[0xC];
    /* 0x0C */ Vec3f pos;
} ObjPos;

/* DKR include/structs.h Particle, reduced (JFG offsets) */
typedef struct Particle {
    /* 0x00 */ Vec3s rotation;
    /* 0x06 */ u8 pad6[2];
    /* 0x08 */ f32 scale;
    /* 0x0C */ Vec3f position;
    /* 0x18 */ f32 scaleVelocity;
    /* 0x1C */ Vec3f velocity;
    /* 0x28 */ u8 pad28[0x30 - 0x28];
    /* 0x30 */ Vec3f localPos;
    /* 0x3C */ f32 forwardVel; // also downOffset / downAcceleration in DKR
    /* 0x40 */ f32 gravity;
    /* 0x44 */ u8 pad44[0x48 - 0x44];
    /* 0x48 */ ObjPos *parentObj;
    /* 0x4C */ u8 pad4C[0x5E - 0x4C];
    /* 0x5E */ Vec3s angularVelocity;
} Particle;

extern s32 gParticleUpdateRate;
void func_800496B0(s32 arg0, Vec3s *rotation, Vec3f *in, Vec3f *out); // DKR: vec3f_rotate
void func_8004944C(Vec3s *rotation, Vec3f *vec);                     // DKR: vec3f_rotate_py

/* Moves a particle by its velocities, offset by its parent's position (0x80062E0C). */
/**
 * Apply translation, rotation and scale based off the velocities of the particle.
 * Applies further translation based on the position of the parent.
 * DKR: move_particle_basic_parent.
 */
void move_particle_basic_parent(Particle *particle) {
    s32 i;
    ObjPos *parent;

    i = gParticleUpdateRate;
    while (i-- > 0) {
        particle->localPos.x += particle->velocity.x;
        particle->localPos.y += particle->velocity.y;
        particle->velocity.y -= particle->gravity;
        particle->localPos.z += particle->velocity.z;
        particle->rotation.y_rotation += particle->angularVelocity.y_rotation;
        particle->rotation.x_rotation += particle->angularVelocity.x_rotation;
        particle->rotation.z_rotation += particle->angularVelocity.z_rotation;
        particle->scale += particle->scaleVelocity;
    }
    particle->position.x = particle->localPos.x;
    particle->position.y = particle->localPos.y;
    particle->position.z = particle->localPos.z;
    parent = particle->parentObj;
    if (parent) {
        particle->position.x += parent->pos.x;
        particle->position.y += parent->pos.y;
        particle->position.z += parent->pos.z;
    }
}

/* Rotates and scales a particle, placing it below its local position relative to the parent (0x80062F0C). */
/**
 * Applies rotation and scale based on the particle's velocities.
 * Translates the particle downward in the object's local coordinate system by downOffset.
 * Applies additional translation based on the parent's position.
 * DKR: move_particle_attached_to_parent.
 */
void move_particle_attached_to_parent(Particle *particle) {
    s32 i = gParticleUpdateRate;
    ObjPos *parent;

    while (i-- > 0) {
        particle->rotation.y_rotation += particle->angularVelocity.y_rotation;
        particle->rotation.x_rotation += particle->angularVelocity.x_rotation;
        particle->rotation.z_rotation += particle->angularVelocity.z_rotation;
        particle->scale += particle->scaleVelocity;
    }

    particle->position.x = 0.0f;
    particle->position.y = -particle->forwardVel;
    particle->position.z = 0.0f;
    func_800496B0(1, &particle->rotation, &particle->position, &particle->position);
    particle->position.x += particle->localPos.x;
    particle->position.y += particle->localPos.y;
    particle->position.z += particle->localPos.z;

    parent = particle->parentObj;
    if (parent != NULL) {
        particle->position.x += parent->pos.x;
        particle->position.y += parent->pos.y;
        particle->position.z += parent->pos.z;
    }
}

/* Moves a particle with a downward local acceleration (0x80063030). */
/**
 * Apply translation, rotation and scale based off the velocities of the particle.
 * Apply additional acceleration always directed downward in the local coordinate system
 * DKR: move_particle_with_acceleration.
 */
void move_particle_with_acceleration(Particle *particle) {
    s32 i = 0;
    Vec3f acceleration;

    while (i++ < gParticleUpdateRate) {
        particle->position.x += particle->velocity.x;
        particle->position.y += particle->velocity.y;
        particle->position.z += particle->velocity.z;
        particle->scale += particle->scaleVelocity;
        particle->rotation.y_rotation += particle->angularVelocity.y_rotation;
        particle->rotation.x_rotation += particle->angularVelocity.x_rotation;
        particle->rotation.z_rotation += particle->angularVelocity.z_rotation;

        acceleration.x = 0.0f;
        acceleration.y = -particle->forwardVel;
        acceleration.z = 0.0f;
        func_800496B0(1, &particle->rotation, &acceleration, &acceleration);
        particle->velocity.x += acceleration.x;
        particle->velocity.y += acceleration.y;
        particle->velocity.y -= particle->gravity;
        particle->velocity.z += acceleration.z;
    }
}

/* Moves a particle by its velocities and gravity (0x80063190). */
/**
 * Apply translation, rotation and scale based off the velocities of the particle.
 * DKR: move_particle_basic.
 */
void move_particle_basic(Particle *particle) {
    s32 i = 0;

    while (i++ < gParticleUpdateRate) {
        particle->position.x += particle->velocity.x;
        particle->position.y += particle->velocity.y;
        particle->velocity.y -= particle->gravity;
        particle->position.z += particle->velocity.z;
        particle->scale += particle->scaleVelocity;
        particle->rotation.y_rotation += particle->angularVelocity.y_rotation;
        particle->rotation.x_rotation += particle->angularVelocity.x_rotation;
        particle->rotation.z_rotation += particle->angularVelocity.z_rotation;
    }
}

/* Moves a particle forward along its rotation (0x80063244). */
/**
 * Apply translation, rotation and scale based off the velocities of the particle.
 * Velocity is set from forward velocity.
 * The particle always moves forward in the local coordinate system
 * DKR: move_particle_forward.
 */
void move_particle_forward(Particle *particle) {
    s32 i = 0;

    while (i++ < gParticleUpdateRate) {
        particle->velocity.x = 0.0f;
        particle->velocity.y = 0.0f;
        particle->velocity.z = -particle->forwardVel;
        func_8004944C(&particle->rotation, &particle->velocity);
        particle->position.x += particle->velocity.x;
        particle->position.y += particle->velocity.y - particle->gravity;
        particle->position.z += particle->velocity.z;
        particle->scale += particle->scaleVelocity;
        particle->rotation.y_rotation += particle->angularVelocity.y_rotation;
        particle->rotation.x_rotation += particle->angularVelocity.x_rotation;
        particle->rotation.z_rotation += particle->angularVelocity.z_rotation;
    }
}

INCLUDE_ASM("asm/nonmatchings/5B700", func_80063358);


extern WeatherParticleList *D_801008E0[5];
extern s32 D_800A5DD8;
void set_active_camera(s32 arg0);
void func_8004046C(Gfx **dList, Mtx **matrix);
void func_8005D93C(Gfx **dList, Vertex **vtx, WeatherParticleList *list);
void func_80062A90(Gfx **dList, Vertex **vtx);
void func_8005D2E8(Gfx **dList, Mtx **matrix, Vertex **vtx, WeatherParticleList *list);
void func_8005D660(Gfx **dList, Mtx **matrix, Vertex **vtx, WeatherParticleList *list);

/* JFG: render all the particles into the current vertex buffer, then flip buffers (0x80063730). */
void render_all_particles(Gfx **dList, Mtx **matrix) {
    Vertex *vtx = D_800A5D8C[D_800A5DD8];

    gDPPipeSync((*dList)++);
    set_active_camera(0);
    func_8004046C(dList, matrix);
    func_8005D93C(dList, &vtx, D_801008E0[0]);
    func_8005D93C(dList, &vtx, D_801008E0[1]);
    func_80062A90(dList, &vtx);
    func_80062A00(dList, &vtx);
    func_8005D2E8(dList, matrix, &vtx, D_801008E0[2]);
    func_8005D660(dList, matrix, &vtx, D_801008E0[3]);
    D_800A5DD8 ^= 1;
}

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
            if (p->kind != 0x80 && p->unk2E == 3) {
                p->pos.x += obj->pos.x;
                p->pos.y += obj->pos.y;
                p->pos.z += obj->pos.z;
                p->unk48 = 0;
            }
        }
    }
}
