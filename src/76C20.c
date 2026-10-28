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

/* Size: 0x2C bytes */
typedef struct AnimNode {
    /* 0x00 */ u8 pad0[0x24];
    /* 0x24 */ struct AnimNode *prev;
    /* 0x28 */ struct AnimNode *next;
} AnimNode;

typedef struct AnimPath {
    /* 0x00 */ u8 unk0;
    /* 0x01 */ u8 unk1;
    /* 0x02 */ u8 pad2[2];
    /* 0x04 */ s16 unk4;
    /* 0x06 */ u8 unk6;
    /* 0x07 */ u8 unk7;
    /* 0x08 */ AnimObject *obj;
    /* 0x0C */ f32 unkC;
    /* 0x10 */ f32 unk10;
    /* 0x14 */ u8 unk14;
    /* 0x15 */ u8 unk15;
    /* 0x16 */ u8 flags; // JFG: 0x80 = the path loops
    /* 0x17 */ u8 numNodes;
    /* 0x18 */ AnimNode *curNode;
    /* 0x1C */ f32 unk1C;
    /* 0x20 */ AnimNode *nodes;
    /* 0x24 */ u8 unk24;
    /* 0x25 */ u8 unk25;
    /* 0x26 */ u8 unk26;
    /* 0x27 */ u8 unk27;
    /* 0x28 */ u8 pad28[2];
    /* 0x2A */ s16 unk2A;
    /* 0x2C */ f32 unk2C;
    /* 0x30 */ f32 unk30;
} AnimPath;
extern AnimPath **D_801045B0;

void free_object(AnimObject *object);
void mempool_free(void *data);
void audspat_point_stop(void *point);
void audspat_play_sound_at_position(u16 soundId, f32 x, f32 y, f32 z, u8 flags, void **handlePtr);

extern s32 *D_801047C8;
extern s32 D_801047CC;
extern s32 D_801047D0;

/* Starts reading a bit stream (0x80076020). */
void bitstream_init(s32 *arg0) {
    D_801047C8 = arg0;
    D_801047CC = *arg0;
    D_801047D0 = 0x80;
}

/* JFG: read count bits (most significant first) from the bit stream (0x80076044). */
s32 bitstream_read_bits(u32 count) {
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

/* JFG: read a signed count-bit number (sign-extended) from the bit stream (0x800760C0). */
s32 bitstream_read_signed_bits(u32 count) {
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

/* Ends reading the bit stream, storing the position after it (0x80076154). */
void bitstream_end(void) {
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

#ifdef NON_MATCHING
/* NON_MATCHING: count and total swapped between s2 and s3 (16 declaration orders tried) */
/* JFG: the length of the command list at D_800A74C8 (0 if it is broken). */
s32 func_800761E8() { // JFG: no parameters (the caller passes &D_800A74C8)
    u16 *cmd = (u16 *) D_800A74C8;
    s32 len;
    s32 total = 0;
    s32 done = FALSE;
    s32 count = 0;

    if (cmd != NULL) {
        do {
            if ((u8) (cmd[1] >> 8) == 0x7F) {
                done = TRUE;
            }
            len = func_800761C8(cmd);
            total += len;
            if (len != 0) {
                count++;
                cmd += len >> 1;
                if (count > 0x2000) {
                    done = TRUE;
                }
            } else {
                total = 0;
                done = TRUE;
            }
        } while (!done);
    }
    return total;
}
#else
INCLUDE_ASM("asm/nonmatchings/76C20", func_800761E8);
s32 func_800761E8(s32 *arg0);
#endif

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

/* The module trap (0x80054530) under a prototype with a float argument: needs
   `func_80054530_f32 = 0x80054530;` in undefined_syms.us.txt. */
void func_80054530_f32(AnimPath *path, f32 arg1, s32 arg2, s32 arg3);

/* JFG: restart path index from its first node (unless it is held), resetting its object's state. */
void anim_path_restart(u8 index) {
    AnimPath *path = D_801045B0[index];
    AnimObject *obj;

    if (path != NULL && !(path->flags & 8)) {
        path->unk1 = path->unk0;
        path->unkC = path->unk4 / (f32) 16384;
        path->unk14 = path->unk6;
        path->unk15 = path->unk7;
        path->curNode = path->nodes;
        path->flags &= 0x80;
        path->unk10 = 1.0f;
        path->unk1C = 0.0f;
        obj = path->obj;
        if (obj != NULL) {
            obj->flags |= 0x400;
            path->unk24 = 0xFF;
            path->unk25 = 0xFF;
            path->unk26 = 0;
            path->unk27 = 0;
            path->unk2A = 0;
            path->unk2C = 1.0f;
            path->unk30 = 0.0f;
            if (path->obj->unk5C != 0) {
                path->obj->model->flags &= ~1;
            }
            func_80054530_f32(path, 0.0f, 0, 0);
            if (obj->audioPoint) {
                audspat_point_stop((void *) (s32) obj->audioPoint); // the int cast loads it into v0
                obj->audioPoint = NULL;
            }
        }
    }
}

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

#ifdef NON_MATCHING
/* NON_MATCHING: the audio point is loaded into a0 (target: v0, moved to a0 in the jal delay slot) */
void func_80076ADC(u8 index) {
    AnimPath *path = D_801045B0[index];
    AnimObject *obj;

    if (path != NULL) {
        obj = path->obj;
        if (obj != NULL) {
            if (obj->audioPoint != NULL) {
                audspat_point_stop(obj->audioPoint);
            }
        }
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/76C20", func_80076ADC);
#endif

INCLUDE_ASM("asm/nonmatchings/76C20", func_80076B44);

/* JFG: link path index's nodes to their neighbours; the ends link to each other on a looping path, else to themselves. */
void anim_path_link_nodes(u8 index) {
    AnimPath *path = D_801045B0[index];
    s32 i;

    if (path != NULL) {
        if (path->numNodes >= 2) {
            for (i = 0; i < path->numNodes; i++) {
                if (i > 0) {
                    path->nodes[i].prev = &path->nodes[i - 1];
                }
                if (i < path->numNodes - 1) {
                    path->nodes[i].next = &path->nodes[i + 1];
                }
            }
        }
        if (path->flags & 0x80) {
            path->nodes[0].prev = &path->nodes[path->numNodes - 1];
            path->nodes[path->numNodes - 1].next = &path->nodes[0];
        } else {
            path->nodes[0].prev = &path->nodes[0];
            path->nodes[path->numNodes - 1].next = &path->nodes[path->numNodes - 1];
        }
    }
}

typedef struct CamPoint CamPoint;

/* Size: 0x14 bytes */
typedef struct AnimTrigger {
    /* 0x00 */ u8 unk0;
    /* 0x01 */ u8 pad1[3];
    /* 0x04 */ s32 unk4;
    /* 0x08 */ u8 pad8[4];
    /* 0x0C */ s32 unkC;
    /* 0x10 */ u8 pad10[4];
} AnimTrigger;

/* Size: 0x8 bytes */
typedef struct AnimSlot {
    /* 0x00 */ s8 unk0;
    /* 0x01 */ u8 pad1[7];
} AnimSlot;

/* Size: 0x10 bytes */
typedef struct AnimEntry {
    /* 0x00 */ s32 unk0;
    /* 0x04 */ u8 pad4[0xC];
} AnimEntry;

extern s32 *D_801045B4;
extern CamPoint *D_801045B8[4];
extern s32 D_801045C8[16];
extern AnimTrigger D_80104608[8];
extern AnimSlot D_801046A8[8];
extern AnimEntry D_80104708[12];
extern s32 D_800A74E0;
extern s16 D_801046EE;
extern s16 D_801046F4;
extern s32 D_801046F8;
extern s16 D_801046FC;
extern s16 D_80104702;
extern s16 D_80104704;
s32 *asset_table_load(u32 assetIndex);
void *mempool_alloc_safe(s32 size, u32 colourTag);
void func_80054530(); // a module function (patched in by the runtime linker)

/* JFG: load the path offset table, allocate the 256 path slots and clear all the sequence state. */
void anim_system_init(void) {
    s32 i;

    D_801045B4 = asset_table_load(0x41);
    D_801045B0 = mempool_alloc_safe(0x400, 0x4080F0FF);
    for (i = 0; i < 256; i++) {
        D_801045B0[i] = NULL;
    }
    for (i = 0; i < 4; i++) {
        D_801045B8[i] = NULL;
    }
    for (i = 0; i < 16; i++) {
        D_801045C8[i] = 0;
    }
    for (i = 0; i < 8; i++) {
        D_80104608[i].unk0 = 0xFF;
        D_80104608[i].unk4 = 0;
        D_80104608[i].unkC = 0;
    }
    for (i = 0; i < 8; i++) {
        D_801046A8[i].unk0 = -1;
    }
    D_800A74E0 = 0;
    i = 0;
    while (i < 12) {
        D_80104708[i++].unk0 = 0;
    }
    D_801046EE = 0;
    D_801046F4 = 0;
    D_801046F8 = 0;
    D_80104702 = 0xFF;
    D_80104704 = D_80104702;
    D_801046FC = 0;
    func_80054530();
}

void sound_stop(s32 soundHandle);

void func_80077004(void) {
    s32 i;
    for (i = 0; i < 16; i++) {
        if (D_801045C8[i] != 0) {
            sound_stop(D_801045C8[i]);
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
s32 asset_load(u32 assetIndex, u32 address, s32 assetOffset, s32 size);

#ifdef NON_MATCHING
/* NON_MATCHING: one temp register (the reloaded index is t3, target t2) */
/* JFG: load entry index of asset 0x42 (offsets in D_801045B4) into a new buffer, unless it is already loaded. */
void func_800770A4(s32 index) {
    s32 start;
    void *buffer;

    if (index != -1 && index != D_800A74B8) {
        func_80077060();
        start = D_801045B4[index];
        D_800A74B4 = D_801045B4[index + 1] - start;
        if (D_800A74B4 > 0) {
            if ((D_800A74B0 = buffer = mempool_alloc_safe(D_800A74B4, 0x4080F0FF)) != NULL) {
                asset_load(0x42, (u32) buffer, start, D_800A74B4);
                D_800A74B8 = index;
            }
        }
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/76C20", func_800770A4);
#endif

INCLUDE_ASM("asm/nonmatchings/76C20", func_80077158);

typedef struct LevelHeader {
    /* 0x00 */ u8 pad0[0xAC];
    /* 0xAC */ s8 unkAC; // JFG: the camera FoV
} LevelHeader;
LevelHeader *level_header(void);
void func_80077468(void);
void func_800773AC(void);
void func_800773EC(void);
extern s32 D_800A74C0;
extern f32 D_800A74E4;
extern f32 D_800A74E8;
extern s32 D_800A74EC;
extern s32 D_800A74D4;

/* JFG: find sequence id in the loaded table (entries: id << 24 | offset, ending with id 0xFF) and start it. */
void sequence_start(s32 id) {
    u32 *table;
    u32 entry;
    s32 entryId;
    s32 offset;

    if (id >= 0 && id < 256 && id != D_800A74C0) {
        func_80077158();
        table = D_800A74B0;
        do {
            entry = *table++;
            entryId = entry >> 24;
        } while (id != entryId && entryId != 0xFF);
        if (entryId == id) {
            offset = entry & 0xFFFFFF;
            if (offset != 0xFFFFFF) {
                D_800A74C0 = entryId;
                D_800A74BC = (s32 *) ((u8 *) D_800A74B0 + offset);
                func_80077468();
                func_800773AC();
                func_800773EC();
                D_800A74E4 = level_header()->unkAC;
                D_800A74E8 = 0.0f;
                D_800A74EC = 0;
                D_800A74D4 = 1;
            }
        }
    }
}

void func_8007637C(u8 index);

void func_800773AC(void) {
    s32 i;
    for (i = 0; i < 256; i++) {
        func_8007637C(i);
    }
}

void anim_path_restart(u8 index);
void func_80054530(); // a module function (patched in by the runtime linker)
extern s32 D_800A74D8;
extern f32 D_800A74DC;

void func_800773EC(void) {
    s32 i;

    if (D_800A74BC != NULL) {
        for (i = 0; i < 256; i++) {
            anim_path_restart(i);
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

struct CamPoint {
    /* 0x00 */ s16 yaw;
    /* 0x02 */ s16 pitch;
    /* 0x04 */ s16 roll;
    /* 0x06 */ u8 pad6[6];
    /* 0x0C */ f32 x;
    /* 0x10 */ f32 y;
    /* 0x14 */ f32 z;
    /* 0x18 */ u8 pad18[0x16];
    /* 0x2E */ s16 unk2E;
};

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
