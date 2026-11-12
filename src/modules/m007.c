/*
 * Runlink module 7: the hint text shown on screen (normal hints and special
 * ones that interrupt them), and the long hints with their conditions and actions.
 */
#include "module.h"

/* the .rodata strings the data refers to (first in .rodata) */
INCLUDE_ASM("asm/nonmatchings/modules/m007/ro", rodata);

static void *func_m007_00578(s32 hint, s32 kind);
static s32 func_m007_00390(s32 condition);
static void func_m007_00DF0(void);
static void func_m007_0070C(void);
#ifdef NON_MATCHING
static s32 func_m007_007E4(s32 dir);
#else
s32 rl_m007_007E4(s32 dir);
#define func_m007_007E4 rl_m007_007E4
#endif
static void func_m007_016B4(void);
#ifdef NON_MATCHING
static void *func_m007_00988(s32 id, s32 kind);
static void *func_m007_00C90(s32 id, s32 asset);
#else
void *rl_m007_00988(s32 id, s32 kind);
#define func_m007_00988 rl_m007_00988
#endif
extern void *rl_m007_00B00(s32 id, s32 asset);
extern void *func_8004A57C(s32 size, s32 colour); /* DKR mempool_alloc_safe */
extern void func_8004D530(s32 asset, void *dest, s32 offset, s32 size); /* DKR load_asset_to_address */
extern u8 rl_m007_02A04[]; /* the texts of pages without one */
extern u8 rl_m007_02A14[];
extern s32 rl_m007_012E0(s32 condition);
extern void func_8004AA80(void *data); /* frees */
extern s32 func_80043510(s32 player); /* the buttons pressed */
extern s32 func_80043558(s32 player); /* the stick's x */
extern s32 func_800435C8(s32 player); /* the stick's y */
extern void *func_8004D37C(s32 asset);
extern s32 func_8005A7D0(void);

/* A hint's entry: shown when its condition holds */
typedef struct M007Choice {
    /* 0x00 */ u8 cond1; /* bit 7: negated */
    /* 0x01 */ u8 op;    /* 0: both, 1: either, else: the first */
    /* 0x02 */ u8 cond2; /* bit 7: negated */
    /* 0x03 */ u8 unk3;
    /* 0x04 */ s16 text;
    /* 0x06 */ s16 unk6;
} M007Choice;

/* A hint in the hint table (one after another) */
typedef struct M007Hint {
    /* 0x00 */ s16 count;
    /* 0x02 */ M007Choice choices[1]; /* count of them */
} M007Hint;

extern M007Hint *rl_m007_02A50; /* the hint table */
extern void *rl_m007_02A54; /* the hint texts */
extern s32 rl_m007_02A6C, rl_m007_02A70, rl_m007_02A74;

/* The module's .data (still assembly), by each variable's module offset */
extern s32 rl_m007_02A24, rl_m007_02A28, rl_m007_02A2C, rl_m007_02A30, rl_m007_02A34;
extern Object *rl_m007_02A60;
#define sHintActive rl_m007_02A24
#define sHintSpecial rl_m007_02A28
#define sHintTriedToEnd rl_m007_02A2C
#define sHintPrevious rl_m007_02A30
#define sHintPreviousArg rl_m007_02A34
#define sHintObject rl_m007_02A60
extern s32 rl_m007_02A20; /* the text's fade (0..256) */
/* A page of a long hint (size 0x10) */
typedef struct M007PageRec {
    /* 0x00 */ u8 *text;      /* an offset in the file until loaded */
    /* 0x04 */ s16 condition; /* shown when it holds */
    /* 0x06 */ s16 cond2;     /* A does the action when it holds */
    /* 0x08 */ s16 action;    /* the kind in the top 4 bits */
    /* 0x0A */ s16 elseHint;  /* shown when cond2 does not hold, or -1 */
    /* 0x0C */ s16 thenHint;  /* shown after the action, or -1 */
    /* 0x0E */ s16 padE;
} M007PageRec;

/* The pages of a long hint */
typedef struct M007Pages {
    /* 0x00 */ s16 count;
    /* 0x02 */ s16 current;
    /* 0x04 */ M007PageRec recs[1]; /* count of them */
} M007Pages;

/* A hint on the stack of hints shown (size 8) */
typedef struct M007Shown {
    /* 0x00 */ s16 hint;
    /* 0x02 */ s16 kind;
    /* 0x04 */ void *data;
} M007Shown;

/* A loaded hint text (size 8) */
typedef struct M007Text {
    /* 0x00 */ s32 id;
    /* 0x04 */ void *data;
} M007Text;

extern s32 rl_m007_02A44; /* the kind of hint shown */
extern s32 rl_m007_02A48; /* texts loaded */
extern void *rl_m007_02A58;
extern M007Pages *rl_m007_02A5C;
extern M007Text rl_m007_02C58[32];
extern M007Shown rl_m007_02D58[16];

/* The module's .bss (still assembly): the controller as the hint sees it */
extern s32 rl_m007_02C30; /* A pressed */
extern s32 rl_m007_02C34; /* B pressed */
extern s32 rl_m007_02C38; /* Z pressed */
extern s32 rl_m007_02C3C; /* stick pushed up */
extern s32 rl_m007_02C40; /* stick pushed down */
extern s32 rl_m007_02C44; /* stick pushed left */
extern s32 rl_m007_02C48; /* stick pushed right */
extern s32 rl_m007_02C4C; /* the stick's x repeat timer */
extern s32 rl_m007_02C50; /* the stick's y repeat timer */
extern s32 rl_m007_02A64; /* an object to load for the hint, or -1 */
extern s32 rl_m007_02A68; /* a level's start hint, or -1 */

/* A level object entry built on the stack */
typedef struct M007Entry {
    /* 0x00 */ s16 objectID;
    /* 0x02 */ u8 size;
    /* 0x03 */ u8 unk3;
    /* 0x04 */ s16 x;
    /* 0x06 */ s16 y;
    /* 0x08 */ s16 z;
    /* 0x0A */ u8 angle;
    /* 0x0B */ u8 unkB;
    /* 0x0C */ f32 scale;
} M007Entry;

void func_m007_00000(void);
void func_m007_02824(void);
static void func_m007_00E0C(s32 updateRate);
static void func_m007_01018(s32 updateRate);
extern s32 func_8007230C(s32 font); /* a font's line height */
extern s32 runlink_module_loaded(s32 module);
extern void func_m032_02540(void);
extern s32 rl_m007_02A38; /* the lines left to scroll (negative: back) */
extern s32 rl_m007_02A3C; /* the first line shown */
extern s32 rl_m007_02A40; /* more lines below */
extern s32 rl_m007_02A4C; /* the hints on the stack */
static void func_m007_01214(s32 updateRate);
static void func_m007_01F80(Gfx **dl);
static void func_m007_020D4(Gfx **dl);
static void func_m007_01E34(Gfx **dl, s8 *src, s32 x, s32 y, s32 w, s32 h, s32 alpha);
static s32 func_m007_023D4(Gfx **dl, u8 *text, s32 skip);
static void func_m007_017E8(s32 action);

/* A box to fill (as m075's) */
typedef struct M007Box {
    s16 x1, y1, x2, y2;
    s32 colour;
} M007Box;

extern void menu_fill_boxes(Gfx **dl, s32 count, M007Box *boxes, s32 arg3);
extern void func_8006FC8C(s32 r, s32 g, s32 b, s32 a);
extern void func_8006FC68(s32 r, s32 g, s32 b, s32 a, s32 opacity);
extern void func_8006FCA8(Gfx **dl, s32 x, s32 y, char *text, s32 flags);

/* The module's .data: the hint box's outline, 9 bytes a box (still assembly) */
extern s8 rl_m007_02960[];
/* The module's .rodata (still assembly): the "more" marks */
extern void func_8006FC14(s32 arg0);
extern void func_8003B620(s16 x, s16 y, s16 z, s16 angle);
extern s32 func_800468CC(void);
extern s32 menu_mode_get(void);
extern void level_goto(s32 a, s32 b, s32 c, s32 d, s32 e, s32 f);
extern Object *func_80005F14(void *entry, s32 arg1);

/* Frees the hint texts and clears the hint state */
void func_m007_00000(void) {
    s32 i;

    func_m007_02824();
    rl_m007_02A64 = -1;
    rl_m007_02A68 = -1;
    rl_m007_02A6C = 0;
    rl_m007_02A74 = 0;
    if (rl_m007_02A50 != NULL) {
        func_8004AA80(rl_m007_02A50);
    }
    if (rl_m007_02A54 != NULL) {
        func_8004AA80(rl_m007_02A54);
    }
    for (i = 0; i < 32; i++) {
        if (rl_m007_02A48 && rl_m007_02C58[i].data != NULL) {
            func_8004AA80(rl_m007_02C58[i].data);
        }
        rl_m007_02C58[i].data = NULL;
        rl_m007_02C58[i].id = -1;
    }
    rl_m007_02A48 = 0;
    rl_m007_02A4C = 0;
    rl_m007_02A20 = 0;
    sHintActive = 0;
    sHintSpecial = 0;
    sHintPrevious = -1;
    sHintPreviousArg = -1;
    rl_m007_02A38 = 0;
    rl_m007_02A3C = 0;
    rl_m007_02A40 = 0;
    rl_m007_02A50 = NULL;
    rl_m007_02A54 = NULL;
    rl_m007_02A58 = NULL;
    rl_m007_02A5C = NULL;
}

/* Starts hint `hint` (0 to 44) with the first of its entries whose condition holds */
void func_m007_00148(s32 hint, s32 arg1) {
    M007Hint *p;
    s32 a;
    M007Choice *c;
    s32 n;
    s32 b;
    s32 asset;

    func_m007_00000();
    if (hint >= 0 && hint < 45) {
        rl_m007_02A6C = arg1;
        rl_m007_02A50 = func_8004D37C(8);
        switch (func_8005A7D0()) {
            case 1:
                asset = 11;
                break;
            case 2:
                asset = 13;
                break;
            case 3:
                asset = 15;
                break;
            case 4:
                asset = 17;
                break;
            default:
                asset = 9;
                break;
        }
        rl_m007_02A54 = func_8004D37C(asset);
        p = rl_m007_02A50;
        while (hint--) {
            p = (M007Hint *) &p->choices[p->count];
        }
        n = p->count - 1;
        c = p->choices;
        while (n--) {
            a = func_m007_00390(c->cond1 & 0x7F);
            if (c->cond1 & 0x80) {
                a = !a;
            }
            b = func_m007_00390(c->cond2 & 0x7F);
            if (c->cond2 & 0x80) {
                b = !b;
            }
            if (c->op == 0) {
                if (a && b) {
                    break;
                }
            } else if (c->op == 1) {
                if (a || b) {
                    break;
                }
            } else if (a) {
                break;
            }
            c++;
        }
        rl_m007_02A70 = c->unk6;
        rl_m007_02A74 = c->unk3;
        if (func_m007_00578(c->text, 3)) {
            sHintActive = 1;
        }
        func_m007_00DF0();
    }
}

extern s32 rl_m007_02A78[12]; /* .data (still assembly): game flags that 0032C counts */

/* How many of the twelve flags are set (called from 00390) */
static s32 func_m007_0032C(void) {
    s32 i;
    s32 count = 0;

    i = 0;
    while (i < 12) {
        if (game_flag_get(rl_m007_02A78[i++])) {
            count++;
        }
    }
    return count;
}

/* The player's data (its object's unk68), as 00390 uses it */
typedef struct M007Player {
    /* 0x000 */ s8 player;
    /* 0x001 */ s8 character;
    /* 0x002 */ u8 pad2[0x189 - 0x2];
    /* 0x189 */ s8 unk189;
} M007Player;

/* A character's items (what func_80047C1C returns) */
typedef struct M007Items {
    /* 0x00 */ s16 unk0;
    /* 0x02 */ s16 unk2;
    /* 0x04 */ u8 pad4[0xA - 0x4];
    /* 0x0A */ u16 flags; /* bit per weapon owned */
    /* 0x0C */ u8 padC[0x12 - 0xC];
    /* 0x12 */ s16 count;
    /* 0x14 */ u16 ammo[16];
    /* 0x34 */ u16 maxAmmo[16];
    /* 0x54 */ u8 pad54[0x59 - 0x54];
    /* 0x59 */ u8 unk59;
    /* 0x5A */ u8 pad5A[0x6F - 0x5A];
    /* 0x6F */ u8 weapons; /* weapons owned */
} M007Items;

/* What weapon_info_get returns: a weapon's properties */
typedef struct M007WeaponInfo {
    /* 0x00 */ u8 pad0[3];
    /* 0x03 */ u8 unk3;
    /* 0x04 */ u8 pad4[0x24 - 0x4];
    /* 0x24 */ s16 maxAmmo;
    /* 0x26 */ u8 pad26[2];
    /* 0x28 */ s16 ammo;
} M007WeaponInfo;

extern M007Items *func_80047C1C(s32 player, s32 character);
extern s32 character_has_item(s32 player, s32 character, s32 item);
extern s32 player_chars_have_item(s32 item);
extern void character_item_give(s32 player, s32 character, s32 item);
extern void character_item_take(s32 player, s32 character, s32 item);
extern M007WeaponInfo *weapon_info_get(s32 weapon);
extern void func_80047EFC(s32 flag, s32 value); /* sets a game flag */
extern void func_80000F2C(void);                /* stops the music */
extern void func_80000510(s32 song);            /* plays a song */

/* Whether hint condition `condition` holds (with no player, every one does) */
static s32 func_m007_00390(s32 condition) {
    M007Player *p;
    Object *obj;
    M007Items *items;

    obj = func_8000E4D0(0);
    if (obj == NULL) {
        return TRUE;
    }
    p = obj->unk68;
    items = func_80047C1C(p->player, p->character);
    switch (condition) {
        case 1:
            return rl_m007_02A6C;
        case 2:
            return character_has_item(p->player, p->character, 1);
        case 3:
            return player_chars_have_item(0x15);
        case 4:
            return items->flags & 0x20;
        case 6:
            return game_flag_get(0x59);
        case 5:
            return game_flag_get(0x20);
        case 7:
            return game_flag_get(0x21);
        case 8:
            return game_flag_get(0x22);
        case 9:
            return game_flag_get(0x23);
        case 17:
            return game_flag_get(0x24);
        case 10:
            return func_m007_0032C() == 0;
        case 11:
            return func_m007_0032C() < 7;
        case 12:
            return func_m007_0032C() >= 7;
        case 13:
            return func_m007_0032C() == 12;
        case 14:
            return game_flag_get(0x58);
        case 15:
            return p->unk189 == 0;
        case 16:
            return p->unk189 == 2;
        case 18:
            return player_chars_have_item(0x16);
        case 19:
            return game_flag_get(0x2B);
        case 20:
            return player_chars_have_item(0x11);
    }
    return TRUE;
}

/*
 * Shows hint text `hint` (kind 1: pages, 2: special, else scrolling), pushing it on the stack of
 * hints shown; while a special hint is up it is only remembered. Returns its text.
 */
static void *func_m007_00578(s32 hint, s32 kind) {
    void *data = NULL;

    if (sHintSpecial) {
        sHintPrevious = hint;
        sHintPreviousArg = kind;
        return NULL;
    }
    sHintPrevious = -1;
    sHintPreviousArg = -1;
    if (rl_m007_02A4C < 16) {
        if (kind == 2) {
            rl_m007_02A68 = hint;
            sHintSpecial = 2;
        } else if (kind == 1) {
            rl_m007_02A5C = func_m007_00988(hint, 1);
            if (func_m007_007E4(0)) {
                data = rl_m007_02A5C;
                rl_m007_02A44 = 1;
            }
        } else {
            rl_m007_02A58 = func_m007_00988(hint, 4);
            rl_m007_02A38 = 0;
            rl_m007_02A3C = 0;
            rl_m007_02A40 = 0;
            rl_m007_02A44 = kind;
            data = rl_m007_02A58;
        }
        if (data != NULL) {
            rl_m007_02D58[rl_m007_02A4C].hint = hint;
            rl_m007_02D58[rl_m007_02A4C].kind = kind;
            rl_m007_02D58[rl_m007_02A4C].data = data;
            rl_m007_02A4C++;
        }
        if (runlink_module_loaded(0x20)) {
            func_m032_02540();
        }
    }
    return data;
}

/* Goes back to the hint under the one shown, if there is one */
static void func_m007_0070C(void) {
    M007Shown *s;

    if (rl_m007_02A4C > 0 && --rl_m007_02A4C > 0) {
        func_m007_02824();
        s = &rl_m007_02D58[rl_m007_02A4C - 1];
        rl_m007_02A44 = s->kind;
        if (s->kind != 1) {
            rl_m007_02A58 = s->data;
            rl_m007_02A38 = 0;
            rl_m007_02A3C = 0;
            rl_m007_02A40 = 0;
        } else {
            rl_m007_02A5C = s->data;
            if (!func_m007_007E4(0)) {
                func_m007_0070C();
            }
        }
        if (runlink_module_loaded(0x20)) {
            func_m032_02540();
        }
    }
}

#ifdef NON_MATCHING
/*
 * Counts the pages whose condition holds, and turns to the next one that holds in direction `dir`
 * (0: the current one if it holds, else the one before or after).
 * NON_MATCHING: the original walks the pages from the start of the block (4 bytes before the
 * first) and schedules the first loop's increments before the call.
 */
static s32 func_m007_007E4(s32 dir) {
    M007Pages *p = rl_m007_02A5C;
    s32 i;
    M007PageRec *rec;
    s32 count = 0;

    if (p != NULL) {
        for (i = 0, rec = p->recs; i < p->count; i++, rec++) {
            if (rl_m007_012E0(rec->condition)) {
                count++;
            }
        }
        if (count) {
            i = p->current;
            if (dir > 0) {
                do {
                    i++;
                } while (i < p->count && !rl_m007_012E0(p->recs[i].condition));
            } else {
                rec = &p->recs[i];
                if (dir < 0) {
                    do {
                        i--;
                    } while (i >= 0 && !rl_m007_012E0(p->recs[i].condition));
                } else if (!rl_m007_012E0(rec->condition)) {
                    do {
                        i--;
                        rec--;
                    } while (i >= 0 && !rl_m007_012E0(rec->condition));
                    if (i < 0) {
                        i = p->current;
                        do {
                            i++;
                        } while (i < p->count && !rl_m007_012E0(p->recs[i].condition));
                    }
                }
            }
            if (i >= 0 && i < p->count) {
                p->current = i;
            }
        }
    }
    return count;
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m007", func_m007_007E4);
#endif

#ifdef NON_MATCHING
/*
 * Text `id` (kind 1: pages, else plain), loaded into a free slot if it is not loaded yet.
 * NON_MATCHING: the frame is two words bigger and the registers differ; func_m007_00C90 (close:
 * its loop's pointer starts 4 bytes later than the original's) is only called from here, so it
 * stays assembly with it.
 */
static void *func_m007_00988(s32 id, s32 kind) {
    M007Text *t;
    s32 i;
    s32 asset;

    for (t = rl_m007_02C58; t < &rl_m007_02C58[32]; t++) {
        if (t->data != NULL && id == t->id) {
            return t->data;
        }
    }
    for (i = 0; i < 32 && rl_m007_02C58[i].data != NULL; i++) {
    }
    if (i >= 32) {
        return NULL;
    }
    t = &rl_m007_02C58[i];
    switch (func_8005A7D0()) {
        case 1:
            asset = 12;
            break;
        case 2:
            asset = 14;
            break;
        case 4:
            asset = 18;
            break;
        default:
            asset = 10;
            break;
    }
    if (kind != 1) {
        t->data = rl_m007_00B00(id, asset);
    } else {
        t->data = func_m007_00C90(id, asset);
    }
    if (t->data != NULL) {
        t->id = id;
        rl_m007_02A48++;
    }
    return t->data;
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m007", func_m007_00988);
#endif

INCLUDE_ASM("asm/nonmatchings/modules/m007/ro", func_m007_00B00);

#ifdef NON_MATCHING

/* Loads text `id` from asset `asset` as pages, turning their text offsets into pointers */
static void *func_m007_00C90(s32 id, s32 asset) {
    UNUSED s32 *offsets;
    s32 start;
    s32 size;
    M007Pages *p;
    s32 i;
    M007PageRec *rec;

    start = ((s32 *) rl_m007_02A54)[id];
    size = ((s32 *) rl_m007_02A54)[id + 1] - start;
    p = func_8004A57C(size, 0xFF00FF);
    if (p != NULL) {
        func_8004D530(asset, p, start, size);
        for (i = 0, rec = p->recs; i < p->count; i++, rec++) {
            if (rec->text == NULL) {
                if (rec->action == 0x4000) {
                    rec->text = rl_m007_02A04;
                } else {
                    rec->text = rl_m007_02A14;
                }
            } else {
                rec->text = rec->text + (s32) p;
            }
        }
    }
    return p;
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m007", func_m007_00C90);
#endif

/* Stops the hint text */
void func_m007_00D6C(void) {
    sHintActive = 0;
    sHintTriedToEnd = 0;
}

s32 func_m007_00D80(void) {
    return sHintActive;
}

s32 func_m007_00D90(void) {
    return sHintSpecial;
}

/* Stops the special hint text, going back to the previous one if there was one */
void func_m007_00DA0(void) {
    sHintSpecial = 0;
    if (sHintPrevious != -1) {
        func_m007_00578(sHintPrevious, sHintPreviousArg);
    }
}

s32 func_m007_00DE0(void) {
    return sHintTriedToEnd;
}

/* Clears the stick's repeat timers */
static void func_m007_00DF0(void) {
    rl_m007_02C4C = 0;
    rl_m007_02C50 = 0;
    sHintTriedToEnd = 0;
}

/* Reads the controller: the buttons, and the stick's directions with auto-repeat */
static void func_m007_00E0C(s32 updateRate) {
    s32 buttons;
    s32 x;
    s32 y;

    buttons = func_80043510(0);
    x = func_80043558(0);
    y = func_800435C8(0);
    rl_m007_02C30 = buttons & 0x8000;
    rl_m007_02C34 = buttons & 0x4000;
    rl_m007_02C38 = buttons & 0x1000;
    rl_m007_02C3C = 0;
    rl_m007_02C40 = 0;
    rl_m007_02C44 = 0;
    rl_m007_02C48 = 0;
    if (y >= -30 && y < 31) {
        rl_m007_02C50 = 0;
    } else if (y >= 31) {
        if (rl_m007_02C50 > 0) {
            if ((rl_m007_02C50 -= updateRate) <= 0) {
                rl_m007_02C50 = 10;
                rl_m007_02C3C = 1;
            }
        } else {
            rl_m007_02C50 = 20;
            rl_m007_02C3C = 1;
        }
    } else if (y < -30) {
        if (rl_m007_02C50 < 0) {
            if ((rl_m007_02C50 += updateRate) >= 0) {
                rl_m007_02C50 = -10;
                rl_m007_02C40 = 1;
            }
        } else {
            rl_m007_02C50 = -20;
            rl_m007_02C40 = 1;
        }
    }
    /* (the original's dead zone for x is empty) */
    if (x > -30 && x < -30) {
        rl_m007_02C4C = 0;
    } else if (x >= 31) {
        if (rl_m007_02C4C > 0) {
            if ((rl_m007_02C4C -= updateRate) <= 0) {
                rl_m007_02C4C = 10;
                rl_m007_02C48 = 1;
            }
        } else {
            rl_m007_02C4C = 20;
            rl_m007_02C48 = 1;
        }
    } else if (x < -30) {
        if (rl_m007_02C4C < 0) {
            if ((rl_m007_02C4C += updateRate) <= 0) {
                rl_m007_02C4C = -10;
                rl_m007_02C44 = 1;
            }
        } else {
            rl_m007_02C4C = -20;
            rl_m007_02C44 = 1;
        }
    }
}

/* A scrolling hint's controls: A scrolls on or ends it, B scrolls back */
static void func_m007_01018(s32 updateRate) {
    s32 lines = 88 / func_8007230C(2) - 1;

    if (rl_m007_02A38 != 0) {
        if (rl_m007_02A38 < 0) {
            rl_m007_02A38++;
            rl_m007_02A3C--;
        } else {
            rl_m007_02A38--;
            rl_m007_02A3C++;
        }
    } else if (rl_m007_02A40 && rl_m007_02C30) {
        rl_m007_02A38 = lines;
        if (runlink_module_loaded(0x20)) {
            func_m032_02540();
        }
    } else if (rl_m007_02A3C >= lines && rl_m007_02C34) {
        rl_m007_02A38 = -lines;
        if (runlink_module_loaded(0x20)) {
            func_m032_02540();
        }
    }
    if (!rl_m007_02A40 && rl_m007_02C30) {
        if (rl_m007_02A44 == 4 && rl_m007_02A4C >= 2) {
            func_m007_0070C();
        } else if (rl_m007_02A44 == 3 && rl_m007_02A74 != 0) {
            func_m007_00578(rl_m007_02A70, rl_m007_02A74);
        } else {
            sHintTriedToEnd = 1;
        }
    } else if (rl_m007_02C34 && rl_m007_02A4C >= 2) {
        func_m007_0070C();
    }
}

/* A long hint's controls: B ends it, A moves on, up and down turn the pages */
static void func_m007_01214(s32 updateRate) {
    if (rl_m007_02C34) {
        func_m007_0070C();
    } else if (rl_m007_02C30) {
        func_m007_016B4();
    } else if (rl_m007_02C3C && rl_m007_02A5C->current > 0) {
        func_m007_007E4(-1);
    } else if (rl_m007_02C40 && rl_m007_02A5C->current < rl_m007_02A5C->count - 1) {
        func_m007_007E4(1);
    }
}

INCLUDE_ASM("asm/nonmatchings/modules/m007/ro", func_m007_012E0);

/*
 * Whether a long hint's condition `condition` holds. With no player, p and items are read unset (as the
 * original does).
 */
static int func_m007_01504(s32 condition) {
    Object *obj;
    M007Player *p;
    M007Items *items;

    obj = func_8000E4D0(0);
    if (obj != NULL) {
        p = obj->unk68;
        if (p != NULL) {
            items = func_80047C1C(p->player, p->character);
        }
    }
    switch (condition) {
        case 0:
            return character_has_item(p->player, p->character, 0);
        case 1:
            return character_has_item(p->player, p->character, 0x10);
        case 2:
            if (game_flag_get(0x2C) && game_flag_get(0x2D) && game_flag_get(0x2E)) {
                return TRUE;
            }
            return FALSE;
        case 3:
            return character_has_item(p->player, p->character, 0x14);
        case 4:
            return items->count >= 5;
        case 5:
            return items->count >= 10;
        case 6:
            return character_has_item(p->player, p->character, 0x17) &&
                   character_has_item(p->player, p->character, 0x18) &&
                   character_has_item(p->player, p->character, 0x19);
        case 7:
            return character_has_item(p->player, p->character, 0x1A);
    }
    return TRUE;
}

/* A paged hint's A button: does the page's action if its condition holds, then shows its next hint */
static void func_m007_016B4(void) {
    M007PageRec *rec;
    M007Pages *p = rl_m007_02A5C;

    if (p != NULL) {
        rec = &p->recs[p->current];
        if (func_m007_01504(rec->cond2)) {
            switch (rec->action & 0xF000) {
                case 0:
                    func_m007_017E8(rec->action);
                    break;
                case 0x1000:
                    func_m007_00578(rec->action & 0xFFF, 4);
                    break;
                case 0x2000:
                    func_m007_00578(rec->action & 0xFFF, 1);
                    break;
                case 0x3000:
                    rl_m007_02A68 = rec->action & 0xFFF;
                    sHintSpecial = 2;
                    break;
                case 0x4000:
                    func_m007_0070C();
                    break;
                default:
                    sHintActive = 0;
                    break;
            }
            if (rec->thenHint != -1) {
                func_m007_00578(rec->thenHint, 4);
            }
        } else if (rec->elseHint != -1) {
            func_m007_00578(rec->elseHint, 4);
        }
    }
}

/*
 * Does a long hint's action `action`: gives or takes items, sets game flags, changes the music. With no player,
 * p and items are read unset (as the original does).
 */
static void func_m007_017E8(s32 action) {
    M007WeaponInfo *info;
    Object *obj;
    M007Player *p;
    M007Items *items;
    s32 i;

    obj = func_8000E4D0(0);
    if (obj != NULL) {
        p = obj->unk68;
        if (p != NULL) {
            items = func_80047C1C(p->player, p->character);
        }
    }
    switch (action) {
        case 0:
            character_item_give(p->player, p->character, 1);
            sHintSpecial = 1;
            rl_m007_02A64 = 0x17D;
            break;
        case 1:
            character_item_give(p->player, p->character, 0x10);
            sHintSpecial = 1;
            rl_m007_02A64 = 0x18A;
            break;
        case 2:
            character_item_give(p->player, p->character, 0x11);
            character_item_take(p->player, p->character, 0x10);
            sHintSpecial = 1;
            rl_m007_02A64 = 0x185;
            break;
        case 3:
        case 4:
            if (action == 4) {
                character_item_take(p->player, p->character, 0x17);
                character_item_take(p->player, p->character, 0x18);
                character_item_take(p->player, p->character, 0x19);
            }
            info = weapon_info_get(5);
            items->flags |= 0x20;
            items->weapons++;
            items->maxAmmo[5] = info->maxAmmo;
            items->ammo[5] = info->ammo;
            items->unk59 = info->unk3;
            sHintSpecial = 1;
            rl_m007_02A64 = 0x271;
            break;
        case 5:
            character_item_give(p->player, p->character, 0x15);
            character_item_take(p->player, p->character, 0x14);
            sHintSpecial = 1;
            rl_m007_02A64 = 0x27D;
            break;
        case 6:
            items->unk0 = items->unk2 * 5 + 4;
            obj->interactObj->hitPoints = items->unk0 << 8;
            items->count -= 5;
            break;
        case 7:
            for (i = 0; i < 16; i++) {
                if (items->flags & (1 << i)) {
                    items->ammo[i] = items->maxAmmo[i];
                }
            }
            items->count -= 10;
            break;
        case 8:
            func_80047EFC(0x20, 1);
            character_item_take(p->player, p->character, 0x1A);
            sHintSpecial = 1;
            rl_m007_02A64 = 0x2E6;
            break;
        case 9:
            func_80047EFC(0x21, 1);
            sHintSpecial = 1;
            rl_m007_02A64 = 0x2E8;
            break;
        case 10:
            func_80047EFC(0x22, 1);
            sHintSpecial = 1;
            rl_m007_02A64 = 0x2E9;
            break;
        case 11:
            func_80047EFC(0x23, 1);
            sHintSpecial = 1;
            rl_m007_02A64 = 0x2EA;
            break;
        case 12:
            func_80047EFC(0x24, 1);
            sHintSpecial = 1;
            rl_m007_02A64 = 0x2EB;
            break;
        case 13:
            character_item_give(p->player, p->character, 0x16);
            sHintSpecial = 1;
            rl_m007_02A64 = 0x2C1;
            break;
        case 14:
            func_80000F2C();
            func_80000510(0x3C);
            break;
        case 15:
            func_80000F2C();
            func_80000510(0x3D);
            break;
        case 16:
            func_80000F2C();
            func_80000510(0x3E);
            break;
        case 17:
            func_80000F2C();
            func_80000510(0x41);
            break;
        case 18:
            func_80047EFC(0x2B, 1);
            sHintSpecial = 1;
            rl_m007_02A64 = 0x6C;
            break;
        default:
            func_m007_0070C();
            break;
    }
}

/* Every frame: the hint text's fade in and out, and its drawing */
void func_m007_01CCC(Gfx **dl, s32 updateRate) {
    func_m007_00E0C(updateRate);
    if (sHintActive && !sHintSpecial) {
        if (rl_m007_02A44 != 1 && rl_m007_02A58 != 0) {
            func_m007_01018(updateRate);
        } else if (rl_m007_02A5C != NULL) {
            func_m007_01214(updateRate);
        }
        rl_m007_02A20 += updateRate * 16;
        if (rl_m007_02A20 > 256) {
            rl_m007_02A20 = 256;
        }
    } else if (rl_m007_02A20 > 0) {
        rl_m007_02A20 -= updateRate * 16;
        if (rl_m007_02A20 <= 0) {
            if (!sHintSpecial) {
                func_m007_00000();
            } else {
                rl_m007_02A20 = 0;
            }
        }
    }
    func_8006FC14(1);
    if (rl_m007_02A20 != 0) {
        if (rl_m007_02A44 != 1) {
            func_m007_01F80(dl);
        } else {
            func_m007_020D4(dl);
        }
    }
    func_8006FC14(0);
}

/* Fills the boxes of an outline (9 bytes a box) at (x, y), stretched by (w, h), at `alpha` (0..128) */
static void func_m007_01E34(Gfx **dl, s8 *src, s32 x, s32 y, s32 w, s32 h, s32 alpha) {
    s32 n;
    M007Box boxes[24];
    M007Box *b = boxes;

    n = 0;

    while (src[0] >= 0 && n != 24) {
        b->x1 = src[1] + x;
        if (src[0]) {
            b->x1 += w;
        }
        b->y1 = src[3] + y;
        if (src[2]) {
            b->y1 += h;
        }
        b->x2 = src[5] + x;
        if (src[4]) {
            b->x2 += w;
        }
        b->y2 = src[7] + y;
        if (src[6]) {
            b->y2 += h;
        }
        n++;
        b->colour = (src[8] * alpha) >> 7;
        src += 9;
        b++;
    }
    menu_fill_boxes(dl, n, boxes, 0);
}

/* Draws a scrolling hint: its box, its text, and the marks for more text above and below */
static void func_m007_01F80(Gfx **dl) {
    func_m007_01E34(dl, rl_m007_02960, 0x5C, 0x14, 0xD0, 0x58, (rl_m007_02A20 * 160) >> 8);
    rl_m007_02A40 = func_m007_023D4(dl, rl_m007_02A58, rl_m007_02A3C);
    func_8006FC8C(0, 0, 0, 0);
    func_8006FC68(0xFF, 0xC0, 0, 0xFF, (rl_m007_02A20 * 255) >> 8);
    if (rl_m007_02A3C) {
        func_8006FCA8(dl, 0xC4, 0x14, "~", 0xC);
    }
    if (rl_m007_02A40 || (rl_m007_02A44 == 4 && rl_m007_02A4C >= 2) || (rl_m007_02A44 == 3 && rl_m007_02A74)) {
        func_8006FCA8(dl, 0xC4, 0x6C, "$", 0xC);
    }
}

extern void func_8006FC54(s32 font);
extern u8 *func_8007216C(s32 font, u8 *text, s32 width, u8 **line); /* the next line of a text */

/* Draws a long hint's pages (the current one in white) and the "more" mark if they don't all fit */
static void func_m007_020D4(Gfx **dl) {
    s32 height;
    s32 i;
    s32 lines;
    s32 x;
    s32 y;
    s32 first;
    s32 alpha;
    u8 *text;
    u8 *line;
    u8 c;

    func_m007_01E34(dl, rl_m007_02960, 0x5C, 0x14, 0xD0, 0x58, (rl_m007_02A20 * 160) >> 8);
    i = 0;
    height = func_8007230C(2);
    lines = 88 / height - 1;
    func_8006FC54(2);
    func_8006FC8C(0, 0, 0, 0);
    y = ((88 - lines * height) >> 1) + 20;
    alpha = (rl_m007_02A20 * 255) >> 8;
    for (; i < rl_m007_02A5C->count && lines > 0; i++) {
        first = TRUE;
        x = 0x60;
        if (i == rl_m007_02A5C->current) {
            func_8006FC68(0xFF, 0xFF, 0xFF, 0xFF, alpha);
        } else {
            func_8006FC68(0, 0xC0, 0xC0, 0xFF, alpha);
        }
        text = rl_m007_02A5C->recs[i].text;
        if (!rl_m007_012E0(rl_m007_02A5C->recs[i].condition)) {
            text = NULL;
        }
        while (text != NULL && lines > 0) {
            text = func_8007216C(2, text, 200, &line);
            if (text != NULL) {
                c = *text;
                *text = 0;
                func_8006FCA8(dl, x, y, (char *) line, 0);
                *text = c;
                y += height;
                if (first) {
                    x += 8;
                    first = FALSE;
                }
                lines--;
            }
        }
    }
    func_8006FC8C(0, 0, 0, 0);
    func_8006FC68(0xFF, 0xC0, 0, 0xFF, alpha);
    if (i < rl_m007_02A5C->count || text != NULL) {
        func_8006FCA8(dl, 0xC4, 0x6C, "$", 0xC);
    }
}

/*
 * Draws a scrolling hint's text from line `skip` on. The text is a list of commands: 1 a blank line
 * (4 bytes), 2-5 a text (its address in the low 24 bits; 2 at the right, 3 plain, 4 at the far
 * right, 5 with its first line indented), 6 the text colour and 7 the shadow colour (8 bytes);
 * anything else ends it. Returns whether lines are left below.
 */
static s32 func_m007_023D4(Gfx **dl, u8 *text, s32 skip) {
    UNUSED s32 pad1;
    UNUSED s32 pad2;
    UNUSED s32 pad3;
    s32 flags;
    UNUSED s32 pad4;
    s32 height;
    s32 done;
    s32 more;
    s32 lines;
    s32 y;
    s32 x;
    s32 width;
    s32 indent;
    u8 *str;
    u8 c;
    u8 *line;
    UNUSED s32 pad5;
    UNUSED s32 pad6;

    more = 0;
    done = 0;
    height = func_8007230C(2);
    lines = 88 / height - 1;
    func_8006FC8C(0, 0, 0, 0);
    func_8006FC68(0xFF, 0xFF, 0xFF, 0xFF, (rl_m007_02A20 * 255) >> 8);
    func_8006FC54(2);
    y = ((88 - lines * height) >> 1) + 20;
    do {
        flags = 0;
        x = 0x60;
        width = 200;
        str = NULL;
        indent = FALSE;
        switch (*text) {
            case 1:
                if (skip == 0) {
                    lines--;
                    y += height;
                } else {
                    skip--;
                }
                text += 4;
                break;
            case 2:
                str = (u8 *) ((*(u32 *) text & 0xFFFFFF) | 0x80000000);
                text += 4;
                flags = 4;
                x = 0xC4;
                break;
            case 3:
                str = (u8 *) ((*(u32 *) text & 0xFFFFFF) | 0x80000000);
                text += 4;
                break;
            case 4:
                str = (u8 *) ((*(u32 *) text & 0xFFFFFF) | 0x80000000);
                text += 4;
                flags = 1;
                x = 0x127;
                break;
            case 5:
                str = (u8 *) ((*(u32 *) text & 0xFFFFFF) | 0x80000000);
                text += 4;
                indent = TRUE;
                break;
            case 6:
                /* (the command's bytes read after stepping over it) */
                text += 8;
                c = text[-2];
                func_8006FC68(text[-7], text[-6], text[-5], text[-3], (c * rl_m007_02A20) >> 8);
                break;
            case 7:
                text += 8;
                func_8006FC8C(text[-7], text[-6], text[-5], text[-3]);
                break;
            default:
                done = TRUE;
                break;
        }
        while (str != NULL && lines >= 0) {
            if (indent) {
                x += 8;
                width -= 8;
            }
            str = func_8007216C(2, str, width, &line);
            if (str != NULL) {
                if (skip == 0) {
                    lines--;
                    if (lines >= 0) {
                        c = *str;
                        *str = 0;
                        func_8006FCA8(dl, x, y, (char *) line, flags);
                        *str = c;
                        y += height;
                    }
                } else {
                    skip--;
                }
            }
            if (indent) {
                x -= 8;
                width += 8;
                indent = FALSE;
            }
        }
        if (lines < 0) {
            more = TRUE;
            done = TRUE;
        }
    } while (!done);
    return more;
}

/* Loads the hint's object, if one is waiting */
void func_m007_02704(f32 x, f32 y, f32 z, f32 angle, f32 scale) {
    M007Entry entry;
    UNUSED Object *obj;

    if (rl_m007_02A64 != -1) {
        func_m007_02824();
        entry.objectID = rl_m007_02A64;
        entry.size = 0x10;
        entry.unk3 = 0;
        entry.x = x;
        entry.y = y;
        entry.z = z;
        entry.angle = (s32) angle;
        entry.unkB = 0;
        entry.scale = scale;
        sHintObject = func_80005F14(&entry, 1);
        if (sHintObject != NULL) {
            sHintObject->level_entry = NULL;
        }
        rl_m007_02A64 = -1;
    }
}

/* Lets the hint's object go */
void func_m007_02824(void) {
    if (sHintObject != NULL) {
        OBJ_STATE(sHintObject, s32)[0] = 1;
        sHintObject = NULL;
    }
}

/* At a level's start: shows its start hint, if it has one */
void func_m007_0284C(void) {
    Object *player;
    UNUSED s32 pad;
    s32 arg;

    if (rl_m007_02A68 != -1) {
        player = func_8000E4D0(0);
        if (player != NULL) {
            func_8003B620(player->trans.x_position, player->trans.y_position + 2.0f, player->trans.z_position,
                          player->trans.rotation.y_rotation);
        }
        arg = func_800468CC();
        level_goto(rl_m007_02A68, arg, 0, menu_mode_get(), 1, 0);
        rl_m007_02A68 = -1;
    }
}

INCLUDE_ASM("asm/nonmatchings/modules/m007", data);

/* (.rodata: C, and the assembly functions' own from ro/) */

INCLUDE_ASM("asm/nonmatchings/modules/m007", bss);

