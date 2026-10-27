#include "common.h"

typedef struct Vec3f {
    f32 x, y, z;
} Vec3f;

typedef struct Obj7EHeader {
    /* 0x00 */ u8 pad0[0x71];
    /* 0x71 */ s8 unk71;
} Obj7EHeader;

typedef struct Obj7EData {
    /* 0x000 */ u8 pad0[0x12A];
    /* 0x12A */ u8 unk12A;
    /* 0x12B */ u8 pad12B;
    /* 0x12C */ s16 unk12C;
    /* 0x12E */ s16 unk12E;
    /* 0x130 */ s16 unk130;
    /* 0x132 */ u8 unk132;
    /* 0x133 */ s8 unk133;
} Obj7EData;

typedef struct Obj7E {
    /* 0x00 */ u8 pad0[0x3A];
    /* 0x3A */ s8 unk3A;
    /* 0x3B */ u8 pad3B[0x40 - 0x3B];
    /* 0x40 */ Obj7EHeader *header;
    /* 0x44 */ u8 pad44[0x5C - 0x44];
    /* 0x5C */ Obj7EData *data;
} Obj7E;

extern s32 D_801047E0;
extern Obj7E **D_801047E4;
extern void *D_801047E8;
extern void *D_801047EC;
extern void *D_801047F4;
extern void *D_801047F8;
extern void *D_801047FC;
extern void *D_80104800;
void *mempool_alloc_safe(s32 size, u32 colourTag);
void mempool_free(void *data);

void func_80079BD0(void) {
    D_801047E4 = mempool_alloc_safe(0x140, 0xFFFF);
    D_801047E8 = mempool_alloc_safe(0x400, 0xFFFF);
    D_801047EC = mempool_alloc_safe(0x400, 0xFFFF);
}

/* DKR include/level.h LevelHeader, reduced (JFG offsets) */
typedef struct LevelHeader {
    /* 0x00 */ u8 pad0[0xEE];
    /* 0xEE */ s16 unkEE;
    /* 0xF0 */ s16 unkF0;
} LevelHeader;
LevelHeader *level_header(void);

void func_80079C20(void) {
    LevelHeader *header;

    D_801047E0 = 0;
    header = level_header();
    D_80104800 = mempool_alloc_safe(header->unkF0 * 0x30, 0xFFFF);
    D_801047F4 = mempool_alloc_safe(header->unkEE * 4, 0xFFFF);
    D_801047F8 = mempool_alloc_safe(header->unkEE * 2, 0xFFFF);
    D_801047FC = mempool_alloc_safe(header->unkEE * 2, 0xFFFF);
}

void func_80079CC4(void) {
    mempool_free(D_80104800);
    mempool_free(D_801047F4);
    mempool_free(D_801047F8);
    mempool_free(D_801047FC);
}

INCLUDE_ASM("asm/nonmatchings/7A7D0", func_80079D1C);

INCLUDE_ASM("asm/nonmatchings/7A7D0", func_8007A018);

INCLUDE_ASM("asm/nonmatchings/7A7D0", func_8007A324);

INCLUDE_ASM("asm/nonmatchings/7A7D0", func_8007A534);

INCLUDE_ASM("asm/nonmatchings/7A7D0", func_8007B99C);

INCLUDE_ASM("asm/nonmatchings/7A7D0", func_8007BB00);

INCLUDE_ASM("asm/nonmatchings/7A7D0", func_8007BCD0);

INCLUDE_ASM("asm/nonmatchings/7A7D0", func_8007C2E8);

INCLUDE_ASM("asm/nonmatchings/7A7D0", func_8007CCF0);

INCLUDE_ASM("asm/nonmatchings/7A7D0", func_8007CF28);

/* Size: 0x34 bytes */
typedef struct CollisionSphere {
    /* 0x00 */ u8 pad0[0xC];
    /* 0x0C */ Vec3f pos;
    /* 0x18 */ u8 pad18[0x24 - 0x18];
    /* 0x24 */ f32 radius;
    /* 0x28 */ u8 pad28[0x34 - 0x28];
} CollisionSphere;

typedef struct SphereSet {
    /* 0x00 */ u8 pad0[0x32];
    /* 0x32 */ s16 count;
    /* 0x34 */ u8 pad34[0x40 - 0x34];
    /* 0x40 */ CollisionSphere *spheres;
} SphereSet;

#ifdef NON_MATCHING
/* NON_MATCHING: float register allocation (r/dx/dy/dz in f16-f22 rotated against the target) */
/* JFG: TRUE if the sphere overlaps any sphere of the set. */
s32 func_8007D0B4(CollisionSphere *sphere, SphereSet *set) {
    s32 i = set->count;
    CollisionSphere *other = set->spheres;
    f32 x = sphere->pos.x;
    f32 y = sphere->pos.y;
    f32 z = sphere->pos.z;
    f32 radius = sphere->radius;
    f32 dx, dy, dz, r;

    while (i--) {
        r = other->radius + radius;
        r *= r;
        dx = other->pos.x - x;
        dy = other->pos.y - y;
        dz = other->pos.z - z;
        other++;
        if (dx * dx + dy * dy + dz * dz < r) {
            return TRUE;
        }
    }
    return FALSE;
}
#else
INCLUDE_ASM("asm/nonmatchings/7A7D0", func_8007D0B4);
#endif

INCLUDE_ASM("asm/nonmatchings/7A7D0", func_8007D168);

INCLUDE_ASM("asm/nonmatchings/7A7D0", func_8007DE98);

INCLUDE_ASM("asm/nonmatchings/7A7D0", func_8007E150);

INCLUDE_ASM("asm/nonmatchings/7A7D0", func_8007E328);

Obj7E **func_8007E484(s32 *count) {
    *count = D_801047E0;
    return D_801047E4;
}

void func_8007E5EC(Obj7E *obj, s32 arg1);
void func_8007E794(Obj7E *obj);

void func_8007E4A0(Obj7E *obj) {
    Obj7EData *data = obj->data;
    s8 value = obj->header->unk71;

    if (value >= 0) {
        data->unk133 = value;
    } else {
        data->unk133 = obj->unk3A;
    }
    func_8007E5EC(obj, 1);
    func_8007E5EC(obj, 1);
    func_8007E794(obj);
}

void func_8007E504(void) {
    s32 i;
    Obj7E *obj;
    Obj7EData *data;
    s8 value;

    for (i = 0; i < D_801047E0; i++) {
        obj = D_801047E4[i];
        data = obj->data;
        value = obj->header->unk71;
        if (value >= 0) {
            data->unk133 = value;
        } else {
            data->unk133 = obj->unk3A;
        }
        func_8007E5EC(obj, 0);
        if (data->unk12C != 0) {
            func_8007E794(obj);
        }
        data->unk12E = 0;
        data->unk132 = 0;
        data->unk130 = 0;
        data->unk12A = 0;
    }
}

INCLUDE_ASM("asm/nonmatchings/7A7D0", func_8007E5EC);

INCLUDE_ASM("asm/nonmatchings/7A7D0", func_8007E794);

INCLUDE_ASM("asm/nonmatchings/7A7D0", func_8007E9E0);

INCLUDE_ASM("asm/nonmatchings/7A7D0", func_8007EB6C);

INCLUDE_ASM("asm/nonmatchings/7A7D0", func_8007F280);

INCLUDE_ASM("asm/nonmatchings/7A7D0", func_8007FC0C);

INCLUDE_ASM("asm/nonmatchings/7A7D0", func_800805F4);

INCLUDE_ASM("asm/nonmatchings/7A7D0", func_800809F4);

INCLUDE_ASM("asm/nonmatchings/7A7D0", func_80081658);

INCLUDE_ASM("asm/nonmatchings/7A7D0", func_80081CB8);

INCLUDE_ASM("asm/nonmatchings/7A7D0", func_80082118);

INCLUDE_ASM("asm/nonmatchings/7A7D0", func_800822D8);

INCLUDE_ASM("asm/nonmatchings/7A7D0", func_80082D80);

INCLUDE_ASM("asm/nonmatchings/7A7D0", func_800835E0);

INCLUDE_ASM("asm/nonmatchings/7A7D0", func_80083914);
