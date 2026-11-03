/*
 * Runlink module 137: reflections in the level's reflective floors. The
 * first function collects the heights of the level's reflective surfaces
 * (up to 15 different ones), the second picks the objects to reflect (up to
 * 60, each with a value from the visibility test), and the third (still in
 * assembly) draws them.
 */
#include "module.h"
#include "gfx.h"

/* A batch of a level segment's triangles (size 0x10) */
typedef struct M137Batch {
    /* 0x00 */ u8 pad0[0x6];
    /* 0x06 */ s16 verticesOffset;
    /* 0x08 */ s16 facesOffset;
    /* 0x0A */ u8 padA[0xC - 0xA];
    /* 0x0C */ u32 flags; /* 0x20000000: reflective */
} M137Batch;

/* A level segment (size 0x48) */
typedef struct M137Segment {
    /* 0x00 */ struct Vertex *vertices;
    /* 0x04 */ struct Triangle *triangles;
    /* 0x08 */ u8 pad8[0xC - 0x8];
    /* 0x0C */ M137Batch *batches;
    /* 0x10 */ u8 pad10[0x28 - 0x10];
    /* 0x28 */ s16 numberOfBatches;
    /* 0x2A */ u8 pad2A[0x48 - 0x2A];
} M137Segment;

typedef struct M137Level {
    /* 0x00 */ u8 pad0[0x4];
    /* 0x04 */ M137Segment *segments;
    /* 0x08 */ u8 pad8[0x1A - 0x8];
    /* 0x1A */ s16 numberOfSegments;
} M137Level;

/* An object picked for reflecting */
typedef struct M137Reflected {
    /* 0x0 */ Object *obj;
    /* 0x4 */ f32 value;
} M137Reflected;

/* What objects keep at 0x88 */
typedef struct M137ObjState {
    /* 0x0 */ u8 pad0[0xA];
    /* 0xA */ u8 reflected;
} M137ObjState;

/* What obj->header points at here */
typedef struct M137Header {
    /* 0x00 */ u8 pad0[0x1E];
    /* 0x1E */ s8 unk1E;
} M137Header;

/* An object's model instance */
typedef struct M137Model {
    /* 0x00 */ u8 pad0[0x94];
    /* 0x94 */ s16 height;
} M137Model;

extern ObjectTransform *func_80041DE0(void); /* the camera */
extern s32 track_height_at(f32 x, f32 z, f32 *y, s32 flags);
static s32 is_seen(Object *obj, ObjectTransform *camera, f32 *value);

static u8 sDrawData[0x80];
static s32 sSurfaceCount;
static s32 sReflectedCount;
static M137Reflected sReflected[60];
static s16 sSurfaceHeights[15];

#ifdef NON_MATCHING
/* the loop invariants get t-registers instead of s0-s4, and (vi0 + offset) * 10 is distributed */
void func_m137_00000(M137Level *level) {
    s32 i;
    s32 j;
    s32 k;
    s32 isNew;
    s16 height;
    s32 next;
    M137Segment *seg;
    M137Batch *batch;

    sSurfaceCount = 0;
    for (i = 0; i < level->numberOfSegments; i++) {
        seg = &level->segments[i];
        for (j = 0; j < seg->numberOfBatches; j++) {
            batch = &seg->batches[j];
            if (batch->flags & 0x20000000) {
                height = seg->vertices[seg->triangles[batch->facesOffset].vi0 + batch->verticesOffset].y;
                isNew = TRUE;
                next = sSurfaceCount + 1;
                for (k = 0; k < sSurfaceCount; k++) {
                    if (height == sSurfaceHeights[k]) {
                        isNew = FALSE;
                    }
                }
                if (isNew) {
                    sSurfaceHeights[sSurfaceCount] = height;
                    sSurfaceCount = next;
                    if (next >= 15) {
                        j = seg->numberOfBatches;
                        i = level->numberOfSegments;
                    }
                }
            }
        }
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m137", func_m137_00000);
#endif

void func_m137_0016C(s32 count, Object **objs) {
    ObjectTransform *camera;
    s32 i;
    f32 value;
    Object *obj;

    camera = func_80041DE0();
    sReflectedCount = 0;
    i = count - 1;
    while (i >= 0) {
        obj = objs[i--];
        if (is_seen(obj, camera, &value)) {
            OBJ_STATE(obj, M137ObjState)->reflected = TRUE;
            sReflected[sReflectedCount].obj = obj;
            sReflected[sReflectedCount].value = value;
            sReflectedCount++;
            if (sReflectedCount >= 60) {
                i = -1;
            }
        } else {
            OBJ_STATE(obj, M137ObjState)->reflected = FALSE;
        }
    }
}

INCLUDE_ASM("asm/nonmatchings/modules/m137", func_m137_0025C);

/*
 * Whether obj should be drawn mirrored: it stands on one of the surfaces, or
 * its mirror image under one of them can be seen from the camera
 */
static s32 is_seen(Object *obj, ObjectTransform *camera, f32 *value) {
    UNUSED s32 pad[2];
    HitResult hit;
    Vec3f pos;
    f32 height;
    s32 seen;
    s32 i;
    f32 h;
    M137Model *model;
    f32 d;

    if (((M137Header *) obj->header)->unk1E == 0) {
        model = ((M137Model **) obj->modelInstances)[obj->modelIndex];
        height = model->height * obj->trans.scale;
    } else {
        height = 0.0f;
    }
    seen = FALSE;
    if ((track_height_at(obj->trans.x_position, obj->trans.z_position, value, 0x20000000) & 0x20000000) &&
        *value - 5.0f < obj->trans.y_position) {
        seen = TRUE;
    } else {
        pos.x = obj->trans.x_position;
        pos.z = obj->trans.z_position;
        for (i = 0; i < sSurfaceCount; i++) {
            h = sSurfaceHeights[i];
            d = obj->trans.y_position - h;
            pos.y = h - d - height;
            if (func_80018808(NULL, (Vec3f *) &camera->x_position, &pos, &hit, 0, 0x20000000)) {
                seen = TRUE;
                *value = hit.pos.y;
                i = sSurfaceCount;
            }
        }
    }
    return seen;
}

INCLUDE_ASM("asm/nonmatchings/modules/m137", data);
