/*
 * Weather. Functions in C here are from the Diddy Kong Racing decompilation
 * (CC0 1.0), src/weather.c; see CREDITS.md.
 */
#include "common.h"

/* DKR src/weather.h and include/structs.h (reduced) */
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

extern SnowLimits gSnowPlane; // Uses negative values, so all distances will be checked in reverse.
extern WeatherData gWeather;
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
/* JFG layout (55130.c has more) */
struct TextureHeader {
    /* 0x00 */ u8 pad0[0x12];
    /* 0x12 */ u16 numOfTextures;
};

/* DKR src/weather.h. Size: 0x2C bytes */
typedef struct SnowGfxData {
    /* 0x00 */ void *pos;
    /* 0x04 */ s32 unk4; // DKR: size
    /* 0x08 */ TextureHeader *texture; // DKR: a union with the type in the presets
    /* 0x0C */ s32 offsetX;
    /* 0x10 */ s32 offsetY;
    /* 0x14 */ s32 offsetZ;
    /* 0x18 */ s32 radiusX;
    /* 0x1C */ s32 radiusY;
    /* 0x20 */ s32 radiusZ;
    /* 0x24 */ s16 vertOffsetW;
    /* 0x26 */ s16 vertOffsetH;
    /* 0x28 */ s16 vertWidth;
    /* 0x2A */ s16 vertHeight;
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

/* DKR include/structs.h. Size: 0x10 bytes */
typedef struct SnowPosData {
    /* 0x00 */ s32 x_position;
    /* 0x04 */ s32 y_position;
    /* 0x08 */ s32 z_position;
    /* 0x0C */ s8 unused_C;
    /* 0x0D */ s8 unused_D;
    /* 0x0E */ s8 unused_E;
    /* 0x0F */ u8 index;
} SnowPosData;

/* DKR include/structs.h (Triangle). Size: 0x10 bytes */
typedef struct SnowTriangle {
    /* 0x00 */ u8 flags;
    /* 0x01 */ u8 vi0;
    /* 0x02 */ u8 vi1;
    /* 0x03 */ u8 vi2;
    /* 0x04 */ s16 u0, v0;
    /* 0x08 */ s16 u1, v1;
    /* 0x0C */ s16 u2, v2;
} SnowTriangle;

/* DKR include/structs.h (the start of Vertex) */
typedef struct SnowVertex {
    /* 0x00 */ s16 x, y, z;
    /* 0x06 */ u8 r, g, b, a;
} SnowVertex;

/* DKR include/structs.h (the start of TextureHeader) */
typedef struct SnowTexture {
    /* 0x00 */ u8 width;
    /* 0x01 */ u8 height;
} SnowTexture;

extern SnowGfxData D_800A5940[]; // DKR: gWeatherPresets
void *mempool_alloc_safe(s32 size, u32 colourTag);
void rain_init(s32 density, s32 intensity, s32 opacity);
void snow_init(void); // DKR: snow_init
s32 rand_range(s32 min, s32 max);

#ifdef NON_MATCHING
/* NON_MATCHING (16 words, ROM length and frame; was 58): the target loads preset->texture before storing
   gSnowGfx.pos, materialises &gSnowTriangles for its store, and gives the triangle indices v0/a1 (mine a1/a2).
   numOfElements declared last and reused for density * 4 (as DKR) gives the frame: its second value is spilled to
   its own word. */
/**
 * Free all existing weather data.
 * If the weather type allows rain, then allocate and initialise rain.
 * Afterwards, allocate and set the defaults for snow model data.
 * JFG: the whole preset is copied, rain_init also takes the density, and the triangle count is fixed.
 * DKR: weather_reset
 */
void func_8005AD90(s32 weatherType, s32 density, s32 velX, s32 velY, s32 velZ, s32 intensity, s32 opacity) {
    s16 width;
    s16 height;
    s32 allocSize;
    s32 i;
    SnowGfxData *preset;
    SnowPosData *pos;
    SnowVertex *verts;
    s32 j;
    SnowTriangle *tris;
    u8 *src;
    u8 *dst;
    u8 n;
    s32 numOfElements;

    weather_free();
    gWeather.velX = velX;
    gWeather.velXStep = 0;
    gWeather.velXTarget = velX;
    gWeather.velY = velY;
    gWeather.velYStep = 0;
    gWeather.velYTarget = velY;
    gWeather.velZStep = 0;
    gWeather.intensityStep = 0;
    gWeather.opacityStep = 0;
    gWeather.shiftTime = 0;
    gWeather.velZ = velZ;
    gWeather.velZTarget = velZ;
    gWeather.intensity = intensity;
    gWeather.intensityTarget = intensity;
    gWeather.opacity = opacity;
    gWeather.opacityTarget = opacity;
    if (weatherType > WEATHER_RAIN) {
        weatherType = WEATHER_RAIN;
    }
    preset = &D_800A5940[weatherType];
    if ((s32) preset->texture == WEATHER_RAIN) {
        rain_init(density, intensity + 1, opacity + 1);
        return;
    }
    src = (u8 *) preset;
    dst = (u8 *) &gSnowGfx;
    n = sizeof(SnowGfxData) - 1;
    do {
        *dst++ = *src++;
    } while (n--);
    gSnowGfx.pos = mempool_alloc_safe(preset->unk4 * 12, 0xAA55FFFF);
    if ((s32) preset->texture == WEATHER_SNOW) {
        snow_init();
    }
    numOfElements = density;
    gSnowDensity = numOfElements;
    gSnowTriIndices = mempool_alloc_safe(numOfElements * 2, 0xAA55FFFF);
    gSnowPhysics = mempool_alloc_safe(numOfElements * sizeof(SnowPosData), 0xAA55FFFF);
    pos = gSnowPhysics;
    for (i = 0; i < gSnowDensity; i++) {
        pos->x_position = rand_range(0, gSnowGfx.radiusX);
        pos->y_position = rand_range(0, gSnowGfx.radiusY);
        pos->z_position = rand_range(0, gSnowGfx.radiusZ);
        pos->unused_C = 1 << (rand_range(0, 32) + 5);
        pos->unused_D = 1 << (rand_range(0, 32) + 5);
        pos->unused_E = 1 << (rand_range(0, 32) + 5);
        pos->index = rand_range(0, gSnowGfx.unk4 - 1);
        pos++;
        if (density) {}
    }
    numOfElements = numOfElements * 4;
    allocSize = sizeof(SnowVertex);
    allocSize *= numOfElements;
    gSnowVertexData[0] = mempool_alloc_safe(allocSize, 0xAA55FFFF);
    gSnowVertexData[1] = mempool_alloc_safe(allocSize, 0xAA55FFFF);
    for (j = 0; j < 2; j++) {
        verts = (SnowVertex *) gSnowVertexData[j];
        for (i = 0; i < numOfElements; i++) {
            verts->r = 255;
            verts->g = 255;
            verts->b = 255;
            verts->a = 255;
            verts++;
        }
    }
    width = (((SnowTexture *) gSnowGfx.texture)->width << 5) - 1;
    height = (((SnowTexture *) gSnowGfx.texture)->height << 5) - 1;
    gSnowTriangles = tris = mempool_alloc_safe(gSnowTriCount * sizeof(SnowTriangle), 0xAA55FFFF);
    for (i = 0; i < gSnowTriCount; i += 2) {
        tris[i].flags = 0;
        tris[i].vi0 = (i << 1) + 3;
        tris[i].u0 = 0;
        tris[i].v0 = height;
        tris[i].vi1 = (i << 1) + 1;
        tris[i].u1 = width;
        tris[i].v1 = 0;
        tris[i].vi2 = (i << 1) + 0;
        tris[i].u2 = 0;
        tris[i].v2 = 0;
        tris[i + 1].flags = 0;
        tris[i + 1].vi0 = (i << 1) + 3;
        tris[i + 1].u0 = 0;
        tris[i + 1].v0 = height;
        tris[i + 1].vi1 = (i << 1) + 2;
        tris[i + 1].u1 = width;
        tris[i + 1].v1 = height;
        tris[i + 1].vi2 = (i << 1) + 1;
        tris[i + 1].u2 = width;
        tris[i + 1].v2 = 0;
    }
    gSnowVertexFlip = 0;
}
#else
INCLUDE_ASM("asm/nonmatchings/5B700", func_8005AD90);
#endif

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

/* DKR include/structs.h, size 10 */
struct Vertex {
    /* 0x00 */ s16 x, y, z;
    /* 0x06 */ u8 r, g, b, a;
};

/* DKR include/f3ddkr.h (JFG: the length is numTris << 4, which IDO does not fold into an expression) */
#define G_TRIN 5
#define gSPPolygon(dl, ptr, numTris, texEnabled)                                                        \
    {                                                                                                   \
        Gfx *_g = (Gfx *) (dl);                                                                         \
        _g->words.w0 = _SHIFTL((((numTris) - 1) << 4) | (texEnabled), 16, 8) | _SHIFTL(G_TRIN, 24, 8) | \
                       _SHIFTL(((numTris) << 4), 0, 16);                                                \
        _g->words.w1 = (unsigned int) (ptr);                                                            \
    }
/* JFG: DKR's gSPSelectMatrixDKR(pkt, 0) */
#define gSPSelectMatrix0(pkt)      \
    {                              \
        Gfx *_g = (Gfx *) (pkt);   \
        _g->words.w0 = 0xBC00000A; \
        _g->words.w1 = 0;          \
    }
/* JFG: loads n + 1 vertices (this file's form of the vertex command) */
#define gSPVertexN1(pkt, v, n) gDma1p(pkt, G_VTX, v, (n) * 8 + (n) * 2 + 8, ((n) * 8) | ((u32) (v) & 6))

extern s32 D_800A59F8; // DKR: gSnowVertCount
Mtx *get_projection_matrix_s16(void);
void func_80055C00(Gfx **dList, TextureHeader *tex, s32 flags, s32 frame);
void func_80048C00(); // DKR: mtxf_transform_dir (mtx, in, out)

struct ObjectSegment {
    /* 0x00 */ s16 y_rotation;
    /* 0x02 */ u8 pad2[0xC - 2];
    /* 0x0C */ Vec3f pos;
};

/**
 * Build a camera facing quad (4 vertices) for every snowflake that is close enough.
 * The snowflake positions wrap around the camera inside the snow box.
 * JFG: the quad half sizes are read once, before the loop.
 * DKR: snow_vertices
 */
#ifdef NON_MATCHING
/* NON_MATCHING: register allocation (the quad half sizes and the vertex count pointer swap s-registers) */
void snow_vertices(void) {
    s32 posX;
    s32 posY;
    s32 posZ;
    Vec3f posF;
    s32 i;
    Vertex *verts;
    s32 camX;
    s32 camY;
    s32 camZ;
    s16 offsetW;
    s16 offsetH;

    offsetW = gSnowGfx.vertOffsetW;
    if (offsetW) {}
    offsetH = gSnowGfx.vertOffsetH;
    camX = gWeatherCamera->pos.x * 65536.0f;
    camY = gWeatherCamera->pos.y * 65536.0f;
    camZ = gWeatherCamera->pos.z * 65536.0f;
    D_800A59F8 = 0;
    verts = gSnowVerts;
    for (i = 0; i < gSnowParticleCount; i++) {
        posF.x = (((((SnowPosData *) gSnowPhysics)[i].x_position - camX) & gSnowGfx.radiusX) + gSnowGfx.offsetX) *
                 (1.0f / 65536.0f);
        posF.y = (((((SnowPosData *) gSnowPhysics)[i].y_position - camY) & gSnowGfx.radiusY) + gSnowGfx.offsetY) *
                 (1.0f / 65536.0f);
        posF.z = (((((SnowPosData *) gSnowPhysics)[i].z_position - camZ) & gSnowGfx.radiusZ) + gSnowGfx.offsetZ) *
                 (1.0f / 65536.0f);
        func_80048C00(gWeatherCameraMatrix, &posF, &posF);
        posZ = posF.z;
        if ((s16) posZ < gSnowPlane.near && gSnowPlane.current < (s16) posZ) {
            posY = posF.y;
            posX = posF.x;
            verts[0].x = posX - offsetW;
            verts[0].y = posY + offsetH;
            verts[0].z = posZ;
            verts[1].x = posX + offsetW;
            verts[1].y = posY + offsetH;
            verts[1].z = posZ;
            verts[2].x = posX + offsetW;
            verts[2].y = posY - offsetH;
            verts[2].z = posZ;
            verts[3].x = posX - offsetW;
            verts[3].y = posY - offsetH;
            D_800A59F8 += 4;
            verts[3].z = posZ;
            ((s16 *) gSnowTriIndices)[D_800A59F8 >> 2] = i;
            verts += 4;
        }
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/5B700", func_8005B62C);
#endif

/**
 * Load and execute the draw commands for the falling snowflakes, seen with snowy weather enabled.
 * JFG: tinted with fixed primitive and environment colours, reset to white afterwards.
 * DKR: snow_render
 */
void snow_render(void) {
    s32 i;
    Mtx *mtx;
    Vertex *vtx;

    if (gSnowGfx.texture != NULL) {
        gSnowVertOffset = 4;
        gSnowTriCount = 2;
        if (D_800A59F8 >= 4) {
            i = 0;
            mtx = get_projection_matrix_s16();
            gSPMatrix(gCurrWeatherDisplayList++, (u8 *) mtx + 0x80000000, 0);
            gSPSelectMatrix0(gCurrWeatherDisplayList++);
            func_80055C00(&gCurrWeatherDisplayList, gSnowGfx.texture, 2, 0);
            gDPSetPrimColor(gCurrWeatherDisplayList++, 0, 0, 0xE0, 0xF0, 0xFF, 0xA0);
            gDPSetEnvColor(gCurrWeatherDisplayList++, 0x40, 0x80, 0xFF, 0x00);
            while (i + gSnowVertOffset < D_800A59F8) {
                vtx = &gSnowVerts[i];
                gSPVertexN1(gCurrWeatherDisplayList++, (u8 *) vtx + 0x80000000, gSnowVertOffset);
                gSPPolygon(gCurrWeatherDisplayList++, (u8 *) gSnowTriangles + 0x80000000, gSnowTriCount, 1);
                i += gSnowVertOffset;
            }
            vtx = &gSnowVerts[i];
            gSPVertexN1(gCurrWeatherDisplayList++, (u8 *) vtx + 0x80000000, D_800A59F8 - i);
            gSPPolygon(gCurrWeatherDisplayList++, (u8 *) gSnowTriangles + 0x80000000, (s32) (D_800A59F8 - i) >> 1, 1);
            gDPSetPrimColor(gCurrWeatherDisplayList++, 0, 0, 0xFF, 0xFF, 0xFF, 0xFF);
            gDPSetEnvColor(gCurrWeatherDisplayList++, 0xFF, 0xFF, 0xFF, 0xFF);
        }
    }
}

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

/**
 * Sets the fade times for the rain visuals and how long to take to fade in.
 * The game will slowly ease into the intended values.
 * JFG: f32 constants.
 * DKR: rain_set
 */
void rain_set(s32 lightningFrequency, s32 opacity, f32 time) {
    gRainHiddenTimer = osTvType == OS_TV_PAL ? 50.0f * time : 60.0f * time;
    gLightningFrequencyTarget = lightningFrequency;
    gLightningFrequencyStep = (gLightningFrequencyTarget - gLightningFrequency) / gRainHiddenTimer;
    gRainOpacityTarget = opacity;
    gRainOpacityStep = (gRainOpacityTarget - gRainOpacity) / gRainHiddenTimer;
}

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

/* JFG: gRainOpacity scaled by D_800A5D1C, as a fraction clamped to 0.0..1.0 (0x8005BFC4). */
f32 rain_amount_get(void) {
    f32 amount = (f32) ((s32) ((gRainOpacity >> 2) * D_800A5D1C) >> 14) / 65536.0f;

    if (amount < 0.0f) {
        amount = 0.0f;
    }
    if (amount > 1.0f) {
        amount = 1.0f;
    }
    return amount;
}

/* JFG: a weather position in 16.16 fixed point */
typedef struct WeatherPosition {
    /* 0x00 */ u8 pad0[0xC];
    /* 0x0C */ s32 x;
    /* 0x10 */ u8 pad10[0x18 - 0x10];
    /* 0x18 */ s32 y;
    /* 0x1C */ u8 pad1C[0x24 - 0x1C];
    /* 0x24 */ s32 z;
} WeatherPosition;
extern WeatherPosition D_80100848;
/* The module trap (0x80054530) under a prototype with float arguments: needs
   `func_80054530_fff = 0x80054530;` in undefined_syms.us.txt. */
void func_80054530_fff(f32 x, f32 y, f32 z, s32 updateRate);
void rain_sound(s32 updateRate);
void func_8005C188(s32 updateRate);
void rain_lightning(s32 updateRate);

/**
 * When active (single player only), call all the functions related to the behaviour and rendering of
 * rain, and if necessary, thunder.
 * JFG: a module draws the rain (at the weather position, 5 units lower).
 * DKR: rain_update
 */
void rain_update(s32 updateRate) {
    if (get_current_viewport() == 0 && gWeatherType != WEATHER_SNOW) {
        if (gRainHiddenTimer > 0) {
            if (updateRate < gRainHiddenTimer) {
                gRainHiddenTimer -= updateRate;
                gLightningFrequency += gLightningFrequencyStep * updateRate;
                gRainOpacity += gRainOpacityStep * updateRate;
            } else {
                gRainHiddenTimer = 0;
                gLightningFrequency = gLightningFrequencyTarget;
                gRainOpacity = gRainOpacityTarget;
            }
        }
        func_80054530_fff(D_80100848.x / (f32) 0x10000, D_80100848.y / (f32) 0x10000 - 5.0f,
                          D_80100848.z / (f32) 0x10000, updateRate);
        rain_sound(updateRate);
        func_8005C188(updateRate);
        rain_lightning(updateRate);
    }
}

/* JFG: a rain splash or ripple, size 0x2C */
typedef struct RainSplash {
    /* 0x00 */ s16 y_rotation, x_rotation, z_rotation;
    /* 0x06 */ s16 flags; // 0: free, 1: a splash sprite, 2: a ripple on water
    /* 0x08 */ f32 scale;
    /* 0x0C */ f32 x_position;
    /* 0x10 */ f32 y_position;
    /* 0x14 */ f32 z_position;
    /* 0x18 */ s16 opacity;
    /* 0x1A */ u8 pad1A[0x28 - 0x1A];
    /* 0x28 */ f32 animFrame;
} RainSplash;

/* JFG: a floor or hit height entry: the height first */
typedef struct SplashHeight {
    /* 0x00 */ f32 y;
    /* 0x04 */ u8 pad4[0x13 - 4];
    /* 0x13 */ s8 surfaceType; // 2: water
} SplashHeight;

typedef struct SplashRacer {
    /* 0x00 */ u8 pad0[0xC];
    /* 0x0C */ f32 x_position;
    /* 0x10 */ f32 y_position;
    /* 0x14 */ f32 z_position;
} SplashRacer;

extern RainSplash D_800A5A14[16];
/* JFG: the ripple's corner offsets, x then z */
extern f32 D_800A5CF8, D_800A5CFC, D_800A5D00, D_800A5D04;
extern f32 D_800A5D08, D_800A5D0C, D_800A5D10, D_800A5D14;
extern u8 D_800A5CD8[];   // JFG: the ripple's two triangles
SplashRacer *get_racer_object_by_port(s32 port);
s32 func_8001B078(f32 x, f32 z, s32 flags, SplashHeight ***heights);
s32 func_80082D80(f32 x, f32 z, s32 flags, SplashHeight ***heights);
void render_sprite_billboard(Gfx **dList, Mtx **mtx, Vertex **verts, RainSplash *splash, SpriteAsset *sprite, s32 flags,
                             s32 alpha);
f32 func_80049C30(s32 angle); // cosine of a 16-bit angle
f32 func_80049C34(s32 angle); // sine of a 16-bit angle

#ifdef NON_MATCHING
/**
 * If it's raining hard enough, start new rain splashes at random spots near the player: a splash sprite on
 * the ground, or a ripple on water. Every live splash is then drawn, a sprite as a billboard and a ripple
 * as a growing textured quad.
 * JFG: 16 splashes, a ripple kind, the floor and hit heights instead of the wave heights.
 * DKR: rain_render_splashes
 * NON_MATCHING: 69 words: splash and onHit swap s1/s2 (and loop 2's 0xFF with them) and one FPR pair
 * in the ripple vertices; the three empty ifs on always-set values (i, intensity) are levers that got
 * it from 492 words to 71.
 */
void func_8005C188(s32 updateRate) {
    s32 intensity;
    SplashRacer *racer;
    RainSplash *splash;
    s32 i;
    s32 found;
    s32 angle;
    f32 dist;
    f32 x;
    f32 z;
    s32 onFloor;
    s32 onHit;
    s32 pad[2];
    SplashHeight **floorHeights;
    SplashHeight **hitHeights;

    if (gRainSplashGfx == NULL || D_800A5D48 == NULL) {
        return;
    }
    intensity = (s32) ((gRainOpacity >> 2) * D_800A5D1C) >> 14;
    if (intensity > 0x4000) {
        racer = get_racer_object_by_port(0);
        if (racer != NULL) {
            gRainSplashDelay -= updateRate;
            if (intensity) {}
            if (gRainSplashDelay <= 0) {
                do {
                    i = 16;
                    found = FALSE;
                    splash = D_800A5A14;
                    do {
                        i--;
                        if (splash->flags == 0) {
                            found = TRUE;
                        } else {
                            splash++;
                        }
                    } while (i > 0 && !found);
                    if (found) {
                        angle = rand_range(0, 0x10000);
                        dist = rand_range(30, 300);
                        x = func_80049C34(angle) * dist + racer->x_position;
                        z = func_80049C30(angle) * dist + racer->z_position;
                        onFloor = func_8001B078(x, z, 0x400, &floorHeights);
                        onHit = func_80082D80(x, z, 0x400, &hitHeights);
                        if (onFloor || onHit) {
                            splash->x_position = x;
                            splash->z_position = z;
                            splash->flags = 1;
                            splash->animFrame = 0.0f;
                            splash->opacity = rand_range(0x40, (intensity >> 10) + 0x60);
                            if (onFloor && onHit) {
                                onFloor = 0;
                                if (i) {}
                            }
                            if (onFloor) {
                                splash->y_position = floorHeights[0]->y;
                                if (floorHeights[0]->surfaceType == 2) {
                                    splash->flags++;
                                }
                            } else {
                                splash->y_position = hitHeights[0]->y;
                            }
                        }
                    }
                    gRainSplashDelay += 2;
                    if (gRainSplashDelay >= 0) {
                        gRainSplashDelay = gRainSplashDelay - (intensity >> 10) + 0x40;
                        if (gRainSplashDelay < 0) {
                            gRainSplashDelay = 0;
                        }
                    }
                } while (gRainSplashDelay <= 0);
            }
        }
    }

    gDPSetEnvColor(gCurrWeatherDisplayList++, 255, 255, 255, 0);
    splash = D_800A5A14;
    for (i = 0; i < 16; i++, splash++) {
        if (i) {}
        if (splash->flags != 0) {
            splash->animFrame += updateRate * 0.175f;
            if (splash->animFrame < 4.0f) {
                if (splash->flags == 1) {
                    gDPSetPrimColor(gCurrWeatherDisplayList++, 0, 0, 255, 255, 255, splash->opacity);
                    render_sprite_billboard(&gCurrWeatherDisplayList, &gCurrWeatherMatrix, &gCurrWeatherVertexList,
                                            splash, gRainSplashGfx, 0xE, 0);
                } else {
                    func_80055C00(&gCurrWeatherDisplayList, D_800A5D48, 0xE, 0);
                    gDPSetPrimColor(gCurrWeatherDisplayList++, 0, 0, 192, 224, 255, 255);
                    gSPVertexN1(gCurrWeatherDisplayList++, (u8 *) gCurrWeatherVertexList + 0x80000000, 4);
                    gSPPolygon(gCurrWeatherDisplayList++, D_800A5CD8 + 0x80000000, 2, 1);
                    gCurrWeatherVertexList->x = D_800A5CF8 * splash->animFrame + splash->x_position;
                    gCurrWeatherVertexList->y = splash->y_position;
                    gCurrWeatherVertexList->z = D_800A5D08 * splash->animFrame + splash->z_position;
                    gCurrWeatherVertexList->r = 255;
                    gCurrWeatherVertexList->g = 255;
                    gCurrWeatherVertexList->b = 255;
                    gCurrWeatherVertexList->a = 255;
                    gCurrWeatherVertexList++;
                    gCurrWeatherVertexList->x = D_800A5CFC * splash->animFrame + splash->x_position;
                    gCurrWeatherVertexList->y = splash->y_position;
                    gCurrWeatherVertexList->z = D_800A5D0C * splash->animFrame + splash->z_position;
                    gCurrWeatherVertexList->r = 255;
                    gCurrWeatherVertexList->g = 255;
                    gCurrWeatherVertexList->b = 255;
                    gCurrWeatherVertexList->a = 255;
                    gCurrWeatherVertexList++;
                    gCurrWeatherVertexList->x = D_800A5D00 * splash->animFrame + splash->x_position;
                    gCurrWeatherVertexList->y = splash->y_position;
                    gCurrWeatherVertexList->z = D_800A5D10 * splash->animFrame + splash->z_position;
                    gCurrWeatherVertexList->r = 255;
                    gCurrWeatherVertexList->g = 255;
                    gCurrWeatherVertexList->b = 255;
                    gCurrWeatherVertexList->a = 255;
                    gCurrWeatherVertexList++;
                    gCurrWeatherVertexList->x = D_800A5D04 * splash->animFrame + splash->x_position;
                    gCurrWeatherVertexList->y = splash->y_position;
                    gCurrWeatherVertexList->z = D_800A5D14 * splash->animFrame + splash->z_position;
                    gCurrWeatherVertexList->r = 255;
                    gCurrWeatherVertexList->g = 255;
                    gCurrWeatherVertexList->b = 255;
                    gCurrWeatherVertexList->a = 255;
                    gCurrWeatherVertexList++;
                }
            } else {
                splash->flags = 0;
            }
        }
    }
    gDPSetPrimColor(gCurrWeatherDisplayList++, 0, 0, 255, 255, 255, 255);
}
#else
INCLUDE_ASM("asm/nonmatchings/5B700", func_8005C188);
#endif

extern u8 gThunderTransition[];
extern s32 D_800A6784;
s32 check_fadeout_transition(void);
void transition_begin(void *transition); // DKR: transition_begin
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
                transition_begin(gThunderTransition);
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

/* Loads the snow texture (0x8005CBBC). */
void snow_init(void) {
    D_801008B8 = 0;
    D_801008BC = 0;
    D_801008C0 = 0;
    D_801008C4 = 0;
    D_800A59D0 = load_texture(*gWeatherAssetTable);
}

extern s32 D_801008C8; // JFG: the wind's target (x, y, z), picked again when D_801008B8 runs out
extern s32 D_801008CC;
extern s32 D_801008D0;
void func_8004944C(Vec3s *rotation, Vec3f *vec); // DKR: vec3f_rotate_py

/**
 * Snow drift: every few seconds pick a new random wind (yaw anywhere, pitched slightly down),
 * ease the current wind (D_801008BC) toward it, then move every snowflake by the wind plus the
 * weather velocity, wrapped inside the snow box.
 * JFG: not in DKR; DKR's snow_update moves the flakes by the velocity only.
 */
void snow_wind_update(s32 updateRate) {
    s32 curX;
    s32 curY;
    s32 curZ;
    s32 targetX;
    s32 targetY;
    s32 targetZ;
    s32 velX;
    s32 velY;
    s32 velZ;
    s32 sumX;
    s32 sumY;
    s32 sumZ;
    SnowPosData *pos;
    Vec3s angle;
    Vec3f vec;
    s32 i;

    D_801008B8 -= updateRate;
    if (D_801008B8 <= 0) {
        D_801008B8 = rand_range(0xB4, 0x12C);
        angle.y_rotation = rand_range(-0x8000, 0x7FFF);
        angle.x_rotation = rand_range(-0x800, -0x400);
        vec.z = -(f32) rand_range(0x28000, 0x58000);
        func_8004944C(&angle, &vec);
        D_801008C8 = vec.x;
        D_801008CC = vec.y;
        D_801008D0 = vec.z;
    }
    curX = D_801008BC;
    curY = D_801008C0;
    curZ = D_801008C4;
    targetX = D_801008C8;
    targetY = D_801008CC;
    targetZ = D_801008D0;
    velX = gWeather.velX;
    velY = gWeather.velY;
    velZ = gWeather.velZ;
    sumX = 0;
    sumY = 0;
    sumZ = 0;
    i = updateRate;
    while (i--) {
        curX += (targetX - curX) >> 6;
        curY += (targetY - curY) >> 6;
        curZ += (targetZ - curZ) >> 6;
        sumX += curX + velX;
        sumY += curY + velY;
        sumZ += curZ + velZ;
    }
    D_801008BC = curX;
    D_801008C0 = curY;
    D_801008C4 = curZ;
    sumX >>= 1;
    sumY >>= 1;
    sumZ >>= 1;
    targetX = gSnowGfx.radiusX;
    targetY = gSnowGfx.radiusY;
    targetZ = gSnowGfx.radiusZ;
    pos = gSnowPhysics;
    i = gSnowDensity;
    while (i--) {
        pos->x_position = (pos->x_position + sumX) & targetX;
        pos->y_position = (pos->y_position + sumY) & targetY;
        pos->z_position = (pos->z_position + sumZ) & targetZ;
        pos++;
    }
}
