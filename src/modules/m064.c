/*
 * Runlink module 64: an object made of rotating beams of light. Each beam
 * turns at its own speed, leaves a fading trail of up to eight past positions
 * that is drawn as a strip of triangles, can carry a light, and has a glow
 * sprite drawn at its end.
 */
#include "module.h"
#include "gfx.h"

/* One past position of a beam (and the start of a beam). Size 0x18 */
typedef struct M064Segment {
    /* 0x00 */ Vec3s rot;
    /* 0x06 */ s8 life;
    /* 0x07 */ u8 alpha;
    /* 0x08 */ f32 width;
    /* 0x0C */ Vec3f pos;
} M064Segment;

/* An object light as far as this module uses it */
typedef struct M064Light {
    /* 0x00 */ u8 pad0[0x18];
    /* 0x18 */ Vec3f pos;
} M064Light;

/* Size 0x258 */
typedef struct M064Beam {
    /* 0x000 */ Vec3s rot;
    /* 0x006 */ s16 length;
    /* 0x008 */ f32 width;
    /* 0x00C */ Vec3f pos; /* its end */
    /* 0x018 */ s16 endWidth;
    /* 0x01A */ Vec3s rotSpeed;
    /* 0x020 */ u8 r;
    /* 0x021 */ u8 g;
    /* 0x022 */ u8 b;
    /* 0x023 */ u8 alpha;
    /* 0x024 */ f32 unk24;
    /* 0x028 */ M064Light *light;
    /* 0x02C */ u8 trailHead;
    /* 0x02D */ u8 trailTail;
    /* 0x02E */ u8 trailCount;
    /* 0x02F */ u8 buffer; /* which vertex set is current */
    /* 0x030 */ M064Segment trail[8];
    /* 0x0F0 */ Vertex verts[2][18];
} M064Beam;

typedef struct M064State {
    /* 0x0 */ u8 count;
    /* 0x2 */ s16 alpha;
    /* 0x4 */ M064Beam *beams;
} M064State;

/* Size 0xE */
typedef struct M064BeamEntry {
    /* 0x0 */ s8 rot[3];
    /* 0x3 */ s8 rotSpeed[3];
    /* 0x6 */ u8 r;
    /* 0x7 */ u8 g;
    /* 0x8 */ u8 b;
    /* 0x9 */ u8 width; /* tenths */
    /* 0xA */ u8 endWidth;
    /* 0xB */ u8 length; /* quarters */
    /* 0xC */ u16 flags; /* 1: has a light */
} M064BeamEntry;

typedef struct M064Setup {
    LevelObjectEntryCommon common;
    /* 0x0A */ u8 count;
    /* 0x0B */ u8 alpha;
    /* 0x0C */ M064BeamEntry beams[1];
} M064Setup;

typedef struct M064Header {
    /* 0x00 */ u8 pad0[0xAC];
    /* 0xAC */ s32 *glow;
} M064Header;

extern void *func_8004A57C(s32 size, s32 colour); /* DKR mempool_alloc_safe */
extern M064Light *func_800210A4(Object *obj, LightParams *params); /* DKR light_add_from_object_header */
extern void light_intensity_set(M064Light *light, s32 intensity);
/* rotates count vectors by rot */
extern void func_800496B0(s32 count, Vec3s *rot, void *in, void *out);
extern void mtx_pop(Gfx **dl);
extern void func_80055C00(Gfx **dl, void *tex, s32 flags, s32 offset); /* sets a texture */
extern void func_8005600C(s32 r, s32 g, s32 b, s32 r2, s32 g2, s32 b2);
extern void func_80056064(void);
extern void render_sprite_billboard(Gfx **dl, s32 arg1, s32 arg2, M064Beam *beam, s32 sprite, s32 arg5, s32 alpha);

/* The trail's strip: eighteen vertices, sixteen triangles */
static Triangle sTrailTris[16] = {
    { 0x40, 0, 1, 2, { { 0, 0 }, { 0, 0x1E0 }, { 0x100, 0 } } },
    { 0x40, 1, 2, 3, { { 0, 0x1E0 }, { 0x100, 0 }, { 0x100, 0x1E0 } } },
    { 0x40, 2, 3, 4, { { 0x100, 0 }, { 0x100, 0x1E0 }, { 0x200, 0 } } },
    { 0x40, 3, 4, 5, { { 0x100, 0x1E0 }, { 0x200, 0 }, { 0x200, 0x1E0 } } },
    { 0x40, 4, 5, 6, { { 0x200, 0 }, { 0x200, 0x1E0 }, { 0x300, 0 } } },
    { 0x40, 5, 6, 7, { { 0x200, 0x1E0 }, { 0x300, 0 }, { 0x300, 0x1E0 } } },
    { 0x40, 6, 7, 8, { { 0x300, 0 }, { 0x300, 0x1E0 }, { 0x400, 0 } } },
    { 0x40, 7, 8, 9, { { 0x300, 0x1E0 }, { 0x400, 0 }, { 0x400, 0x1E0 } } },
    { 0x40, 8, 9, 10, { { 0x400, 0 }, { 0x400, 0x1E0 }, { 0x500, 0 } } },
    { 0x40, 9, 10, 11, { { 0x400, 0x1E0 }, { 0x500, 0 }, { 0x500, 0x1E0 } } },
    { 0x40, 10, 11, 12, { { 0x500, 0 }, { 0x500, 0x1E0 }, { 0x600, 0 } } },
    { 0x40, 11, 12, 13, { { 0x500, 0x1E0 }, { 0x600, 0 }, { 0x600, 0x1E0 } } },
    { 0x40, 12, 13, 14, { { 0x600, 0 }, { 0x600, 0x1E0 }, { 0x700, 0 } } },
    { 0x40, 13, 14, 15, { { 0x600, 0x1E0 }, { 0x700, 0 }, { 0x700, 0x1E0 } } },
    { 0x40, 14, 15, 16, { { 0x700, 0 }, { 0x700, 0x1E0 }, { 0x800, 0 } } },
    { 0x40, 15, 16, 17, { { 0x700, 0x1E0 }, { 0x800, 0 }, { 0x800, 0x1E0 } } },
};

/* Adds the two vertices across a segment */
static void beam_add_edge(Vertex **v, M064Segment *seg, s32 alpha) {
    Vec3f pts[2];

    pts[0].x = -seg->width;
    pts[0].y = 0.0f;
    pts[0].z = 0.0f;
    pts[1].x = seg->width;
    pts[1].y = 0.0f;
    pts[1].z = 0.0f;
    func_800496B0(2, &seg->rot, pts, pts);
    pts[0].x += seg->pos.x;
    pts[0].y += seg->pos.y;
    pts[0].z += seg->pos.z;
    pts[1].x += seg->pos.x;
    pts[1].y += seg->pos.y;
    pts[1].z += seg->pos.z;
    (*v)->x = pts[0].x;
    (*v)->y = pts[0].y;
    (*v)->z = pts[0].z;
    (*v)->r = 0xFF;
    (*v)->g = 0xFF;
    (*v)->b = 0xFF;
    (*v)->a = alpha;
    (*v)++;
    (*v)->x = pts[1].x;
    (*v)->y = pts[1].y;
    (*v)->z = pts[1].z;
    (*v)->r = 0xFF;
    (*v)->g = 0xFF;
    (*v)->b = 0xFF;
    (*v)->a = alpha;
    (*v)++;
}

/* Builds the trail's vertices, from the beam's end back through its trail */
static void beam_build_verts(M064Beam *beam) {
    s32 i;
    f32 width;
    Vertex *v;

    if (beam->trailCount != 0) {
        beam->buffer ^= 1;
        v = beam->verts[beam->buffer];
        width = beam->width;
        beam->width = beam->endWidth;
        beam_add_edge(&v, (M064Segment *) beam, beam->alpha);
        beam->width = width;
        i = beam->trailTail;
        do {
            i--;
            if (i < 0) {
                i = 7;
            }
            beam_add_edge(&v, &beam->trail[i], beam->trail[i].alpha);
        } while (i != beam->trailHead);
    }
}

/* Ages the trail, adds the beam's last position to it and turns the beam */
static void beam_update(Object *obj, M064Beam *beam, s32 updateRate) {
    M064State *state = obj->unk68;
    M064Segment *seg;
    M064Light *light;
    s32 count;
    s32 i;

    count = beam->trailCount;
    if (count != 0) {
        i = beam->trailHead;
        do {
            seg = &beam->trail[i];
            i++;
            if (i >= 8) {
                i = 0;
            }
            seg->life--;
            if (seg->life < 0) {
                beam->trailHead = i;
                beam->trailCount--;
            } else {
                seg->alpha = (seg->life * 36 * beam->alpha) >> 8;
            }
        } while (i != beam->trailTail);
        count = beam->trailCount;
    }
    beam->alpha = state->alpha;
    if (count < 8 && beam->alpha != 0) {
        seg = &beam->trail[beam->trailTail];
        beam->trailCount = count + 1;
        beam->trailTail++;
        if (beam->trailTail >= 8) {
            beam->trailTail = 0;
        }
        seg->rot.y_rotation = beam->rot.y_rotation;
        seg->rot.x_rotation = beam->rot.x_rotation;
        seg->rot.z_rotation = beam->rot.z_rotation;
        seg->life = 7;
        seg->alpha = (seg->life * 36 * beam->alpha) >> 8;
        seg->width = beam->endWidth;
        seg->pos.x = beam->pos.x;
        seg->pos.y = beam->pos.y;
        seg->pos.z = beam->pos.z;
    }
    beam->rot.y_rotation += beam->rotSpeed.y_rotation * updateRate;
    beam->rot.x_rotation += beam->rotSpeed.x_rotation * updateRate;
    beam->rot.z_rotation += beam->rotSpeed.z_rotation * updateRate;
    beam->pos.x = 0.0f;
    beam->pos.y = beam->length;
    beam->pos.z = 0.0f;
    func_800496B0(1, &beam->rot, &beam->pos, &beam->pos);
    beam->pos.x += obj->trans.x_position;
    beam->pos.y += obj->trans.y_position;
    beam->pos.z += obj->trans.z_position;
    if ((light = beam->light) != NULL) {
        light->pos.x = beam->pos.x;
        light->pos.y = beam->pos.y;
        light->pos.z = beam->pos.z;
        light_intensity_set(light, beam->alpha);
    }
}

static void beam_tick(Object *obj, M064Beam *beam, s32 updateRate) {
    beam_update(obj, beam, updateRate);
    beam_build_verts(beam);
}

void func_m064_00614(Object *obj, M064Setup *setup) {
    M064State *state = obj->unk68;
    M064Beam *beam;
    LightParams lp;
    M064BeamEntry *entry;
    s32 i;

    if ((state->beams = func_8004A57C(setup->count * sizeof(M064Beam), 0xFF0000FF)) != NULL) {
        beam = state->beams;
        state->count = setup->count;
        entry = setup->beams;
        state->alpha = setup->alpha;
        i = setup->count;
        while (i--) {
            beam->rot.y_rotation = entry->rot[0] << 8;
            beam->rot.x_rotation = entry->rot[1] << 8;
            beam->rot.z_rotation = entry->rot[2] << 8;
            beam->length = entry->length << 2;
            beam->width = entry->width * 0.1f;
            beam->pos.x = 0.0f;
            beam->pos.y = beam->length;
            beam->pos.z = 0.0f;
            func_800496B0(1, &beam->rot, &beam->pos, &beam->pos);
            beam->pos.x += obj->trans.x_position;
            beam->pos.y += obj->trans.y_position;
            beam->pos.z += obj->trans.z_position;
            beam->endWidth = entry->endWidth;
            beam->rotSpeed.y_rotation = entry->rotSpeed[0] << 4;
            beam->rotSpeed.x_rotation = entry->rotSpeed[1] << 4;
            beam->rotSpeed.z_rotation = entry->rotSpeed[2] << 4;
            beam->r = entry->r;
            beam->g = entry->g;
            beam->b = entry->b;
            beam->alpha = setup->alpha;
            beam->unk24 = 0.0f;
            if (entry->flags & 1) {
                lp.type = 1;
                lp.unk1 = 3;
                lp.flags = entry->flags;
                lp.unk3 = -1;
                lp.unk4 = 0;
                lp.unk6 = 0;
                lp.unk8 = 0;
                lp.unkA = 0;
                lp.radius = 0xC0;
                lp.radius2 = 0xC0;
                lp.r = beam->r;
                lp.g = beam->g;
                lp.b = beam->b;
                lp.a = beam->alpha;
                lp.unk14 = -1;
                lp.unk16 = 0;
                lp.unk17 = 0;
                beam->light = func_800210A4(NULL, &lp);
            } else {
                beam->light = NULL;
            }
            beam->trailHead = 0;
            beam->trailTail = 0;
            beam->trailCount = 0;
            beam->buffer = 1;
            entry++;
            beam++;
        }
    } else {
        state->count = 0;
    }
    obj->level_entry = NULL;
}

void func_m064_008E0(Object *obj, s32 updateRate) {
    M064State *state = obj->unk68;
    M064Beam *beam = state->beams;
    s32 i = state->count;

    while (i--) {
        beam_tick(obj, beam++, updateRate);
    }
}

/*
 * gSPVertexJFG with the count given as the last index (n - 1) and the length
 * written in shifts, as this module computes them from a run-time count
 */
#define gSPVertexJFGLast(pkt, v, last, v0) \
    gDma1p(pkt, G_VTX, v, ((last) << 3) + ((last) << 1) + 8, (last) << 3 | ((u32) (v) & 6) | (v0))

/* Draws a beam's trail */
static void beam_draw_trail(Gfx **dl, M064Beam *beam) {
    s32 count = beam->trailCount;
    s32 numTris;
    s32 numVerts;

    if (count != 0) {
        numTris = count * 2;
        numVerts = numTris + 2;
        gDPSetPrimColor((*dl)++, 0, 0, 255, 255, 255, 255);
        gDPSetEnvColor((*dl)++, beam->r, beam->g, beam->b, 255);
        gSPVertexJFGLast((*dl)++, OS_K0_TO_PHYSICAL(beam->verts[beam->buffer]), numVerts, 0);
        gSPPolygon((*dl)++, OS_K0_TO_PHYSICAL(sTrailTris), numTris, 1);
    }
}

/* Draws the glow at a beam's end */
static void beam_draw_glow(Gfx **dl, s32 arg1, s32 arg2, M064Beam *beam, s32 sprite) {
    if (beam->alpha != 0) {
        func_8005600C(255, 255, 255, beam->r, beam->g, beam->b);
        render_sprite_billboard(dl, arg1, arg2, beam, sprite, 14, beam->alpha);
        func_80056064();
    }
}

void func_m064_00B10(Gfx **dl, s32 arg1, s32 arg2, Object *obj) {
    M064State *state = obj->unk68;
    M064Beam *beam;
    s32 i;

    mtx_pop(dl);
    func_80055C00(dl, *obj->modelInstances, 0x213, 0);
    i = state->count;
    beam = state->beams;
    while (i--) {
        beam_draw_trail(dl, beam++);
    }
    i = state->count;
    beam = state->beams;
    while (i--) {
        beam_draw_glow(dl, arg1, arg2, beam++, *((M064Header *) obj->header)->glow);
    }
    gDPPipeSync((*dl)++);
    gDPSetPrimColor((*dl)++, 0, 0, 255, 255, 255, 255);
    gDPSetEnvColor((*dl)++, 255, 255, 255, 255);
}
