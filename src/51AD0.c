#include "common.h"

#ifdef NON_MATCHING
/* NON_MATCHING: same logic, but the target computes every compare straight into $v0 (mine uses $at/$v1 + move) */
/* JFG: whether an object id is one of a fixed set (0x11C..0x120 and six others). */
s32 func_80050ED0(s32 id) {
    return (id >= 0x11C && id < 0x121) || id == 0x66 || id == 0x70 || id == 0x90 || id == 0x97 || id == 0x157 ||
           id == 0xA5;
}
#else
INCLUDE_ASM("asm/nonmatchings/51AD0", func_80050ED0);
#endif

typedef struct TroopObject TroopObject;

/* The state of an object that heads a linked list of followers */
typedef struct LeaderState {
    /* 0x00 */ u8 pad0[0x58];
    /* 0x58 */ TroopObject *firstMember;
    /* 0x5C */ u8 pad5C[0x64 - 0x5C];
    /* 0x64 */ u8 unk64;
} LeaderState;

/* The state of a follower, linked into its leader's list */
typedef struct MemberState {
    /* 0x00 */ u8 pad0[0x15];
    /* 0x15 */ u8 unk15;
    /* 0x16 */ u8 pad16[0x28 - 0x16];
    /* 0x28 */ TroopObject *next;
    /* 0x2C */ u8 pad2C[0x64 - 0x2C];
    /* 0x64 */ s32 unk64;
    /* 0x68 */ u8 pad68[0xA8 - 0x68];
    /* 0xA8 */ s32 unkA8;
} MemberState;

struct TroopObject {
    /* 0x00 */ u8 pad0[0x11];
    /* 0x11 */ u8 unk11;
    /* 0x12 */ u8 pad12[0x68 - 0x12];
    /* 0x68 */ LeaderState *state;
    /* 0x6C */ u8 pad6C[0xA0 - 0x6C];
    /* 0xA0 */ u8 unkA0;
};

extern s32 D_800FE9F0;
extern TroopObject **D_800A38E4;

TroopObject **func_80050F44(s32 *count) {
    *count = D_800FE9F0;
    return D_800A38E4;
}

INCLUDE_ASM("asm/nonmatchings/51AD0", func_80050F60);

/* JFG: the spawn entry of a troop leader (0x12 bytes) */
typedef struct LeaderSpawn {
    /* 0x00 */ s16 objectID;
    /* 0x02 */ u8 size;
    /* 0x04 */ s16 x, y, z;
    /* 0x0A */ s16 unkA;
    /* 0x0C */ s16 unkC;
    /* 0x0E */ s16 unkE;
    /* 0x10 */ u8 unk10;
} LeaderSpawn;

typedef struct SpawnedState {
    /* 0x00 */ union {
        s32 word;
        struct {
            u32 unk0 : 4;
            u32 unk4 : 1;
            u32 unk5 : 4;
            u32 unk9 : 1;
        } bits;
    } unk0;
    /* 0x04 */ u8 pad4[0x24 - 0x4];
    /* 0x24 */ s32 unk24;
    /* 0x28 */ u8 pad28[0xA6 - 0x28];
    /* 0xA6 */ s16 unkA6;
} SpawnedState;

typedef struct SpawnedObject {
    /* 0x00 */ u8 pad0[0x6];
    /* 0x06 */ s16 flags;
    /* 0x08 */ u8 pad8[0x3C - 0x8];
    /* 0x3C */ void *levelEntry;
    /* 0x40 */ u8 pad40[0x68 - 0x40];
    /* 0x68 */ SpawnedState *state;
} SpawnedObject;

s32 func_80050F60(void);
void *spawn_object(void *entry, s32 arg1);
extern u8 D_800FE9B8[];
extern SpawnedObject *D_800FEA08;
extern SpawnedState *D_800FEA0C;
extern s16 *D_800FEA10;
extern s32 D_800FEA14;

/* JFG: spawn object id once per slot (func_80050F60) and keep it and its state in D_800FEA08/D_800FEA0C. */
void troop_leader_spawn(s32 id) {
    u8 *done = &D_800FE9B8[func_80050F60()];
    LeaderSpawn spawn;

    if (*done == 0) {
        *done = 1;
        D_800FEA14 = 0;
        spawn.objectID = id;
        spawn.size = 0x12;
        spawn.x = 0;
        spawn.y = 0;
        spawn.z = 0;
        spawn.unkA = 0;
        spawn.unkC = 0;
        spawn.unkE = *D_800FEA10;
        spawn.unk10 = 0;
        if ((D_800FEA08 = spawn_object(&spawn, 4)) != NULL) {
            D_800FEA08->levelEntry = NULL;
            D_800FEA0C = D_800FEA08->state;
            D_800FEA08->flags |= 0x400;
            D_800FEA0C->unk0.word = 0;
            D_800FEA0C->unk0.bits.unk4 = 1;
            D_800FEA0C->unk24 = 0;
        }
    }
}

extern s32 D_800FE9A0;
extern s32 *D_800FE9AC;

void func_8005131C(s32 arg0) {
    D_800FE9AC[D_800FE9A0] = arg0;
    D_800FE9A0++;
}

extern TroopObject **D_800FE9A4;
extern s32 D_800FE9A8;

void func_80051350(TroopObject *obj) {
    D_800FE9A4[D_800FE9A8] = obj;
    if (++D_800FE9A8 >= D_800FE9A0) {
        D_800FE9A8 = 0;
    }
}

/* JFG: take the object with byte 0x11 == id out of the ring list. */
TroopObject *func_800513A0(s32 id) {
    s32 count = D_800FE9A0;
    TroopObject **list = D_800FE9A4;
    TroopObject *obj;

    if (count > 0x100) {
        return NULL;
    }
    if (D_800FE9A4 != NULL) {
        while (count--) {
            obj = *list;
            if (obj != NULL && id == obj->unk11) {
                *list = NULL;
                return obj;
            }
            list++;
        }
    }
    return NULL;
}

void *mempool_alloc_safe(s32 size, u32 colourTag);
s32 rand_range(s32 min, s32 max);
extern s32 D_800FE9B0; // JFG: set to 1 by func_80051420
extern s32 D_800FE9F4; // JFG: a random frame count, 240 to 360

/* JFG: allocate the 0x400-byte buffer func_8005131C fills (D_800FE9AC) and empty the leader and ring lists. */
void func_80051420(void) {
    D_800FE9AC = (s32 *) mempool_alloc_safe(0x400, 0xFFFF00FF);
    D_800FE9B0 = 1;
    D_800FE9A4 = NULL;
    D_800FE9A0 = 0;
    D_800FE9F0 = 0;
    D_800FE9F4 = rand_range(0xF0, 0x168);
}

/* JFG: the last active object whose state byte 0x64 is id. */
TroopObject *func_80051484(s16 id) {
    s32 i = D_800FE9F0;
    TroopObject *obj;

    while (i--) {
        obj = D_800A38E4[i];
        if (obj->unkA0 == 0 && id == obj->state->unk64) {
            return obj;
        }
    }
    return NULL;
}


/* JFG: the leader with id troopId, then the active follower on its list whose byte 0x15 is memberId. */
TroopObject *func_80051508(s16 troopId, s16 memberId) {
    s32 i = D_800FE9F0;
    TroopObject *obj;
    LeaderState *leader;
    TroopObject *member;
    MemberState *state;

    while (i--) {
        obj = D_800A38E4[i];
        leader = obj->state;
        if (troopId == leader->unk64) {
            for (member = leader->firstMember; member != NULL; member = state->next) {
                state = (MemberState *) member->state;
                if (member->unkA0 == 0 && memberId == state->unk15) {
                    return member;
                }
            }
            return NULL;
        }
    }
    return NULL;
}

/* JFG: in every leader's followers, zero state word 0x64 where it equals id. */
void func_800515D0(s32 id) {
    s32 i = D_800FE9F0;
    TroopObject *obj;
    LeaderState *leader;
    TroopObject *member;
    MemberState *state;

    while (i--) {
        obj = D_800A38E4[i];
        leader = obj->state;
        for (member = leader->firstMember; member != NULL; member = state->next) {
            state = (MemberState *) member->state;
            if (id == state->unk64) {
                state->unk64 = 0;
            }
        }
    }
}

/* JFG: zero state word 0xA8 of the first follower (of any leader) where it equals id. */
void func_80051660(s32 id) {
    s32 i = D_800FE9F0;
    TroopObject *obj;
    LeaderState *leader;
    TroopObject *member;
    MemberState *state;

    while (i--) {
        obj = D_800A38E4[i];
        leader = obj->state;
        for (member = leader->firstMember; member != NULL; member = state->next) {
            state = (MemberState *) member->state;
            if (id == state->unkA8) {
                state->unkA8 = 0;
                return;
            }
        }
    }
}

/* JFG: a follower's colour choices */
typedef struct ColourState {
    /* 0x00 */ u8 pad0[3];
    /* 0x03 */ u8 unk3; // JFG: the normal values (r = 3, b = 4, g = 5)
    /* 0x04 */ u8 unk4;
    /* 0x05 */ u8 unk5;
    /* 0x06 */ u8 unk6; // JFG: the mode 3 colours
    /* 0x07 */ u8 unk7;
    /* 0x08 */ u8 unk8;
    /* 0x09 */ u8 unk9; // JFG: the alternative colours
    /* 0x0A */ u8 unkA;
    /* 0x0B */ u8 unkB;
    /* 0x0C */ u8 padC;
    /* 0x0D */ u8 unkD; // JFG: a threshold in percent (from 100)
    /* 0x0E */ u8 padE[0x2A - 0xE];
    /* 0x2A */ s16 unk2A;
    /* 0x2C */ u8 pad2C[2];
    /* 0x2E */ s16 unk2E;
    /* 0x30 */ u8 unk30;
    /* 0x31 */ u8 unk31;
    /* 0x32 */ u8 unk32;
    /* 0x33 */ u8 unk33;
} ColourState;

typedef struct ColourObject {
    /* 0x00 */ u8 pad0[0x68];
    /* 0x68 */ ColourState *state;
} ColourObject;

/* JFG: pick a follower's three colour values by its mode and the leader's ratio (D_800FEA0C's 0xA6 / its 0x2A). */
void troop_member_colours_get(ColourObject *obj, u8 *r, u8 *g, u8 *b) {
    ColourState *s = obj->state;
    f32 ratio;

    if (D_800FEA0C != NULL && s != NULL && s->unk2A != 0) {
        ratio = (f32) D_800FEA0C->unkA6 / (f32) s->unk2A;
    } else {
        ratio = 1.0f;
    }
    if (s->unk30 == 3) {
        *r = s->unk6;
        *g = s->unk8;
        *b = s->unk7;
    } else if (s->unk30 == 4 || ratio * 100.0f < (f32) (100 - s->unkD)) {
        *r = s->unk9;
        *g = s->unkB;
        *b = s->unkA;
    } else {
        *r = s->unk3;
        *g = s->unk5;
        *b = s->unk4;
    }
    s->unk32 = *b;
    s->unk31 = *r;
    if (*b == 8 && D_800FEA0C != NULL && D_800FEA0C->unk0.bits.unk9) {
        *b = 9;
        s->unk2E = 0;
    }
    switch (s->unk33) {
        case 0:
            break;
        case 0x1A:
            *r = 10;
            *b = s->unk33;
            break;
        default:
            *b = s->unk33;
            break;
    }
}

INCLUDE_ASM("asm/nonmatchings/51AD0", func_80051880);

/* JFG: a troop leader's state (behaviour 0x17) */
typedef struct LeaderState2 {
    /* 0x00 */ u8 pad0;
    /* 0x01 */ u8 unk1; // JFG: the troop size
    /* 0x02 */ u8 pad2[0x2C - 0x2];
    /* 0x2C */ u16 unk2C;
    /* 0x2E */ u8 pad2E[0x30 - 0x2E];
    /* 0x30 */ u8 unk30;
    /* 0x31 */ u8 pad31[0x63 - 0x31];
    /* 0x63 */ u8 unk63;
    /* 0x64 */ u8 unk64; // JFG: the troop id
    /* 0x65 */ u8 pad65[0x75 - 0x65];
    /* 0x75 */ u8 unk75;
    /* 0x76 */ u8 pad76[0x88 - 0x76];
    /* 0x88 */ s32 unk88;
    /* 0x8C */ s32 unk8C;
} LeaderState2;

typedef struct LeaderObject {
    /* 0x00 */ u8 pad0[0x48];
    /* 0x48 */ s16 behaviorId;
    /* 0x4A */ u8 pad4A[0x68 - 0x4A];
    /* 0x68 */ LeaderState2 *state;
    /* 0x6C */ u8 pad6C[0xA0 - 0x6C];
    /* 0xA0 */ u8 freed;
} LeaderObject;

/* JFG: a level object entry as func_80051CCC sees it */
typedef struct TroopEntry {
    /* 0x00 */ s16 objectID;
    /* 0x02 */ u8 size;
    /* 0x03 */ u8 pad3[0x11 - 0x3];
    /* 0x11 */ u8 troopId;
    /* 0x12 */ u8 pad12[0x18 - 0x12];
    /* 0x18 */ s8 memberIndex;
} TroopEntry;

void func_800118F4(s32 *a, s32 *b, s32 index);
LeaderObject **objGetObjList(s32 *arg0, s32 *cnt);
s32 objflags_reserve_ids(u8 arg0);
void func_80026824(u8 arg0, s32 arg1);
s16 func_80005EC0(s32 type);

#ifdef NON_MATCHING
/* NON_MATCHING: the target strength-reduces the object loop to i * 4 (compared with last * 4 each time) and its frame is 8 bytes bigger */
/* JFG: for every live troop leader, number the members in both object entry lists (by troop id), and start it. */
void func_80051CCC(void) {
    s32 first;
    s32 last;
    LeaderObject **objs;
    LeaderObject *obj;
    LeaderState2 *state;
    s32 i;
    TroopEntry *lists[2];
    s32 j;
    s32 offset;
    s32 sizes[2];
    TroopEntry *entry;

    func_800118F4((s32 *) &lists[1], &sizes[1], 1);
    func_800118F4((s32 *) &lists[0], &sizes[0], 0);
    objs = objGetObjList(&first, &last);
    for (i = first; i < last; i++) {
        obj = objs[i];
        if (obj->freed == 0 && obj->behaviorId == 0x17) {
            state = obj->state;
            if (state->unk2C != 0x2CE) {
                state->unk8C = state->unk88 = objflags_reserve_ids(state->unk1);
                if (state->unk75) {
                    func_80026824(state->unk75, 0x20);
                }
                for (j = 0; j < 2; j++) {
                    entry = lists[j];
                    for (offset = 0; offset < sizes[j]; offset += entry->size, entry = (TroopEntry *) ((u8 *) entry + entry->size)) {
                        if (func_80005EC0(entry->objectID) == 0x18 && state->unk64 == entry->troopId) {
                            entry->memberIndex = state->unk8C;
                            state->unk8C++;
                        }
                    }
                }
                if (state->unk63 >= state->unk1) {
                    state->unk30 = 7;
                }
            }
        }
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/51AD0", func_80051CCC);
#endif

INCLUDE_ASM("asm/nonmatchings/51AD0", func_80051EA4);

extern u32 *D_800A38E0;
u32 *asset_table_load(u32 assetIndex);

void func_80052678(void) {
    D_800A38E0 = asset_table_load(0x3E);
}

extern s32 D_800FE9FC;
extern u8 *D_800FEA00;
extern s32 D_800FEA04;
s32 func_80052CFC(); // JFG: load a runlink module (also applies relocations: four arguments)

/* JFG: keep a list of records (s16 id, u8 length); start event 3 if one has id 0x61, 5 or 0x4F. */
void func_8005269C(u8 *data, s32 size) {
    s32 offset;
    u8 *rec;

    D_800FE9FC = 0;
    D_800FEA00 = data;
    D_800FEA04 = size;
    rec = data;
    for (offset = 0; offset < size; offset += rec[2], rec += rec[2]) {
        if (*(s16 *) rec == 0x61 || *(s16 *) rec == 5 || *(s16 *) rec == 0x4F) {
            func_80052CFC(3);
            return;
        }
    }
}

s32 runlink_module_loaded(s32 module); // JFG: whether a runlink module is loaded
void func_80054530();          // a module function (patched in by the runtime linker)

/* JFG: forward a position and three bytes to module 3 (the enemy code), if it is loaded. */
void func_80052734(s32 a0, s16 a1, s16 a2, s16 a3, u8 a4, u8 a5, u8 a6) {
    if (runlink_module_loaded(3)) {
        func_80054530(a0, a1, a2, a3, a4, a5, a6);
    }
}

/* JFG: keep two objects and their word at 0x68 (zero for none). */
void func_80052798(SpawnedObject *a, u8 *b) {
    if ((D_800FEA08 = a) != NULL) {
        D_800FEA0C = a->state;
    } else {
        D_800FEA0C = NULL;
    }
    if ((D_800FEA10 = (s16 *) b) != NULL) {
        D_800FEA14 = *(s32 *) (b + 0x68);
    } else {
        D_800FEA14 = 0;
    }
}

INCLUDE_ASM("asm/nonmatchings/51AD0", func_800527F0);

/* Size: 0x20 bytes */
typedef struct ModuleEntry {
    /* 0x00 */ s32 unk0; // the module's code address (0 when not loaded)
    /* 0x04 */ s32 unk4;
    /* 0x08 */ s32 unk8;
    /* 0x0C */ s32 unkC;
    /* 0x10 */ s32 unk10;
    /* 0x14 */ u16 unk14;
    /* 0x16 */ u8 pad16[0x1C - 0x16];
    /* 0x1C */ s32 unk1C; // an entry point offset (-1 for none)
} ModuleEntry;
extern ModuleEntry *gRunlinkModules;

INCLUDE_ASM("asm/nonmatchings/51AD0", func_80052874);

typedef union PatchWord {
    u32 word;
    struct {
        u32 op : 6;
        u32 target : 26;
    } j;
    struct {
        u16 hi;
        u16 lo;
    } half;
} PatchWord;

/* Applies one relocation to an instruction or word and flushes the caches (0x80052A00). */
void runlink_reloc_apply(PatchWord *insn, u32 value, u8 type) {
    u32 hi;

    switch (type) {
        case 2:
            insn->word = value;
            break;
        case 4:
            insn->j.target = value >> 2;
            break;
        case 5:
            hi = value >> 16;
            if (value & 0x8000) {
                hi++;
            }
            insn->half.lo = hi;
            break;
        case 6:
            insn->half.lo = value;
            break;
    }
    osWritebackDCache(insn, 4);
    osInvalICache(insn, 4);
}

INCLUDE_ASM("asm/nonmatchings/51AD0", func_80052AB8);

INCLUDE_ASM("asm/nonmatchings/51AD0", func_80052CFC);

/* JFG: a module relocation (8 bytes) */
typedef struct ModuleReloc {
    /* 0x00 */ s32 symbol;
    /* 0x04 */ u32 offset : 24;
    /* 0x07 */ u32 kind : 4;
    /* 0x07 */ u32 type : 4; // JFG: 3 = in the data section
} ModuleReloc;

extern s32 gNumRunlinkModules; // JFG: the number of modules
extern u32 *D_800FEAA8;        // JFG: the symbol table (module << 20 | offset)
extern ModuleReloc *D_800FEAA4; // JFG: the main segment's relocations
extern u32 D_800FEAB0;          // JFG: their number
extern u32 D_800FEAB8[5];       // JFG: the section addresses of the module being searched
extern u8 gMusicPlayer[];
extern u8 gMusicSequenceData[];
void sound_volume_change(s32 behaviour);
void func_80054530();

#ifdef NON_MATCHING
/* NON_MATCHING: module->unk0 in v1 (target v0), temps numbered one lower in the search loop, and 3 is reloaded into a3 for the call */
/* JFG: if a word still points at the module trap, find the relocation that wrote it and apply it again (TRUE if found). */
s32 func_800531C8(u32 *addr) {
    ModuleEntry *module;
    ModuleReloc *reloc;
    s32 i;
    u32 n;
    s32 section;
    s32 symModule;

    if ((u32) func_80054530 != *addr) {
        return FALSE;
    }
    module = gRunlinkModules;
    for (i = 0; i < gNumRunlinkModules; i++, module++) {
        if (module->unk0 != 0) {
            if (i == 0) {
                D_800FEAB8[1] = (u32) sound_volume_change;
                D_800FEAB8[2] = (u32) gMusicPlayer;
                D_800FEAB8[3] = (u32) gMusicSequenceData;
                reloc = D_800FEAA4;
                n = D_800FEAB0;
                D_800FEAB8[4] = (u32) reloc;
            } else {
                D_800FEAB8[1] = module->unk0;
                D_800FEAB8[2] = D_800FEAB8[1] + module->unk8;
                D_800FEAB8[3] = D_800FEAB8[2] + module->unkC;
                reloc = (ModuleReloc *) (D_800FEAB8[4] = D_800FEAB8[3] + module->unk10);
                n = (u32) module->unk14 >> 3;
            }
            while (n--) {
                section = reloc->type == 3 ? 2 : 1;
                if ((u32) addr == D_800FEAB8[section] + reloc->offset) {
                    symModule = D_800FEAA8[reloc->symbol] >> 20;
                    if (symModule >= 0xFFC) {
                        symModule = 0;
                    }
                    func_80052CFC(symModule, reloc, D_800FEAB8, 3);
                    return TRUE;
                }
                reloc++;
            }
        }
    }
    return FALSE;
}
#else
INCLUDE_ASM("asm/nonmatchings/51AD0", func_800531C8);
#endif

typedef struct ModuleSlot {
    /* 0x00 */ s32 unk0;
    /* 0x04 */ s32 module;
} ModuleSlot;
extern ModuleSlot D_800FEAD8[16];
extern s32 gNumRunlinkModules; // JFG: the number of modules
void func_80053D2C(s32 module);
void func_8005372C(s32 module);
void mempool_free(void *data);

/* Returns a module's load address (0 when not loaded) (0x80053370). */
s32 runlink_module_loaded(s32 index) {
    return gRunlinkModules[index].unk0;
}

/* JFG: run a module's entry point, first loading it if it is in a slot. */
void func_8005338C(s32 module) {
    ModuleEntry *entry = &gRunlinkModules[module];
    ModuleSlot *slot;
    s32 i;

    if (entry->unk1C != -1) {
        if (entry->unk0 == 0) {
            slot = D_800FEAD8;
            i = 15;
            do {
                if (module == slot->module) {
                    func_80053D2C(module);
                    break;
                }
                slot++;
            } while (i--);
        }
        if (entry->unk0 != 0) {
            ((void (*)(void))(entry->unk0 + entry->unk1C))();
        }
    }
}

INCLUDE_ASM("asm/nonmatchings/51AD0", func_8005342C);

typedef struct UnloadTimer {
    u16 timer : 10;
    u16 unk : 6;
} UnloadTimer;
extern UnloadTimer *D_800FEB58;

#ifdef NON_MATCHING
/* NON_MATCHING: register allocation: the code address is loaded into a0 (mine v0), the relocation word into a0 (mine v1), addr in a3 */
/* JFG: unload a module: free its code (or its slot's buffer) and point the main segment's references to it back at the trap. */
void func_8005372C(s32 module) {
    ModuleEntry *entry = &gRunlinkModules[module];
    ModuleSlot *slot;
    ModuleReloc *reloc;
    u32 i;
    s32 found;
    s32 symModule;
    u32 addr;
    s32 type;

    func_8005338C(module);
    found = FALSE;
    if (entry->unk0 == 0) {
        slot = D_800FEAD8;
        i = 15;
        do {
            if (module == slot->module) {
                found = TRUE;
                break;
            }
            slot++;
        } while (i--);
        if (found) {
            mempool_free((void *) (slot->unk0 + entry->unk8));
            slot->module = 0xFFB;
        }
    } else {
        mempool_free((void *) entry->unk0);
        entry->unk0 = 0;
        D_800FEB58[module].timer = 0;
        D_800FEB58[module].unk = 0;
        reloc = D_800FEAA4;
        i = D_800FEAB0;
        while (i--) {
            symModule = D_800FEAA8[reloc->symbol] >> 20;
            type = reloc->type;
            if (symModule >= 0xFFC) {
                symModule = 0;
            }
            if (symModule == module) {
                if (type == 3) {
                    addr = (u32) gMusicPlayer + reloc->offset;
                    reloc->type = 0;
                } else {
                    addr = reloc->offset + (u32) sound_volume_change;
                }
                runlink_reloc_apply((PatchWord *) addr, reloc->kind == 4 ? (u32) func_80054530 : 0, reloc->kind);
            }
            reloc->type = type;
            reloc++;
        }
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/51AD0", func_8005372C);
#endif

#ifdef NON_MATCHING
/* NON_MATCHING: 2 words: the target sets up &gRunlinkModules (s3) before i = 15 */
/* JFG: unload every module (but 0), then free every used slot's buffer. */
void func_80053934(void) {
    s32 i;
    ModuleSlot *slot = D_800FEAD8;

    for (i = gNumRunlinkModules - 1; i > 0; i--) {
        func_8005372C(i);
    }
    i = 15;
    do {
        if (slot->module != 0xFFB) {
            mempool_free((void *) (gRunlinkModules[slot->module].unk8 + slot->unk0));
            slot->module = 0xFFB;
        }
        slot++;
    } while (i--);
}
#else
INCLUDE_ASM("asm/nonmatchings/51AD0", func_80053934);
#endif

INCLUDE_ASM("asm/nonmatchings/51AD0", func_800539E0);

s32 mempool_free_timer_get();
void func_8005342C(s32 module);
void mempool_free_timer(s32 state);
void *mempool_alloc_fixed(s32 size, u8 *address, u32 colourTag);
extern s32 D_800A38F0; // JFG: set while a module is unloaded into a slot

/* JFG: move a loaded module into the first free slot: unload it there with the free timer off, then re-reserve its memory. */
void runlink_module_move_to_slot(s32 module) {
    ModuleEntry *entry = &gRunlinkModules[module];
    ModuleSlot *slot = D_800FEAD8;
    s32 i;
    s32 timer;

    if (entry->unk0 != 0) {
        i = 15;
        do {
            if (slot->module == 0xFFB) {
                timer = mempool_free_timer_get();
                slot->unk0 = entry->unk0;
                slot->module = module;
                mempool_free_timer(0);
                D_800A38F0 = 1;
                func_8005342C(module);
                D_800A38F0 = 0;
                mempool_free_timer(timer);
                mempool_alloc_fixed(entry->unkC + entry->unk10 + entry->unk14, (u8 *) (slot->unk0 + entry->unk8),
                                    0x7F7F7FFF);
                return;
            }
            slot++;
        } while (i--);
    }
}

INCLUDE_ASM("asm/nonmatchings/51AD0", func_80053D2C);

#ifdef NON_MATCHING
/* NON_MATCHING: 4 words: the target loads the module into $v0 (then move $a0 in the jal slot), mine into $a0 */
/* JFG: unload the module of every used slot (0xFFB = free). */
void func_8005414C(void) {
    ModuleSlot *slot = D_800FEAD8;
    s32 i = 15;

    do {
        if (slot->module != 0xFFB) {
            func_80053D2C(slot->module);
        }
        slot++;
    } while (i--);
}
#else
INCLUDE_ASM("asm/nonmatchings/51AD0", func_8005414C);
#endif

/* JFG: set the frames before a module is unloaded (10 bits) and its other 6-bit count (0x800541B0). */
void runlink_unload_timer_set(s32 index, u16 timer, u8 value) {
    D_800FEB58[index].timer = timer;
    D_800FEB58[index].unk = value;
}

extern s32 D_800A38FC; // JFG: nonzero while the unload timers run
void func_8005342C(s32 module);

/* JFG: decrement every module's unload timer and unload the module when it reaches zero (0x80054204). */
void runlink_unload_timers_update(void) {
    s32 i = gNumRunlinkModules;
    UnloadTimer *t;

    if (D_800A38FC != 0) {
        while (i--) {
            t = &D_800FEB58[i];
            if (t->unk != 0) {
                t->unk--;
            }
            if (t->timer != 0) {
                if (--t->timer == 0) {
                    func_8005342C(i);
                }
            }
        }
    }
}

/* JFG: unload the modules with a running timer and no other count right now. */
void func_800542C4(void) {
    s32 i = gNumRunlinkModules;
    UnloadTimer *t;

    while (i--) {
        t = &D_800FEB58[i];
        if (t->timer != 0 && t->unk == 0) {
            func_8005342C(i);
            t->timer = 0;
            t->unk = 0;
        }
    }
}

extern s32 D_800A38F4; // JFG: set when the addresses are not to be resolved
extern s32 D_800A38F8; // JFG: the number of symbols
extern char D_800AD140[];
char *func_800527F0(s32 index);

#ifdef NON_MATCHING
/* NON_MATCHING: same code shape; temps numbered one higher and the symbol word / module address in swapped registers */
/* JFG: the module and offset of an address (TRUE if it is in a loaded module), and the name of the nearest symbol below it. */
s32 func_80054368(u32 address, s32 *module, s32 *offset, char **name) {
    u32 *sym = D_800FEAA8;
    u32 best = 0;
    s32 n;
    ModuleEntry *m;
    u32 off;
    s32 bestIndex;

    *module = 0;
    *offset = 0;
    if (name != NULL) {
        *name = D_800AD140;
    }
    if (D_800A38F4) {
        *offset = address - 0x80000450;
        return TRUE;
    }
    if (name != NULL) {
        n = D_800A38F8;
        while (n--) {
            m = &gRunlinkModules[*sym >> 20];
            if (m->unk0 != 0) {
                off = *sym & 0xFFFFF;
                if ((u32) m->unk8 >= off && address >= m->unk0 + off && best < m->unk0 + off) {
                    best = m->unk0 + off;
                    bestIndex = sym - D_800FEAA8;
                }
            }
            sym++;
        }
        if (best != 0) {
            *name = func_800527F0(bestIndex);
        }
    }
    n = gNumRunlinkModules;
    m = gRunlinkModules;
    while (n--) {
        if (address >= (u32) m->unk0 && (u32) (m->unk0 + m->unk8) >= address) {
            *module = m - gRunlinkModules;
            *offset = address - m->unk0;
            return TRUE;
        }
        m++;
    }
    return FALSE;
}
#else
INCLUDE_ASM("asm/nonmatchings/51AD0", func_80054368);
#endif
