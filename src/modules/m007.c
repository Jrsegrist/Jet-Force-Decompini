/*
 * Runlink module 7: the hint text shown on screen (normal hints and special
 * ones that interrupt them). Only its small state functions are in C so far.
 */
#include "module.h"

static void *func_m007_00578(s32 hint, s32 kind);
extern s32 rl_m007_00390(s32 condition);
static void func_m007_00DF0(void);
static void func_m007_0070C(void);
#ifdef NON_MATCHING
static s32 func_m007_007E4(s32 dir);
#else
s32 rl_m007_007E4(s32 dir);
#define func_m007_007E4 rl_m007_007E4
#endif
extern void rl_m007_016B4(void);
extern void *rl_m007_00988(s32 hint, s32 arg1);
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
/* A page of a long hint (size 0x10); the first page also holds the count and the page shown */
typedef struct M007Pages {
    /* 0x00 */ s16 count;
    /* 0x02 */ s16 current;
    /* 0x04 */ u8 pad4[4];
    /* 0x08 */ s16 condition; /* shown when it holds */
    /* 0x0A */ u8 padA[0x10 - 0xA];
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
extern void rl_m007_01F80(s32 arg0);
extern void rl_m007_020D4(s32 arg0);
extern void func_8006FC14(s32 arg0);
extern void func_8003B620(s16 x, s16 y, s16 z, s16 angle);
extern s32 func_800468CC(void);
extern s32 menu_mode_get(void);
extern void func_8004665C(s32 a, s32 b, s32 c, s32 d, s32 e, s32 f);
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
            a = rl_m007_00390(c->cond1 & 0x7F);
            if (c->cond1 & 0x80) {
                a = !a;
            }
            b = rl_m007_00390(c->cond2 & 0x7F);
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

INCLUDE_ASM("asm/nonmatchings/modules/m007", func_m007_0032C);

INCLUDE_ASM("asm/nonmatchings/modules/m007", func_m007_00390);

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
            rl_m007_02A5C = rl_m007_00988(hint, 1);
            if (func_m007_007E4(0)) {
                data = rl_m007_02A5C;
                rl_m007_02A44 = 1;
            }
        } else {
            rl_m007_02A58 = rl_m007_00988(hint, 4);
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
 * The first loop's two increments come after the call here (before it in the original), and the
 * pointer for dir 0 is computed into v0 and moved.
 */
static s32 func_m007_007E4(s32 dir) {
    M007Pages *p = rl_m007_02A5C;
    s32 i;
    M007Pages *page;
    s32 count = 0;

    if (p != NULL) {
        for (i = 0, page = p; i < p->count; i++, page++) {
            if (rl_m007_012E0(page->condition)) {
                count++;
            }
        }
        if (count) {
            i = p->current;
            if (dir > 0) {
                do {
                    i++;
                } while (i < p->count && !rl_m007_012E0(p[i].condition));
            } else {
                page = &p[i];
                if (dir < 0) {
                    do {
                        i--;
                    } while (i >= 0 && !rl_m007_012E0(p[i].condition));
                } else if (!rl_m007_012E0(page->condition)) {
                    do {
                        i--;
                        page--;
                    } while (i >= 0 && !rl_m007_012E0(page->condition));
                    if (i < 0) {
                        i = p->current;
                        do {
                            i++;
                        } while (i < p->count && !rl_m007_012E0(p[i].condition));
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

INCLUDE_ASM("asm/nonmatchings/modules/m007", func_m007_00988);

INCLUDE_ASM("asm/nonmatchings/modules/m007", func_m007_00B00);

INCLUDE_ASM("asm/nonmatchings/modules/m007", func_m007_00C90);

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
        rl_m007_016B4();
    } else if (rl_m007_02C3C && rl_m007_02A5C->current > 0) {
        func_m007_007E4(-1);
    } else if (rl_m007_02C40 && rl_m007_02A5C->current < rl_m007_02A5C->count - 1) {
        func_m007_007E4(1);
    }
}

INCLUDE_ASM("asm/nonmatchings/modules/m007", func_m007_012E0);

INCLUDE_ASM("asm/nonmatchings/modules/m007", func_m007_01504);

INCLUDE_ASM("asm/nonmatchings/modules/m007", func_m007_016B4);

INCLUDE_ASM("asm/nonmatchings/modules/m007", func_m007_017E8);

/* Every frame: the hint text's fade in and out, and its drawing */
void func_m007_01CCC(s32 arg0, s32 updateRate) {
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
            rl_m007_01F80(arg0);
        } else {
            rl_m007_020D4(arg0);
        }
    }
    func_8006FC14(0);
}

INCLUDE_ASM("asm/nonmatchings/modules/m007", func_m007_01E34);

INCLUDE_ASM("asm/nonmatchings/modules/m007", func_m007_01F80);

INCLUDE_ASM("asm/nonmatchings/modules/m007", func_m007_020D4);

INCLUDE_ASM("asm/nonmatchings/modules/m007", func_m007_023D4);

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
        func_8004665C(rl_m007_02A68, arg, 0, menu_mode_get(), 1, 0);
        rl_m007_02A68 = -1;
    }
}

INCLUDE_ASM("asm/nonmatchings/modules/m007", data);

INCLUDE_ASM("asm/nonmatchings/modules/m007", rodata);

INCLUDE_ASM("asm/nonmatchings/modules/m007", bss);

