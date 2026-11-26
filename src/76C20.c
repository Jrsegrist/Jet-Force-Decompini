/*
 * Animation paths and sequences: objects moved along paths by the level's sequence scripts, and the
 * sequence's camera points.
 */
#include "common.h"

typedef struct AnimModel {
    /* 0x00 */ u8 pad0[0xA];
    /* 0x0A */ u16 flags;
} AnimModel;

typedef struct AnimObjectHeader {
    /* 0x00 */ u8 pad0[0x1F];
    /* 0x1F */ s8 numVariants; // JFG: the number of values modelIndex can take
    /* 0x20 */ u8 pad20[0x25 - 0x20];
    /* 0x25 */ s8 unk25; // JFG: the number of unk74 entries (func_80077ABC)
    /* 0x26 */ u8 unk26; // JFG: the number of unk78 entries (func_80077ABC)
    /* 0x27 */ u8 pad27[0x5C - 0x27];
    /* 0x5C */ f32 unk5C;
} AnimObjectHeader;

/* JFG: what an object's unk50 points to (func_80077ABC; unk1C points to another) */
typedef struct AnimObjectSub50 {
    /* 0x00 */ f32 unk0;
    /* 0x04 */ u8 pad4[0xC - 0x4];
    /* 0x0C */ u8 unkC;
    /* 0x0D */ u8 unkD;
    /* 0x0E */ u8 padE[0x1C - 0xE];
    /* 0x1C */ struct AnimObjectSub50 *unk1C;
    /* 0x20 */ u8 pad20[0xAC - 0x20];
    /* 0xAC */ f32 unkAC;
    /* 0xB0 */ u8 padB0[0xBA - 0xB0];
    /* 0xBA */ u8 unkBA;
} AnimObjectSub50;

/* JFG: an entry of an object's unk68 table (12 bytes; func_80077ABC) */
typedef struct AnimObjectSub68 {
    /* 0x00 */ s32 unk0;
    /* 0x04 */ u8 unk4;
    /* 0x05 */ u8 unk5;
    /* 0x06 */ u8 unk6;
    /* 0x07 */ u8 unk7;
    /* 0x08 */ u8 unk8;
    /* 0x09 */ u8 pad9[3];
} AnimObjectSub68;

/* JFG: a texture as func_80077ABC changes it */
typedef struct AnimTexture {
    /* 0x00 */ u8 pad0[0x6];
    /* 0x06 */ s16 flags;
    /* 0x08 */ u8 pad8[0x14 - 0x8];
    /* 0x14 */ s16 unk14;
} AnimTexture;

/* JFG: a texture list entry (8 bytes) */
typedef struct AnimTextureEntry {
    /* 0x00 */ AnimTexture *texture;
    /* 0x04 */ u32 flags;
} AnimTextureEntry;

typedef struct AnimModelHeader {
    /* 0x00 */ u8 pad0[0x10];
    /* 0x10 */ u8 numTextures;
    /* 0x11 */ u8 pad11[0x18 - 0x11];
    /* 0x18 */ AnimTextureEntry *textures;
} AnimModelHeader;

/* JFG: a model instance of an object (its model header and per-texture flags) */
typedef struct AnimModelInstance {
    /* 0x00 */ AnimModelHeader *header;
    /* 0x04 */ u8 pad4[0x74 - 0x4];
    /* 0x74 */ AnimTextureEntry *textures; // JFG: flags: the low byte is the header texture's index
} AnimModelInstance;

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
    /* 0x28 */ u8 pad28[0x39 - 0x28];
    /* 0x39 */ u8 unk39;
    /* 0x3A */ s8 modelIndex;
    /* 0x3B */ u8 pad3B;
    /* 0x3C */ s32 unk3C;
    /* 0x40 */ AnimObjectHeader *header;
    /* 0x44 */ u8 pad44[4];
    /* 0x48 */ s16 behaviorId;
    /* 0x4A */ u8 pad4A[2];
    /* 0x4C */ AnimModel *model;
    /* 0x50 */ AnimObjectSub50 *unk50;
    /* 0x54 */ u8 pad54[0x5C - 0x54];
    /* 0x5C */ s32 unk5C;
    /* 0x60 */ u8 pad60[0x68 - 0x60];
    /* 0x68 */ AnimObjectSub68 *unk68;
    /* 0x6C */ AnimModelInstance **models;
    /* 0x70 */ u8 pad70[0x74 - 0x70];
    /* 0x74 */ void **unk74;
    /* 0x78 */ void **unk78;
    /* 0x7C */ u8 pad7C[0x84 - 0x7C];
    /* 0x84 */ u32 unk84;
    /* 0x88 */ void *audioPoint;
    /* 0x8C */ s32 unk8C;
    /* 0x90 */ u8 pad90[0x9F - 0x90];
    /* 0x9F */ u8 unk9F;
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
    /* 0x28 */ u8 pitchBase;
    /* 0x29 */ u8 pitchPerSpeed;
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

/* The length of a sequence command (0x800761C8). */
s8 command_length(u16 *arg0) {
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
            len = command_length(cmd);
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
s32 char_id_of_player(s32 player);
s32 game_flag_get(s32 flag);
s16 pending_level_character(void);
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
                c = char_id_of_player(0);
                if (game_flag_get(0x5F)) {
                    c |= 4;
                }
                path->unk2 = D_800A7570[c & 7];
            }
            if (path->unk2 == 0x106 || path->unk2 == 0x8A || path->unk2 == 0x10A) {
                type = level_header()->unk6C;
                if (type == 1 || type == 2) {
                    path->unk2 = D_800A7580[pending_level_character() & 3];
                } else {
                    path->unk2 = D_800A7588[pending_level_character() & 3];
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
                    obj->modelIndex = pending_level_character() & 3;
                    if (obj->modelIndex == 3) {
                        obj->modelIndex = D_800A4FC0 & 3;
                    }
                    if (obj->modelIndex >= obj->header->numVariants) {
                        obj->modelIndex = 0;
                    }
                }
                if (path->obj->unk5C != 0) {
                    path->obj->model->flags &= ~1;
                }
            }
        }
        path->pitchBase = 100;
        path->pitchPerSpeed = 0;
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

/* Starts path index and shows its object (0x800767EC). */
void anim_path_enable_run(u8 index) {
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

/* Stops path index and hides its object (0x8007688C). */
void anim_path_disable_run(u8 index) {
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
    /* 0x01 */ u8 pad1;
    /* 0x02 */ s16 unk2; // JFG: ticks (func_80077ABC)
    /* 0x04 */ s32 unk4;
    /* 0x08 */ s32 unk8; // JFG: unk4's step per tick (func_80077ABC)
    /* 0x0C */ s32 unkC;
    /* 0x10 */ s32 unk10; // JFG: unkC's step per tick (func_80077ABC)
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
struct ALSoundState; // audio.h's SoundHandle
extern struct ALSoundState *D_801045C8[16]; // the sequences' sound handles
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

void sound_stop(struct ALSoundState *soundHandle);

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
void sequence_rewind(void);
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
                sequence_rewind();
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

/* Restarts every path and the sequence clock (0x800773EC). */
void sequence_rewind(void) {
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
   NON_MATCHING: ROM length; the command pointer and its second halfword held in locals (cmd, word: the ROM
   keeps both in registers, stores the pointer before the state, and reads the low byte from the register),
   which gives the ROM's command block except that every temporary from the (s8) conversion on is one register
   early (positional 17 -> 9 words, aligned 64 -> 56). Before that: 42 words (from 57: frames
   u32 (it holds a u16 or u16 * 6 / 10, so every value is the same), the clock advanced in place, empty ifs on
   rate, set there; cmd takes the frame word an unused local used to hold). The int-cast read of
   D_800A74EC in the clock compare (keeps its address in a register) took the aligned distance 89 -> 64.
   The ROM divides signed (`div`): frames * 6 is at most 65535 * 6, so the (s32) cast gives the same quotient
   and the ROM's instruction; declaring frames s32 instead re-colours the whole command block (64 -> 115). */
void func_80077614(s32 updateRate) {
    s32 i;
    AnimPath *path;
    s32 rate;
    u32 frames;
    u16 *cmd;
    u16 word;
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
        cmd = (u16 *) D_800A74CC;
        if (cmd != NULL && D_800A74D4 == 1) {
            word = cmd[1];
            if ((word >> 8) == 0x7B) {
                if (cmd[0] / 100.0f < (D_800A74D8 + updateRate) * frameTime) {
                    frames = cmd[0];
                    if (osTvType == 0) {
                        frames = cmd[0] >> 1;
                    } else {
                        frames = (s32) (frames * 6) / 10;
                    }
                    updateRate = frames - D_800A74D8;
                    D_800A74CC = (s32) (cmd + 2);
                    D_800A74D4 = (s8) word;
                    if (D_800A74D4 == 0) {
                        rate = updateRate;
                    }
                }
            }
        }
        if (updateRate > 0) {

            if (rate) {}
            for (i = 0; i < 4; i++) {
                D_801045B8[i] = NULL;
            }
            if (D_800A74EC != 0) {
                if (updateRate < *(s32 *) ((u32) &D_800A74EC)) {
                    D_800A74E4 += D_800A74E8 * updateRate;
                    D_800A74EC -= updateRate;
                } else {
                    D_800A74E4 += D_800A74E8 * D_800A74EC;
                    D_800A74EC = 0;
                }
            }
            D_800A74D8 += updateRate;
            D_800A74DC = D_800A74D8 * frameTime;
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
                        if (path->pitchBase != 100 || path->pitchPerSpeed != 0) {
                            speed = sqrtf(obj->vx * obj->vx + obj->vy * obj->vy + obj->vz * obj->vz);
                            audspat_point_set_pitch(obj->audioPoint, path->pitchBase + path->pitchPerSpeed * speed);
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

void sound_play(u16 soundID, struct ALSoundState **handlePtr);
void music_play(u8 seqID);
void music_volume_fade(f32 time, u8 volume);
void music_volume_fade_stop(void);
void music_volume_set(u8 volume);
void music_jingle_play_safe(u8 jingleID);
void music_jingle_volume_fade(f32 time, u8 volume);
void music_jingle_volume_fade_stop(void);
void music_jingle_volume_set(u8 volume);
s32 transition_begin(void *transition); // DKR: transition_begin (a FadeTransition)
void set_current_text(s32 textID);
void free_message_box(void);
void weather_set(s32 velX, s32 velY, s32 velZ, s32 intensity, s32 opacity, s32 time);
void slowly_change_fog(s32 fogIdx, s32 red, s32 green, s32 blue, s32 near, s32 far, f32 seconds);
void spawn_object_from_list(s16 index, s16 x, s16 y, s16 z, s32 arg4, s32 arg5);
void cam_shake_start(s32 index, f32 phase1, f32 phase2, f32 phase3, s32 arg4);
s32 obj_anim_set(void *obj, s32 anim, f32 frame);
void func_80047570(void);
void level_goto(s32 level, s32 arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5);
void func_80047608(s32 arg0, s32 level, s32 arg2);
void func_8005AAE8(s32 arg0);
u8 menu_mode_get(void);
void func_80047EFC(s32 flag, s32 set);
u8 *get_settings(void);
void func_800468A4(s32 arg0);
void input_player_disable(s32 player);
void input_player_enable(s32 player);

/* JFG: the level model's textures, as func_80077ABC reads them */
typedef struct AnimLevelModel {
    /* 0x00 */ AnimTextureEntry *textures;
    /* 0x04 */ u8 pad4[0x18 - 0x4];
    /* 0x18 */ s16 numTextures;
} AnimLevelModel;
AnimLevelModel *get_current_level_model(void);

/* JFG: a fade as transition_begin takes it (69050.c FadeTransition) */
typedef struct AnimFade {
    /* 0x00 */ u8 type;
    /* 0x01 */ u8 red;
    /* 0x02 */ u8 green;
    /* 0x03 */ u8 blue;
    /* 0x04 */ u16 duration;
    /* 0x06 */ u16 endTimer;
} AnimFade;

extern s16 D_801046FE;
extern s16 D_80104700;

#ifdef NON_MATCHING
/**
 * JFG: run the sequence's commands whose time has come (when a module function allows it). Each command is u16 words:
 * its time (hundredths of a second, against the sequence clock D_800A74DC), op << 8 | arg, then the op's parameters;
 * 0x7F00 ends the list. Ops 0x00-0x0A run, stop, restart, show, hide, lock and unlock paths and set their values;
 * 0x20-0x2B play and stop sounds and music; 0x40-0x4D start fades, text, weather, fog, objects, camera shakes,
 * level-model texture animations and triggers (D_80104608), and call module functions; 0x60-0x6F set a path object's
 * animation, model, flags, textures and module values; 0x74-0x7E reach the game's flags and levels, input, the clock
 * (0x7B: wait, then set the sequence state; 0x7E: rewind to the list's start).
 * NON_MATCHING: a first body written from the ROM, checked equal to it by simulation (the ROM's frame is 0x1D8).
 * NOT PORTABLE: D_800A74C8 and D_800A74CC hold the command pointers as s32.
 */
void func_80077ABC(void) {
    AnimPath **paths;
    u16 *cmd;
    u16 word;
    u16 ticks;
    f32 time;
    f32 now;
    f32 late;
    u8 index;
    AnimPath **slot;
    AnimPath *path;
    AnimObject *obj;
    struct ALSoundState **handle;
    u16 value;
    u16 value2;
    s32 n;
    f32 fa;
    f32 fb;
    AnimFade fade;
    AnimLevelModel *level;
    AnimTexture *tex;
    AnimTrigger *trigger;
    AnimObjectSub50 *sub;
    AnimObjectSub50 *sub2;
    AnimObjectSub68 *entry;
    AnimModelInstance *model;
    AnimTextureEntry *texEntry;
    s32 kind;
    s32 world;
    s32 level2;
    s32 character;
    u8 flags;

    paths = D_801045B0;
    if (((s32 (*)(void *)) func_80054530)(osRomBase) == 0) {
        return;
    }
    cmd = (u16 *) D_800A74CC;
    if (cmd == NULL) {
        return;
    }
    word = cmd[1];
    if (word != 0x7F00) {
        ticks = cmd[0];
        time = ticks / 100.0f;
        now = D_800A74DC;
        if (time < now && D_800A74D4 == 1) {
            do {
                late = now - time;
                switch (word >> 8) {
                    case 0x00: // run a path from where it is
                        index = word & 0xFF;
                        cmd += 2;
                        anim_path_enable_run(index);
                        func_80054530_f32(paths[index], late, 0, 0);
                        word = cmd[1];
                        break;
                    case 0x01:
                        index = word & 0xFF;
                        cmd += 2;
                        anim_path_disable_run(index);
                        word = cmd[1];
                        break;
                    case 0x02: // restart a path (and run it on if it was running)
                        index = word & 0xFF;
                        slot = &paths[index];
                        path = *slot;
                        cmd += 2;
                        if (path != NULL) {
                            if (path->flags & 1) {
                                anim_path_restart(index);
                                anim_path_enable_run(index);
                                func_80054530_f32(*slot, late, 0, 0);
                            } else {
                                anim_path_restart(index);
                            }
                        }
                        word = cmd[1];
                        break;
                    case 0x03:
                        path = paths[word & 0xFF];
                        cmd += 2;
                        if (path != NULL) {
                            path->flags |= 2;
                            if (path->obj != NULL && (path->flags & 5)) {
                                D_801045B8[0] = (CamPoint *) path->obj;
                            }
                        }
                        word = cmd[1];
                        break;
                    case 0x04:
                        path = paths[word & 0xFF];
                        cmd += 2;
                        if (path != NULL) {
                            path->flags &= ~2;
                        }
                        word = cmd[1];
                        break;
                    case 0x05:
                        index = word & 0xFF;
                        cmd += 2;
                        func_80076928(index);
                        word = cmd[1];
                        break;
                    case 0x06:
                        value = cmd[2];
                        path = paths[word & 0xFF];
                        cmd += 3;
                        if (path != NULL) {
                            path->unk15 = value >> 8;
                            path->unk1 = value & 0xFF;
                        }
                        word = cmd[1];
                        break;
                    case 0x07:
                        fa = cmd[2] * 0.001f;
                        path = paths[word & 0xFF];
                        cmd += 3;
                        if (path != NULL) {
                            path->unk10 = fa;
                        }
                        word = cmd[1];
                        break;
                    case 0x08:
                        fa = cmd[2];
                        fb = cmd[3];
                        path = paths[word & 0xFF];
                        cmd += 4;
                        if (path != NULL) {
                            fa *= 0.01f;
                            path->unk2A = (s32) (fb * 60.0f * 0.01f);
                            if (path->unk2A == 0) {
                                path->unk2C = fa;
                            } else {
                                path->unk30 = (fa - path->unk2C) / path->unk2A;
                            }
                        }
                        word = cmd[1];
                        break;
                    case 0x09:
                        index = word & 0xFF;
                        cmd += 2;
                        anim_path_lock(index);
                        word = cmd[1];
                        break;
                    case 0x0A:
                        index = word & 0xFF;
                        cmd += 2;
                        anim_path_unlock(index);
                        word = cmd[1];
                        break;
                    case 0x20:
                        value = cmd[2];
                        cmd += 3;
                        sound_play(value, NULL);
                        word = cmd[1];
                        break;
                    case 0x21:
                        cmd += 2;
                        music_play(word & 0xFF);
                        music_volume_fade_stop();
                        word = cmd[1];
                        break;
                    case 0x22:
                        fa = cmd[2];
                        cmd += 3;
                        music_volume_fade(fa / 100.0f - late, word & 0xFF);
                        word = cmd[1];
                        break;
                    case 0x23:
                        cmd += 2;
                        music_volume_set(word & 0xFF);
                        music_volume_fade_stop();
                        word = cmd[1];
                        break;
                    case 0x24:
                        cmd += 2;
                        music_jingle_play_safe(word & 0xFF);
                        music_jingle_volume_fade_stop();
                        word = cmd[1];
                        break;
                    case 0x25:
                        fa = cmd[2];
                        cmd += 3;
                        music_jingle_volume_fade(fa / 100.0f - late, word & 0xFF);
                        word = cmd[1];
                        break;
                    case 0x26:
                        cmd += 2;
                        music_jingle_volume_set(word & 0xFF);
                        music_jingle_volume_fade_stop();
                        word = cmd[1];
                        break;
                    case 0x27: // a sound in one of the sequence's sound slots
                        handle = &D_801045C8[word & 0xFF];
                        value = cmd[2];
                        cmd += 3;
                        if (*handle != NULL) {
                            sound_stop(*handle);
                            *handle = NULL;
                        }
                        sound_play(value, handle);
                        word = cmd[1];
                        break;
                    case 0x28:
                        handle = &D_801045C8[word & 0xFF];
                        cmd += 2;
                        if (*handle != NULL) {
                            sound_stop(*handle);
                            *handle = NULL;
                        }
                        word = cmd[1];
                        break;
                    case 0x29: // a sound at a path's object
                        path = paths[word & 0xFF];
                        value = cmd[2];
                        cmd += 3;
                        if (path != NULL) {
                            obj = path->obj;
                            if (obj != NULL) {
                                if (obj->audioPoint != NULL) {
                                    audspat_point_stop(obj->audioPoint);
                                    obj->audioPoint = NULL;
                                }
                                audspat_play_sound_at_position(value, obj->x, obj->y, obj->z, 1, &obj->audioPoint);
                            }
                        }
                        word = cmd[1];
                        break;
                    case 0x2A:
                        path = paths[word & 0xFF];
                        cmd += 2;
                        if (path != NULL) {
                            obj = path->obj;
                            if (obj != NULL) {
                                if (obj->audioPoint != NULL) {
                                    audspat_point_stop(obj->audioPoint);
                                    obj->audioPoint = NULL;
                                }
                            }
                        }
                        word = cmd[1];
                        break;
                    case 0x2B:
                        value = cmd[2];
                        path = paths[word & 0xFF];
                        cmd += 3;
                        if (path != NULL) {
                            path->pitchBase = value >> 8;
                            path->pitchPerSpeed = value & 0xFF;
                        }
                        word = cmd[1];
                        break;
                    case 0x40: // a screen fade (type 7: a module's)
                        value = cmd[2];
                        value2 = cmd[3];
                        fa = cmd[4];
                        fb = cmd[5];
                        cmd += 6;
                        if ((word & 0xF) == 7) {
                            func_80054530((value >> 8) & 0xFF, (value2 >> 8) & 0xFF, value2 & 0xFF,
                                          (value & 0xFF) ? 0 : 0xFF, (s32) (fa * 0.01f * 60.0f));
                        } else {
                            fade.type = word & 0xFF;
                            if (value & 0xFF) {
                                fade.type = (word & 0xFF) | 0x80;
                            }
                            fade.red = (value >> 8) & 0xFF;
                            fade.green = (value2 >> 8) & 0xFF;
                            fade.blue = value2 & 0xFF;
                            fade.duration = (s32) (fa / 100.0f * 60.0f);
                            if (0.0f < fb) {
                                fade.endTimer = (s32) (fb / 100.0f * 60.0f);
                            } else {
                                fade.endTimer = 0xFFFF;
                            }
                            transition_begin(&fade);
                        }
                        word = cmd[1];
                        break;
                    case 0x41:
                        cmd += 2;
                        set_current_text(word & 0xFF);
                        word = cmd[1];
                        break;
                    case 0x42:
                        cmd += 2;
                        free_message_box();
                        word = cmd[1];
                        break;
                    case 0x43:
                        fa = cmd[2];
                        cmd += 3;
                        weather_set(0, 0, 0, (s32) ((f32) (word & 0xFF) / 255.0f * 65535.0f), 0xFFFF,
                                    (s32) (fa * 0.01f * 60.0f));
                        word = cmd[1];
                        break;
                    case 0x44:
                        fa = cmd[2];
                        weather_set((s32) ((s16) cmd[3] / 256.0f), (s32) ((s16) cmd[4] / 256.0f),
                                    (s32) ((s16) cmd[5] / 256.0f), 1, 1, (s32) fa);
                        cmd += 6;
                        word = cmd[1];
                        break;
                    case 0x45:
                        fa = cmd[3];
                        value = cmd[2];
                        slowly_change_fog(0, word & 0xFF, (value >> 8) & 0xFF, value & 0xFF, cmd[4], cmd[5],
                                          fa / 100.0f - late);
                        cmd += 6;
                        word = cmd[1];
                        break;
                    case 0x46: // an object at a path's object
                        path = paths[word & 0xFF];
                        value = cmd[2];
                        cmd += 3;
                        if (path != NULL) {
                            obj = path->obj;
                            if (obj != NULL) {
                                spawn_object_from_list(value, (s32) obj->x, (s32) obj->y, (s32) obj->z, 100, 0);
                            }
                        }
                        word = cmd[1];
                        break;
                    case 0x47:
                        cam_shake_start(0, cmd[2] / 100.0f, cmd[3] / 100.0f, cmd[4] / 100.0f, word & 0xFF);
                        cmd += 5;
                        word = cmd[1];
                        break;
                    case 0x48: // a level-model texture's animation speed
                        value = cmd[2];
                        value2 = cmd[3];
                        cmd += 4;
                        level = get_current_level_model();
                        if (level != NULL && value < level->numTextures) {
                            tex = level->textures[value].texture;
                            if (tex != NULL) {
                                tex->unk14 = (value2 << 8) / 6000;
                            }
                        }
                        word = cmd[1];
                        break;
                    case 0x49:
                        fa = cmd[5];
                        trigger = &D_80104608[(word & 0xFF) & 7];
                        trigger->unk0 = cmd[2];
                        n = ((s16) cmd[3] * 0x10000) / 6000;
                        world = ((s16) cmd[4] * 0x10000) / 6000;
                        cmd += 6;
                        trigger->unk2 = (s32) (fa * (0.01f * 60.0f));
                        trigger->unk8 = (n - trigger->unk4) / trigger->unk2;
                        trigger->unk10 = (world - trigger->unkC) / trigger->unk2;
                        word = cmd[1];
                        break;
                    case 0x4A:
                        value = cmd[2];
                        value2 = cmd[3];
                        cmd += 4;
                        if ((value >> 8) != 0) {
                            func_80054530(word & 0xFF, value & 0xFF, value2 >> 8, value2 & 0xFF, value >> 8);
                        } else {
                            func_80054530(word & 0xFF);
                        }
                        word = cmd[1];
                        break;
                    case 0x4B:
                        fa = cmd[2];
                        cmd += 3;
                        D_801046FE = (s32) (fa * 60.0f * 0.01f);
                        D_801046FC = D_801046FE;
                        D_80104700 = D_80104704;
                        D_80104702 = word & 0xFF;
                        word = cmd[1];
                        break;
                    case 0x4C:
                        func_80054530(word & 0xFF, cmd[2] >> 8, cmd[2] & 0xFF, cmd[3] & 0xFF00, (cmd[3] & 0xFF) << 8,
                                      cmd[4] & 0xFF00, (cmd[4] & 0xFF) << 8, cmd[5], cmd[6]);
                        cmd += 7;
                        word = cmd[1];
                        break;
                    case 0x4D:
                        fa = cmd[3];
                        value = cmd[6];
                        func_80054530(cmd[2], (s32) fa, cmd[4], cmd[5], value >> 8, value & 0xFF, word & 0xFF);
                        cmd += 7;
                        word = cmd[1];
                        break;
                    case 0x60: // a path object's animation
                        value = cmd[2];
                        fa = cmd[3] / 16384.0f;
                        fb = cmd[4] / 16384.0f;
                        path = paths[word & 0xFF];
                        cmd += 5;
                        if (path != NULL) {
                            obj = path->obj;
                            if (obj != NULL) {
                                obj_anim_set(obj, (value >> 8) & 0xFF, fa);
                                path->unk14 = value & 0xFF;
                                path->unkC = fb;
                            }
                        }
                        word = cmd[1];
                        break;
                    case 0x61:
                        path = paths[word & 0xFF];
                        value = cmd[2];
                        cmd += 3;
                        if (path != NULL) {
                            obj = path->obj;
                            if (obj != NULL && value < obj->header->numVariants) {
                                obj->modelIndex = value;
                            }
                        }
                        word = cmd[1];
                        break;
                    case 0x62:
                        n = ((u32) cmd[2] << 16) | cmd[3];
                        path = paths[word & 0xFF];
                        cmd += 4;
                        if (path != NULL) {
                            obj = path->obj;
                            if (obj != NULL) {
                                obj->unk84 |= n;
                            }
                        }
                        word = cmd[1];
                        break;
                    case 0x63:
                        n = ((u32) cmd[2] << 16) | cmd[3];
                        path = paths[word & 0xFF];
                        cmd += 4;
                        if (path != NULL) {
                            obj = path->obj;
                            if (obj != NULL) {
                                obj->unk84 &= ~n;
                            }
                        }
                        word = cmd[1];
                        break;
                    case 0x64:
                        path = paths[word & 0xFF];
                        cmd += 2;
                        if (path != NULL) {
                            obj = path->obj;
                            if (obj != NULL) {
                                obj->flags &= ~0x400;
                            }
                        }
                        word = cmd[1];
                        break;
                    case 0x65:
                        path = paths[word & 0xFF];
                        cmd += 2;
                        if (path != NULL) {
                            obj = path->obj;
                            if (obj != NULL) {
                                obj->flags |= 0x400;
                            }
                        }
                        word = cmd[1];
                        break;
                    case 0x66:
                        path = paths[word & 0xFF];
                        value = cmd[2];
                        cmd += 3;
                        if (path != NULL) {
                            obj = path->obj;
                            if (obj != NULL) {
                                obj->unk8C = value;
                            }
                        } else {
                            D_801046F8 = value;
                        }
                        word = cmd[1];
                        break;
                    case 0x67:
                        path = paths[word & 0xFF];
                        value = cmd[2];
                        cmd += 3;
                        if (path != NULL) {
                            obj = path->obj;
                            if (obj != NULL) {
                                if (value != 0) {
                                    obj->flags |= 4;
                                } else {
                                    obj->flags &= 0xFFFB;
                                }
                            }
                        }
                        word = cmd[1];
                        break;
                    case 0x68:
                        fa = cmd[3];
                        value = cmd[2];
                        path = paths[word & 0xFF];
                        cmd += 4;
                        if (path != NULL) {
                            obj = path->obj;
                            if (obj != NULL) {
                                flags = path->unk25;
                                path->unk25 = value;
                                path->unk27 = 0;
                                path->unk24 = flags;
                                path->unk26 = (s32) (fa / 100.0f * 60.0f);
                                obj->unk39 = flags;
                            }
                        }
                        word = cmd[1];
                        break;
                    case 0x69:
                        fa = cmd[3];
                        value = cmd[2];
                        path = paths[word & 0xFF];
                        cmd += 4;
                        if (path != NULL) {
                            obj = path->obj;
                            if (obj != NULL && (obj->behaviorId == 0x5F || obj->behaviorId == 0x96)) {
                                entry = &obj->unk68[value >> 8];
                                if (entry->unk0 != 0) {
                                    entry->unk6 = value & 0xFF;
                                    entry->unk7 = 0;
                                    entry->unk5 = entry->unk4;
                                    entry->unk8 = (s32) (fa / 100.0f * 60.0f);
                                }
                            }
                        }
                        word = cmd[1];
                        break;
                    case 0x6A: // a module function on one of a path object's unk74 entries
                        fa = cmd[5];
                        value = cmd[2];
                        value2 = cmd[3];
                        kind = cmd[4];
                        path = paths[word & 0xFF];
                        cmd += 6;
                        if (path != NULL) {
                            obj = path->obj;
                            if (obj != NULL && obj->unk74 != NULL && (value >> 8) < obj->header->unk25) {
                                ((void (*)(void *, s32, s32, s32, s32, f32, s32)) func_80054530)(
                                    obj->unk74[value >> 8], value & 0xFF, value2 >> 8, value2 & 0xFF, kind, fa, 1);
                            }
                        }
                        word = cmd[1];
                        break;
                    case 0x6B: // the same on one of its unk78 entries
                        fa = cmd[5];
                        value = cmd[2];
                        value2 = cmd[3];
                        kind = cmd[4];
                        path = paths[word & 0xFF];
                        cmd += 6;
                        if (path != NULL) {
                            obj = path->obj;
                            if (obj != NULL && obj->unk78 != NULL && (value >> 8) < obj->header->unk26) {
                                ((void (*)(void *, s32, s32, s32, s32, f32, s32)) func_80054530)(
                                    obj->unk78[value >> 8], value & 0xFF, value2 >> 8, value2 & 0xFF, kind, fa, 0);
                            }
                        }
                        word = cmd[1];
                        break;
                    case 0x6C:
                        path = paths[word & 0xFF];
                        value = cmd[2] & 0xFF;
                        cmd += 3;
                        if (path != NULL) {
                            obj = path->obj;
                            if (obj != NULL) {
                                obj->unk9F = value;
                            }
                        }
                        word = cmd[1];
                        break;
                    case 0x6D:
                        fa = (s16) cmd[3] / 256.0f;
                        path = paths[word & 0xFF];
                        kind = cmd[2] & 0xFF;
                        value = cmd[5];
                        fb = (s16) cmd[4] / 256.0f;
                        cmd += 6;
                        if (path != NULL) {
                            obj = path->obj;
                            if (obj != NULL) {
                                sub = obj->unk50;
                                if (sub != NULL) {
                                    switch (kind) {
                                        case 0:
                                            sub->unkC = 0;
                                            break;
                                        case 1:
                                            sub->unkC = 5;
                                            obj->unk50->unkD = 1;
                                            break;
                                        case 2:
                                        case 3:
                                        case 4:
                                            sub->unkC = 1;
                                            obj->unk50->unkD = kind - 1;
                                            break;
                                        case 5:
                                        case 6:
                                        case 7:
                                        case 8:
                                            if (kind == 5) {
                                                sub->unkC = 0xD;
                                            } else {
                                                sub->unkC = 9;
                                            }
                                            sub2 = obj->unk50->unk1C;
                                            if (sub2 != NULL && (sub2->unkC & 8)) {
                                                sub2->unkBA = value;
                                                if (kind == 5) {
                                                    sub2->unkD = 1;
                                                } else {
                                                    sub2->unkD = kind - 5;
                                                }
                                                if (value == 0) {
                                                    sub2->unk0 = obj->unk50->unk0;
                                                    sub2->unkAC = obj->header->unk5C;
                                                } else if (value == 2) {
                                                    sub2->unk0 = fa;
                                                    sub2->unkAC = fb;
                                                }
                                            }
                                            break;
                                    }
                                }
                            }
                        }
                        word = cmd[1];
                        break;
                    case 0x6E:
                        n = cmd[4];
                        value = cmd[2];
                        value2 = cmd[3];
                        fa = cmd[6];
                        kind = cmd[5];
                        path = paths[word & 0xFF];
                        cmd += 7;
                        if (path != NULL) {
                            obj = path->obj;
                            if (obj != NULL) {
                                func_80054530(obj, value >> 8, value & 0xFF, value2, n, kind,
                                              (s32) (fa * 0.01f * 60.0f));
                            }
                        }
                        word = cmd[1];
                        break;
                    case 0x6F: // a path object's model texture: its animation speed and flags 0x40000/0x80000
                        value = cmd[2];
                        path = paths[word & 0xFF];
                        value2 = cmd[3];
                        n = (value & 0xFF00) >> 8;
                        cmd += 4;
                        kind = value & 0xFF;
                        if (path != NULL) {
                            obj = path->obj;
                            if (obj != NULL) {
                                model = obj->models[obj->modelIndex];
                                if (n < model->header->numTextures) {
                                    texEntry = &model->textures[n];
                                    tex = model->header->textures[texEntry->flags & 0xFF].texture;
                                    texEntry->flags &= 0xFFF1FFFF;
                                    tex->flags &= 0xFFF1FFFF;
                                    tex->unk14 = (value2 << 8) / 6000;
                                    switch (kind) {
                                        case 1:
                                            texEntry->flags |= 0x80000;
                                            tex->flags |= 0x80000;
                                            break;
                                        case 2:
                                            texEntry->flags |= 0x40000;
                                            tex->flags |= 0x40000;
                                            break;
                                    }
                                }
                            }
                        }
                        word = cmd[1];
                        break;
                    case 0x74:
                        cmd += 2;
                        func_80054530(1);
                        word = cmd[1];
                        break;
                    case 0x75:
                        cmd += 2;
                        func_80054530(0);
                        word = cmd[1];
                        break;
                    case 0x76:
                        cmd += 2;
                        func_80047570();
                        word = cmd[1];
                        break;
                    case 0x77:
                        cmd += 2;
                        if (game_flag_get(0x5A) && game_flag_get(0x5B) && game_flag_get(0x5C)) {
                            character = pending_level_character();
                            level_goto(0xD4, character, 0, menu_mode_get(), 1, 0);
                        } else {
                            func_80047608(5, 0x7F, 0);
                            func_8005AAE8(0);
                        }
                        word = cmd[1];
                        break;
                    case 0x78:
                        value = cmd[2];
                        cmd += 3;
                        func_80047EFC(value, word & 0xFF);
                        word = cmd[1];
                        break;
                    case 0x79: // ease D_800A74E4 to the arg over the given time
                        fb = word & 0xFF;
                        fa = cmd[2];
                        cmd += 3;
                        D_800A74EC = (s32) (fa * 60.0f * 0.01f);
                        if (D_800A74EC > 0) {
                            D_800A74E8 = (fb - D_800A74E4) / D_800A74EC;
                        } else {
                            D_800A74E4 = fb;
                        }
                        word = cmd[1];
                        break;
                    case 0x7A: // go to a level: (level & 0xFFF, 0xFFF for the settings' one; entry >> 12), arg: the
                               // character + 1 in bits 4-7 (0: the pending one), bits 0-3 for func_800468A4
                        value = cmd[2];
                        character = (word >> 4) & 0xF;
                        level2 = value & 0xFFF;
                        kind = word & 0xF;
                        world = value >> 12;
                        cmd += 3;
                        if (level2 == 0xFFF) {
                            level2 = get_settings()[1];
                        }
                        n = pending_level_character();
                        if (character != 0) {
                            character--;
                            if ((n & 3) == 3 && (character & 3) != 3) {
                                character = D_800A4FC0;
                            }
                        } else {
                            character = n;
                        }
                        level_goto(level2, character, world, (level2 == 0x168) ? 0x17 : menu_mode_get(), 1, 0);
                        func_800468A4(kind);
                        word = cmd[1];
                        break;
                    case 0x7B: // set the clock to this command's time and the sequence state to the arg
                        D_800A74D8 = (ticks * 60) / 100;
                        D_800A74DC = cmd[0] * 0.01f;
                        cmd += 2;
                        D_800A74D4 = (s8) (cmd[-1] & 0xFF);
                        word = cmd[1];
                        break;
                    case 0x7C:
                        cmd += 2;
                        input_player_disable(word & 3);
                        word = cmd[1];
                        break;
                    case 0x7D:
                        cmd += 2;
                        input_player_enable(word & 3);
                        word = cmd[1];
                        break;
                    case 0x7E: // back to the list's start, the clock set back by this command's time
                        D_800A74D8 -= (ticks * 60) / 100;
                        D_800A74DC = now - cmd[0] * 0.01f;
                        cmd = (u16 *) D_800A74C8;
                        word = cmd[1];
                        break;
                }
                if (word == 0x7F00) {
                    break;
                }
                ticks = cmd[0];
                time = ticks / 100.0f;
                now = D_800A74DC;
            } while (time < now && D_800A74D4 == 1);
        }
    }
    D_800A74CC = (s32) cmd;
}
#else
INCLUDE_ASM("asm/nonmatchings/76C20", func_80077ABC);
#endif

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

/* JFG: copy camera point index onto the camera (facing the other way) and set the FoV (0x80079890). */
CamPoint *cam_point_copy_to_camera(s32 index, CamTarget *cam) {
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

/* JFG: the path of an object (NULL if none) (0x80079AA4). */
AnimPath *anim_path_for_obj(AnimObject *obj) {
    s32 i;

    for (i = 0; i < 256; i++) {
        if (D_801045B0[i] != NULL && D_801045B0[i]->obj == obj) {
            return D_801045B0[i];
        }
    }
    return NULL;
}

/* A free path slot, or -1 (0x80079B6C). */
s32 anim_path_free_slot(void) {
    s32 i = 256;
    while (i--) {
        if (D_801045B0[i] == NULL) {
            return i;
        }
    }
    return -1;
}

extern s32 D_800A74D4;

/* Lets the sequence run (0x80079BA8). */
void sequence_resume(void) {
    D_800A74D4 = 0x1;
}

extern s32 D_800A74D4;

/* Holds the sequence (0x80079BB8). */
void sequence_hold(void) {
    D_800A74D4 = 0;
}
