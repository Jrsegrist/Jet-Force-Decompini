/*
 * Runlink module 13: the player's aiming crosshair. Each frame it casts a ray
 * along the aim (as far as the weapon reaches), locks on when the ray hits a
 * target object, and draws the crosshair at the hit point: a set of 2D lines
 * per weapon (mirrored or rotated copies of a few lines, coloured by lock-on),
 * a rotating 3D centre piece for some weapons, and the readouts some weapons
 * have (distance and zoom, the ammo bars, a "LOCK" sign).
 */
#include "module.h"
#include "gfx.h"

/* One line of a crosshair (the screen coordinates are relative to its centre) */
typedef struct M013Line {
    /* 0x0 */ s16 x0;
    /* 0x2 */ s16 y0;
    /* 0x4 */ s16 x1;
    /* 0x6 */ s16 y1;
    /* 0x8 */ u16 copies : 5;      /* how the line is repeated (see func_m013_004A8) */
    /* 0x8 */ u16 lockedColour : 1; /* a brighter colour when locked on */
    /* 0x8 */ u16 lockedOnly : 1;
    /* 0x8 */ u16 unlockedOnly : 1;
    /* 0x9 */ u16 colour : 8;
} M013Line;

/* A weapon's crosshair */
typedef struct M013Crosshair {
    /* 0x0 */ M013Line *lines;
    /* 0x4 */ u16 count;
    /* 0x6 */ u16 centre; /* 0: no centre piece, 1: still, 2: turned by 45 degrees */
    /* 0x8 */ u32 colour; /* of the centre piece */
    /* 0xC */ u32 lockedColour;
} M013Crosshair;

/* What a player's crosshair can lock on to */
typedef struct M013Target {
    /* 0x00 */ u8 pad0[0x93];
    /* 0x93 */ u8 lockedBy[4]; /* per player */
} M013Target;

/* A player's state (the object's 0x68 block) as the crosshair uses it */
typedef struct M013Player {
    /* 0x000 */ s8 player;
    /* 0x001 */ s8 character;
    /* 0x002 */ u8 pad2[0x1BC - 0x2];
    /* 0x1BC */ M013Target *target;
    /* 0x1C0 */ u8 pad1C0[0x1CA - 0x1C0];
    /* 0x1CA */ s16 aimY;
    /* 0x1CC */ s16 aimX;
    /* 0x1CE */ u8 pad1CE[0x1D0 - 0x1CE];
    /* 0x1D0 */ s16 unk1D0;
    /* 0x1D2 */ u8 pad1D2[0x1EC - 0x1D2];
    /* 0x1EC */ f32 unk1EC;
    /* 0x1F0 */ u8 pad1F0[0x53E - 0x1F0];
    /* 0x53E */ s16 unk53E;
    /* 0x540 */ u8 unk540 : 7;
    /* 0x540 */ u8 locked : 1;
    /* 0x541 */ u8 lockTimer : 4;
    /* 0x541 */ u8 unk541 : 4;
} M013Player;

/* What func_80047C1C returns */
typedef struct M013Weapons {
    /* 0x00 */ u8 pad0[0x70];
    /* 0x70 */ u8 weapon;
} M013Weapons;

/* gWeaponInfo's entries */
typedef struct M013WeaponInfo {
    /* 0x00 */ u8 pad0[7];
    /* 0x07 */ u8 unk7;
    /* 0x08 */ u8 pad8[0x10 - 0x8];
    /* 0x10 */ f32 range;
    /* 0x14 */ u8 pad14[0x30 - 0x14];
} M013WeaponInfo;

extern Gfx *D_800FF398; /* the display list being built */
extern Mtx *D_800FF39C;
extern u8 D_800A4FC4;
extern M013WeaponInfo gWeaponInfo[]; /* gWeaponInfo */

extern s16 cam_get_active_camera_id(void);                    /* the active camera's number */
extern void set_active_camera(s32 camera);             /* sets the active camera */
extern ObjectTransform *cam_get_active_camera(void);
extern M013Weapons *func_80047C1C(s32 player, s32 character);
extern void func_8003B418(Object *obj, M013Player *p, f32 *x, f32 *y, f32 *z); /* where the aim starts */
extern s32 func_8007BCD0(Object *obj, f32 dist, f32 *distOut);
extern void viewport_apply(Gfx **dList, Mtx **mtx);
extern void rendermode_reset(Gfx **dList);
extern void func_80055C00(Gfx **dList, void *tex, s32 arg2, s32 arg3);
extern f32 get_distance_to_camera(f32 x, f32 y, f32 z); /* the distance to the camera */
extern s32 func_80041E58(f32 x, f32 y, f32 z, f32 *screenX, f32 *screenY, s32 arg5); /* projects to the screen */
extern s32 func_800373E4(M013Weapons *w, s32 weapon);                              /* the weapon's ammo capacity */
extern void s32_to_decimal_string(char *buf, s32 value, s32 arg2);                          /* writes a number */
extern void debug_text_add(s16 x, s16 y, char *text);                               /* draws a text */
extern void screen_line_add_in_window(s16 x0, s16 y0, s16 x1, s16 y1, s32 colour, s32 window); /* draws a line */
extern void mtx_cam_push(Gfx **dList, Mtx **mtx, ObjectTransform *trans, f32 scaleY, f32 offsetY);
extern void func_80041DC0(Gfx **dList);

void func_m013_004A8(M013Player *p, s16 x, s16 y, ObjectTransform *trans, f32 dist);

static Vertex sCrossVertices[] = {
    { -5, 16, 0, 160, 160, 160, 255 },  { 5, 16, 0, 160, 160, 160, 255 },   { 0, 2, 0, 255, 255, 255, 255 },
    { 16, 5, 0, 160, 160, 160, 255 },   { 16, -5, 0, 160, 160, 160, 255 },  { 2, 0, 0, 255, 255, 255, 255 },
    { 5, -16, 0, 160, 160, 255, 255 },  { -5, -16, 0, 160, 160, 255, 255 }, { 0, -2, 0, 255, 255, 255, 255 },
    { -16, -5, 0, 160, 160, 255, 255 }, { -16, 5, 0, 160, 160, 255, 255 },  { -2, 0, 0, 255, 255, 255, 255 },
};
static Triangle sCrossTriangles[] = {
    { 0x40, 0, 1, 2, { { 0, 0 }, { 0, 0 }, { 0, 0 } } },
    { 0x40, 3, 4, 5, { { 0, 0 }, { 0, 0 }, { 0, 0 } } },
    { 0x40, 6, 7, 8, { { 0, 0 }, { 0, 0 }, { 0, 0 } } },
    { 0x40, 9, 10, 11, { { 0, 0 }, { 0, 0 }, { 0, 0 } } },
};
static M013Line sLines0[] = {
    { -18, 0, -10, 0, 3, 0, 0, 0, 0x0 },
    { -9, -3, -9, 3, 3, 0, 0, 0, 0x0 },
};
static M013Line sLines8[] = {
    { -12, 0, -8, 0, 3, 0, 0, 0, 0x0 },     { -13, -3, -13, 3, 3, 0, 0, 0, 0x20 },
    { -12, 4, -10, 6, 3, 0, 0, 0, 0x22 },   { -12, -4, -10, -6, 3, 0, 0, 0, 0x22 },
    { -1, -1, -3, -3, 3, 0, 0, 0, 0x2 },    { -9, -9, -9, -8, 1, 0, 1, 0, 0x27 },
    { -9, 9, -9, 10, 1, 0, 1, 0, 0x26 },
};
static M013Line sLines9[] = {
    { -12, 0, -8, 0, 3, 0, 0, 0, 0x0 },       { -13, -3, -13, 3, 3, 0, 0, 0, 0x20 },
    { -12, 4, -10, 6, 3, 0, 0, 0, 0x22 },     { -12, -4, -10, -6, 3, 0, 0, 0, 0x22 },
    { -12, -12, -15, -15, 3, 0, 0, 1, 0x2 },  { -16, -13, -12, -17, 3, 0, 0, 1, 0x2 },
    { -7, -7, -10, -10, 3, 0, 1, 0, 0x2 },    { -11, -8, -7, -12, 3, 0, 1, 0, 0x2 },
};
static M013Line sLines2[] = {
    { -22, 0, -18, 0, 3, 0, 0, 0, 0x0 },     { -16, 0, -15, 0, 3, 0, 0, 0, 0x0 },
    { -13, 0, -9, 0, 3, 0, 0, 0, 0x0 },      { -14, -14, -8, -14, 3, 0, 0, 0, 0x20 },
    { -14, -13, -14, -8, 3, 0, 0, 0, 0x20 },
};
/* The ammo bars (the first five) light up as the weapon fills */
static M013Line sLines3[] = {
    { 24, 12, 28, 12, 0, 0, 0, 0, 0x1 },     { 24, 6, 29, 6, 0, 0, 0, 0, 0x1 },
    { 24, 0, 31, 0, 0, 0, 0, 0, 0x1 },       { 24, -6, 34, -6, 0, 0, 0, 0, 0x1 },
    { 24, -12, 39, -12, 0, 0, 0, 0, 0x1 },   { -20, -13, -20, 13, 1, 0, 0, 0, 0x0 },
    { -19, -13, -16, -13, 1, 0, 0, 0, 0x0 }, { -19, 13, -16, 13, 1, 0, 0, 0, 0x0 },
    { -21, 0, -26, 0, 0, 0, 0, 0, 0x0 },     { -6, -6, -10, -10, 3, 0, 0, 0, 0x2 },
    { -7, -5, -4, -8, 3, 0, 0, 0, 0x2 },
};
static M013Line sLines6[] = {
    { 17, -12, 17, 30, 1, 0, 0, 0, 0x24 },   { 22, -7, 22, 30, 1, 0, 0, 0, 0x24 },
    { -25, 0, -25, 1, 1, 0, 0, 0, 0x25 },    { -23, 6, -23, 30, 1, 0, 0, 0, 0x24 },
    { -18, 11, -18, 30, 1, 0, 0, 0, 0x24 },  { -29, 6, -29, -6, 1, 0, 0, 0, 0x0 },
    { -28, 7, -17, 16, 1, 0, 0, 0, 0x2 },    { -28, -7, -17, -18, 1, 0, 0, 0, 0x2 },
    { -17, -18, -12, 0, 1, 0, 0, 0, 0x2 },   { -17, 18, -12, 0, 1, 0, 0, 0, 0x2 },
    { -9, 8, -9, -8, 1, 1, 0, 0, 0x20 },     { -8, -8, -3, -8, 1, 1, 0, 0, 0x20 },
    { -8, 8, -3, 8, 1, 1, 0, 0, 0x20 },      { -13, 0, -10, 0, 1, 1, 0, 0, 0x20 },
    { 0, 13, 0, 9, 2, 1, 0, 0, 0x20 },
};
static M013Line sLines5[] = {
    { -16, -9, -8, -17, 3, 1, 0, 0, 0x2 },   { -17, 3, -17, 8, 3, 1, 0, 0, 0x0 },
    { -17, -3, -17, -8, 3, 1, 0, 0, 0x0 },   { -10, -6, -5, -11, 3, 0, 0, 0, 0x22 },
    { -10, -3, -10, -5, 3, 0, 0, 0, 0x20 },  { -10, 3, -10, 5, 3, 0, 0, 0, 0x20 },
    { -24, 0, -13, 0, 3, 0, 0, 0, 0x20 },    { -21, 3, -18, 3, 2, 1, 0, 0, 0x0 },
    { -19, 8, -18, 8, 2, 1, 0, 0, 0x0 },     { 21, 3, 18, 3, 0, 1, 0, 0, 0x0 },
    { 19, 8, 18, 8, 0, 1, 0, 0, 0x0 },
};
static M013Line sLines1[] = {
    { 16, 8, 7, 17, 3, 0, 0, 0, 0x2 },       { 16, 3, 16, 7, 3, 0, 0, 0, 0x0 },
    { 3, 16, 7, 16, 3, 0, 0, 0, 0x0 },       { -3, 20, -3, 17, 4, 0, 0, 0, 0x0 },
    { 27, 14, 13, 27, 4, 1, 0, 0, 0x22 },    { 27, 13, 27, 4, 4, 1, 0, 0, 0x20 },
    { 13, 27, 4, 27, 4, 1, 0, 0, 0x20 },     { 28, 4, 31, 4, 4, 1, 0, 0, 0x20 },
    { -4, -28, -4, -31, 0, 1, 0, 0, 0x20 },  { 28, 8, 31, 8, 0, 1, 0, 0, 0x20 },
    { -23, -23, -20, 5, 0, 1, 0, 0, 0x22 },  { -10, -6, -5, -11, 3, 1, 0, 0, 0x22 },
    { -10, -3, -10, -5, 3, 1, 0, 0, 0x20 },  { -10, 3, -10, 5, 3, 1, 0, 0, 0x20 },
};
static M013Line sLines4[] = {
    { 17, -12, 17, 30, 1, 0, 0, 0, 0x24 },   { 22, -7, 22, 30, 1, 0, 0, 0, 0x24 },
    { -25, 0, -25, 1, 1, 0, 0, 0, 0x25 },    { -23, 6, -23, 30, 1, 0, 0, 0, 0x24 },
    { -18, 11, -18, 30, 1, 0, 0, 0, 0x24 },  { -29, 6, -29, -6, 1, 0, 0, 0, 0x0 },
    { -28, 7, -17, 16, 1, 0, 0, 0, 0x2 },    { -28, -7, -17, -18, 1, 0, 0, 0, 0x2 },
    { -17, -18, -12, 0, 1, 0, 0, 0, 0x2 },   { -17, 18, -12, 0, 1, 0, 0, 0, 0x2 },
    { -34, 0, -30, 0, 1, 0, 0, 0, 0x0 },     { -9, -9, -5, -5, 3, 1, 0, 0, 0x22 },
    { -6, -5, -8, 0, 1, 1, 0, 0, 0x22 },     { -6, 5, -8, 0, 1, 1, 0, 0, 0x22 },
    { -8, 3, -8, -3, 1, 1, 0, 0, 0x20 },     { 0, 23, 0, 12, 2, 0, 0, 1, 0x20 },
    { -3, 24, 3, 24, 2, 0, 0, 1, 0x20 },     { 0, 15, 0, 4, 2, 0, 1, 0, 0x10 },
    { -3, 16, 3, 16, 2, 0, 1, 0, 0x10 },
};
/* Screen-wide and screen-high lines (a sniper's sight) */
static M013Line sLines7[] = {
    { 24, -24, 24, 24, 5, 0, 0, 0, 0x0 },  { 25, 0, 104, 0, 5, 0, 0, 0, 0x0 },
    { 56, -8, 56, 8, 5, 0, 0, 0, 0x0 },    { 80, -16, 80, 16, 5, 0, 0, 0, 0x0 },
    { 88, -4, 88, 4, 5, 0, 0, 0, 0x0 },    { -8, 16, 8, 16, 6, 0, 0, 0, 0x0 },
    { 0, 17, 0, 80, 6, 0, 0, 0, 0x0 },     { -22, 0, -6, 0, 3, 0, 0, 0, 0x20 },
};
/* By weapon */
static M013Crosshair sCrosshairs[] = {
    { sLines0, 2, 2, 0xA0FF10C0, 0xFF2010C0 },  { sLines1, 14, 0, 0xA0FF10C0, 0xFF2010C0 },
    { sLines2, 5, 1, 0xA0FF10C0, 0xFF2010C0 },  { sLines3, 11, 1, 0xA0FF10C0, 0xFF2010C0 },
    { sLines4, 19, 0, 0xA0FF10C0, 0xFF2010C0 }, { sLines5, 11, 0, 0xA0FF10C0, 0xFF2010C0 },
    { sLines6, 15, 0, 0xA0FF10C0, 0xFF2010C0 }, { sLines7, 8, 2, 0xA0FF10C0, 0xFF2010C0 },
    { sLines8, 7, 0, 0xA0FF10C0, 0xFF2010C0 },  { sLines9, 8, 0, 0xA0FF10C0, 0xFF2010C0 },
    { sLines8, 7, 0, 0xA0FF10C0, 0xFF2010C0 },  { sLines8, 7, 0, 0xA0FF10C0, 0xFF2010C0 },
    { sLines8, 7, 0, 0xA0FF10C0, 0xFF2010C0 },  { sLines0, 2, 2, 0xA0FF10C0, 0xFF2010C0 },
    { sLines0, 2, 2, 0xA0FF10C0, 0xFF2010C0 },  { sLines0, 2, 2, 0xA0FF10C0, 0xFF2010C0 },
};

s32 func_m013_00000(void) {
    return TRUE;
}

void func_m013_00008(Object *obj, M013Player *p, s32 arg2) {
    UNUSED s32 pad;
    HitResult hit;
    Vec3s rot;
    s16 camId;
    s16 hitTarget;
    f32 screenX;
    f32 screenY;
    f32 dist;
    f32 ratio;
    f32 range;
    Vec3f dir;
    Vec3f end;
    ObjectTransform trans;
    M013Weapons *w;
    M013WeaponInfo *info;
    ObjectTransform *cam;
    Gfx *gfx;
    Gfx *gfx2;
    u8 weapon;

    camId = cam_get_active_camera_id();
    set_active_camera(p->player);
    cam = cam_get_active_camera();
    w = func_80047C1C(p->player, p->character);
    weapon = w->weapon;
    trans.scale = 0.8f;
    trans.rotation.y_rotation = -0x8000 - cam->rotation.y_rotation;
    trans.rotation.x_rotation = -cam->rotation.x_rotation;
    trans.rotation.z_rotation = 0;
    func_8003B418(obj, p, &trans.x_position, &trans.y_position, &trans.z_position);
    info = &gWeaponInfo[weapon];
    range = (f32) (info->unk7 * 6) * info->range;
    if (range > 3000.0f) {
        range = 3000.0f;
    }
    dir.z = -range;
    rot.y_rotation = p->aimY;
    rot.x_rotation = p->aimX;
    func_8004944C(&rot, &dir);
    end.x = trans.x_position + dir.x;
    end.y = trans.y_position + dir.y;
    end.z = trans.z_position + dir.z;
    hitTarget = 0;
    dist = range;
    if (func_80018808(NULL, (Vec3f *) &trans.x_position, &end, &hit, 0x2000, 0)) {
        if (hit.obj != NULL && hit.obj->behaviorId == 0x74) {
            hitTarget = 1;
        }
        dist = *(f32 *) &hit.pad1C[4]; /* the distance to the hit */
    }
    p->locked = func_8007BCD0(obj, dist, &dist);
    if (hitTarget) {
        p->locked = 1;
    }
    ratio = dist / range;
    if (p->locked) {
        p->lockTimer = 10;
        trans.scale += 0.1f * func_80049C34(p->lockTimer << 11);
    } else if (w->weapon != 9 && w->weapon != 1) {
        if (p->lockTimer == 0) {
            if (p->target != NULL) {
                p->target->lockedBy[p->player] = 0;
                p->target = NULL;
            }
        } else {
            p->lockTimer--;
        }
    }
    if (ratio > 0.0f) {
        viewport_apply(&D_800FF398, &D_800FF39C);
        rendermode_reset(&D_800FF398);
        func_80055C00(&D_800FF398, NULL, 5, 0);
        trans.x_position += dir.x * ratio;
        trans.y_position += dir.y * ratio;
        trans.z_position += dir.z * ratio;
        dist = -get_distance_to_camera(trans.x_position, trans.y_position, trans.z_position) / 3000.0f;
        trans.scale *= 1.0f + 3.0f * dist;
        gfx = D_800FF398++; /* gDPPipeSync */
        gfx->words.w1 = 0;
        gfx->words.w0 = 0xE7000000;
        gfx2 = D_800FF398++; /* gDPSetEnvColor(255, 255, 255, 0) */
        gfx2->words.w0 = 0xFB000000, gfx2->words.w1 = 0xFFFFFF00;
        if (func_80041E58(trans.x_position, trans.y_position, trans.z_position, &screenX, &screenY, 1)) {
            func_m013_004A8(p, screenX, screenY, &trans, dist);
        }
    }
    if (camId != p->player) {
        set_active_camera(camId);
        if (ratio > 0.0f) {
            viewport_apply(&D_800FF398, &D_800FF39C);
        }
    }
}

void func_m013_004A8(M013Player *p, s16 x, s16 y, ObjectTransform *trans, f32 dist) {
    M013Weapons *w;
    M013Crosshair *entry;
    M013Line *line;
    s32 ammo; /* the weapon's ammo capacity, then how many ammo bars to light */
    f32 fraction;
    char buf[12];
    s32 colour;
    s32 count;
    s32 tmp; /* a text's x, then the window to draw in */
    u8 player;
    s32 locked;
    Gfx *gfx;

    w = func_80047C1C(p->player, p->character);
    entry = &sCrosshairs[w->weapon];
    line = entry->lines;
    ammo = func_800373E4(w, w->weapon);
    player = p->player;
    count = entry->count;
    locked = p->locked;
    switch (entry - sCrosshairs) {
        case 5:
            s32_to_decimal_string(buf, dist * 50000.0f, 1);
            tmp = (s16) (x + 20);
            debug_text_add(tmp, y - 16, buf);
            s32_to_decimal_string(buf, p->unk1D0 * 0.54931640625f, 1);
            debug_text_add(tmp, y - 8, buf);
            break;
        case 1:
            if (locked) {
                tmp = (s16) (x + 27);
                debug_text_add(tmp, y - 15, "####");
                debug_text_add(tmp, y - 8, "LOCK");
            } else {
                s32_to_decimal_string(buf, dist * 50000.0f, 1);
                tmp = (s16) (x + 27);
                debug_text_add(tmp, y - 15, buf);
                s32_to_decimal_string(buf, p->unk1D0 * 0.54931640625f, 1);
                debug_text_add(tmp, y - 8, buf);
            }
            break;
        case 3:
            fraction = (p->unk53E - 32) / (ammo - 32.0f);
            ammo = 5.0f * fraction;
            while (ammo-- > 0) {
                sLines3[ammo].colour |= 0x20;
            }
            break;
        case 6:
            fraction = p->unk53E / (f32) ammo;
            ammo = 5.0f - 5.0f * fraction;
            while (ammo-- > 0) {
                sLines6[ammo].colour &= ~0x20;
            }
            break;
        case 4:
            fraction = p->unk53E / (f32) ammo;
            ammo = 5.0f - 5.0f * fraction;
            while (ammo-- > 0) {
                sLines4[ammo].colour &= ~0x20;
            }
            break;
        case 11:
            fraction = ((p->unk53E - 32) >> 4) * 100.0f;
            s32_to_decimal_string(buf, fraction, 1);
            debug_text_add(x + 20, y - 16, buf);
            break;
        case 7:
            fraction = (p->unk1EC - 1.0f) * 225.0f + 100.0f;
            s32_to_decimal_string(buf, fraction, 1);
            debug_text_add(32, y - 8, buf);
            break;
    }
    while (count--) {
        if ((line->lockedOnly && !locked) || (line->unlockedOnly && locked)) {
            line++;
            continue;
        }
        colour = line->colour;
        if (line->lockedColour && locked) {
            colour += 0x10;
        }
        tmp = player;
        if (line->copies == 5) {
            if (!D_800A4FC4) {
                screen_line_add_in_window(line->x0, line->y0 + y, line->x1, line->y1 + y, (u8) colour, tmp);
                screen_line_add_in_window(320 - line->x1, line->y0 + y, 320 - line->x0, line->y1 + y, (u8) colour, tmp);
            }
        } else if (line->copies == 6) {
            if (!D_800A4FC4) {
                screen_line_add_in_window(line->x0 + x, line->y0, line->x1 + x, line->y1, (u8) colour, tmp);
                screen_line_add_in_window(line->x0 + x, 240 - line->y1, line->x1 + x, 240 - line->y0, (u8) colour, tmp);
            }
        } else {
            screen_line_add_in_window(x + line->x0, y + line->y0, x + line->x1, y + line->y1, (u8) colour, tmp);
            switch (line->copies) {
                case 1:
                    screen_line_add_in_window(x - line->x0, y + line->y0, x - line->x1, y + line->y1, colour | 0x40, tmp);
                    break;
                case 2:
                    screen_line_add_in_window(x + line->x0, y - line->y0, x + line->x1, y - line->y1, (u8) colour, tmp);
                    break;
                case 4:
                    screen_line_add_in_window(x - line->x0, y - line->y0, x - line->x1, y - line->y1, colour | 0x40, tmp);
                    break;
                case 3:
                    screen_line_add_in_window(x - line->x0, y - line->y0, x - line->x1, y - line->y1, (u8) (colour | 0x40),
                                  tmp);
                    screen_line_add_in_window(x - line->y0, y + line->x0, x - line->y1, y + line->x1, (u8) (colour | 0x40),
                                  tmp);
                    screen_line_add_in_window(x + line->y0, y - line->x0, x + line->y1, y - line->x1, (u8) colour, tmp);
                    break;
            }
        }
        line++;
    }
    switch (entry - sCrosshairs) {
        case 3:
            ammo = 5.0f * fraction;
            while (ammo-- > 0) {
                sLines3[ammo].colour &= ~0x20;
            }
            break;
        case 6:
            ammo = 5.0f - 5.0f * fraction;
            while (ammo-- > 0) {
                sLines6[ammo].colour |= 0x20;
            }
            break;
        case 4:
            ammo = 5.0f - 5.0f * fraction;
            while (ammo-- > 0) {
                sLines4[ammo].colour |= 0x20;
            }
            break;
    }
    if (entry->centre == 2) {
        trans->rotation.z_rotation += 0x2000;
    }
    if (entry->centre != 0) {
        mtx_cam_push(&D_800FF398, &D_800FF39C, trans, 1.0f, 0.0f);
        if (!locked) {
            gfx = D_800FF398++; /* gDPSetPrimColor */
            gfx->words.w0 = 0xFA000000;
            gfx->words.w1 = entry->colour;
        } else {
            gfx = D_800FF398++;
            gfx->words.w0 = 0xFA000000;
            gfx->words.w1 = entry->lockedColour;
        }
        gSPVertexJFG(D_800FF398++, OS_K0_TO_PHYSICAL(sCrossVertices), 13, 0);
        gSPPolygon(D_800FF398++, OS_K0_TO_PHYSICAL(sCrossTriangles), 4, 0);
        func_80041DC0(&D_800FF398);
    }
    if (entry->centre == 2) {
        trans->rotation.z_rotation -= 0x2000;
    }
}
