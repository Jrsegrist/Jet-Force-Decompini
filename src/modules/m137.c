/*
 * Runlink module 137: reflections in the level's reflective floors. The
 * first function collects the heights of the level's reflective surfaces
 * (up to 15 different ones), the second picks the objects to reflect (up to
 * 60, each with a value from the visibility test), and the third draws them
 * mirrored (C for it is NON_MATCHING).
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
    s32 vi;
    s32 face;

    sSurfaceCount = 0;
    for (i = 0; i < level->numberOfSegments; i++) {
        seg = &level->segments[i];
        for (j = 0; j < seg->numberOfBatches; j++) {
            batch = &seg->batches[j];
            if (batch->flags & 0x20000000) {
                face = batch->facesOffset;
                vi = batch->verticesOffset;
                vi += seg->triangles[face].vi0;
                height = seg->vertices[vi].y;
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

#ifdef NON_MATCHING
/* An object's model instance as the drawing sees it */
typedef struct M137InstModel {
    /* 0x00 */ u8 pad0[0x11];
    /* 0x11 */ u8 animatesTextures;
    /* 0x12 */ u8 pad12[0x4E - 0x12];
    /* 0x4E */ s8 unk4E;
    /* 0x4F */ u8 pad4F[0x74 - 0x4F];
    /* 0x74 */ Gfx *dlOpaque;
    /* 0x78 */ Gfx *dlTranslucent;
} M137InstModel;

typedef struct M137Instance {
    /* 0x00 */ M137InstModel *model;
    /* 0x04 */ void *vertices;
    /* 0x08 */ u8 pad8[0xA - 0x8];
    /* 0x0A */ s8 unkA;
    /* 0x0B */ s8 matrixIndex;
    /* 0x0C */ u8 padC[0x10 - 0xC];
    /* 0x10 */ MtxF *matrices[2];
    /* 0x18 */ u8 pad18[0x78 - 0x18];
    /* 0x78 */ void *texAnims[2];
} M137Instance;

typedef struct M137Header2 {
    /* 0x00 */ u8 pad0[0x1E];
    /* 0x1E */ s8 kind;
    /* 0x1F */ s8 numberOfModelIds;
    /* 0x20 */ u8 pad20[0xAC - 0x20];
    /* 0xAC */ void **textures;
    /* 0xB0 */ u8 padB0[0xCC - 0xB0];
    /* 0xCC */ f32 lodDistance;
} M137Header2;

/* A player's behaviour data (obj->unk68 of behaviour 1) */
typedef struct M137Player {
    /* 0x000 */ s8 unk0;
    /* 0x001 */ s8 unk1;
    /* 0x002 */ u8 pad2[0x78 - 0x2];
    /* 0x078 */ MtxF weaponMtx;
    /* 0x0B8 */ u8 padB8[0x564 - 0xB8];
    /* 0x564 */ struct M137Pack *pack;
} M137Player;

typedef struct M137Pack {
    /* 0x000 */ u8 pad0[0x620];
    /* 0x620 */ u32 pad620 : 6;
    /* 0x620 */ u32 count : 6;
} M137Pack;

typedef struct M137Weapon {
    /* 0x00 */ u8 pad0[0x70];
    /* 0x70 */ u8 model;
} M137Weapon;

typedef struct M137Holder {
    /* 0x0 */ u8 pad0[0x4];
    /* 0x4 */ Object *weapon;
} M137Holder;

typedef struct M137Data40 {
    /* 0x00 */ u8 pad0[0xC];
    /* 0x0C */ Gfx *dlOpaque;
    /* 0x10 */ Gfx *dlTranslucent;
} M137Data40;

typedef struct M137DataA2 {
    /* 0x00 */ u8 pad0[0x63];
    /* 0x63 */ u8 unk63;
    /* 0x64 */ u8 pad64[0x6C - 0x64];
    /* 0x6C */ f32 unk6C;
} M137DataA2;

typedef struct M137InteractA2 {
    /* 0x00 */ u8 pad0[0xC];
    /* 0x0C */ Vec3f a;
    /* 0x18 */ Vec3f b;
} M137InteractA2;

typedef struct M137Data98 {
    /* 0x000 */ Triangle tris[6][16];
    /* 0x600 */ Vertex verts[6][18];
    /* 0xA38 */ u8 padA38[0xA54 - 0xA38];
    /* 0xA54 */ f32 unkA54;
    /* 0xA58 */ u8 padA58[0xA76 - 0xA58];
    /* 0xA76 */ s16 unkA76;
    /* 0xA78 */ s16 unkA78;
    /* 0xA7A */ u8 padA7A[0xA7F - 0xA7A];
    /* 0xA7F */ u8 unkA7F;
    /* 0xA80 */ u8 padA80[0xA83 - 0xA80];
    /* 0xA83 */ u8 colour;
} M137Data98;

typedef struct M137Texture {
    /* 0x0 */ u8 pad0[6];
    /* 0x6 */ s16 flags;
} M137Texture;

typedef struct M137Data28 {
    /* 0x000 */ Vertex verts[40]; /* two sets of 20 */
    /* 0x190 */ M137Texture *texture;
    /* 0x194 */ u8 pad194[0x1B4 - 0x194];
    /* 0x1B4 */ u8 envR;
    /* 0x1B5 */ u8 envG;
    /* 0x1B6 */ u8 envB;
    /* 0x1B7 */ u8 pad1B7[0x1C4 - 0x1B7];
    /* 0x1C4 */ u8 primR;
    /* 0x1C5 */ u8 primG;
    /* 0x1C6 */ u8 primB;
    /* 0x1C7 */ u8 pad1C7[0x1D0 - 0x1C7];
    /* 0x1D0 */ s32 frame;
    /* 0x1D4 */ u16 alpha;
    /* 0x1D6 */ u8 pad1D6[0x1DA - 0x1D6];
    /* 0x1DA */ u8 set;
} M137Data28;

extern s32 rl_m137_01750;        /* which of the two camera matrices this frame */
extern s32 rl_m137_01754[][2];   /* env colours */
extern u8 D_800A0838[];
extern u8 D_800F2D58[];
extern MtxF *get_view_proj_matrix_f32(void); /* the camera matrix */
extern void func_80048D84(MtxF *mf, Mtx *m);
extern f32 get_distance_to_camera(f32 x, f32 y, f32 z);
extern void func_80049DC0(ObjectTransform *trans, MtxF *out);
extern void func_8004A014(ObjectTransform *trans, MtxF *out);
extern void func_80049AE8(s32 angle, MtxF *mtx);
extern void func_80048C7C(MtxF *a, MtxF *b, MtxF *out);
extern M137Weapon *func_80047C1C(s32 a, s32 b);
extern void func_m095_0148C(Object *obj, struct M137Pack *pack, Gfx **dl, Mtx **mtx, s32 arg4);
extern void func_80055B80(Gfx **dl);
extern void func_80055C00(Gfx **dl, void *tex, s32 flags, s32 texOffset);
extern void func_8000A300(Gfx **dl, s32 arg1, M137Instance *inst, M137DataA2 *data, Vec3f *from, Vec3f *to, f32 arg6,
                          s32 arg7, s32 colour, s32 arg9);
extern void render_sprite_billboard_at(Gfx **dl, Mtx **mtx, s32 arg2, void *tex, s32 x, s32 y, s32 z, s32 angle, f32 size,
                          f32 arg9, s32 arg10, s32 arg11);
extern void func_8000DB94(Gfx **dl, Mtx **mtx, s32 arg2, Object *obj);

#define gSPVertexM137(pkt, v, n) \
    gDma1p(pkt, G_VTX, v, ((n) << 3) + ((n) << 1) + 8, (n) << 3 | ((u32) (v) & 6))
#define gSPPartMatrices(pkt, lo, mtx)      \
    {                                      \
        Gfx *_g = (Gfx *) (pkt);           \
        _g->words.w0 = 0xBF000000 | (lo);  \
        _g->words.w1 = (u32) (mtx);        \
    }
#define gSPCommand02(pkt, a)               \
    {                                      \
        Gfx *_g = (Gfx *) (pkt);           \
        _g->words.w0 = 0x02000050;         \
        _g->words.w1 = (u32) (a);          \
    }

/*
 * Draws the reflections: each picked object mirrored in the surface under
 * it (its model, a held weapon, special effects), then sets the camera
 * matrix back.
 */
/* (equivalent; not matched: stack layout and register allocation) */
void func_m137_0025C(Gfx **dl, Mtx **mtx, s32 arg2) {
    M137Instance *lodInst;
    M137Player *pd;
    MtxF *cam;
    Vec3f to;
    Vec3f from;
    MtxF inverse;
    MtxF objMtx;
    MtxF result;
    f32 surfaceY;
    f32 dist;
    f32 savedY;
    ObjectTransform inv;
    ObjectTransform trans;
    s32 needReset;
    s32 i;
    s32 hasTexAnims;
    void *tex;
    f32 reflY;
    M137Reflected *entry;
    s32 lod;
    Object *obj;
    M137Instance *inst;
    M137InstModel *model;
    s32 drawn;
    Gfx *dlist;
    s32 flag;
    s32 k;

    rl_m137_01750 ^= 1;
    cam = get_view_proj_matrix_f32();
    func_80048D84(cam, (Mtx *) sDrawData + rl_m137_01750);
    gSPSetGeometryMode((*dl)++, 0x1000);
    needReset = 0;
    i = 0;
    if (sReflectedCount > 0) {
        entry = sReflected;
        do {
            obj = entry->obj;
            i++;
            surfaceY = entry->value;
            dist = obj->trans.y_position - surfaceY;
            entry++;
            if (dist < 0.0f) {
                dist = -dist;
            }
            flag = obj->trans.flags & 0x400;
            if (!flag && ((M137Header2 *) obj->header)->kind == 0) {
                inst = ((M137Instance **) obj->modelInstances)[obj->modelIndex];
                model = inst->model;
                hasTexAnims = FALSE;
                lod = 0;
                drawn = FALSE;
                if (model->animatesTextures) {
                    hasTexAnims = TRUE;
                }
                if (((M137Header2 *) obj->header)->lodDistance > 0.0f) {
                    if (((M137Header2 *) obj->header)->lodDistance <
                        -get_distance_to_camera(obj->trans.x_position, obj->trans.y_position, obj->trans.z_position)) {
                        lod = 1;
                    }
                }
                if (obj->modelIndex + lod >= ((M137Header2 *) obj->header)->numberOfModelIds) {
                    lod = 0;
                }
                lodInst = ((M137Instance **) obj->modelInstances)[obj->modelIndex + lod];
                if (obj->opacity == 0xFF) {
                    dlist = lodInst->model->dlOpaque;
                } else {
                    dlist = lodInst->model->dlTranslucent;
                }
                if (model->unk4E == 0) {
                    trans.x_position = obj->trans.x_position;
                    trans.y_position = surfaceY - dist;
                    trans.z_position = obj->trans.z_position;
                    trans.scale = -obj->trans.scale;
                    trans.rotation.z_rotation = obj->trans.rotation.z_rotation;
                    trans.rotation.x_rotation = obj->trans.rotation.x_rotation;
                    trans.rotation.y_rotation = obj->trans.rotation.y_rotation + 0x8000;
                    func_80049DC0(&trans, &objMtx);
                    if (obj->behaviorId == 0x99) {
                        func_80049AE8(*(s16 *) ((u8 *) obj->unk68 + 0x78), &objMtx);
                    }
                    func_80048C7C(&objMtx, cam, &result);
                    func_80048D84(&result, *mtx);
                    gDma1p((*dl)++, G_MTX, (u32) *mtx + 0x80000000, sizeof(Mtx), 1);
                    drawn = TRUE;
                    (*mtx)++;
                } else if (inst->unkA == 0) {
                    inv.x_position = -obj->trans.x_position;
                    inv.y_position = -obj->trans.y_position;
                    inv.scale = 1.0f;
                    inv.z_position = -obj->trans.z_position;
                    inv.rotation.z_rotation = -obj->trans.rotation.z_rotation;
                    inv.rotation.x_rotation = -obj->trans.rotation.x_rotation;
                    inv.rotation.y_rotation = -obj->trans.rotation.y_rotation;
                    needReset = TRUE;
                    func_8004A014(&inv, &inverse);
                    trans.y_position = surfaceY - dist;
                    trans.x_position = obj->trans.x_position;
                    trans.z_position = obj->trans.z_position;
                    trans.scale = -1.0f;
                    trans.rotation.z_rotation = obj->trans.rotation.z_rotation;
                    trans.rotation.x_rotation = obj->trans.rotation.x_rotation;
                    trans.rotation.y_rotation = obj->trans.rotation.y_rotation + 0x8000;
                    func_80049DC0(&trans, &objMtx);
                    func_80048C7C(&inverse, &objMtx, &result);
                    func_80048C7C(&result, cam, &result);
                    func_80048D84(&result, *mtx);
                    gDma1p((*dl)++, G_MTX, (u32) *mtx + 0x80000000, sizeof(Mtx), 0);
                    drawn = TRUE;
                    (*mtx)++;
                }
                if (drawn) {
                    gDPSetPrimColor((*dl)++, 0, 0, 255, 255, 255, obj->opacity);
                    gSPPartMatrices((*dl)++, ((u32) inst->matrices[inst->matrixIndex] + 0x80000000) & 0xFFFFFF,
                                    (u32) lodInst->vertices + 0x80000000);
                    if (hasTexAnims) {
                        if (lod) {
                            gSPCommand02((*dl)++, D_800A0838 + 0x80000000);
                        } else {
                            gSPCommand02((*dl)++, (u32) inst->texAnims[inst->matrixIndex] + 0x80000000);
                        }
                    }
                    gSPDisplayList((*dl)++, (u32) dlist + 0x80000000);
                    gSPPartMatrices((*dl)++, 0, 0);
                    if (obj->behaviorId == 1 || obj->behaviorId == 0x33) {
                        MtxF *weaponMtx;
                        u32 weaponModel;
                        Object *weapon;
                        M137Instance *winst;
                        void *wtex;
                        Gfx *wdl;

                        if (obj->behaviorId == 1) {
                            pd = obj->unk68;
                            weaponModel = func_80047C1C(pd->unk0, pd->unk1)->model;
                            weaponMtx = &pd->weaponMtx;
                        } else {
                            weaponMtx = obj->unk68;
                            weaponModel = OBJ_STATE(obj, s32)[1];
                        }
                        if (*(M137Holder **) &obj->pad60[0] != NULL &&
                            (weapon = (*(M137Holder **) &obj->pad60[0])->weapon) != NULL) {
                            if ((s32) weaponModel >= 9) {
                                weaponModel = 8;
                            }
                            winst = ((M137Instance **) weapon->modelInstances)[weaponModel];
                            if (winst != NULL) {
                                wtex = D_800A0838;
                                if (obj->opacity < 0xFF) {
                                    wdl = winst->model->dlTranslucent;
                                } else {
                                    wdl = winst->model->dlOpaque;
                                }
                                if (winst->model->animatesTextures) {
                                    wtex = winst->texAnims[winst->matrixIndex];
                                }
                                func_80048D84(weaponMtx, *mtx);
                                gDma1p((*dl)++, G_MTX, (u32) *mtx + 0x80000000, sizeof(Mtx), 0x81);
                                (*mtx)++;
                                gSPPartMatrices((*dl)++,
                                                ((u32) winst->matrices[winst->matrixIndex] + 0x80000000) & 0xFFFFFF,
                                                (u32) winst->vertices + 0x80000000);
                                gSPCommand02((*dl)++, (u32) wtex + 0x80000000);
                                gSPDisplayList((*dl)++, (u32) wdl + 0x80000000);
                                gSPPartMatrices((*dl)++, 0, 0);
                            }
                        }
                        if (obj->behaviorId == 1) {
                            if (pd->pack != NULL && pd->pack->count != 0) {
                                gMoveWd((*dl)++, 0x0A, 0, 0);
                                func_m095_0148C(obj, pd->pack, dl, mtx, arg2);
                            }
                        }
                    }
                    gMoveWd((*dl)++, 0x0A, 0, 0);
                    func_80055B80(dl);
                    gDPSetPrimColor((*dl)++, 0, 0, 255, 255, 255, 255);
                }
            } else if (!flag && ((M137Header2 *) obj->header)->kind == 4) {
                if (obj->behaviorId == 0x40) {
                    M137Data40 *d = obj->unk68;

                    trans.y_position = surfaceY - dist;
                    trans.x_position = obj->trans.x_position;
                    trans.z_position = obj->trans.z_position;
                    trans.scale = -obj->trans.scale;
                    trans.rotation.z_rotation = obj->trans.rotation.z_rotation;
                    trans.rotation.x_rotation = obj->trans.rotation.x_rotation;
                    trans.rotation.y_rotation = obj->trans.rotation.y_rotation + 0x8000;
                    func_80049DC0(&trans, &objMtx);
                    func_80048C7C(&objMtx, cam, &result);
                    func_80048D84(&result, *mtx);
                    gDma1p((*dl)++, G_MTX, (u32) *mtx + 0x80000000, sizeof(Mtx), 1);
                    (*mtx)++;
                    func_80055B80(dl);
                    gDPSetPrimColor((*dl)++, 0, 0, 255, 255, 255, obj->opacity);
                    gDPSetEnvColor((*dl)++, 255, 255, 255, 0);
                    gSPPartMatrices((*dl)++, 0, 0);
                    gSPCommand02((*dl)++, D_800A0838 + 0x80000000);
                    if (obj->opacity == 0xFF) {
                        gSPDisplayList((*dl)++, (u32) d->dlOpaque + 0x80000000);
                    } else {
                        gSPDisplayList((*dl)++, (u32) d->dlTranslucent + 0x80000000);
                    }
                    gSPPartMatrices((*dl)++, 0, 0);
                    gMoveWd((*dl)++, 0x0A, 0, 0);
                    func_80055B80(dl);
                    gDPSetPrimColor((*dl)++, 0, 0, 255, 255, 255, 255);
                    gDPSetEnvColor((*dl)++, 255, 255, 255, 0);
                } else if (obj->behaviorId == 0xA2) {
                    M137DataA2 *d = obj->unk68;
                    M137InteractA2 *ia = (M137InteractA2 *) obj->interactObj;

                    if (d->unk63) {
                        to.x = ia->a.x;
                        to.y = surfaceY - (ia->a.y - surfaceY);
                        to.z = ia->a.z;
                    } else {
                        to.x = ia->b.x;
                        to.y = surfaceY - (ia->b.y - surfaceY);
                        to.z = ia->b.z;
                    }
                    from.x = obj->trans.x_position;
                    from.y = surfaceY - dist;
                    from.z = obj->trans.z_position;
                    func_8000A300(dl, arg2, ((M137Instance **) obj->modelInstances)[obj->modelIndex], d, &from, &to,
                                  d->unk6C, -1, OBJ_STATE(obj, s32)[1] | 0xFF, 0x200);
                } else if (obj->behaviorId == 0x98) {
                    M137Data98 *d = obj->unk68;

                    if (d->unkA78 != 0) {
                        tex = ((M137Header2 *) obj->header)->textures[0];
                        reflY = surfaceY - dist;
                        trans.y_position = reflY;
                        trans.x_position = obj->trans.x_position;
                        trans.z_position = obj->trans.z_position;
                        trans.scale = -obj->trans.scale;
                        trans.rotation.z_rotation = obj->trans.rotation.z_rotation;
                        trans.rotation.x_rotation = obj->trans.rotation.x_rotation;
                        trans.rotation.y_rotation = obj->trans.rotation.y_rotation + 0x8000;
                        func_80049DC0(&trans, &objMtx);
                        func_80048C7C(&objMtx, cam, &result);
                        func_80048D84(&result, *mtx);
                        gDma1p((*dl)++, G_MTX, (u32) *mtx + 0x80000000, sizeof(Mtx), 1);
                        (*mtx)++;
                        func_80055C00(dl, tex, 0x206, 0);
                        gDPSetPrimColor((*dl)++, 0, 0, 255, 255, 255, 0xC0);
                        {
                            Gfx *_g = (*dl)++;

                            _g->words.w0 = 0xFB000000;
                            _g->words.w1 = rl_m137_01754[d->colour][0];
                        }
                        for (k = 0; k < 3; k++) {
                            gSPVertexM137((*dl)++, (u32) d->verts[k * 2] + 0x80000000, 18);
                            gSPPolygon((*dl)++, (u32) d->tris[k * 2] + 0x80000000, 16, 1);
                            gSPVertexM137((*dl)++, (u32) d->verts[k * 2 + 1] + 0x80000000, 18);
                            gSPPolygon((*dl)++, (u32) d->tris[k * 2 + 1] + 0x80000000, 16, 1);
                            if (k == 0) {
                                gDPPipeSync((*dl)++);
                                {
                                    Gfx *_g = (*dl)++;

                                    _g->words.w0 = 0xFB000000;
                                    _g->words.w1 = rl_m137_01754[d->colour][1];
                                }
                            }
                        }
                        gMoveWd((*dl)++, 0x0A, 0, 0);
                        if (d->unkA7F) {
                            if (needReset) {
                                gDma1p((*dl)++, G_MTX, (u32) ((Mtx *) sDrawData + rl_m137_01750) + 0x80000000,
                                       sizeof(Mtx), 0);
                                needReset = FALSE;
                            }
                            render_sprite_billboard_at(dl, mtx, arg2, ((M137Header2 *) obj->header)->textures[1],
                                          obj->trans.x_position, reflY, obj->trans.z_position, d->unkA76, d->unkA54,
                                          0.0f, 0x20000006, 0xC0);
                        }
                        gDPSetPrimColor((*dl)++, 0, 0, 255, 255, 255, 255);
                        gDPSetEnvColor((*dl)++, 255, 255, 255, 0);
                    }
                } else if (obj->behaviorId == 0x28) {
                    M137Data28 *d;

                    if (obj->trans.rotation.x_rotation == 0) {
                        trans.x_position = obj->trans.x_position;
                        trans.y_position = surfaceY - dist;
                        trans.z_position = obj->trans.z_position;
                        trans.scale = -obj->trans.scale;
                        trans.rotation.z_rotation = obj->trans.rotation.z_rotation;
                        trans.rotation.x_rotation = obj->trans.rotation.x_rotation;
                        trans.rotation.y_rotation = obj->trans.rotation.y_rotation + 0x8000;
                        func_80049DC0(&trans, &objMtx);
                        d = obj->unk68;
                        if (d->texture->flags & 0x40) {
                            gDPSetEnvColor((*dl)++, d->envR, d->envG, d->envB, d->frame);
                        } else {
                            gDPSetEnvColor((*dl)++, d->envR, d->envG, d->envB, 255);
                        }
                        gDPSetPrimColor((*dl)++, 0, 0, d->primR, d->primG, d->primB, d->alpha >> 8);
                        func_80048C7C(&objMtx, cam, &result);
                        func_80048D84(&result, *mtx);
                        gDma1p((*dl)++, G_MTX, (u32) *mtx + 0x80000000, sizeof(Mtx), 1);
                        (*mtx)++;
                        func_80055C00(dl, d->texture, 0x20E, d->frame << 8);
                        gSPVertexM137((*dl)++, (u32) &d->verts[d->set * 20] + 0x80000000, 20);
                        gSPPolygon((*dl)++, D_800F2D58 + 0x80000000, 16, 1);
                        gSPPolygon((*dl)++, D_800F2D58 + 0x80000100, 4, 1);
                        gMoveWd((*dl)++, 0x0A, 0, 0);
                        func_80055B80(dl);
                        gDPSetEnvColor((*dl)++, 255, 255, 255, 255);
                        gDPSetPrimColor((*dl)++, 0, 0, 255, 255, 255, 255);
                    }
                }
            } else if (!flag) {
                if (((M137Header2 *) obj->header)->kind == 1) {
                    if (needReset) {
                        gDma1p((*dl)++, G_MTX, (u32) ((Mtx *) sDrawData + rl_m137_01750) + 0x80000000, sizeof(Mtx),
                               0);
                        needReset = FALSE;
                    }
                    savedY = obj->trans.y_position;
                    obj->trans.y_position = surfaceY - dist;
                    obj->trans.scale = -obj->trans.scale;
                    func_8000DB94(dl, mtx, arg2, obj);
                    obj->trans.y_position = savedY;
                }
            }
        } while (i < sReflectedCount);
    }
    gSPClearGeometryMode((*dl)++, 0x1000);
    if (needReset) {
        gDma1p((*dl)++, G_MTX, (u32) ((Mtx *) sDrawData + rl_m137_01750) + 0x80000000, sizeof(Mtx), 0);
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m137", func_m137_0025C);
#endif

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
