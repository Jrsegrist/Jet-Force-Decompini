#ifndef MODULES_GFX_H
#define MODULES_GFX_H

/*
 * The game's own vertex and triangle formats and the microcode commands that
 * load them, as the modules use them. Vertex, Triangle and gSPPolygon are
 * Diddy Kong Racing's (DKR decomp, include/structs.h and include/f3ddkr.h,
 * CC0); JFG's vertex command has a different length field from DKR's.
 */

#include "ultra64.h"

/* Size 10 */
typedef struct Vertex {
    /* 0x00 */ s16 x;
    /* 0x02 */ s16 y;
    /* 0x04 */ s16 z;
    /* 0x06 */ u8 r;
    /* 0x07 */ u8 g;
    /* 0x08 */ u8 b;
    /* 0x09 */ u8 a;
} Vertex;

/* Size 0x10 */
typedef struct Triangle {
    /* 0x00 */ u8 flags; /* 0x40: draw the back face */
    /* 0x01 */ u8 vi0;
    /* 0x02 */ u8 vi1;
    /* 0x03 */ u8 vi2;
    /* 0x04 */ s16 uv[3][2];
} Triangle;

#define G_TRIN 5

/* Loads n vertices at v into the vertex buffer from index v0 */
#define gSPVertexJFG(pkt, v, n, v0) \
    gDma1p(pkt, G_VTX, v, (n) * 10 - 2, ((n) - 1) << 3 | ((u32) (v) & 6) | (v0))

/* Draws numTris triangles at ptr (DKR) */
#define gSPPolygon(dl, ptr, numTris, texEnabled)                                                                   \
    {                                                                                                              \
        Gfx *_g = (Gfx *) (dl);                                                                                    \
        _g->words.w0 =                                                                                             \
            _SHIFTL((((numTris) - 1) << 4) | (texEnabled), 16, 8) | _SHIFTL(G_TRIN, 24, 8) | _SHIFTL(((numTris) * 16), 0, 16); \
        _g->words.w1 = (unsigned int) (ptr);                                                                       \
    }

#endif
