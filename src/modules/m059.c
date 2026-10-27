/*
 * Runlink module 59: the credits on the front end. Up to five lines of text
 * slide in from alternate sides, stay for a while (or until the script lets
 * them go) and slide out again; a small script of commands decides which
 * texts come next.
 */
#include "module.h"

/* Script commands (each an s16, some followed by arguments) */
#define M059_SHOW -1    /* count, time: bring in the next `count` texts for `time` */
#define M059_HOLD -2    /* line: keep that line in */
#define M059_RELEASE -3 /* line: let that line go */
#define M059_END -4

typedef struct M059Line {
    /* 0x0 */ char *text; /* NULL: free */
    /* 0x4 */ s16 timer;
    /* 0x6 */ s16 x;
    /* 0x8 */ s16 y;
    /* 0xA */ s8 held;
} M059Line;

extern Gfx *D_800FF398; /* the display list being built */
extern void func_80001248(s32 sound, s32 arg1);
extern void load_first_unlocked_level();
extern void set_text_font(s32 font);                                              /* DKR */
extern void set_text_colour(s32 red, s32 green, s32 blue, s32 alpha, s32 opacity); /* DKR */
extern void draw_text(Gfx **dList, s32 x, s32 y, char *text, s32 alignment);     /* DKR */

static char *sTexts[] = {
    "CREDITS",
    "Jet Force Gemini Team",
    "R. Beanland",
    "P. Cunningham",
    "N. Gallagher",
    "S. Hall",
    "N. Hutchinson",
    "A. Innes",
    "A. Lindsay",
    "L. Loveday",
    "S. Mcfarlane",
    "P. Mountain",
    "L. Musgrave",
    "J. Pegg",
    "B. Smyth",
    "M. Wakeley",
    " ",
    "Character Voices",
    "N. Gallagher",
    "A. Lindsay",
    "E. Fischer",
    "Rare Technical Support",
    "P. Cox",
    "D. Crouch",
    "M. Green",
    "P. Mikell",
    "Rare Quality Assurance",
    "H. Ward",
    "A. Munton",
    "M. Carter",
    "L. Munton",
    "G. Phelps",
    "G. Price",
    "J. Silke",
    "G. Stevenson",
    "D. Wong",
    "Thanks To",
    "D. Murphy",
    "M. Edmonds",
    "M. Betteridge",
    "M. Currington",
    "R. Harrison",
    "C. Seavor",
    "Rare USA",
    "E. Hochberg",
    "K. Coll",
    "S. Hochberg",
    "J. Rogowski",
    "NOA Thanks To",
    "M. Fukuda",
    "K. Lobb",
    "M. Kelbaugh",
    "R. Johnson",
    "J. Kalles",
    "C. Needham",
    "P. Taylor",
    "S. Egan",
    "H. Alexander",
    "M. Forrest",
    "M. Leslie",
    "C. Burgin",
    "K. Ando",
    "NOA Tree House",
    "NCL Thank You's",
    "K. Terasaki",
    "M. Goto",
    "H. Yamada",
    "Super Mario Club",
    "NOE Thank You's",
    "H. Uesugi",
    "Y. Edasawa",
    "Production Manager",
    "S. Farmer",
    "Special Thanks To",
    "J. Hochberg",
    "H. Yamauchi",
    "M. Arakawa",
    "H. Lincoln",
    "Producer",
    "C. Stamper",
    "",
    "Executive Producer",
    "T. Stamper",
    "",
    NULL,
};

static s16 sScript[] = {
    M059_SHOW, 1, 180,
    M059_SHOW, 4, 180,
    M059_HOLD, 0,
    M059_SHOW, 3, 180,
    M059_SHOW, 3, 180,
    M059_SHOW, 3, 180,
    M059_SHOW, 3, 180,
    M059_RELEASE, 0,
    M059_SHOW, 4, 180,
    M059_SHOW, 5, 180,
    M059_SHOW, 4, 180,
    M059_HOLD, 0,
    M059_SHOW, 3, 180,
    M059_SHOW, 3, 180,
    M059_RELEASE, 0,
    M059_SHOW, 4, 180,
    M059_HOLD, 0,
    M059_SHOW, 3, 180,
    M059_RELEASE, 0,
    M059_SHOW, 5, 180,
    M059_SHOW, 4, 180,
    M059_HOLD, 0,
    M059_SHOW, 3, 180,
    M059_SHOW, 3, 180,
    M059_SHOW, 3, 180,
    M059_SHOW, 2, 180,
    M059_RELEASE, 0,
    M059_SHOW, 5, 180,
    M059_SHOW, 3, 180,
    M059_SHOW, 2, 180,
    M059_SHOW, 5, 180,
    M059_SHOW, 3, 180,
    M059_SHOW, 3, 180,
    M059_END,
};

/* Even lines come in from the left, odd ones from the right */
static s16 sStartX[2] = { -60, 380 };
static s16 sLineY[5] = { 60, 90, 120, 150, 180 };

static M059Line sLines[5];
static s16 sWait;
static s16 *sScriptPos;
static s16 sNextText;
static s16 sFinished;
static s8 sSoundPlayed;

void func_m059_00000(void) {
    M059Line *line;
    s32 i;

    line = sLines;
    for (i = 0; i < 5; i++) {
        line->timer = 0;
        line->held = 0;
        line->x = sStartX[i & 1];
        line->y = sLineY[i];
        line->text = NULL;
        line++;
    }
    sWait = 0;
    sNextText = 0;
    sFinished = 0;
    sScriptPos = sScript;
    sSoundPlayed = 0;
}

void func_m059_00140(s32 updateRate) {
    M059Line *line;
    s32 i;
    s32 allOut;

    allOut = TRUE;
    for (i = 0, line = sLines; i < 5; i++, line++) {
        if (line->text != NULL) {
            if (line->timer > 0) {
                if (!line->held) {
                    line->timer -= updateRate;
                    allOut = FALSE;
                }
                if (i & 1) {
                    line->x -= updateRate * 8;
                    if (line->x < 160) {
                        line->x = 160;
                    }
                } else {
                    line->x += updateRate * 8;
                    if (line->x > 160) {
                        line->x = 160;
                    }
                }
            } else if (!line->held) {
                if (i & 1) {
                    line->x += updateRate * 8;
                    if (line->x > 380) {
                        line->x = 380;
                        line->text = NULL;
                    } else {
                        allOut = FALSE;
                    }
                } else {
                    line->x -= updateRate * 8;
                    if (line->x < -60) {
                        line->x = -60;
                        line->text = NULL;
                    } else {
                        allOut = FALSE;
                    }
                }
            }
        }
    }
    if (!sFinished) {
        sWait -= updateRate;
        if (sWait <= 0) {
            switch (*sScriptPos) {
                case M059_SHOW:
                    if (allOut) {
                        func_80001248(0x1C4, 0);
                        sSoundPlayed = FALSE;
                        sScriptPos++;
                        i = *sScriptPos++; /* how many texts */
                        sWait = *sScriptPos++;
                        for (line = sLines; i != 0; line++) {
                            if (line->text == NULL) {
                                i--;
                                line->text = sTexts[sNextText++];
                                line->timer = sWait;
                            }
                        }
                        while (*sScriptPos == M059_HOLD) {
                            sScriptPos++;
                            sLines[*sScriptPos++].held = TRUE;
                        }
                    } else if (!sSoundPlayed) {
                        func_80001248(0x1C4, 0);
                        sSoundPlayed = TRUE;
                    }
                    break;
                case M059_RELEASE:
                    sScriptPos++;
                    sLines[*sScriptPos].held = FALSE;
                    sLines[*sScriptPos].timer = 0;
                    sScriptPos++;
                    break;
                case M059_END:
                    sFinished = TRUE;
                    load_first_unlocked_level();
                    break;
            }
        }
    }
    if (!sFinished) {
        set_text_font(2);
        for (i = 0, line = sLines; i != 5; i++, line++) {
            if (line->text != NULL) {
                set_text_colour(0, 0, 0, 255, 255);
                draw_text(&D_800FF398, line->x, line->y, line->text, 12);
                set_text_colour(255, 255, 255, 255, 255);
                draw_text(&D_800FF398, line->x + 1, line->y + 1, line->text, 12);
            }
        }
    }
}
