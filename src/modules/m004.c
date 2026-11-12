/*
 * Runlink module 4: copies of a model (its vertex and triangle buffers and
 * their shadows), built and freed for other modules.
 */
#include "module.h"

/* A model copy and the buffers it owns */
typedef struct M004Copy {
    /* 0x00 */ f32 scale;
    /* 0x04 */ void *unk4;
    /* 0x08 */ s16 unk8;
    /* 0x0A */ s16 unkA;
    /* 0x0C */ u8 unkC;
    /* 0x0D */ u8 unkD;
    /* 0x0E */ u8 unkE;
    /* 0x0F */ u8 unkF;
    /* 0x10 */ u8 pad10[0x20 - 0x10];
    /* 0x20 */ u8 *unk20; /* in unk24, aligned */
    /* 0x24 */ void *unk24;
    /* 0x28 */ void *unk28;
    /* 0x2C */ void *unk2C;
    /* 0x30 */ void *unk30;
    /* 0x34 */ void *unk34; /* optional */
    /* 0x38 */ void *buffers[10];
    /* 0x60 */ void **model;
    /* 0x64 */ void **model2;
    /* 0x68 */ void *unk68;
    /* 0x6C */ u8 pad6C[0xAC - 0x6C];
    /* 0xAC */ f32 unkAC;
    /* 0xB0 */ u8 padB0[0xB8 - 0xB0];
    /* 0xB8 */ u8 numBuffers;
    /* 0xB9 */ u8 pending; /* waiting for its task */
    /* 0xBA */ u8 unkBA;
} M004Copy;

/* What obj->pad60 points at: the objects attached to it */
typedef struct M004Attached {
    /* 0x0 */ s32 count;
    /* 0x4 */ Object *objs[1];
} M004Attached;

/* The 0x20-byte header written before the copy's buffer */
typedef struct M004Block {
    /* 0x00 */ u8 unk0;
    /* 0x01 */ u8 unk1;
    /* 0x02 */ u8 unk2;
    /* 0x03 */ u8 unk3;
    /* 0x04 */ u8 unk4;
    /* 0x05 */ u8 unk5;
    /* 0x06 */ s16 unk6;
    /* 0x08 */ s16 unk8;
    /* 0x0A */ s16 unkA;
    /* 0x0C */ void *unkC;
    /* 0x10 */ s16 unk10;
    /* 0x12 */ s16 unk12;
    /* 0x14 */ s16 unk14;
    /* 0x16 */ s16 unk16;
    /* 0x18 */ u8 unk18;
    /* 0x19 */ u8 unk19;
    /* 0x1A */ u8 unk1A;
    /* 0x1B */ u8 unk1B;
    /* 0x1C */ u8 unk1C;
    /* 0x1D */ u8 unk1D;
    /* 0x1E */ u8 unk1E;
    /* 0x1F */ u8 unk1F;
} M004Block;

/* What obj->shadow points at here */
typedef struct M004Shadow {
    /* 0x00 */ f32 scale;
    /* 0x04 */ u8 pad4[0xC - 0x4];
    /* 0x0C */ u8 unkC;
    /* 0x0D */ u8 unkD;
    /* 0x0E */ u8 unkE;
    /* 0x0F */ u8 padF[0x1C - 0xF];
    /* 0x1C */ M004Copy *copy;
} M004Shadow;

static s32 copy_alloc(Object *obj, M004Copy *copy);
static void copy_models(Object *obj, M004Copy *copy, s32 arg2, s32 arg3);
extern u8 rl_m004_01660[];
extern s8 rl_m004_01728;             /* the number of copies waiting */
extern M004Copy *rl_m004_01880[];     /* the copies waiting */
#ifdef NON_MATCHING
static void func_m004_015A8(M004Copy *copy);
#else
void rl_m004_015A8(M004Copy *copy);
#define func_m004_015A8 rl_m004_015A8
#endif
extern void func_8004DA10(void *arg0, void *arg1, s32 arg2, void *arg3); /* starts a task */
extern s32 func_80096910(OSMesgQueue *mq, OSMesg *msg, s32 flag);
extern OSMesgQueue D_800FE8A8;

extern void *func_8004A57C(s32 size, s32 colour); /* DKR mempool_alloc_safe */
extern void func_8004AA80(void *data); /* DKR mempool_free */
extern void free_3d_model(void *model);
extern void model_batches_build_dlist(void *arg0, void *model, s32 arg2, s32 arg3, s32 arg4, s32 arg5, s32 arg6);

/* Sets up an object's shadow copy; returns FALSE when it cannot be made */
s32 func_m004_00000(Object *obj) {
    M004Block *b;
    M004Copy *c = ((M004Shadow *) obj->shadow)->copy;
    s8 index;
    u8 *m;

    if (!copy_alloc(obj, c)) {
        return FALSE;
    }
    c->unkAC = 1.0f;
    c->model = ((void ***) obj->modelInstances)[obj->modelIndex];
    index = ((s8 *) obj->header)[0x60];
    if (index >= 0) {
        c->model2 = ((void ***) obj->modelInstances)[index];
    } else {
        c->model2 = c->model;
    }
    m = *c->model;
    if (m[0x2F]) {
        c->unkBA = 1;
    }
    copy_models(obj, c, 0x400, 11);
    b = (M004Block *) (c->unk20 - 0x20);
    b->unk0 = 0x40;
    b->unk1 = 0x40;
    b->unk2 = 5;
    b->unk3 = 0;
    b->unk4 = 0;
    b->unk5 = 0;
    b->unk6 = 0;
    b->unk8 = 1;
    b->unkA = 0x38;
    b->unkC = rl_m004_01660;
    b->unk10 = 0;
    b->unk12 = 0;
    b->unk14 = 0;
    b->unk16 = 0x1000;
    b->unk18 = 0;
    b->unk19 = 0;
    b->unk1A = 0;
    b->unk1B = 0;
    b->unk1C = 2;
    b->unk1D = 0;
    b->unk1E = 2;
    b->unk1F = 0;
    c->unk4 = b;
    c->scale = ((M004Shadow *) obj->shadow)->scale;
    c->unkC = ((M004Shadow *) obj->shadow)->unkC | 0x20;
    c->unkD = ((M004Shadow *) obj->shadow)->unkD;
    c->unkE = ((M004Shadow *) obj->shadow)->unkE;
    c->unkF = 0;
    c->unk8 = 0;
    c->unkA = 0;
    return TRUE;
}

/* Runs the waiting copies' tasks one at a time, finishing each one's predecessor */
void func_m004_001BC(void) {
    M004Copy *c;
    M004Copy *prev;
    OSMesg msg;
    s32 i;

    msg = NULL;
    for (i = 0; i < rl_m004_01728; i++) {
        c = rl_m004_01880[i];
        if (i > 0) {
            prev = rl_m004_01880[i - 1];
        } else {
            prev = NULL;
        }
        if (c->pending) {
            func_8004DA10(c->unk28, c->unk30, 3, c->unk20);
            c->pending = FALSE;
            if (prev != NULL) {
                func_m004_015A8(prev);
            }
            func_80096910(&D_800FE8A8, &msg, OS_MESG_BLOCK);
        }
    }
    func_m004_015A8(c);
    rl_m004_01728 = 0;
}

/* Frees a copy's buffers */
void func_m004_002B8(M004Copy *copy) {
    s32 i;

    func_8004AA80(copy->unk24);
    func_8004AA80(copy->unk28);
    func_8004AA80(copy->unk2C);
    if (copy->unk34 != NULL) {
        func_8004AA80(copy->unk34);
    }
    if (copy->unk68 != NULL) {
        free_3d_model(copy->unk68);
    }
    for (i = 0; i < copy->numBuffers; i++) {
        func_8004AA80(copy->buffers[i]);
    }
}

INCLUDE_ASM("asm/nonmatchings/modules/m004/ro", func_m004_0036C);

/* A matrix with two words after it */
typedef struct M004Mtx {
    /* 0x00 */ f32 m[4][4];
    /* 0x40 */ s32 unk40;
    /* 0x44 */ s32 unk44; /* the low byte: the grey of the copy drawn with it */
} M004Mtx;

/* A model instance, as drawn here */
typedef struct M004Inst {
    /* 0x00 */ u8 pad0[4];
    /* 0x04 */ void *triangles;
    /* 0x08 */ u8 pad8[0xB - 0x8];
    /* 0x0B */ s8 buffer;
    /* 0x0C */ u8 padC[0x10 - 0xC];
    /* 0x10 */ void *vertices[2];
} M004Inst;

/* The behaviour state of an object with a copy (OBJ_STATE) */
typedef struct M004State {
    /* 0x0 */ u8 pad0[4];
    /* 0x4 */ s32 model; /* (behaviour 0x33) the model instance of the attached object to draw too */
} M004State;

extern void debug_trace_add(Gfx *dl, char *file, s32 line);
extern void func_80050E90(Gfx **dl, s32 segment, s32 address);
extern void rdp_set_default_modes(Gfx **dl);
extern void func_80048C7C(MtxF *a, MtxF *b, MtxF *out); /* out = a * b */
extern void func_80048D84(MtxF *mtx, Mtx *out);         /* to a fixed-point Mtx */
extern u8 D_800A0838[];
extern s32 D_800A35E8;
extern char rl_m004_01730[]; /* .rodata (still assembly): the file names for debug_trace_add */

/*
 * Builds the display list that draws the object's copies (one per matrix, each a grey of its own) into the
 * copy's 64x64 8-bit texture, and queues it (called from 0036C)
 */
#ifdef NON_MATCHING
/* 13 diffs: the ROM keeps the queue count in t9 and `attached` in t0 (here t1 and v1) */
void func_m004_00CB8(Object *obj, M004Mtx **mtxs, s32 count) {
    Gfx *dl;
    u32 mtx;
    s32 grey;
    s32 colour;
    M004Inst *inst;
    s32 i;
    M004Inst *inst2;
    M004Copy *copy;
    M004Inst *other;
    MtxF *attachMtx;
    MtxF m1;
    MtxF m2;
    s32 index2;
    s32 index;
    s32 attached;

    copy = ((M004Shadow *) obj->shadow)->copy;
    inst = (M004Inst *) copy->model;
    index2 = rl_m004_01728;
    inst2 = (M004Inst *) copy->model2;
    if (index2 < 15) {
        attached = FALSE;
        if (obj->behaviorId == 0x33) {
            attached = TRUE;
            index = OBJ_STATE(obj, M004State)->model;
            attachMtx = obj->unk68;
        }
        mtx = (u32) copy->unk2C;
        debug_trace_add(dl = copy->unk28, rl_m004_01730, 0x25D);
        func_80050E90(&dl, 0, 0);
        func_80050E90(&dl, 1, (s32) copy->unk20);
        gDPSetCycleType(dl++, G_CYC_FILL);
        gDPSetColorImage(dl++, G_IM_FMT_I, G_IM_SIZ_8b, 64, (u32) copy->unk20 + 0x80000000);
        gDPSetFillColor(dl++, 0);
        gDPSetScissor(dl++, G_SC_NON_INTERLACE, 0, 0, 64, 64);
        gDPFillRectangle(dl++, 0, 0, 63, 63);
        rdp_set_default_modes(&dl);
        gSPClearGeometryMode(dl++, 0x1F3204);
        gSPTexture(dl++, 0, 0, 0, G_TX_RENDERTILE, G_OFF);
        gMoveWd(dl++, G_MW_NUMLIGHT, 0, 0);
        gSPViewport(dl++, rl_m004_01660 + 0x800000B8);
        gSPClipRatio(dl++, FRUSTRATIO_2);
        gDPSetScissor(dl++, G_SC_NON_INTERLACE, 1, 1, 63, 63);
        for (i = count - 1; i >= 0; i--) {
            index2 = mtxs[i]->unk44;
            grey = index2 & 0xFF;
            colour = (grey << 24) | (grey << 16) | (grey << 8);
            { Gfx *_g = dl++; _g->words.w0 = 0xFA000000; _g->words.w1 = colour | 0xFF; }
            { Gfx *_g = dl++; _g->words.w0 = 0xFB000000; _g->words.w1 = colour | 0xFF; }
            func_80048C7C((MtxF *) &copy->pad6C, (MtxF *) mtxs[i], &m1);
            func_80048C7C(&m1, (MtxF *) (rl_m004_01660 + 0x78), &m1);
            func_80048C7C(&m1, (MtxF *) (rl_m004_01660 + 0x38), &m2);
            func_80048D84(&m2, (Mtx *) mtx);
            { Gfx *_g = dl++; _g->words.w0 = 0x01000040; _g->words.w1 = mtx + 0x80000000; }
            mtx += sizeof(Mtx);
            { Gfx *_g = dl++; _g->words.w0 = (((u32) inst->vertices[inst->buffer] + 0x80000000) & 0xFFFFFF) | 0xBF000000; _g->words.w1 = (u32) inst2->triangles + 0x80000000; }
            { Gfx *_g = dl++; _g->words.w0 = 0x02000050; _g->words.w1 = (u32) (D_800A0838 + 0x80000000); }
            gSPDisplayList(dl++, (u32) copy->unk34 + 0x80000000);
            { Gfx *_g = dl++; _g->words.w0 = 0xBF000000; _g->words.w1 = 0; }
            if (attached) {
                Object *o = (*(M004Attached **) obj->pad60)->objs[0];
                other = ((M004Inst **) o->modelInstances)[index];
                func_80048C7C(attachMtx, &m2, &m1);
                func_80048D84(&m1, (Mtx *) mtx);
                { Gfx *_g = dl++; _g->words.w0 = 0x01010040; _g->words.w1 = mtx + 0x80000000; }
                mtx += sizeof(Mtx);
                { Gfx *_g = dl++; _g->words.w0 = (((u32) other->vertices[other->buffer] + 0x80000000) & 0xFFFFFF) | 0xBF000000; _g->words.w1 = (u32) other->triangles + 0x80000000; }
                gSPDisplayList(dl++, (u32) copy->buffers[index] + 0x80000000);
                { Gfx *_g = dl++; _g->words.w0 = 0xBF000000; _g->words.w1 = 0; }
            }
        }
        gMoveWd(dl++, 0x0A, 0, 0);
        gDPFullSync(dl++);
        gSPEndDisplayList(dl++);
        debug_trace_add(dl, rl_m004_01730 + 0x10, 0x2A7);
        copy->unk30 = dl;
        copy->pending = TRUE;
        D_800A35E8 = 1;
        rl_m004_01880[rl_m004_01728] = copy;
        rl_m004_01728++;
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m004/ro", func_m004_00CB8);
#endif

extern void func_80049BA0(M004Mtx *mtx); /* identity */

/*
 * Halves the rotation's tilt towards straight down and builds a matrix that shears along the direction it
 * then points in (called from 0036C)
 */
#ifdef NON_MATCHING
/* f0/f2 swapped (y and the constant): the ROM's k is a .rodata literal; needs .rodata in C */
void func_m004_012BC(Vec3s *rot, s32 arg1, M004Mtx *mtx) {
    f32 k;
    Vec3f dir;
    f32 x;
    f32 y;
    f32 z;

    if (rot->x_rotation < 0) {
        rot->x_rotation = 0;
    }
    rot->x_rotation -= 0x4000;
    rot->x_rotation >>= 1;
    rot->x_rotation += 0x4000;
    dir.x = 0.0f;
    dir.y = 0.0f;
    dir.z = -1.0f;
    func_8004944C(rot, &dir);
    x = dir.x;
    z = dir.z;
    y = dir.y;
    mtx->unk44 = arg1;
    func_80049BA0(mtx);
    k = 1.2f;
    mtx->m[0][0] = k;
    mtx->m[1][0] = -(x / y);
    mtx->m[1][1] = 0.0f;
    mtx->m[1][2] = -(z / y);
    mtx->m[2][2] = k;
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m004/ro", func_m004_012BC);
#endif

/* Allocates a copy's buffers; FALSE (with nothing kept) when one cannot be had */
static s32 copy_alloc(Object *obj, M004Copy *copy) {
    s32 n;
    UNUSED s32 pad;
    s32 m;

    copy->unk24 = func_8004A57C(0x10B4, 0xFF7F7FFF);
    if (copy->unk24 == NULL) {
        return FALSE;
    }
    copy->unk20 = (u8 *) (((s32) copy->unk24 + 0x20) & ~0x3F) + 0x80;
    if (obj->behaviorId == 1) {
        n = 40;
        m = 6;
    } else {
        n = 60;
        m = 10;
    }
    copy->unk28 = func_8004A57C(n * 8, 0xFF7F7FFF);
    if (copy->unk28 == NULL) {
        func_8004AA80(copy->unk24);
        return FALSE;
    }
    copy->unk2C = func_8004A57C(m << 6, 0xFF7F7FFF);
    if (copy->unk2C == NULL) {
        func_8004AA80(copy->unk24);
        func_8004AA80(copy->unk28);
        return FALSE;
    }
    return TRUE;
}

/* Copies the model, and for non-players those of the first attached object */
static void copy_models(Object *obj, M004Copy *copy, s32 arg2, s32 arg3) {
    s32 i;
    Object *other;
    void **inst;
    void *model;

    model = *copy->model2;
    model_batches_build_dlist(&copy->unk34, model, arg2, arg3, 0, 0xFF, 1);
    if (obj->behaviorId != 1 && ((s8 *) obj->header)[0x20] > 0) {
        i = 0;
        other = (*(M004Attached **) obj->pad60)->objs[0];
        copy->numBuffers = other->header->numberOfModelIds;
        for (; i < copy->numBuffers; i++) {
            inst = ((void ***) other->modelInstances)[i];
            model_batches_build_dlist(&copy->buffers[i], *inst, arg2, arg3, 0, 0xFF, 1);
        }
    }
}

#ifdef NON_MATCHING
/*
 * Blurs the copy's 64x64 shadow texture, then halves its intensities.
 * The original sums into a temporary and copies it to `sum` after the shift.
 */
static void func_m004_015A8(M004Copy *copy) {
    u8 *p = copy->unk20;
    s32 y;
    s32 x;
    s32 sum;
    u32 *q;

    y = 62;
    while (y--) {
        x = 62;
        while (x--) {
            sum = p[0];
            sum += p[1] + p[2] + p[0x40] + p[0x42] + p[0x80] + p[0x81] + p[0x82];
            p[0x41] = sum >> 3;
            p++;
        }
        p += 2;
    }
    q = (u32 *) copy->unk20;
    y = 1024;
    while (y--) {
        *q = (*q & 0xF0F0F0F0) >> 4;
        q++;
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m004", func_m004_015A8);
#endif

INCLUDE_ASM("asm/nonmatchings/modules/m004", data);

/* (.rodata: C, and the assembly functions' own from ro/) */

INCLUDE_ASM("asm/nonmatchings/modules/m004", bss);
