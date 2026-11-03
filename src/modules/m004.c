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
extern void func_8003DDD8(void *arg0, void *model, s32 arg2, s32 arg3, s32 arg4, s32 arg5, s32 arg6);

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

INCLUDE_ASM("asm/nonmatchings/modules/m004", func_m004_0036C);

INCLUDE_ASM("asm/nonmatchings/modules/m004", func_m004_00CB8);

INCLUDE_ASM("asm/nonmatchings/modules/m004", func_m004_012BC);

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
    func_8003DDD8(&copy->unk34, model, arg2, arg3, 0, 0xFF, 1);
    if (obj->behaviorId != 1 && ((s8 *) obj->header)[0x20] > 0) {
        i = 0;
        other = (*(M004Attached **) obj->pad60)->objs[0];
        copy->numBuffers = other->header->numberOfModelIds;
        for (; i < copy->numBuffers; i++) {
            inst = ((void ***) other->modelInstances)[i];
            func_8003DDD8(&copy->buffers[i], *inst, arg2, arg3, 0, 0xFF, 1);
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

INCLUDE_ASM("asm/nonmatchings/modules/m004", rodata);

INCLUDE_ASM("asm/nonmatchings/modules/m004", bss);
