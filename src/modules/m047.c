/*
 * Runlink module 47: the cutscene sequencer's helpers - scrolling textures,
 * colour fades, curves and paths, effects - and the subtitle boxes shown
 * during cutscenes (up to five, fading in while their timer runs and out after).
 */
#include "module.h"

/* A subtitle (size 0xC) */
typedef struct M047Subtitle {
    /* 0x0 */ s16 timer;
    /* 0x2 */ u16 text; /* index into the game's texts */
    /* 0x4 */ u16 x;
    /* 0x6 */ u16 y;
    /* 0x8 */ u8 r;
    /* 0x9 */ u8 g;
    /* 0xA */ u8 b;
    /* 0xB */ u8 alpha;
} M047Subtitle;

/* A colour fade (size 0x10) */
typedef struct M047ColourFade {
    /* 0x0 */ s32 target; /* < 0: an object's light; else a colour block, address & 0x7FFFFFFF */
    /* 0x4 */ u8 r0;      /* the colour at the start */
    /* 0x5 */ u8 r1;      /* and at the end */
    /* 0x6 */ u8 g0;
    /* 0x7 */ u8 g1;
    /* 0x8 */ u8 b0;
    /* 0x9 */ u8 b1;
    /* 0xA */ u8 a0;
    /* 0xB */ u8 a1;
    /* 0xC */ s16 timer;
    /* 0xE */ s16 duration;
} M047ColourFade;

extern M047ColourFade D_80104708[12];
extern void light_colour_set(s32 light, u8 r, u8 g, u8 b); /* sets a light's colour */
extern void light_intensity_set(s32 light, u8 a);

/* .rodata, still assembly */
extern f32 rl_m047_0264C; /* 0.6 (0x3F199999) */

extern void func_8006FC54(s32 font);
extern void func_8006FC8C(s32 r, s32 g, s32 b, s32 a);
extern void func_8006FC68(s32 r, s32 g, s32 b, s32 a, s32 opacity);
extern void func_8006FCA8(Gfx **dl, s32 x, s32 y, char *text, s32 flags);
extern s32 func_800706D4(char *text, s32 x, s32 font); /* DKR get_text_width */
extern void func_m157_00000(Gfx **dl, u8 shape, s32 x, s32 y, s32 width, s32 height, u32 colour);

extern char **gMenuText; /* the game's texts */

static M047Subtitle sSubtitles[5];

INCLUDE_ASM("asm/nonmatchings/modules/m047", func_m047_00000);

/* Moves the colour fades on by `updateRate` frames */
void func_m047_00130(s32 updateRate) {
    s32 i;
    s32 target;
    s32 r;
    s32 g;
    s32 b;
    s32 a;
    u32 frac;
    u8 *colour;
    M047ColourFade *fade;

    fade = D_80104708;
    i = 12;
    while (i--) {
        target = fade->target;
        if (target != 0) {
            r = fade->r1;
            g = fade->g1;
            b = fade->b1;
            a = fade->a1;
            if (updateRate >= fade->timer) {
                fade->target = 0;
            } else {
                fade->timer -= updateRate;
                frac = (fade->timer << 16) / fade->duration;
                r += (s32) ((fade->r0 - fade->r1) * frac) >> 16;
                g += (s32) ((fade->g0 - fade->g1) * frac) >> 16;
                b += (s32) ((fade->b0 - fade->b1) * frac) >> 16;
                a += (s32) ((fade->a0 - fade->a1) * frac) >> 16;
            }
            colour = (u8 *) (target | 0x80000000);
            if (target < 0) {
                light_colour_set(target, r, g, b);
                light_intensity_set(target, a);
            } else {
                colour[0x38] = r;
                colour[0x39] = g;
                colour[0x3A] = b;
                colour[0x3B] = a;
            }
        }
        fade++;
    }
}

INCLUDE_ASM("asm/nonmatchings/modules/m047", func_m047_002C4);

INCLUDE_ASM("asm/nonmatchings/modules/m047", func_m047_007DC);

INCLUDE_ASM("asm/nonmatchings/modules/m047", func_m047_0084C);

INCLUDE_ASM("asm/nonmatchings/modules/m047", func_m047_013D0);

INCLUDE_ASM("asm/nonmatchings/modules/m047", func_m047_01560);

/*
 * Starts changing a colour to (r, g, b, a) over `time`: an object's light (isLight) or a colour block
 * (target + 0x38).
 */
void func_m047_01798(u8 *target, s32 r, s32 g, s32 b, s32 a, f32 time, s32 isLight) {
    s32 i;
    M047ColourFade *fade;

    if (target != NULL) {
        fade = D_80104708;
        i = 12;
        while (i--) {
            if (fade->target == 0) {
                if (isLight) {
                    fade->target = (s32) target;
                    fade->r0 = target[0x40];
                    fade->g0 = target[0x41];
                    fade->b0 = target[0x42];
                    fade->a0 = target[0x43];
                } else {
                    fade->target = (s32) target & 0x7FFFFFFF;
                    fade->r0 = target[0x38];
                    fade->g0 = target[0x39];
                    fade->b0 = target[0x3A];
                    fade->a0 = target[0x3B];
                }
                fade->r1 = r;
                fade->g1 = g;
                fade->b1 = b;
                fade->a1 = a;
                fade->timer = time * rl_m047_0264C;
                fade->duration = fade->timer;
                return;
            }
            fade++;
        }
    }
}

INCLUDE_ASM("asm/nonmatchings/modules/m047", func_m047_018AC);

INCLUDE_ASM("asm/nonmatchings/modules/m047", func_m047_01BAC);

INCLUDE_ASM("asm/nonmatchings/modules/m047", func_m047_01D38);

/* Clears the subtitles */
void func_m047_0206C(void) {
    s32 i;
    M047Subtitle *c;

    for (i = 0, c = sSubtitles; i < 5; i++, c++) {
        c->alpha = 0;
        c->timer = 0;
    }
}

/* Shows text `text` at (x, y) in colour (r, g, b) for `timer` frames, in the first free subtitle */
void func_m047_020AC(s32 text, s32 timer, s32 x, s32 y, s32 r, s32 g, s32 b) {
    s32 i;
    M047Subtitle *c;

    for (i = 0, c = sSubtitles; i < 5; i++, c++) {
        if (c->alpha == 0 && c->timer <= 0) {
            c->text = text;
            c->timer = timer;
            c->x = x;
            c->y = y;
            c->r = r;
            c->g = g;
            c->b = b;
            c->alpha = 0;
            return;
        }
    }
}

/* Counts the subtitles' timers down, fading them in while they run and out after */
void func_m047_0226C(s32 updateRate) {
    s32 i;
    s32 alpha;
    M047Subtitle *c;

    c = sSubtitles;
    for (i = 0; i < 5; i++) {
        if (c->timer > 0 || c->alpha > 0) {
            if (c->timer > 0) {
                c->timer -= updateRate;
                alpha = c->alpha + updateRate * 8;
                if (alpha >= 256) {
                    c->alpha = 255;
                } else {
                    c->alpha = alpha;
                }
            } else if (c->alpha > 0) {
                alpha = c->alpha - updateRate * 8;
                if (alpha < 0) {
                    c->alpha = 0;
                } else {
                    c->alpha = alpha;
                }
            }
        }
        c++;
    }
}

/* Draws the subtitles: a shaded box, the text's shadow and the text */
void func_m047_023FC(Gfx **dl) {
    s32 i;
    s32 width;
    s32 x;
    s32 y;
    M047Subtitle *c;

    c = sSubtitles;
    func_8006FC54(2);
    func_8006FC8C(0, 0, 0, 0);
    for (i = 0; i < 5; i++, c++) {
        if (c->alpha > 0) {
            width = func_800706D4(gMenuText[c->text], 2, 1) + 40;
            x = c->x - (width >> 1);
            y = c->y - 12;
            func_8006FC68(0, 0, 0, 255, c->alpha);
            func_m157_00000(dl, 0, x, y, width, 24, c->alpha >> 1);
            func_8006FCA8(dl, c->x + 1, c->y + 1, gMenuText[c->text], 12);
            func_8006FC68(c->r, c->g, c->b, 255, c->alpha);
            func_8006FCA8(dl, c->x, c->y, gMenuText[c->text], 12);
        }
    }
}

INCLUDE_ASM("asm/nonmatchings/modules/m047", data);

INCLUDE_ASM("asm/nonmatchings/modules/m047", rodata);