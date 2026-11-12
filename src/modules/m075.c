/*
 * Runlink module 75: a coin-op game cabinet in the levels. When the player
 * stands in front of it and presses A, the camera swings round to face its
 * screen and the screen opens. Paying a token from the character's stock
 * gives a credit; START then launches the cabinet's game (a level). The
 * screen shows the game's best time and, every so often, the five fastest
 * times with initials; a cabinet with no game shows a test card and "OUT OF
 * ORDER". B (or Z) walks away.
 */
#include "module.h"
#include "gfx.h"

/* The cabinet's level entry */
typedef struct M075Setup {
    /* 0x00 */ LevelObjectEntryCommon common;
    /* 0x0A */ u8 radius;   /* of the area in front of it */
    /* 0x0B */ s8 minY;     /* of the player's height, from the cabinet's */
    /* 0x0C */ s8 maxY;
    /* 0x0D */ u8 camDist;  /* the camera's, in fours */
    /* 0x0E */ s8 camHeight;
    /* 0x0F */ s8 camYaw;   /* in 1/256ths of a turn */
    /* 0x10 */ s8 lookX;    /* the point the camera looks at, in fours from the cabinet */
    /* 0x11 */ s8 lookY;
    /* 0x12 */ s8 lookZ;
    /* 0x13 */ u8 pad13;
    /* 0x14 */ s16 game;    /* the level it launches; 0: out of order */
    /* 0x16 */ u8 yaw;
    /* 0x17 */ u8 gameArg;
} M075Setup;

/* obj->unk68 */
typedef struct M075State {
    /* 0x00 */ f32 radiusSq;
    /* 0x04 */ f32 minY;
    /* 0x08 */ f32 maxY;
    /* 0x0C */ f32 camDist;
    /* 0x10 */ f32 camHeight;
    /* 0x14 */ s32 camYaw;
    /* 0x18 */ f32 lookX;
    /* 0x1C */ f32 lookY;
    /* 0x20 */ f32 lookZ;
    /* 0x24 */ s16 game;
    /* 0x26 */ s16 timer;
    /* 0x28 */ u8 credit;
    /* 0x29 */ u8 gameArg;
    /* 0x2A */ u8 open;    /* how far the screen is open, 0 to 0xC0 */
    /* 0x2B */ u8 started;
    /* 0x2C */ Object *player; /* the one using it */
} M075State;

/* What the player object's unk68 holds here */
typedef struct M075Racer {
    /* 0x000 */ s8 player;
    /* 0x001 */ s8 character;
    /* 0x002 */ u8 pad2[0x198 - 0x2];
    /* 0x198 */ u8 cabinetMode; /* 1 using a cabinet, 2 leaving it */
    /* 0x199 */ u8 pad199[0x568 - 0x199];
    /* 0x568 */ u8 unk568;
    /* 0x569 */ u8 pad569[0x5C8 - 0x569];
    /* 0x5C8 */ s16 unk5C8;
} M075Racer;

/* A character's record (func_80047C1C) */
typedef struct M075Character {
    /* 0x00 */ u8 pad0[0x12];
    /* 0x12 */ s16 tokens;
} M075Character;

/* A best-times entry: the time in hundredths, and three initials (indices into sLetters) */
typedef struct M075Record {
    u32 time : 14;
    u32 initial0 : 6;
    u32 initial1 : 6;
    u32 initial2 : 6;
} M075Record;

/* What obj->header points at here (the screen's sprite at 0xAC) */
typedef struct M075Header {
    /* 0x00 */ u8 pad0[0xAC];
    /* 0x00 */ s32 *sprite;
} M075Header;

/* A sprite to draw: the first fields of an Object */
typedef struct M075Billboard {
    /* 0x00 */ ObjectTransform trans;
    /* 0x18 */ s16 animFrame;
    /* 0x1A */ s16 unk1A;
    /* 0x1C */ Vec3f velocity;
    /* 0x28 */ f32 unk28;
} M075Billboard;

typedef struct MenuBox {
    s16 x1, y1, x2, y2;
    s32 colour;
} MenuBox;

extern Object *func_80041DE0(void); /* the active camera object */
extern f32 sqrtf(f32);
extern s32 func_80049C84(f32 x, f32 z); /* arctangent as a 16-bit angle */
extern f32 func_80049BEC(f32 rate, s32 updateRate);
extern s32 func_80049D80(s32 from, s32 to); /* the difference between two angles */
extern s32 character_has_item(s32 player, s32 character, s32 item); /* whether the character has the item */
extern s32 func_80043510(s32 player);                      /* the buttons pressed */
extern M075Character *func_80047C1C(s32 player, s32 character);
extern void func_80001170(s32 arg0);
extern void func_80048518(s32 arg0);
extern u8 menu_mode_get(void);
extern void level_goto(s32 arg0, s32 level, s32 arg2, s32 arg3, s32 arg4, s32 arg5);
extern Object *D_800A32B8; /* the cabinet whose screen is open */
extern u8 D_800A4FBC;
extern u32 D_800FF318[];
extern void menu_fill_boxes(Gfx **dl, s32 count, MenuBox *boxes, s32 arg3);
extern void func_8006FC54(s32 font);
extern void func_8006FC8C(s32 r, s32 g, s32 b, s32 a);
extern void func_8006FC68(s32 r, s32 g, s32 b, s32 a, s32 opacity);
extern void func_8006FCA8(Gfx **dl, s32 x, s32 y, char *text, s32 flags);
extern void func_80040D64(Gfx **dl, Mtx **mtx);
extern void func_80056000(s32 arg0);
extern void render_ortho_triangle_image(Gfx **dl, Mtx **mtx, Vertex **verts, M075Billboard *board, s32 sprite, s32 flags,
                          s32 alpha);
extern void viewport_apply(Gfx **dl, Mtx **mtx);
extern void func_80055B80(Gfx **dl);

/* The lights round the screen of the cabinet that launches level 0x18D; their colours run round */
static MenuBox sLights[20] = {
    { 0x98, 0x80, 0xA0, 0x82, 0xFF0000FF }, { 0xA0, 0x80, 0xA8, 0x82, 0xFF0000FF },
    { 0x98, 0x82, 0xA0, 0x84, 0xFF8000FF }, { 0xA0, 0x82, 0xA8, 0x84, 0xFF8000FF },
    { 0x9A, 0x84, 0x9D, 0x86, 0xFFFF00FF }, { 0xA3, 0x84, 0xA6, 0x86, 0xFFFF00FF },
    { 0x9A, 0x86, 0x9D, 0x88, 0x80FF00FF }, { 0xA3, 0x86, 0xA6, 0x88, 0x80FF00FF },
    { 0x9A, 0x88, 0x9D, 0x8A, 0x00FF00FF }, { 0xA3, 0x88, 0xA6, 0x8A, 0x00FF00FF },
    { 0x9A, 0x8A, 0x9D, 0x8C, 0x0080FFFF }, { 0xA3, 0x8A, 0xA6, 0x8C, 0x0080FFFF },
    { 0x9A, 0x8C, 0x9D, 0x8E, 0x0000FFFF }, { 0xA3, 0x8C, 0xA6, 0x8E, 0x0000FFFF },
    { 0x9A, 0x8E, 0x9D, 0x90, 0x8000FFFF }, { 0xA3, 0x8E, 0xA6, 0x90, 0x8000FFFF },
    { 0x98, 0x90, 0xA0, 0x92, 0xFF00FFFF }, { 0xA0, 0x90, 0xA8, 0x92, 0xFF00FFFF },
    { 0x98, 0x92, 0xA0, 0x94, 0xFF0080FF }, { 0xA0, 0x92, 0xA8, 0x94, 0xFF0080FF },
};

/* The test card's set-up: sync, no z-buffer or fog, a primitive-colour combine, fill mode */
static u32 sTestCardDL[12] = {
    0xE7000000, 0x00000000, /* gsDPPipeSync */
    0xB6000000, 0x00010001, /* gsSPClearGeometryMode */
    0xFC71FEE3, 0xFFFFF7FB, /* gsDPSetCombineLERP */
    0xEF000C0F, 0x00504240, /* gsDPSetOtherMode */
    0xE7000000, 0x00000000, /* gsDPPipeSync */
    0xB8000000, 0x00000000, /* gsSPEndDisplayList */
};

static char sLetters[64] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789.-!? ***********************";

/* The camera at the cabinet */
static f32 sLookX;
static f32 sLookY;
static f32 sLookZ;
static f32 sCentreX;
static f32 sCentreY;
static f32 sCentreZ;
static f32 sCamDist;
static f32 sCamHeight;
static s16 sCamYaw;
static f32 sTargetDist;
static f32 sTargetHeight;
static s16 sTargetYaw;

/* Starts the camera's swing from where it is to the cabinet's view */
void func_m075_00000(Object *obj) {
    f32 dx;
    f32 dy;
    f32 dz;
    M075State *state = obj->unk68;
    Object *camera;

    camera = func_80041DE0();
    sCentreX = obj->trans.x_position;
    sCentreY = obj->trans.y_position;
    sCentreZ = obj->trans.z_position;
    sLookX = state->lookX;
    sLookY = state->lookY;
    sLookZ = state->lookZ;
    dx = camera->trans.x_position - sCentreX;
    dy = camera->trans.y_position - sCentreY;
    dz = camera->trans.z_position - sCentreZ;
    sCamDist = sqrtf(dx * dx + dz * dz);
    sCamHeight = dy;
    sCamYaw = 0x8000 - func_80049C84(dx, dz);
    sTargetDist = state->camDist;
    sTargetHeight = state->camHeight;
    sTargetYaw = state->camYaw;
}

/* The camera while the cabinet is in use */
void func_m075_00114(Object *camera, s32 updateRate) {
    f32 t;
    s32 turn;
    s32 i;
    f32 dx;
    f32 dy;
    f32 dz;
    s16 yaw;
    s16 pitch;

    t = 1.0f - func_80049BEC(0.975f, updateRate);
    sCamDist += (sTargetDist - sCamDist) * t;
    sCamHeight += (sTargetHeight - sCamHeight) * t;
    i = updateRate;
    while (i--) {
        turn = func_80049D80(sCamYaw, sTargetYaw) >> 4;
        if (turn < -0x200) {
            sCamYaw -= 0x200;
        } else if (turn > 0x200) {
            sCamYaw += 0x200;
        } else {
            sCamYaw += turn;
        }
    }
    camera->trans.x_position = func_80049C34(sCamYaw) * sCamDist + sCentreX;
    camera->trans.y_position = sCentreY + sCamHeight;
    camera->trans.z_position = sCentreZ - func_80049C30(sCamYaw) * sCamDist;
    dx = camera->trans.x_position - sLookX;
    dy = camera->trans.y_position - sLookY;
    dz = camera->trans.z_position - sLookZ;
    yaw = 0x8000 - func_80049C84(dx, dz);
    pitch = func_80049C84(dy, sqrtf(dx * dx + dz * dz));
    i = updateRate;
    while (i--) {
        camera->trans.rotation.y_rotation += func_80049D80(camera->trans.rotation.y_rotation, yaw) >> 4;
        camera->trans.rotation.x_rotation += func_80049D80(camera->trans.rotation.x_rotation, pitch) >> 4;
    }
}

void func_m075_0038C(Object *obj, M075Setup *setup) {
    M075State *state = obj->unk68;

    obj->trans.rotation.y_rotation = setup->yaw << 8;
    state->radiusSq = setup->radius * setup->radius;
    state->minY = obj->trans.y_position + setup->minY;
    state->maxY = obj->trans.y_position + setup->maxY;
    state->camDist = setup->camDist * 4;
    state->camHeight = setup->camHeight * 4;
    state->camYaw = setup->camYaw << 8;
    state->lookX = obj->trans.x_position + setup->lookX * 4;
    state->lookY = obj->trans.y_position + setup->lookY * 4;
    state->lookZ = obj->trans.z_position + setup->lookZ * 4;
    state->game = setup->game;
    state->credit = 0;
    state->gameArg = setup->gameArg;
    state->open = 0;
    state->started = 0;
    state->player = NULL;
    if (state->game == 0x18D && character_has_item(0, 0, 0x1B) == 0) {
        state->game = 0;
    }
}

/*
 * NON_MATCHING: the original keeps state->player in a1 and loads updateRate
 * into v1 after the first if; here they land in v0 (and IDO hoists the load
 * into the branches' delay slots). The frame needs four more words than the
 * named locals (pad).
 */
#ifdef NON_MATCHING
void func_m075_004FC(Object *obj, s32 updateRate) {
    s32 pad[4];
    M075State *state = obj->unk68;
    M075Racer *racer;
    Object *player;
    M075Character *chr;
    s32 buttons;
    s16 diff;
    f32 dx;
    f32 dz;
    f32 y;

    player = func_8000E4D0(0);
    if (player != NULL) {
        racer = player->unk68;
        buttons = func_80043510(0);
        if (state->started) {
            buttons = 0;
        }
        state->timer += updateRate;
        if (state->player == NULL) {
            if ((buttons & 0x8000) && racer->unk568 == 0 && player->interactObj->hitPoints >= 0x100) {
                diff = obj->trans.rotation.y_rotation - player->trans.rotation.y_rotation;
                if (diff > -0x2000 && diff < 0x2000) {
                    dx = player->trans.x_position - obj->trans.x_position;
                    dz = player->trans.z_position - obj->trans.z_position;
                    if (dx * dx + dz * dz < state->radiusSq) {
                        y = player->trans.y_position;
                        if (state->minY <= y && y <= state->maxY) {
                            racer->cabinetMode = 1;
                            state->player = player;
                            state->timer = 0;
                            func_m075_00000(obj);
                            func_80048518(10);
                        }
                    }
                }
            }
            if (updateRate * 4 < state->open) {
                state->open -= updateRate * 4;
            } else {
                state->open = 0;
            }
        } else {
            racer = state->player->unk68;
            if (state->open > 0x80) {
                if (state->game == 0) {
                    if (buttons & 0xC000) {
                        racer->cabinetMode = 2;
                        state->player = NULL;
                    }
                } else if (buttons & 0x4000) {
                    racer->cabinetMode = 2;
                    state->player = NULL;
                } else if (!state->credit && (buttons & 0x8000)) {
                    chr = func_80047C1C(racer->player, racer->character);
                    if (chr->tokens > 0) {
                        func_80001170(13);
                        chr->tokens--;
                        state->credit = 1;
                    } else {
                        func_80001248(0x16E, 0);
                    }
                } else if (state->credit && (buttons & 0x9000)) {
                    func_80001248(0x171, 0);
                    state->started = 1;
                    D_800A4FBC = state->gameArg;
                    level_goto(state->game, 3, 0, menu_mode_get(), 1, 0);
                }
            }
            state->open += updateRate;
            if (state->open > 0xC0) {
                state->open = 0xC0;
            }
        }
        if (state->player != NULL) {
            racer = state->player->unk68;
            if (racer->unk5C8 < 10) {
                racer->unk5C8 = 10;
            }
        }
        if (state->open <= 0x40) {
            if (obj == D_800A32B8) {
                D_800A32B8 = NULL;
            }
        } else {
            D_800A32B8 = obj;
        }
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m075", func_m075_004FC);
#endif

/* Draws the lights round the screen, their colours moved on two places */
static void draw_lights(Gfx **dl, s32 alpha) {
    s32 colour0;
    s32 colour1;
    s32 i;

    i = 0;
    while (i < 20) {
        sLights[i].colour = (sLights[i].colour & ~0xFF) | (alpha & 0xFF);
        i++;
    }
    colour0 = sLights[0].colour;
    colour1 = sLights[1].colour;
    i = 2;
    while (i < 20) {
        sLights[i - 2].colour = sLights[i].colour;
        i++;
    }
    sLights[18].colour = colour0;
    sLights[19].colour = colour1;
    menu_fill_boxes(dl, 20, sLights, 0);
}

static void draw_time(Gfx **dl, s32 x, s32 y, s32 time);
static void draw_records(Gfx **dl, M075Record *records, s32 alpha);
static M075Record *get_records(s32 game);

void func_m075_00900(Gfx **dl, Mtx **mtx, Vertex **verts, Object *obj) {
    s32 size;
    s32 alpha;
    char line1[0x40];
    char line2[0x40];
    char line3[0x40];
    MenuBox box;
    M075Billboard board;
    s32 sprite;
    M075Record *records;
    M075State *state;
    s32 *spritePtr;

    func_80048518(10);
    state = obj->unk68;
    if (state->open > 0x40) {
        func_8006FC8C(0, 0, 0, 0);
        size = (state->open - 0x40) << 2;
        if (size > 0x100) {
            size = 0x100;
        }
        box.x1 = 160 - ((size * 120) >> 8);
        box.y1 = 120 - ((size * 90) >> 8);
        box.x2 = ((size * 120) >> 8) + 160;
        box.y2 = ((size * 90) >> 8) + 120;
        box.colour = 0xE0;
        menu_fill_boxes(dl, 1, &box, 0);
        alpha = (state->open - 0x80) << 2;
        if (alpha > 0) {
            if (alpha >= 0x100) {
                alpha = 0xFF;
            }
            records = get_records(state->game);
            if (state->game != 0 && !state->credit && (state->timer & 0x200)) {
                draw_records(dl, records, alpha);
            } else if (obj->header != NULL && (spritePtr = ((M075Header *) obj->header)->sprite) != NULL) {
                if ((sprite = *spritePtr) != 0) {
                    board.trans.rotation.y_rotation = 0;
                    board.trans.rotation.x_rotation = 0;
                    board.trans.rotation.z_rotation = 0;
                    board.trans.scale = 1.0f;
                    board.trans.x_position = 0.0f;
                    board.trans.y_position = 0.0f;
                    board.trans.z_position = 1.0f;
                    board.unk28 = 0;
                    func_80040D64(dl, mtx);
                    func_80056000(0);
                    gDPSetPrimColor((*dl)++, 0, 0, 255, 255, 255, (alpha * 224) >> 8);
                    gDPSetEnvColor((*dl)++, 255, 255, 255, 255);
                    render_ortho_triangle_image(dl, mtx, verts, &board, sprite, 4, alpha);
                    func_80056000(1);
                    viewport_apply(dl, mtx);
                    gDPSetPrimColor((*dl)++, 0, 0, 255, 255, 255, 255);
                }
                if (state->game == 0x18D) {
                    draw_lights(dl, alpha);
                }
            }
            if (state->credit) {
                sprintf(line1, "Push START button");
            } else {
                sprintf(line1, "Insert a token to play");
            }
            sprintf(line2, "Credits %d", state->credit);
            sprintf(line3, "A JETFORCE SYSTEMS ARCADE GAME");
            func_8006FC54(2);
            func_8006FC68(32, 255, 32, 255, alpha);
            func_8006FCA8(dl, 160, 40, "Best Time: ", 1);
            func_8006FC68(255, 255, 255, 255, alpha);
            draw_time(dl, 160, 40, records->time);
            if (state->game != 0 && ((state->timer & 0x20) || state->credit)) {
                func_8006FC68(255, 255, 64, 255, alpha);
                func_8006FCA8(dl, 160, 158, line1, 4);
            }
            func_8006FC68(255, 255, 255, 255, alpha);
            func_8006FCA8(dl, 160, 178, line2, 4);
            func_8006FC68(255, 128, 32, 255, alpha);
            func_8006FCA8(dl, 160, 194, line3, 4);
        }
        if (state->game == 0) {
            gDPSetPrimColor((*dl)++, 0, 0, 0x80, 0x80, 0x80, 0xA0);
            gDPSetEnvColor((*dl)++, 255, 255, 255, 255);
            gSPDisplayList((*dl)++, sTestCardDL);
            gDPFillRectangle((*dl)++, box.x1, box.y1, box.x2, box.y2);
            func_80055B80(dl);
            alpha = (state->open - 0x80) << 2;
            if (alpha > 0) {
                if (alpha >= 0x100) {
                    alpha = 0xFF;
                }
                func_8006FC54(2);
                func_8006FC68(0, 0, 0, 255, alpha);
                func_8006FCA8(dl, 161, 159, "OUT OF ORDER", 12);
                func_8006FC68(255, 224, 16, 255, alpha);
                func_8006FCA8(dl, 160, 158, "OUT OF ORDER", 12);
            }
            gDPSetPrimColor((*dl)++, 0, 0, 255, 255, 255, 255);
        }
    }
}

/* The best-times table of a game */
static M075Record *get_records(s32 game) {
    if (game == 0x102) {
        return (M075Record *) &D_800FF318[5];
    }
    if (game == 0x18D) {
        return (M075Record *) &D_800FF318[10];
    }
    return (M075Record *) &D_800FF318[20];
}

/* Draws a time in hundredths as m'ss"cc, one digit at a time */
static void draw_time(Gfx **dl, s32 x, s32 y, s32 time) {
    s32 minutes;
    s32 tenSeconds;
    s32 seconds;
    s32 tenths;
    s32 hundredths;
    char digit[8];

    minutes = time / 60000;
    time -= minutes * 60000;
    tenSeconds = time / 6000;
    time -= tenSeconds * 6000;
    seconds = time / 1000;
    time -= seconds * 1000;
    tenths = time / 100;
    time -= tenths * 100;
    hundredths = time / 10;
    time -= hundredths * 10;
    func_8006FC54(2);
    digit[0] = minutes + '0';
    func_8006FCA8(dl, x + 4, y, digit, 4);
    digit[0] = tenSeconds + '0';
    func_8006FCA8(dl, x + 12, y, digit, 4);
    func_8006FCA8(dl, x + 18, y, ":", 4);
    digit[0] = seconds + '0';
    func_8006FCA8(dl, x + 24, y, digit, 4);
    digit[0] = tenths + '0';
    func_8006FCA8(dl, x + 32, y, digit, 4);
    func_8006FCA8(dl, x + 38, y, ".", 4);
    digit[0] = hundredths + '0';
    func_8006FCA8(dl, x + 44, y, digit, 4);
    digit[0] = time + '0';
    func_8006FCA8(dl, x + 52, y, digit, 4);
}

/* Draws the five fastest times */
static void draw_records(Gfx **dl, M075Record *records, s32 alpha) {
    s32 y;
    s32 i;
    char letter[2];

    func_8006FC54(2);
    func_8006FC8C(0, 0, 0, 0);
    func_8006FC68(255, 224, 32, 255, alpha);
    func_8006FCA8(dl, 160, 68, "FASTEST TIMES", 4);
    letter[1] = '\0';
    y = 84;
    i = 5;
    while (i--) {
        func_8006FC68(255, 255, 255, 255, alpha);
        draw_time(dl, 116, y, records->time);
        func_8006FC68(32, 128, 255, 255, alpha);
        letter[0] = sLetters[records->initial0];
        func_8006FCA8(dl, 184, y, letter, 4);
        letter[0] = sLetters[records->initial1];
        func_8006FCA8(dl, 192, y, letter, 4);
        letter[0] = sLetters[records->initial2];
        func_8006FCA8(dl, 200, y, letter, 4);
        records++;
        y += 12;
    }
}
