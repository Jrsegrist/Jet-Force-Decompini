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

typedef struct SquadObject SquadObject;

typedef struct SquadState {
    /* 0x00 */ u8 pad0[0x58];
    /* 0x58 */ SquadObject *firstMember;
    /* 0x5C */ u8 pad5C[0x64 - 0x5C];
    /* 0x64 */ u8 unk64;
} SquadState;

/* The state of a squad member (in its leader's list) */
typedef struct MemberState {
    /* 0x00 */ u8 pad0[0x15];
    /* 0x15 */ u8 unk15;
    /* 0x16 */ u8 pad16[0x28 - 0x16];
    /* 0x28 */ SquadObject *next;
    /* 0x2C */ u8 pad2C[0x64 - 0x2C];
    /* 0x64 */ s32 unk64;
    /* 0x68 */ u8 pad68[0xA8 - 0x68];
    /* 0xA8 */ s32 unkA8;
} MemberState;

struct SquadObject {
    /* 0x00 */ u8 pad0[0x11];
    /* 0x11 */ u8 unk11;
    /* 0x12 */ u8 pad12[0x68 - 0x12];
    /* 0x68 */ SquadState *state;
    /* 0x6C */ u8 pad6C[0xA0 - 0x6C];
    /* 0xA0 */ u8 unkA0;
};

extern s32 D_800FE9F0;
extern SquadObject **D_800A38E4;

SquadObject **func_80050F44(s32 *count) {
    *count = D_800FE9F0;
    return D_800A38E4;
}

INCLUDE_ASM("asm/nonmatchings/51AD0", func_80050F60);

INCLUDE_ASM("asm/nonmatchings/51AD0", func_8005122C);

extern s32 D_800FE9A0;
extern s32 *D_800FE9AC;

void func_8005131C(s32 arg0) {
    D_800FE9AC[D_800FE9A0] = arg0;
    D_800FE9A0++;
}

extern SquadObject **D_800FE9A4;
extern s32 D_800FE9A8;

void func_80051350(SquadObject *obj) {
    D_800FE9A4[D_800FE9A8] = obj;
    if (++D_800FE9A8 >= D_800FE9A0) {
        D_800FE9A8 = 0;
    }
}

/* JFG: take the object with byte 0x11 == id out of the ring list. */
SquadObject *func_800513A0(s32 id) {
    s32 count = D_800FE9A0;
    SquadObject **list = D_800FE9A4;
    SquadObject *obj;

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
extern s32 D_800FE9B0; // JFG: squad module flags
extern s32 D_800FE9F4; // JFG: a squad timer (240 to 360 frames)

/* JFG: set up the squad lists before the objects are loaded. */
void func_80051420(void) {
    D_800FE9AC = (s32 *) mempool_alloc_safe(0x400, 0xFFFF00FF);
    D_800FE9B0 = 1;
    D_800FE9A4 = NULL;
    D_800FE9A0 = 0;
    D_800FE9F0 = 0;
    D_800FE9F4 = rand_range(0xF0, 0x168);
}

/* JFG: the last active object whose state byte 0x64 is id. */
SquadObject *func_80051484(s16 id) {
    s32 i = D_800FE9F0;
    SquadObject *obj;

    while (i--) {
        obj = D_800A38E4[i];
        if (obj->unkA0 == 0 && id == obj->state->unk64) {
            return obj;
        }
    }
    return NULL;
}


/* JFG: find squad squadId's active member memberId. */
SquadObject *func_80051508(s16 squadId, s16 memberId) {
    s32 i = D_800FE9F0;
    SquadObject *obj;
    SquadState *squad;
    SquadObject *member;
    MemberState *state;

    while (i--) {
        obj = D_800A38E4[i];
        squad = obj->state;
        if (squadId == squad->unk64) {
            for (member = squad->firstMember; member != NULL; member = state->next) {
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

/* JFG: clear word 0x64 == id in every squad member's state. */
void func_800515D0(s32 id) {
    s32 i = D_800FE9F0;
    SquadObject *obj;
    SquadState *squad;
    SquadObject *member;
    MemberState *state;

    while (i--) {
        obj = D_800A38E4[i];
        squad = obj->state;
        for (member = squad->firstMember; member != NULL; member = state->next) {
            state = (MemberState *) member->state;
            if (id == state->unk64) {
                state->unk64 = 0;
            }
        }
    }
}

/* JFG: clear the first squad member's word 0xA8 that is id. */
void func_80051660(s32 id) {
    s32 i = D_800FE9F0;
    SquadObject *obj;
    SquadState *squad;
    SquadObject *member;
    MemberState *state;

    while (i--) {
        obj = D_800A38E4[i];
        squad = obj->state;
        for (member = squad->firstMember; member != NULL; member = state->next) {
            state = (MemberState *) member->state;
            if (id == state->unkA8) {
                state->unkA8 = 0;
                return;
            }
        }
    }
}

INCLUDE_ASM("asm/nonmatchings/51AD0", func_800516EC);

INCLUDE_ASM("asm/nonmatchings/51AD0", func_80051880);

INCLUDE_ASM("asm/nonmatchings/51AD0", func_80051CCC);

INCLUDE_ASM("asm/nonmatchings/51AD0", func_80051EA4);

extern u32 *D_800A38E0;
u32 *asset_table_load(u32 assetIndex);

void func_80052678(void) {
    D_800A38E0 = asset_table_load(0x3E);
}

extern s32 D_800FE9FC;
extern u8 *D_800FEA00;
extern s32 D_800FEA04;
s32 func_80052CFC(s32 module); // JFG: load a runlink module

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

s32 func_80053370(s32 module); // JFG: whether a runlink module is loaded
void func_80054530();          // a module function (patched in by the runtime linker)

/* JFG: pass an event to the squad module (3) when it is loaded. */
void func_80052734(s32 a0, s16 a1, s16 a2, s16 a3, u8 a4, u8 a5, u8 a6) {
    if (func_80053370(3)) {
        func_80054530(a0, a1, a2, a3, a4, a5, a6);
    }
}

extern void *D_800FEA08;
extern s32 D_800FEA0C;
extern void *D_800FEA10;
extern s32 D_800FEA14;

/* JFG: keep two objects and their word at 0x68 (zero for none). */
void func_80052798(u8 *a, u8 *b) {
    if ((D_800FEA08 = a) != NULL) {
        D_800FEA0C = *(s32 *) (a + 0x68);
    } else {
        D_800FEA0C = 0;
    }
    if ((D_800FEA10 = b) != NULL) {
        D_800FEA14 = *(s32 *) (b + 0x68);
    } else {
        D_800FEA14 = 0;
    }
}

INCLUDE_ASM("asm/nonmatchings/51AD0", func_800527F0);

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

void func_80052A00(PatchWord *insn, u32 value, u8 type) {
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

INCLUDE_ASM("asm/nonmatchings/51AD0", func_800531C8);

/* Size: 0x20 bytes */
typedef struct ModuleEntry {
    /* 0x00 */ s32 unk0; // the module's code address (0 when not loaded)
    /* 0x04 */ s32 unk4;
    /* 0x08 */ s32 unk8;
    /* 0x0C */ u8 padC[0x1C - 0xC];
    /* 0x1C */ s32 unk1C; // an entry point offset (-1 for none)
} ModuleEntry;
extern ModuleEntry *D_800FEAA0;

typedef struct ModuleSlot {
    /* 0x00 */ s32 unk0;
    /* 0x04 */ s32 module;
} ModuleSlot;
extern ModuleSlot D_800FEAD8[16];
extern s32 D_800FEAAC; // JFG: the number of modules
void func_80053D2C(s32 module);
void func_8005372C(s32 module);
void mempool_free(void *data);

s32 func_80053370(s32 index) {
    return D_800FEAA0[index].unk0;
}

/* JFG: run a module's entry point, first loading it if it is in a slot. */
void func_8005338C(s32 module) {
    ModuleEntry *entry = &D_800FEAA0[module];
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

INCLUDE_ASM("asm/nonmatchings/51AD0", func_8005372C);

#ifdef NON_MATCHING
/* NON_MATCHING: 2 words: the target sets up &D_800FEAA0 (s3) before i = 15 */
/* JFG: unload every module (but 0), then free every used slot's buffer. */
void func_80053934(void) {
    s32 i;
    ModuleSlot *slot = D_800FEAD8;

    for (i = D_800FEAAC - 1; i > 0; i--) {
        func_8005372C(i);
    }
    i = 15;
    do {
        if (slot->module != 0xFFB) {
            mempool_free((void *) (D_800FEAA0[slot->module].unk8 + slot->unk0));
            slot->module = 0xFFB;
        }
        slot++;
    } while (i--);
}
#else
INCLUDE_ASM("asm/nonmatchings/51AD0", func_80053934);
#endif

INCLUDE_ASM("asm/nonmatchings/51AD0", func_800539E0);

INCLUDE_ASM("asm/nonmatchings/51AD0", func_80053C44);

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

typedef struct DestructTimer {
    u16 timer : 10;
    u16 unk : 6;
} DestructTimer;
extern DestructTimer *D_800FEB58;

/* JFG: set a module's destruct timer (10 bits) and its other 6 bits. */
void func_800541B0(s32 index, u16 timer, u8 value) {
    D_800FEB58[index].timer = timer;
    D_800FEB58[index].unk = value;
}

extern s32 D_800A38FC; // JFG: module destruct timers enabled
void func_8005342C(s32 module);

/* JFG: tick the modules' destruct timers; unload a module when its timer runs out. */
void func_80054204(void) {
    s32 i = D_800FEAAC;
    DestructTimer *t;

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
    s32 i = D_800FEAAC;
    DestructTimer *t;

    while (i--) {
        t = &D_800FEB58[i];
        if (t->timer != 0 && t->unk == 0) {
            func_8005342C(i);
            t->timer = 0;
            t->unk = 0;
        }
    }
}

INCLUDE_ASM("asm/nonmatchings/51AD0", func_80054368);
