/*
 * Runlink module 58: the character select hub and the multiplayer player
 * select of the front end. In the hub a teleporter is placed under the
 * player's object; in the multiplayer select each controller joins, picks a
 * character with the stick and confirms, and the game starts when everyone
 * who joined is ready.
 */
#include "module.h"

/* A selectable character */
typedef struct M058Char {
    /* 0x0 */ s8 unk0;
    /* 0x1 */ s8 unk1;
    /* 0x2 */ u8 unk2;
    /* 0x3 */ u8 unk3;
    /* 0x4 */ s32 mask; /* the bit in D_800FF318's unk64 that makes it available */
} M058Char;

/* A player's saved settings (0x76 bytes) */
typedef struct M058Player {
    /* 0x00 */ u8 pad0[4];
    /* 0x04 */ u16 unk4 : 11;
    /* 0x04 */ u16 character : 5;
    /* 0x06 */ u8 pad6[0x74 - 0x6];
    /* 0x74 */ u16 unk74 : 11;
    /* 0x74 */ u16 unk74b : 5;
} M058Player;

/* What get_settings returns */
typedef struct M058Settings {
    /* 0x000 */ u8 pad0[0x15C];
    /* 0x15C */ M058Player players[4];
} M058Settings;

/* The hub player's state (its object's unk68) */
typedef struct M058HubPlayer {
    /* 0x00 */ s32 flags;
    /* 0x04 */ Object *objects[4];
    /* 0x14 */ u8 pad14[0x51 - 0x14];
    /* 0x51 */ s8 index;
} M058HubPlayer;

/* What the objects in M058HubPlayer.objects keep in their unk68 */
typedef struct M058Ref {
    /* 0x0 */ Object *obj;
} M058Ref;

/* The level's sections and their markers (from func_8001B650) */
typedef struct M058Marker {
    /* 0x0 */ u8 type;
    /* 0x1 */ u8 pad1[7];
    /* 0x8 */ s16 index;
    /* 0xA */ u8 padA[6];
} M058Marker;
typedef struct M058Point {
    /* 0x0 */ u8 pad0[4];
    /* 0x4 */ s16 unk4;
    /* 0x6 */ u8 pad6[0xA];
} M058Point;
typedef struct M058Section {
    /* 0x00 */ u8 pad0[4];
    /* 0x04 */ M058Point *points;
    /* 0x08 */ u8 pad8[4];
    /* 0x0C */ M058Marker *markers;
    /* 0x10 */ u8 pad10[0x28 - 0x10];
    /* 0x28 */ s16 markerCount;
    /* 0x2A */ u8 pad2A[0x48 - 0x2A];
} M058Section;
typedef struct M058Level {
    /* 0x00 */ u8 pad0[4];
    /* 0x04 */ M058Section *sections;
    /* 0x08 */ u8 pad8[0x1A - 0x8];
    /* 0x1A */ s16 sectionCount;
} M058Level;

/* What the multiplayer objects keep in their unk68 */
typedef struct M058MultiData {
    /* 0x00 */ u8 pad0[0x12];
    /* 0x12 */ u8 player;
} M058MultiData;

/* What the teleporter keeps in its unk68 */
typedef struct M058Teleporter {
    /* 0x000 */ u8 pad0[0xFCA];
    /* 0xFCA */ s16 timer;
} M058Teleporter;

typedef struct M058Asset {
    /* 0x00 */ u8 pad0[0x28];
    /* 0x28 */ f32 unk28;
} M058Asset;

/* The menu items (D_800FF780, size 0x20; the main segment's MenuItem) */
typedef struct M058MenuItem {
    /* 0x00 */ s16 unk0;
    /* 0x02 */ s16 unk2;
    /* 0x04 */ s16 unk4;
    /* 0x06 */ s16 unk6;
    /* 0x08 */ f32 unk8;
    /* 0x0C */ f32 unkC;
    /* 0x10 */ f32 unk10;
    /* 0x14 */ f32 unk14;
    /* 0x18 */ f32 unk18;
    /* 0x1C */ s8 unk1C;
    /* 0x1D */ s8 unk1D;
    /* 0x1E */ s8 unk1E;
    /* 0x1F */ s8 unk1F;
} M058MenuItem;

typedef struct M058Unlocks {
    /* 0x00 */ u8 pad0[0x64];
    /* 0x64 */ s32 characters;
} M058Unlocks;

extern M058Asset *D_800FF3F8[];
extern M058MenuItem D_800FF780[];
extern s8 D_800FF311;
extern M058Unlocks D_800FF318;
extern s8 D_800FF1B0[4]; /* per player: 0 out, 1 joined, 2 ready */
extern s8 D_800FF1B4[4]; /* per player: the character picked */
extern s32 D_800FF3C8[];  /* the buttons pressed this frame, per player */
extern s8 D_800FF3D8[];   /* the stick moves this frame, per player */
extern u8 D_800A4FCC;
extern u8 D_800A4FD0; /* how many players joined */
extern s8 D_800A5058;
extern u8 D_800A505C;
extern M058Player D_800FEFD8[];
extern u8 D_800FEFD0[];

extern void func_8003DDBC(s32 arg0);
extern void menu_assetgroup_load(s16 *assets); /* loads menu textures */
extern void menu_assetgroup_reset(s16 *assets);
extern void menu_assetgroup_free(s16 *assets); /* frees menu textures */
extern Object *func_80005F14(void *entry, s32 arg1); /* spawns an object from a level entry */
extern s32 fb_mode(void);
extern Object *func_m135_0205C(void);
extern s32 game_flag_get(s32 flag);
extern void func_m041_008B4(s32 arg0, s32 arg1, s32 updateRate);
extern s32 func_80043510(s32 player); /* the buttons held */
extern void music_volume_fade(f32 volume, s32 arg1); /* fades the music */
extern void load_first_unlocked_level(void);
extern s32 transition_begin(s16 *arg0);
extern M058Settings *get_settings(void);
extern void func_80047580(s32 arg0, s32 player);
extern s32 leveltable_region(s32 arg0);
extern void func_80046334(u32 *a, s32 *b, s32 *c);
extern void level_goto(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5);
extern M058Level *func_8001B650(void);
extern void set_active_camera(s32 camera); /* sets the active camera */
extern ObjectTransform *func_80041DE0(void);
extern void input_assign_players(void);
extern s32 screenimage_load(s32 image); /* loads a full-screen image */
extern void music_play(s32 seq);
extern void func_8004797C(void);
extern void func_8004AA80(s32 data); /* frees */
extern void func_80047AE0(M058Player *p, s32 arg1);
extern void func_80043430(s8 *players);
extern void func_m135_0270C(Object *obj, s32 state, s32 character, s32 dir, s32 arg4);

static M058Char sChars[] = {
    { -1, -1, 0, 0, 0 },      { -1, -1, 0, 6, 0 },     { 0, 0, 0, 8, 0x1 },     { 1, 0, 0, 10, 0x2 },
    { 2, 0, 0, 12, 0x4 },     { 4, 0, 4, 0, 0x10 },    { 5, 0, 4, 2, 0x20 },    { 6, 0, 4, 4, 0x40 },
    { 0, 1, 4, 6, 0x100 },    { 1, 1, 4, 8, 0x200 },   { 1, 3, 4, 10, 0x400 },  { 0, 3, 4, 12, 0x800 },
    { 0, 2, 8, 0, 0x1000 },   { 1, 2, 8, 2, 0x2000 },  { 1, 4, 8, 4, 0x4000 },  { 1, 5, 8, 6, 0x8000 },
    { 1, 6, 8, 8, 0x10000 },  { 1, 7, 8, 10, 0x20000 }, { 1, 8, 8, 12, 0x40000 },
};
#define CHAR_COUNT 19
static s16 sList10E8[] = { 0, 1, 2, -1 };
static s16 sList10F0[] = { 3, 4, 5, -1 };
static s16 sList10F8[] = { 0, 0, 0x48, -1 };
static s16 sAssets[] = { 0x1E, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, -1 };
static s16 sImages[] = { 0xB, -1 };
/* The level entries of the two markers over the teleporter, and of the teleporter */
static s16 sMarkerEntry[] = { 0xDB, 0x1800, 0, 0, 0, 0, 0, 0xC8, 0, 0, 0, 0 };
static s8 sHubChars[] = { 1, 0, 2, 0 };
static s16 sTeleporterEntry[] = {
    0x141, 0x2800, 0, 0, 0, -0x118, -0x222, 0x33F, 0x168, 0x2FA,
    0x70D, 0xAD9, 0x5EA, 0x28, 0x5078, 0x2627, 0x110F, 0x3C, 100, 100,
};
static u32 sColours[] = {
    0x000040FF, 0x400000FF, 0x404000FF, 0x004000FF, 0x4040F0FF, 0xF04040FF,
    0xF0F040FF, 0x40F040FF, 0xC0C0FFFF, 0xFFC0C0FF, 0xFFFFC0FF, 0xC0FFC0FF,
};
static char *sNames[] = {
    "PLAYER 1", "PLAYER 2", "PLAYER 3", "PLAYER 4", "VELA", "JUNO", "LUPUS", "LOAD", "LOADED",
};
static s32 sImage = 0;

static Object *sHub;
static M058HubPlayer *sHubPlayer;
static M058Point *sExit;
static s16 sFadeTimer;
static s16 sWaitTimer;
static u8 sExitOpen;
static u8 sLeaving;
static s32 sLastFlags;
static Object *sTeleporter;
static s32 sUnk1C;
static Object *sMarkers[2];
static s32 sMarkerTime;
static s32 sTime;
static Object *sPlayerObjects[4];

void func_m058_00000(void) {
    M058Asset *asset;
    M058MenuItem *item = &D_800FF780[11]; /* (set before the calls: each access gets its own %hi/%lo) */

    func_8003DDBC(0);
    menu_assetgroup_load(sAssets);
    func_8003DDBC(8);
    menu_assetgroup_reset(sImages);
    sExit = NULL;
    sExitOpen = TRUE;
    sHub = NULL;
    sHubPlayer = NULL;
    sMarkerTime = 40;
    sWaitTimer = 480;
    sLeaving = FALSE;
    sLastFlags = 0;
    sMarkers[0] = func_80005F14(sMarkerEntry, 1);
    sMarkers[1] = func_80005F14(sMarkerEntry, 1);
    sMarkers[0]->trans.flags |= 0x400;
    sMarkers[1]->trans.flags |= 0x400;
    asset = D_800FF3F8[item->unk6];
    asset->unk28 = 1.0f;
    if (fb_mode() & 1) {
        item->unk10 += 20.0f;
    }
    sTime = 0;
}

void func_m058_00134(s32 updateRate) {
    M058Level *level;
    M058Section *sections;
    M058Marker *markers;
    s32 i;
    s32 j;
    s32 character;
    M058Settings *settings;
    M058Player *player;
    u32 a;
    s32 b;
    s32 c;

    if (sHub == NULL) {
        sHub = func_m135_0205C();
    }
    if (sHub == NULL) {
        return;
    }
    sHubPlayer = sHub->unk68;
    if (sHubPlayer == NULL) {
        return;
    }
    if (sTime < 30) {
        sTime += updateRate;
    }
    if (!game_flag_get(1) || game_flag_get(0x5B)) {
        func_8007688C(0);
    }
    if (!game_flag_get(0) || game_flag_get(0x5A)) {
        func_8007688C(1);
    }
    if (!game_flag_get(2) || game_flag_get(0x5C)) {
        func_8007688C(2);
    }
    func_m041_008B4(0, 0, updateRate);
    if (!(sLastFlags & 2) && (func_80043510(0) & B_BUTTON) && D_800FF311 && sTime >= 30) {
        music_volume_fade(0.75f, 0);
        load_first_unlocked_level();
    }
    if (sHubPlayer->flags & 4) {
        if ((func_80043510(0) & A_BUTTON) && sWaitTimer != 480) {
            sWaitTimer = 0;
        } else {
            sWaitTimer -= updateRate;
        }
        if (sWaitTimer <= 0) {
            if (!sLeaving) {
                transition_begin(sList10F8);
                sLeaving = TRUE;
                sFadeTimer = 80;
                music_volume_fade(1.2f, 0);
            } else {
                sFadeTimer -= updateRate;
                if (sFadeTimer <= 0) {
                    settings = get_settings();
                    character = sHubChars[sHubPlayer->index];
                    player = &settings->players[character];
                    func_80047580(0, character);
                    if (leveltable_region(player->unk74)) {
                        player->character = 0;
                    }
                    a = player->unk74;
                    b = player->unk74b;
                    c = player->character;
                    func_80046334(&a, &b, &c);
                    player->unk74 = a;
                    player->unk74b = b;
                    player->character = c;
                    level_goto(player->unk74, character, player->unk74b, 16, 1, 0);
                }
            }
        }
    }
    if (sExit == NULL) {
        level = func_8001B650();
        sections = level->sections;
        for (i = 0; i < level->sectionCount; i++) {
            markers = sections[i].markers;
            for (j = 0; j < sections[i].markerCount; j++) {
                if (markers[j].type == 0x31) {
                    sExit = &sections[i].points[markers[j].index];
                }
            }
        }
    } else if (((sExit->unk4 >> 5) % 32) >= 2) {
        sExitOpen = TRUE;
    } else if (sExitOpen) {
        if (sHubPlayer != NULL && (!(sHubPlayer->flags & 2) || sHubPlayer->index == 0)) {
            func_80001248(0xD0, 0);
        }
        sExitOpen = FALSE;
    }
    sLastFlags = sHubPlayer->flags;
}

void func_m058_00618(s32 updateRate) {
    Object *obj;
    M058Teleporter *tp;
    ObjectTransform *cam;
    M058Ref *ref;

    if (sHubPlayer->flags & 2) {
        obj = sHubPlayer->objects[sHubPlayer->index];
        ref = obj->unk68;
        obj = ref->obj;
        if (sTeleporter == NULL) {
            if (sHubPlayer->index == 2) {
                sTeleporterEntry[18] = 0xA5;
                sTeleporterEntry[19] = 0x50;
            } else {
                sTeleporterEntry[18] = 100;
                sTeleporterEntry[19] = 100;
            }
            sTeleporterEntry[2] = obj->trans.x_position;
            sTeleporterEntry[3] = obj->trans.y_position;
            sTeleporterEntry[4] = obj->trans.z_position;
            sTeleporter = func_80005F14(sTeleporterEntry, 1);
        } else if ((tp = sTeleporter->unk68)->timer >= 0x137 && sMarkerTime > 0) {
            sMarkers[0]->trans.flags &= ~0x400;
            sMarkers[1]->trans.flags &= ~0x400;
            set_active_camera(0);
            cam = func_80041DE0();
            sMarkers[0]->trans.x_position = obj->trans.x_position;
            sMarkers[0]->trans.y_position = obj->trans.y_position + 35.0f;
            sMarkers[0]->trans.z_position = obj->trans.z_position;
            sMarkers[0]->trans.rotation.y_rotation = 0xC000 - cam->rotation.y_rotation;
            sMarkers[1]->trans.x_position = obj->trans.x_position;
            sMarkers[1]->trans.y_position = obj->trans.y_position + 35.0f;
            sMarkers[1]->trans.z_position = obj->trans.z_position;
            sMarkers[1]->trans.rotation.y_rotation = 0x4000 - cam->rotation.y_rotation;
            sMarkerTime -= updateRate;
        } else {
            sMarkers[0]->trans.flags |= 0x400;
            sMarkers[1]->trans.flags |= 0x400;
        }
    }
}

void func_m058_00894(void) {
    menu_assetgroup_free(sAssets);
    if (sTeleporter != NULL) {
        func_800079B8(sTeleporter);
        sTeleporter = NULL;
    }
    sHub = NULL;
    func_800079B8(sMarkers[0]);
    func_800079B8(sMarkers[1]);
}

void func_m058_00900(void) {
    s32 i;

    input_assign_players();
    D_800A4FCC = 1;
    sImage = screenimage_load(0);
    music_play(0xE);
    for (i = 0; i < 4; i++) {
        if (D_800FF1B0[i]) {
            if (D_800FF1B4[i] == 0) {
                D_800FF1B0[i] = 2;
            } else {
                D_800FF1B0[i] = 1;
            }
        }
        sPlayerObjects[i] = NULL;
    }
    func_8004797C();
    sTime = 0;
}

#ifdef NON_MATCHING
static void func_m058_00F5C(s32 left, s32 right, s8 *character, s32 arg3);

/* Same logic; IDO's induction pointers for the per-player arrays and the frame differ */
void func_m058_009B8(s32 updateRate) {
    s32 i;
    s32 j;
    s32 n;
    s32 buttons;
    s32 stick;
    s32 start;
    s32 back;
    s32 first;
    s32 count;
    s32 left;
    s32 right;
    s32 joined;
    s32 dir;
    Object **objs;
    s8 *p;
    s8 *picked;
    M058Player *player;
    Object *found;
    Object *obj;

    if (sTime < 30) {
        sTime += updateRate;
    }
    n = 0;
    if (sPlayerObjects[0] == NULL) {
        objs = func_80005E98(&first, &count);
        for (j = first; j < count; j++) {
            if (objs[j]->behaviorId == 0x5C) {
                sPlayerObjects[n++] = objs[j];
            }
        }
    }
    if (D_800A5058 != 0) {
        return;
    }
    if (D_800A505C != 0) {
        return;
    }
    for (i = 0; i < 4; i++) {
        buttons = D_800FF3C8[i];
        stick = D_800FF3D8[i];
        joined = FALSE;
        left = stick < 0 || (buttons & L_TRIG);
        n = 0;
        start = buttons & (A_BUTTON | START_BUTTON);
        back = buttons & B_BUTTON;
        right = stick > 0 || (buttons & R_TRIG);
        switch (D_800FF1B0[i]) {
            case 0:
                if (start) {
                    D_800FF1B0[i] = 1;
                    D_800A4FD0++;
                }
                break;
            case 1:
                if (start) {
                    D_800FF1B0[i] = 2;
                } else if (back && sTime >= 30) {
                    D_800FF1B0[i] = 0;
                    if (--D_800A4FD0 <= 0) {
                        D_800A5058 = 1;
                        music_volume_fade(0.5f, 0);
                        load_first_unlocked_level();
                    }
                } else {
                    func_m058_00F5C(left, right, &D_800FF1B4[i], 0);
                }
                break;
            case 2:
                if (start && D_800A4FD0 >= 2) {
                    for (p = D_800FF1B0; p < &D_800FF1B0[4]; p++) {
                        if (*p == 1) {
                            start = 0;
                        }
                    }
                    if (start) {
                        D_800A5058 = 1;
                        music_volume_fade(0.5f, 0);
                        for (j = 0, p = D_800FF1B0; j < 4; j++, p++) {
                            if (*p) {
                                picked = &D_800FF1B4[j];
                                if (*picked != 1 && *picked != 0) {
                                    player = &D_800FEFD8[n];
                                    func_80047AE0(player, 0);
                                    player->character = sChars[*picked].unk1;
                                    D_800FEFD0[n] = sChars[*picked].unk0;
                                }
                                n++;
                            }
                        }
                        n = 0;
                        func_80043430(D_800FF1B0);
                        level_goto(0, 0, 0, 8, 1, 0);
                    }
                } else if (back) {
                    if (D_800FF1B4[i] == 0) {
                        D_800FF1B4[i] = 1;
                        joined = TRUE;
                    }
                    D_800FF1B0[i] = 1;
                }
                break;
        }
        for (; n < 4; n++) {
            obj = sPlayerObjects[n];
            found = NULL;
            if (obj->unk68 != NULL && ((M058MultiData *) obj->unk68)->player == i) {
                found = obj;
                n = 4;
            }
            if (found != NULL) {
                if (D_800FF3D8[i] < 0 || (D_800FF3C8[i] & L_TRIG)) {
                    dir = 1;
                } else if (D_800FF3D8[i] > 0 || (D_800FF3C8[i] & R_TRIG) || joined) {
                    dir = 2;
                } else {
                    dir = 0;
                }
                if (D_800FF1B0[i] == 2) {
                    dir = 0;
                }
                if (D_800FF1B4[i] == 0) {
                    func_m135_0270C(found, D_800FF1B0[i], D_800FEFD0[i], dir, 1);
                } else {
                    func_m135_0270C(found, D_800FF1B0[i], D_800FF1B4[i], dir, 0);
                }
            }
        }
    }
}

/* Moves a player's character choice left or right to the next available one */
static void func_m058_00F5C(s32 left, s32 right, s8 *character, s32 arg3) {
    s32 found;
    s32 c;

    c = *character;
    found = FALSE;
    if (right) {
        do {
            c++;
            if (c >= CHAR_COUNT) {
                c = 1;
            } else if (sChars[c].mask & D_800FF318.characters) {
                found = TRUE;
            }
        } while (!found);
    } else if (left) {
        do {
            c--;
            if (c < 2) {
                c = CHAR_COUNT;
            } else if (sChars[c].mask & D_800FF318.characters) {
                found = TRUE;
            }
        } while (!found);
    }
    *character = c;
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m058", func_m058_009B8);

INCLUDE_ASM("asm/nonmatchings/modules/m058", func_m058_00F5C);
#endif

void func_m058_01020(void) {
    func_8004AA80(sImage);
}
