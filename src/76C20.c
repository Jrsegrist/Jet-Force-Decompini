#include "common.h"

typedef struct AnimModel {
    /* 0x00 */ u8 pad0[0xA];
    /* 0x0A */ u16 flags;
} AnimModel;

typedef struct AnimObject {
    /* 0x00 */ u8 pad0[0x6];
    /* 0x06 */ s16 flags;
    /* 0x08 */ u8 pad8[0x4];
    /* 0x0C */ f32 x;
    /* 0x10 */ f32 y;
    /* 0x14 */ f32 z;
    /* 0x18 */ u8 pad18[0x34];
    /* 0x4C */ AnimModel *model;
    /* 0x50 */ u8 pad50[0xC];
    /* 0x5C */ s32 unk5C;
    /* 0x60 */ u8 pad60[0x28];
    /* 0x88 */ void *audioPoint;
} AnimObject;

typedef struct AnimPath {
    /* 0x00 */ u8 pad0[0x8];
    /* 0x08 */ AnimObject *obj;
    /* 0x0C */ u8 padC[0xA];
    /* 0x16 */ u8 flags;
} AnimPath;
extern AnimPath **D_801045B0;

void free_object(AnimObject *object);
void mempool_free(void *data);
void audspat_point_stop(void *point);
void audspat_play_sound_at_position(u16 soundId, f32 x, f32 y, f32 z, u8 flags, void **handlePtr);

extern s32 *D_801047C8;
extern s32 D_801047CC;
extern s32 D_801047D0;

void func_80076020(s32 *arg0) {
    D_801047C8 = arg0;
    D_801047CC = *arg0;
    D_801047D0 = 0x80;
}

/* JFG: read count bits (most significant first) from the bit stream. */
s32 func_80076044(u32 count) {
    s32 result = 0;

    if (count != 0) {
        count = 1 << (count - 1); // JFG: now the mask of the next bit
        do {
            if (D_801047D0 == 0) {
                D_801047CC++;
                D_801047D0 = 0x80;
            }
            if (*(u8 *) D_801047CC & D_801047D0) {
                result |= count;
            }
            count >>= 1;
            D_801047D0 = (u32) D_801047D0 >> 1;
        } while (count != 0);
    }
    return result;
}

/* JFG: read a signed count-bit number (sign-extended) from the bit stream. */
s32 func_800760C0(u32 count) {
    s32 result = 0;
    s32 sign;

    if (count != 0) {
        sign = -1 << (count - 1);
        count = 1 << (count - 1);
        do {
            if (D_801047D0 == 0) {
                D_801047CC++;
                D_801047D0 = 0x80;
            }
            if (*(u8 *) D_801047CC & D_801047D0) {
                result |= count;
            }
            count >>= 1;
            D_801047D0 = (u32) D_801047D0 >> 1;
        } while (count != 0);
        if (result & sign) {
            result |= sign;
        }
    }
    return result;
}

void func_80076154(void) {
    if (D_801047D0 != 0x80) {
        D_801047CC++;
    }
    *D_801047C8 = D_801047CC;
}

extern s32 D_800A74C8;
extern s32 D_800A74CC;
extern s32 D_800A74D0;

void func_80076198(void) {
    if (D_800A74C8) {
        D_800A74C8 = 0;
        D_800A74CC = 0;
        D_800A74D0 = 0;
    }
}

extern s8 D_800A74F0[];

s8 func_800761C8(u16 *arg0) {
    return D_800A74F0[(u8) (arg0[1] >> 8)];
}

#define BODY_761E8 \
    if (cmd != NULL) { \
        do { \
            if ((u8) (cmd[1] >> 8) == 0x7F) { \
                done = TRUE; \
            } \
            len = func_800761C8(cmd); \
            total += len; \
            if (len != 0) { \
                count++; \
                cmd += len >> 1; \
                if (count > 0x2000) { \
                    done = TRUE; \
                } \
            } else { \
                total = 0; \
                done = TRUE; \
            } \
        } while (!done); \
    } \
    return total;
INCLUDE_ASM("asm/nonmatchings/76C20", func_800761E8);

extern s32 *D_800A74BC;

/* JFG: start reading the data at D_800A74BC (its first word's low 24 bits are the offset to the start). */
void func_800762A0(void) {
    s32 header = *D_800A74BC;

    D_800A74C8 = (s32) D_800A74BC + (header & 0xFFFFFF);
    D_800A74CC = D_800A74C8;
    D_800A74D0 = func_800761E8(&D_800A74C8);
}

void func_800762F0(u8 index) {
    AnimPath *path = D_801045B0[index];

    if (path != NULL) {
        if (path->obj != NULL) {
            free_object(path->obj);
        }
        mempool_free(path);
    }
    D_801045B0[index] = NULL;
}

INCLUDE_ASM("asm/nonmatchings/76C20", func_8007637C);

INCLUDE_ASM("asm/nonmatchings/76C20", func_800766A8);

void func_800767EC(u8 index) {
    AnimPath *path = D_801045B0[index];

    if (path != NULL) {
        if (!(path->flags & 8) && !(path->flags & 1)) {
            path->flags |= 1;
            path->flags &= ~4;
            if (path->obj != NULL) {
                path->obj->flags &= ~0x400;
                if (path->obj->unk5C != 0) {
                    path->obj->model->flags |= 1;
                }
            }
        }
    }
}

void func_8007688C(u8 index) {
    AnimPath *path = D_801045B0[index];

    if (path != NULL) {
        if (!(path->flags & 8) && (path->flags & 5)) {
            path->flags &= ~5;
            if (path->obj != NULL) {
                path->obj->flags |= 0x400;
                if (path->obj->unk5C != 0) {
                    path->obj->model->flags &= ~1;
                }
            }
        }
    }
}

void func_80076928(u8 index) {
    AnimPath *path = D_801045B0[index];

    if (path != NULL) {
        if (!(path->flags & 8)) {
            if (path->flags & 1) {
                path->flags &= ~1;
            } else if (path->obj != NULL) {
                path->obj->flags &= ~0x400;
                if (path->obj->unk5C != 0) {
                    path->obj->model->flags |= 1;
                }
            }
            path->flags |= 4;
        }
    }
}

void func_800769DC(u8 index) {
    AnimPath *path = D_801045B0[index];
    if (path != NULL) {
        path->flags |= 8;
    }
}

void func_80076A1C(u8 index) {
    AnimPath *path = D_801045B0[index];
    if (path != NULL) {
        path->flags &= ~8;
    }
}

void func_80076A5C(u8 index, u16 soundId) {
    AnimPath *path = D_801045B0[index];
    AnimObject *obj;

    if (path != NULL) {
        obj = path->obj;
        if (obj != NULL) {
            if (obj->audioPoint == NULL) {
                audspat_play_sound_at_position(soundId, obj->x, obj->y, obj->z, 1, &obj->audioPoint);
            }
        }
    }
}

INCLUDE_ASM("asm/nonmatchings/76C20", func_80076ADC);

INCLUDE_ASM("asm/nonmatchings/76C20", func_80076B44);

INCLUDE_ASM("asm/nonmatchings/76C20", func_80076D6C);

INCLUDE_ASM("asm/nonmatchings/76C20", func_80076EA0);

extern s32 D_801045C8[16];
void func_80001228(s32 soundHandle);

void func_80077004(void) {
    s32 i;
    for (i = 0; i < 16; i++) {
        if (D_801045C8[i] != 0) {
            func_80001228(D_801045C8[i]);
            D_801045C8[i] = 0;
        }
    }
}

extern void *D_800A74B0;
extern s32 D_800A74B8;
void mempool_free(void *data);
void func_80077158(void);

void func_80077060(void) {
    if (D_800A74B0 != NULL) {
        mempool_free(D_800A74B0);
        D_800A74B0 = NULL;
        D_800A74B8 = -1;
        func_80077158();
    }
}

extern s32 D_800A74B4;
extern s32 *D_801045B4;
void *mempool_alloc_safe(s32 size, u32 colourTag);
s32 asset_load(u32 assetIndex, u32 address, s32 assetOffset, s32 size);

INCLUDE_ASM("asm/nonmatchings/76C20", func_800770A4);

INCLUDE_ASM("asm/nonmatchings/76C20", func_80077158);

INCLUDE_ASM("asm/nonmatchings/76C20", func_800772C4);

void func_8007637C(u8 index);

void func_800773AC(void) {
    s32 i;
    for (i = 0; i < 256; i++) {
        func_8007637C(i);
    }
}

void func_800766A8(u8 index);
void func_80054530();
extern s32 D_800A74D8;
extern f32 D_800A74DC;

void func_800773EC(void) {
    s32 i;

    if (D_800A74BC != NULL) {
        for (i = 0; i < 256; i++) {
            func_800766A8(i);
        }
        D_800A74CC = D_800A74C8;
        D_800A74D8 = 0;
        D_800A74DC = 0.0f;
        func_80077004();
        func_80054530();
    }
}

INCLUDE_ASM("asm/nonmatchings/76C20", func_80077468);

INCLUDE_ASM("asm/nonmatchings/76C20", func_80077614);

INCLUDE_ASM("asm/nonmatchings/76C20", func_80077ABC);

typedef struct CamPoint {
    /* 0x00 */ s16 yaw;
    /* 0x02 */ s16 pitch;
    /* 0x04 */ s16 roll;
    /* 0x06 */ u8 pad6[6];
    /* 0x0C */ f32 x;
    /* 0x10 */ f32 y;
    /* 0x14 */ f32 z;
    /* 0x18 */ u8 pad18[0x16];
    /* 0x2E */ s16 unk2E;
} CamPoint;

typedef struct CamTarget {
    /* 0x00 */ s16 yaw;
    /* 0x02 */ s16 pitch;
    /* 0x04 */ s16 roll;
    /* 0x06 */ u8 pad6[6];
    /* 0x0C */ f32 x;
    /* 0x10 */ f32 y;
    /* 0x14 */ f32 z;
    /* 0x18 */ u8 pad18[0x26];
    /* 0x3E */ s16 unk3E;
} CamTarget;

extern CamPoint *D_801045B8[];
extern f32 D_800A74E4;
void func_8003F7F4(f32 fov, s32 force);

/* JFG: copy camera point index onto the camera (facing the other way) and set the FoV. */
CamPoint *func_80079890(s32 index, CamTarget *cam) {
    CamPoint *point = D_801045B8[index];

    if (point != NULL) {
        cam->x = point->x;
        cam->y = point->y;
        cam->z = point->z;
        cam->unk3E = point->unk2E;
        cam->yaw = 0x8000 - point->yaw;
        cam->pitch = -point->pitch;
        cam->roll = point->roll;
        func_8003F7F4(D_800A74E4, 0);
    }
    return point;
}

INCLUDE_ASM("asm/nonmatchings/76C20", func_80079930);

/* JFG: the path of an object (NULL if none). */
AnimPath *func_80079AA4(AnimObject *obj) {
    s32 i;

    for (i = 0; i < 256; i++) {
        if (D_801045B0[i] != NULL && D_801045B0[i]->obj == obj) {
            return D_801045B0[i];
        }
    }
    return NULL;
}

s32 func_80079B6C(void) {
    s32 i = 256;
    while (i--) {
        if (D_801045B0[i] == NULL) {
            return i;
        }
    }
    return -1;
}

extern s32 D_800A74D4;

void func_80079BA8(void) {
    D_800A74D4 = 0x1;
}

extern s32 D_800A74D4;

void func_80079BB8(void) {
    D_800A74D4 = 0;
}
