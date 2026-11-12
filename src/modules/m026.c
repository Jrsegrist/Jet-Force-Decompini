/*
 * Runlink module 26: the particle system's set-up (JFG's version of DKR's
 * init_particle_assets and init_particle_buffers, src/particles.c). It
 * builds the triangle and outline tables of the 2- to 8-sided particle
 * shapes, loads the particle and behaviour tables, makes each colour-cycling
 * particle's colour table, and allocates the vertex buffers, the particle
 * lists, the line and point sets, the dummy textures and the emitter pool.
 */
#include "module.h"
#include "gfx.h"

/* A particle in a list (size 0x78) */
typedef struct M026Particle {
    /* 0x00 */ u8 pad0[0x2C];
    /* 0x2C */ s16 kind;
    /* 0x2E */ u8 pad2E[0x4C - 0x2E];
    /* 0x4C */ void *asset;
    /* 0x50 */ u8 pad50[0x78 - 0x50];
} M026Particle;

/* A list of particles with a bit per free slot (size 0x38, then the particles and the bits) */
typedef struct M026List {
    /* 0x00 */ s32 unk0;
    /* 0x04 */ s32 unk4;
    /* 0x08 */ s32 unk8;
    /* 0x0C */ void *unkC;
    /* 0x10 */ s32 unk10;
    /* 0x14 */ M026Particle *particles;
    /* 0x18 */ s32 count;
    /* 0x1C */ s32 unk1C;
    /* 0x20 */ u32 *freeBits;
    /* 0x24 */ s32 lastWord;
    /* 0x28 */ s32 unk28;
    /* 0x2C */ u8 (*unk2C)[16];
    /* 0x30 */ u8 (*unk30)[16];
    /* 0x34 */ u8 (*unk34)[16];
} M026List;

/* A line emitter's segments (size 0x148) */
typedef struct M026LineItem {
    /* 0x00 */ u8 pad0[0x1C];
} M026LineItem;
typedef struct M026LineSet {
    /* 0x000 */ s32 unk0;
    /* 0x004 */ M026LineItem *items[9];
    /* 0x028 */ M026LineItem itemData[9];
    /* 0x124 */ u8 unk124;
    /* 0x125 */ u8 pad125[0x148 - 0x125];
} M026LineSet;

/* A point emitter's particles (size 0xC0) */
typedef struct M026PointSet {
    /* 0x00 */ s32 points[15];
    /* 0x3C */ s32 count;
    /* 0x40 */ u8 pad40[0xA4 - 0x40];
    /* 0xA4 */ u8 unkA4;
    /* 0xA5 */ u8 padA5[0xC0 - 0xA5];
} M026PointSet;

/* The free-emitter pool's entries (size 0x28) */
typedef struct M026PoolEmitter {
    /* 0x00 */ u8 pad0[0x24];
    /* 0x24 */ u8 active;
    /* 0x25 */ u8 pad25[3];
} M026PoolEmitter;

/* A particle descriptor, as far as this module uses it */
typedef struct M026Descriptor {
    /* 0x00 */ s32 flags;
    /* 0x04 */ u8 pad4[0x1F - 0x4];
    /* 0x1F */ u8 colourCount;
    /* 0x20 */ u32 *colours;
    /* 0x24 */ u16 colourCycle;
} M026Descriptor;

/* A particle behaviour, as far as this module uses it */
typedef struct M026Behaviour {
    /* 0x00 */ u8 pad0[0x9C];
    /* 0x9C */ s32 colourLoop;
} M026Behaviour;

/* The colour cycle of 55130.c (colour_cycle_init, update_colour_cycle) */
typedef struct M026ColourCycle {
    /* 0x00 */ s32 unk0;
    /* 0x04 */ s32 unk4;
    /* 0x08 */ u8 colour[4];
    /* 0x0C */ s32 *source; /* the count of colours at 4 */
} M026ColourCycle;

#define COLOUR_TAG_SEMITRANS_GREY 0x80808080

extern Triangle *D_800A5F48[7][2]; /* the triangles of the 2- to 8-sided shapes (front, back) */
extern f32 *D_800A5F80[7];         /* their outline coordinates */
extern M026List *D_801008E0[5];
extern M026Descriptor **D_800A5D9C;
extern s32 D_800A5D94;
extern s32 D_800A5D98;
extern M026Behaviour **D_800A5DA8;
extern s32 D_800A5DA0;
extern s32 D_800A5DA4;
extern s32 D_800A5D7C;
extern s32 D_800A5D80;
extern M026LineSet *D_800A5D84;
extern M026PointSet *D_800A5D88;
extern Vertex *D_800A5D8C[2];
extern void **D_800A5D74;
extern s32 D_800A5D78;
extern M026PoolEmitter *D_800A5DAC;
extern s32 D_800A5DB0;
extern u8 D_800A5F9C[];
extern u8 D_800A5FA8[][16];
extern u8 D_800A6128[];
extern u8 D_800A6138[][16];

extern void *mempool_alloc_safe(s32 size, s32 colour);
extern void mempool_free(void *data);
extern s32 *func_8004D37C(s32 asset); /* DKR asset_table_load */
extern s32 get_misc_asset(s32 index);
extern void colour_cycle_init(M026ColourCycle *cycle, s32 assetID);
extern void update_colour_cycle(M026ColourCycle *cycle, s32 updateRate);
extern void reset_particles(void);
extern void *tex_load_sprite(s32 id, s32 arg1);
extern void *load_texture(s32 id);
extern void *object_model_init(s32 model, s32 arg1); /* loads a model instance */

void func_m026_00518(void);
static M026List *particle_list_new(void *arg0, u8 (*arg1)[16], s32 arg2, s32 arg3, s32 count);
static M026LineSet *line_sets_new(s32 count);
static M026PointSet *point_sets_new(s32 count);

/*
 * Builds the triangles and outline points of the 2- to 8-sided shapes.
 * (`first` is a plain copy; with it IDO keeps &D_800A5F48 in a2 for the first store, as the original.)
 */
static void build_particle_shapes(void) {
    Triangle *front;
    Triangle *first;
    Triangle *back;
    f32 *coords;
    s32 i;
    s32 sides;
    s16 angle;
    s32 n;
    f32 x;
    f32 y;

    D_800A5F48[0][0] = mempool_alloc_safe(0x880, COLOUR_TAG_SEMITRANS_GREY);
    first = D_800A5F48[0][0];
    front = first;
    back = front + 68;
    for (sides = 2; sides < 9; sides++) {
        D_800A5F48[sides - 2][0] = front;
        D_800A5F48[sides - 2][1] = back;
        n = sides - 1;
        for (i = 0; i < n; i++) {
            front[0].flags = 0x40;
            front[0].vi0 = i;
            front[0].vi1 = i + 1;
            front[0].vi2 = sides + 1 + i;
            front[0].uv[0][0] = 0;
            front[0].uv[0][1] = 0;
            front[0].uv[1][0] = 0x200;
            front[0].uv[1][1] = 0;
            front[0].uv[2][0] = 0x200;
            front[0].uv[2][1] = 0x200;
            front[1].flags = 0x40;
            front[1].vi0 = i;
            front[1].vi1 = sides + 1 + i;
            front[1].vi2 = sides + i;
            front[1].uv[0][0] = 0;
            front[1].uv[0][1] = 0;
            front[1].uv[1][0] = 0x200;
            front[1].uv[1][1] = 0x200;
            front[1].uv[2][0] = 0;
            front[1].uv[2][1] = 0x200;
            front += 2;
            back[0].flags = 0x40;
            back[0].vi0 = sides + i;
            back[0].vi1 = sides + 1 + i;
            back[0].vi2 = i + 1;
            back[0].uv[0][0] = 0;
            back[0].uv[0][1] = 0;
            back[0].uv[1][0] = 0x200;
            back[0].uv[1][1] = 0;
            back[0].uv[2][0] = 0x200;
            back[0].uv[2][1] = 0x200;
            back[1].flags = 0x40;
            back[1].vi0 = sides + i;
            back[1].vi1 = i + 1;
            back[1].vi2 = i;
            back[1].uv[0][0] = 0;
            back[1].uv[0][1] = 0;
            back[1].uv[1][0] = 0x200;
            back[1].uv[1][1] = 0x200;
            back[1].uv[2][0] = 0;
            back[1].uv[2][1] = 0x200;
            back += 2;
        }
        if (sides >= 3) {
            front[0].flags = 0x40;
            front[0].vi0 = i;
            front[0].vi1 = 0;
            front[0].vi2 = sides;
            front[0].uv[0][0] = 0;
            front[0].uv[0][1] = 0;
            front[0].uv[1][0] = 0x200;
            front[0].uv[1][1] = 0;
            front[0].uv[2][0] = 0x200;
            front[0].uv[2][1] = 0x200;
            front[1].flags = 0x40;
            front[1].vi0 = i;
            front[1].vi1 = sides;
            front[1].vi2 = i + sides;
            front[1].uv[0][0] = 0;
            front[1].uv[0][1] = 0;
            front[1].uv[1][0] = 0x200;
            front[1].uv[1][1] = 0x200;
            front[1].uv[2][0] = 0;
            front[1].uv[2][1] = 0x200;
            front += 2;
            back[0].flags = 0x40;
            back[0].vi0 = i + sides;
            back[0].vi1 = sides;
            back[0].vi2 = 0;
            back[0].uv[0][0] = 0;
            back[0].uv[0][1] = 0;
            back[0].uv[1][0] = 0x200;
            back[0].uv[1][1] = 0;
            back[0].uv[2][0] = 0x200;
            back[0].uv[2][1] = 0x200;
            back[1].flags = 0x40;
            back[1].vi0 = i + sides;
            back[1].vi1 = 0;
            back[1].vi2 = i;
            back[1].uv[0][0] = 0;
            back[1].uv[0][1] = 0;
            back[1].uv[1][0] = 0x200;
            back[1].uv[1][1] = 0x200;
            back[1].uv[2][0] = 0;
            back[1].uv[2][1] = 0x200;
            back += 2;
        }
    }
    coords = mempool_alloc_safe(0x118, COLOUR_TAG_SEMITRANS_GREY);
    for (sides = 2; sides != 9; sides++) {
        angle = 0;
        D_800A5F80[sides - 2] = coords;
        for (i = 0; i < sides; i++) {
            x = func_80049C30(angle);
            y = func_80049C34(angle);
            *coords++ = (x + x) + (y + y);
            *coords++ = (x + x) - (y + y);
            angle += (s16) (0xFFFF / sides);
        }
    }
}

/* Loads the particle and behaviour tables (DKR init_particle_assets) */
void func_m026_002F8(void) {
    s32 i;

    for (i = 0; i < 5; i++) {
        D_801008E0[i] = NULL;
    }
    D_800A5D9C = (M026Descriptor **) func_8004D37C(0x35);
    while ((s32) D_800A5D9C[D_800A5D94 + 1] != -1) {
        D_800A5D94++;
    }
    D_800A5D98 = (s32) func_8004D37C(0x36);
    for (i = 0; i < D_800A5D94; i++) {
        D_800A5D9C[i] = (M026Descriptor *) ((s32) D_800A5D9C[i] + D_800A5D98);
    }
    func_m026_00518();
    D_800A5DA8 = (M026Behaviour **) func_8004D37C(0x37);
    while ((s32) D_800A5DA8[D_800A5DA0 + 1] != -1) {
        D_800A5DA0++;
    }
    D_800A5DA4 = (s32) func_8004D37C(0x38);
    for (i = 0; i < D_800A5DA0; i++) {
        D_800A5DA8[i] = (M026Behaviour *) ((s32) D_800A5DA8[i] + D_800A5DA4);
        if ((u32) D_800A5DA8[i]->colourLoop != 0xFFFFFFFF) {
            D_800A5DA8[i]->colourLoop = get_misc_asset((u32) D_800A5DA8[i]->colourLoop);
        } else {
            D_800A5DA8[i]->colourLoop = 0;
        }
    }
}

#ifdef NON_MATCHING
/*
 * Makes the colour table of each colour-cycling particle (shared between particles with the same cycle).
 * The last `|` of the colour word comes out with its operands swapped.
 */
void func_m026_00518(void) {
    s32 j;
    M026Descriptor *desc;
    s32 count;
    s32 i;
    M026ColourCycle cycle;
    u32 *colours;

    for (i = 0; i < D_800A5D94; i++) {
        desc = D_800A5D9C[i];
        if (desc->flags & 0x400) {
            for (j = 0; j < i; j++) {
                if (desc->colourCycle == D_800A5D9C[j]->colourCycle) {
                    desc->colours = D_800A5D9C[j]->colours;
                    desc->colourCount = cycle.source[1];
                    if (desc->colourCount == 0) {
                        desc->colourCount = 1;
                    }
                    j = i;
                }
            }
            if (desc->colours == NULL) {
                colour_cycle_init(&cycle, desc->colourCycle);
                count = cycle.source[1];
                if (count == 0) {
                    count = 1;
                }
                if (count >= 0x100) {
                    stubbed_printf("Particle warning: colour cycle table exceeds 255 entries\n");
                    count = 0xFF;
                }
                desc->colours = mempool_alloc_safe(count * 4, COLOUR_TAG_SEMITRANS_GREY);
                desc->colourCount = count;
                colours = desc->colours;
                for (j = 0; j < count; j++) {
                    *colours++ = (cycle.colour[0] << 24) | (cycle.colour[1] << 16) | (cycle.colour[2] << 8) |
                                 (u32) cycle.colour[3];
                    update_colour_cycle(&cycle, 1);
                }
            }
        }
    }
}
#else
/* the format string of the stubbed print above */
static char sColourWarning[] = "Particle warning: colour cycle table exceeds 255 entries\n";
INCLUDE_ASM("asm/nonmatchings/modules/m026", func_m026_00518);
#endif

/* Allocates the particle buffers (DKR init_particle_buffers) */
void func_m026_006E8(s32 maxTriangles, s32 maxRectangles, s32 maxSprites, s32 maxLines, s32 maxPoints,
                     s32 maxModels, s32 maxPool) {
    s32 vertices;
    s32 i;
    s32 id;
    s16 *assets;

    reset_particles();
    if (maxTriangles < 0) {
        maxTriangles = 0x10;
    }
    if (maxRectangles < 0) {
        maxRectangles = 0x10;
    }
    if (maxSprites < 0) {
        maxSprites = 0xD0;
    }
    if (maxModels < 0) {
        maxModels = 0xF;
    }
    if (maxLines < 0) {
        maxLines = 0xA0;
    }
    if (maxPoints < 0) {
        maxPoints = 0x40;
    }
    if (maxPool < 0) {
        maxPool = 0x20;
    }
    vertices = (maxTriangles * 3) + (maxRectangles * 4) + maxSprites + (maxLines * 18) + (maxPoints * 120);
    D_800A5D7C = maxLines;
    D_800A5D80 = maxPoints;
    D_800A5D8C[0] = mempool_alloc_safe(vertices * 2 * sizeof(Vertex), COLOUR_TAG_SEMITRANS_GREY);
    D_800A5D8C[1] = D_800A5D8C[0] + vertices;
    D_801008E0[0] = particle_list_new(D_800A5F9C, D_800A5FA8, 3, 1, maxTriangles);
    D_801008E0[1] = particle_list_new(D_800A6128, D_800A6138, 4, 2, maxRectangles);
    D_801008E0[2] = particle_list_new(NULL, NULL, 0, 0, maxSprites);
    D_801008E0[3] = particle_list_new(NULL, NULL, 0, 0, maxModels);
    D_800A5D88 = point_sets_new(maxPoints);
    D_800A5D84 = line_sets_new(D_800A5D7C);
    build_particle_shapes();
    if (D_800A5D74 == NULL) {
        assets = (s16 *) func_8004D37C(0x3C);
        D_800A5D78 = 0;
        while (assets[D_800A5D78] != -1) {
            D_800A5D78++;
        }
        D_800A5D74 = mempool_alloc_safe(D_800A5D78 * 4, COLOUR_TAG_SEMITRANS_GREY);
        for (i = 0; i < D_800A5D78; i++) {
            id = assets[i];
            switch (id & 0xC000) {
                case 0x8000:
                    D_800A5D74[i] = tex_load_sprite(id & 0x3FFF, 1);
                    break;
                case 0xC000:
                    D_800A5D74[i] = load_texture(id & 0x3FFF);
                    break;
                case 0:
                    D_800A5D74[i] = object_model_init(id, 0);
                    break;
            }
        }
        mempool_free(assets);
    }
    D_800A5DAC = mempool_alloc_safe(maxPool * sizeof(M026PoolEmitter), COLOUR_TAG_SEMITRANS_GREY);
    for (i = 0; i < maxPool; i++) {
        D_800A5DAC[i].active = FALSE;
    }
    D_800A5DB0 = maxPool;
}

/* Allocates a particle list of `count` particles with all slots free */
static M026List *particle_list_new(void *arg0, u8 (*arg1)[16], s32 arg2, s32 arg3, s32 count) {
    M026List *list;
    s32 words;
    s32 rest;
    s32 size;
    s32 i;
    s32 n;
    u32 bit;
    UNUSED s32 pad;

    words = count >> 5;
    rest = count & 0x1F;
    if (rest != 0) {
        words++;
    }
    size = count * sizeof(M026Particle) + sizeof(M026List);
    size = (size - (size & 3)) + sizeof(u32);
    list = mempool_alloc_safe(size + words * sizeof(u32), COLOUR_TAG_SEMITRANS_GREY);
    list->count = count;
    list->freeBits = (u32 *) ((u8 *) list + size);
    list->particles = (M026Particle *) ((u8 *) list + sizeof(M026List));
    list->lastWord = words - 1;
    list->unk28 = 0;
    list->unk1C = 0;
    list->unk8 = arg3;
    list->unk4 = arg2;
    list->unkC = arg0;
    if (arg1 != NULL) {
        n = (24 / arg2) * arg3;
        if (n > 16) {
            n = 16;
        }
        list->unk2C = arg1;
        list->unk30 = arg1 + n;
        list->unk34 = arg1 + n * 2;
    } else {
        list->unk2C = NULL;
        list->unk30 = NULL;
        list->unk34 = NULL;
    }
    for (i = 0; i < count; i++) {
        list->particles[i].asset = NULL;
        list->particles[i].kind = 0x80;
    }
    for (i = 0; i <= list->lastWord; i++) {
        list->freeBits[i] = -1;
    }
    bit = 1;
    if (rest != 0) {
        list->freeBits[list->lastWord] = 0;
        for (i = rest; i > 0; i--) {
            list->freeBits[list->lastWord] |= bit;
            bit <<= 1;
        }
    }
    return list;
}

static M026LineSet *line_sets_new(s32 count) {
    M026LineSet *sets;
    M026LineSet *set;
    s32 i;
    s32 j;

    sets = mempool_alloc_safe(count * sizeof(M026LineSet), COLOUR_TAG_SEMITRANS_GREY);
    set = sets;
    for (i = 0; i < count; i++) {
        set->unk0 = 0;
        set->unk124 = 0;
        set->items[0] = &set->itemData[0];
        for (j = 1; j < 9; j++) {
            set->items[j] = &set->itemData[j];
        }
        set++;
    }
    return sets;
}

static M026PointSet *point_sets_new(s32 count) {
    UNUSED s32 pad;
    UNUSED s32 pad2;
    M026PointSet *sets;
    M026PointSet *set;
    s32 i;
    s32 j;

    sets = mempool_alloc_safe(count * sizeof(M026PointSet), COLOUR_TAG_SEMITRANS_GREY);
    set = sets;
    for (i = 0; i < count; i++) {
        set->count = 0;
        set->unkA4 = 0;
        for (j = 0; j < 15; j++) {
            set->points[j] = 0;
        }
        set++;
    }
    D_801008E0[4] = particle_list_new(NULL, NULL, 0, 0, count * 15);
    return sets;
}
