#include "common.h"

typedef struct AnimModel {
    /* 0x00 */ u8 pad0[0xA];
    /* 0x0A */ u16 flags;
} AnimModel;

typedef struct AnimObjectHeader {
    /* 0x00 */ u8 pad0[0x1F];
    /* 0x1F */ s8 numVariants; // JFG: the number of values unk3A can take
} AnimObjectHeader;

typedef struct AnimObject {
    /* 0x00 */ u8 pad0[0x6];
    /* 0x06 */ s16 flags;
    /* 0x08 */ u8 pad8[0x4];
    /* 0x0C */ f32 x;
    /* 0x10 */ f32 y;
    /* 0x14 */ f32 z;
    /* 0x18 */ u8 pad18[0x1C - 0x18];
    /* 0x1C */ f32 vx; // JFG: velocity (its length sets a path sound's pitch)
    /* 0x20 */ f32 vy;
    /* 0x24 */ f32 vz;
    /* 0x28 */ u8 pad28[0x3A - 0x28];
    /* 0x3A */ s8 unk3A;
    /* 0x3B */ u8 pad3B;
    /* 0x3C */ s32 unk3C;
    /* 0x40 */ AnimObjectHeader *header;
    /* 0x44 */ u8 pad44[4];
    /* 0x48 */ s16 behaviorId;
    /* 0x4A */ u8 pad4A[2];
    /* 0x4C */ AnimModel *model;
    /* 0x50 */ u8 pad50[0xC];
    /* 0x5C */ s32 unk5C;
    /* 0x60 */ u8 pad60[0x28];
    /* 0x88 */ void *audioPoint;
} AnimObject;

/* Size: 0x2C bytes */
typedef struct AnimNode {
    /* 0x00 */ s16 rot[3];
    /* 0x06 */ u8 unk6;
    /* 0x07 */ u8 unk7;
    /* 0x08 */ f32 scale;
    /* 0x0C */ f32 x;
    /* 0x10 */ f32 y;
    /* 0x14 */ f32 z;
    /* 0x18 */ s16 unk18[3];
    /* 0x1E */ u8 unk1E;
    /* 0x1F */ u8 pad1F;
    /* 0x20 */ f32 unk20;
    /* 0x24 */ struct AnimNode *prev;
    /* 0x28 */ struct AnimNode *next;
} AnimNode;

typedef struct AnimPath {
    /* 0x00 */ u8 unk0;
    /* 0x01 */ u8 unk1;
    /* 0x02 */ s16 unk2;
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
    /* 0x28 */ u8 unk28;
    /* 0x29 */ u8 unk29;
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

void sequence_commands_reset(void) {
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

/* JFG: the length of the command list at D_800A74C8 (0 if it is broken). */
s32 command_list_length() { // JFG: no parameters (the caller passes &D_800A74C8)
    u16 *cmd = (u16 *) D_800A74C8;
    s32 len;
    s32 count = 0;
    s32 done = FALSE;
    s32 total = 0;

    if (cmd != NULL) {
        while (!done) {
            if ((u8) (cmd[1] >> 8) == 0x7F) {
                done = TRUE;
            }
            len = func_800761C8(cmd);
            total += len;
            if (len != 0) {
                cmd += len >> 1;
                count++;
                if (count > 0x2000) {
                    done = TRUE;
                }
            } else {
                total = 0;
                done = TRUE;
            }
        }
    }
    return total;
}

extern s32 *D_800A74BC;

/* JFG: start reading the data at D_800A74BC (its first word's low 24 bits are the offset to the start). */
void sequence_commands_begin(void) {
    s32 header = *D_800A74BC;

    D_800A74C8 = (s32) D_800A74BC + (header & 0xFFFFFF);
    D_800A74CC = D_800A74C8;
    D_800A74D0 = command_list_length(&D_800A74C8);
}

void anim_path_free(u8 index) {
    AnimPath *path = D_801045B0[index];

    if (path != NULL) {
        if (path->obj != NULL) {
            free_object(path->obj);
        }
        mempool_free(path);
    }
    D_801045B0[index] = NULL;
    // The file's three debug strings: nothing references them (their calls are compiled out) and where they
    // were is unknown (they read like anim_path_spawn_object's, but there they change its registers).
    stubbed_printf("Path %d, Obj %d\n");
    stubbed_printf("Before Level Number %d (%d)\n");
    stubbed_printf("After  Level Number %d\n");
}

/* JFG: the start of a level object entry (the object ID is an s16 and the size byte follows it) */
typedef struct LevelObjectEntryCommon {
    /* 0x00 */ s16 objectID;
    /* 0x02 */ u8 size;
    /* 0x04 */ s16 x, y, z;
} LevelObjectEntryCommon;

typedef struct LevelHeader {
    /* 0x00 */ u8 pad0[0x6C];
    /* 0x6C */ s8 unk6C;
    /* 0x6D */ u8 pad6D[0xAC - 0x6D];
    /* 0xAC */ s8 unkAC; // JFG: the camera FoV
} LevelHeader;
LevelHeader *level_header(void);
s32 func_800485C8(s32 player);
s32 game_flag_get(s32 flag);
s16 func_800468CC(void);
AnimObject *spawn_object(LevelObjectEntryCommon *entry, s32 arg1);
void obj_queue_add(AnimObject *obj);
extern s16 D_800A7570[8];
extern s16 D_800A7580[4];
extern s16 D_800A7588[4];
extern u8 D_800A4FC0;

/**
 * JFG: spawn the object of path index at its first node, unless it has one: the object ID is first swapped
 * through small tables for some IDs (by character, flag 0x5F or the level type); the new object gets flag
 * 0x400 as on a path restart.
 */
void anim_path_spawn_object(u8 index) {
    s32 c;
    LevelObjectEntryCommon setup;
    s8 type;
    AnimObject *obj;
    AnimNode *node;
    AnimPath *path = D_801045B0[index];

    if (path != NULL) {
        node = path->nodes;
        if (node != NULL && path->obj == NULL && path->unk2 != -1) {
            if (path->unk2 == 0xC5 || path->unk2 == 0xC6) {
                c = func_800485C8(0);
                if (game_flag_get(0x5F)) {
                    c |= 4;
                }
                path->unk2 = D_800A7570[c & 7];
            }
            if (path->unk2 == 0x106 || path->unk2 == 0x8A || path->unk2 == 0x10A) {
                type = level_header()->unk6C;
                if (type == 1 || type == 2) {
                    path->unk2 = D_800A7580[func_800468CC() & 3];
                } else {
                    path->unk2 = D_800A7588[func_800468CC() & 3];
                }            }
            setup.x = node->x;
            setup.y = node->y;
            setup.z = node->z;
            setup.size = 10;
            setup.objectID = path->unk2;
            path->obj = spawn_object(&setup, 1);
            obj = path->obj;
            if (obj != NULL) {
                obj->flags |= 0x400;
                obj->unk3C = 0;
                if (obj->behaviorId == 0x33) {
                    obj_queue_add(path->obj);
                }
                path->unk24 = 0xFF;
                path->unk25 = 0xFF;
                path->unk26 = 0;
                path->unk27 = 0;
                path->unk2A = 0;
                path->unk2C = 1.0f;
                path->unk30 = 0.0f;
                if (path->unk2 == 0x29B || path->unk2 == 0x29C || path->unk2 == 0x29D) {
                    obj->unk3A = func_800468CC() & 3;
                    if (obj->unk3A == 3) {
                        obj->unk3A = D_800A4FC0 & 3;
                    }
                    if (obj->unk3A >= obj->header->numVariants) {
                        obj->unk3A = 0;
                    }
                }
                if (path->obj->unk5C != 0) {
                    path->obj->model->flags &= ~1;
                }
            }
        }
        path->unk28 = 100;
        path->unk29 = 0;
    }
}

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

void anim_path_lock(u8 index) {
    AnimPath *path = D_801045B0[index];
    if (path != NULL) {
        path->flags |= 8;
    }
}

void anim_path_unlock(u8 index) {
    AnimPath *path = D_801045B0[index];
    if (path != NULL) {
        path->flags &= ~8;
    }
}

void anim_path_sound_play(u8 index, u16 soundId) {
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

/* JFG: stop the sound of a path's object, if it has one playing. */
void anim_path_sound_stop(u8 index) {
    AnimPath *path = D_801045B0[index];
    AnimObject *obj;

    if (path != NULL) {
        obj = path->obj;
        if (obj != NULL) {
            if (obj->audioPoint != NULL) {
                audspat_point_stop((void *) (s32) obj->audioPoint); // the int cast loads it into v0
            }
        }
    }
}


/* JFG: unpack numNodes path nodes from the bit stream at data; the next node and stream position go to
 * nodeOut and dataOut. */
void anim_path_nodes_read(s32 numNodes, AnimNode *node, u8 *data, AnimNode **nodeOut, u8 **dataOut) {
    s32 fullScale;
    f32 scaleUnit;

    if (numNodes > 0) {
        scaleUnit = 0.01f;
        do {
        bitstream_init((s32 *) &data);
        node->unk6 = bitstream_read_bits(8);
        fullScale = bitstream_read_bits(1);
        node->unk7 = bitstream_read_bits(1);
        node->x = bitstream_read_signed_bits(18) * 0.125f;
        node->y = bitstream_read_signed_bits(18) * 0.125f;
        node->z = bitstream_read_signed_bits(18) * 0.125f;
        node->rot[0] = bitstream_read_bits(12) * 16;
        node->rot[1] = bitstream_read_bits(12) * 16;
        node->rot[2] = bitstream_read_bits(12) * 16;
        node->scale = (u32) bitstream_read_bits(12) * scaleUnit;
        if (!fullScale) {
            node->scale *= 0.390625f;
        }
        node->unk18[0] = bitstream_read_signed_bits(12) * 4;
        node->unk18[1] = bitstream_read_signed_bits(12) * 4;
        node->unk18[2] = bitstream_read_signed_bits(12) * 4;
        node->unk20 = (u32) bitstream_read_bits(12) * 0.5f;
        node->unk1E = 0;
        bitstream_end();
        numNodes--;
        node++;
        } while (numNodes > 0);
    }
    if (nodeOut != NULL) {
        *nodeOut = node;
    }
    if (dataOut != NULL) {
        *dataOut = data;
    }
}

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

void sequence_sounds_stop_all(void) {
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
void sequence_stop(void);

void sequence_table_free(void) {
    if (D_800A74B0 != NULL) {
        mempool_free(D_800A74B0);
        D_800A74B0 = NULL;
        D_800A74B8 = -1;
        sequence_stop();
    }
}

extern s32 D_800A74B4;
s32 asset_load(u32 assetIndex, u32 address, s32 assetOffset, s32 size);

/* JFG: load entry index of asset 0x42 (offsets in D_801045B4) into a new buffer, unless it is already loaded. */
void sequence_table_load(s32 index) {
    s32 start;
    void *buffer;

    if (index != -1 && index != D_800A74B8) {
        sequence_table_free();
        start = D_801045B4[index];
        D_800A74B4 = D_801045B4[index + 1] - start;
        if (D_800A74B4 > 0) {
            if ((buffer = D_800A74B0 = mempool_alloc_safe(D_800A74B4, 0x4080F0FF)) != NULL) {
                asset_load(0x42, (u32) buffer, start, D_800A74B4);
                D_800A74B8 = index;
            }
        }
    }
}

extern s32 D_800A74C0; // JFG: the current sequence id (-1 for none)
extern s32 D_800A74C4; // JFG: set when the sequence data at D_800A74BC was allocated
void anim_path_free(u8 index);

/* JFG: stop the current sequence: free its data if it owns it, free every path and clear the sequence state. */
void sequence_stop(void) {
    s32 i;

    if (D_800A74BC != NULL) {
        if (D_800A74C4) {
            mempool_free(D_800A74BC);
            D_800A74C4 = 0;
        }
        D_800A74BC = NULL;
        D_800A74C0 = -1;
        sequence_commands_reset();
        for (i = 0; i < 256; i++) {
            anim_path_free(i);
        }
        for (i = 0; i < 4; i++) {
            D_801045B8[i] = NULL;
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
        sequence_sounds_stop_all();
        func_80054530();
    }
}

void sequence_paths_create(void);
void anim_paths_spawn_all(void);
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
        sequence_stop();
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
                sequence_paths_create();
                anim_paths_spawn_all();
                func_800773EC();
                D_800A74E4 = level_header()->unkAC;
                D_800A74E8 = 0.0f;
                D_800A74EC = 0;
                D_800A74D4 = 1;
            }
        }
    }
}

void anim_path_spawn_object(u8 index);

void anim_paths_spawn_all(void) {
    s32 i;
    for (i = 0; i < 256; i++) {
        anim_path_spawn_object(i);
    }
}

void anim_path_restart(u8 index);
void func_80054530(); // a module function (patched in by the runtime linker)
extern u32 D_800A74D8; // JFG: the sequence clock (ticks)
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
        sequence_sounds_stop_all();
        func_80054530();
    }
}

/* JFG: a path's data in the sequence (nodes follow at 8) */
typedef struct AnimPathData {
    /* 0x00 */ u8 numNodes;
    /* 0x01 */ u8 unk1;
    /* 0x02 */ s16 unk2;
    /* 0x04 */ s16 unk4;
    /* 0x06 */ u8 unk6;
    /* 0x07 */ u8 unk7; // JFG: 0x80 = the path loops
    /* 0x08 */ u8 nodes[1];
} AnimPathData;

void anim_path_nodes_read(s32 numNodes, AnimNode *node, u8 *data, AnimNode **nodeOut, u8 **dataOut);
void anim_path_link_nodes(u8 index);

/* JFG: create the sequence's paths (count in the top byte of its first word, then id << 24 | offset per path). */
void sequence_paths_create(void) {
    u32 *entry = (u32 *) D_800A74BC;
    u32 header;
    s32 count;
    u32 e;
    AnimPathData *data;
    s32 slot;
    u8 index;
    u8 flags;
    AnimPath *path;

    header = *entry++;
    count = (header >> 24) & 0xFF;
    sequence_commands_begin();
    while (count > 0) {
        e = *entry++;
        data = (AnimPathData *) ((u8 *) D_800A74BC + (e & 0xFFFFFF));
        slot = (e >> 24) & 0xFF;
        D_801045B0[slot] = mempool_alloc_safe(data->numNodes * sizeof(AnimNode) + sizeof(AnimPath), 0x4080F0FF);
        path = D_801045B0[slot];
        index = slot;
        if (path != NULL) {
            if (data->numNodes > 0) {
                path->nodes = (AnimNode *) (path + 1);
            } else {
                path->nodes = NULL;
            }
            path->unk2 = data->unk2;
            path->obj = NULL;
            path->unk0 = data->unk1;
            path->unk4 = data->unk4;
            path->unk6 = data->unk6;
            path->unk7 = data->unk7;
            path->flags = 0;
            flags = path->unk7;
            if (flags & 0x80) {
                path->flags = 0x80;
                path->unk7 = flags & 0x7F;
            }
            path->numNodes = data->numNodes;
            anim_path_restart(index);
            anim_path_nodes_read(path->numNodes, path->nodes, data->nodes, 0, 0);
            anim_path_link_nodes(index);
        }
        count--;
    }
}

void func_80077ABC(void);
f32 sqrtf(f32 x);
void audspat_point_set_position(void *audioPoint, f32 x, f32 y, f32 z);
void audspat_point_set_pitch(void *audioPoint, u8 pitch);

#ifdef NON_MATCHING
/* JFG: advance the running sequence by updateRate ticks: run its next command when its time has come (0x7B: wait,
   then set the sequence state), its clock, the paths and the positioned sounds of the paths' objects (pitch from the
   object's speed).
   NON_MATCHING: ROM length; the command block's a0-a3/t0 rotated and the state store written before the command
   pointer's (the ROM reads the command's low byte before that store but stores it after). */
void func_80077614(s32 updateRate) {
    s32 i;
    AnimPath *path;
    s32 rate;
    s32 frames;
    u32 time;
    AnimObject *obj;
    f32 speed;
    f32 frameTime;

    if (D_800A74BC != NULL && D_800A74D4 != 0) {
        if (osTvType == 0) {
            frameTime = 0.02f;
        } else {
            frameTime = 1.0f / 60.0f;
        }
        rate = updateRate;
        if (D_800A74E0 > 0) {
            func_80054530(updateRate);
        }
        if (D_800A74CC != 0 && D_800A74D4 == 1) {
            if ((((u16 *) D_800A74CC)[1] >> 8) == 0x7B) {
                if (((u16 *) D_800A74CC)[0] / 100.0f < (D_800A74D8 + updateRate) * frameTime) {
                    frames = ((u16 *) D_800A74CC)[0];
                    if (osTvType == 0) {
                        frames = ((u16 *) D_800A74CC)[0] >> 1;
                    } else {
                        frames = frames * 6 / 10;
                    }
                    updateRate = frames - D_800A74D8;
                    D_800A74D4 = (s8) ((u16 *) D_800A74CC)[1];
                    D_800A74CC += 4;
                    if (D_800A74D4 == 0) {
                        rate = updateRate;
                    }
                }
            }
        }
        if (updateRate > 0) {
            time = D_800A74D8 + updateRate;
            for (i = 0; i < 4; i++) {
                D_801045B8[i] = NULL;
            }
            if (D_800A74EC != 0) {
                if (updateRate < D_800A74EC) {
                    D_800A74E4 += D_800A74E8 * updateRate;
                    D_800A74EC -= updateRate;
                } else {
                    D_800A74E4 += D_800A74E8 * D_800A74EC;
                    D_800A74EC = 0;
                }
            }
            D_800A74D8 = time;
            D_800A74DC = time * frameTime;
            for (i = 0; i < 256; i++) {
                path = D_801045B0[i];
                if (path != NULL && (path->flags & 5)) {
                    func_80054530_f32(path, updateRate * frameTime, updateRate, rate);
                }
            }
            if (D_800A74D4 == 1) {
                func_80077ABC();
            }
            func_80054530(updateRate);
            func_80054530(updateRate);
            func_80054530(updateRate);
            for (i = 0; i < 256; i++) {
                path = D_801045B0[i];
                if (path != NULL) {
                    obj = path->obj;
                    if (obj != NULL && obj->audioPoint != NULL) {
                        audspat_point_set_position((void *) (s32) obj->audioPoint, obj->x, obj->y, obj->z);
                        if (path->unk28 != 100 || path->unk29 != 0) {
                            speed = sqrtf(obj->vx * obj->vx + obj->vy * obj->vy + obj->vz * obj->vz);
                            audspat_point_set_pitch(obj->audioPoint, path->unk28 + path->unk29 * speed);
                        }
                    }
                }
            }
        }
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/76C20", func_80077614);
#endif

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

/* JFG: find up to 3 paths with a node within 32 units (horizontally, 16 vertically) of (x, y, z); the path ids
 * go to pathIds (-1 terminated, 4 entries) and the nearest node index of each to nodeIds. */
void anim_paths_find_near(f32 x, f32 y, f32 z, s16 *pathIds, s16 *nodeIds) {
    s32 count = 0;
    s32 i;
    s32 nodeIndex;
    AnimPath *path;
    AnimNode *node;
    f32 dx;
    f32 dy;
    f32 dz;

    i = 4;
    while (i--) {
        pathIds[i] = -1;
    }
    if (D_801045B0 == NULL) {
        return;
    }
    for (i = 0; i < 0x100; i++) {
        path = D_801045B0[i];
        if (path != NULL) {
            node = path->nodes;
            nodeIndex = 0;
            while (node != NULL) {
                dy = node->y - y;
                if (-16.0f < dy && dy < 16.0f) {
                    dx = node->x - x;
                    dz = node->z - z;
                    if (dx * dx + dz * dz < 1024.0f) {
                        *pathIds++ = i;
                        count++;
                        *nodeIds++ = nodeIndex;
                        if (count == 3) {
                            return;
                        }
                        break;
                    }
                }
                if (node->next == path->nodes || node == node->next) {
                    node = NULL;
                } else {
                    node = node->next;
                    nodeIndex++;
                }
            }
        }
    }
}

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
