/*
 * Runlink module 46: screen effects. Up to eight target markers, each tied
 * to an object slot, shown as three shrinking bracket boxes around the
 * object's screen position and blinking the object; a full-screen colour
 * overlay whose alpha moves to a target over time; and a scaling of a
 * model's vertex colours by a level that moves the same way, tinted by the
 * lighting colour from modules 39, 23 and 29.
 */
#include "module.h"
#include "gfx.h"

/* Size 8 */
typedef struct M046Marker {
    /* 0x00 */ s8 timer; /* -1: free */
    /* 0x01 */ s8 sizes[3];
    /* 0x04 */ u8 r;
    /* 0x05 */ u8 g;
    /* 0x06 */ u8 b;
    /* 0x07 */ u8 slot;
} M046Marker;

typedef struct M046Slot {
    /* 0x00 */ u8 pad0[8];
    /* 0x08 */ Object *obj;
} M046Slot;

/* A rectangle for func_800595F4, size 0xC */
typedef struct M046Rect {
    /* 0x00 */ s16 x0;
    /* 0x02 */ s16 y0;
    /* 0x04 */ s16 x1;
    /* 0x06 */ s16 y1;
    /* 0x08 */ s32 colour;
} M046Rect;

typedef struct M046Colours {
    /* 0x00 */ u8 *rgb;
    /* 0x04 */ s16 *flags;
} M046Colours;

/* Size 0x48 */
typedef struct M046Batch {
    /* 0x00 */ Vertex *verts;
    /* 0x04 */ u8 pad4[0x24 - 0x4];
    /* 0x24 */ s16 numVerts;
    /* 0x26 */ u8 pad26[0x32 - 0x26];
    /* 0x32 */ u8 unk32;
    /* 0x33 */ u8 pad33;
    /* 0x34 */ M046Colours *colours;
    /* 0x38 */ u8 pad38[0x48 - 0x38];
} M046Batch;

typedef struct M046Model {
    /* 0x00 */ u8 pad0[4];
    /* 0x04 */ M046Batch *batches;
    /* 0x08 */ u8 pad8[0x1A - 0x8];
    /* 0x1A */ s16 numBatches;
} M046Model;

extern s32 D_800A74E0; /* markers in use */
extern M046Marker D_801046A8[8];
extern M046Slot **D_801045B0;
extern s16 D_801046E8; /* overlay colour */
extern s16 D_801046EA;
extern s16 D_801046EC;
extern s16 D_801046EE; /* overlay alpha now */
extern s16 D_801046F0; /* ... at the beginning */
extern s16 D_801046F2; /* ... at the end */
extern s16 D_801046F4; /* time left */
extern s16 D_801046F6; /* duration */
extern s16 D_801046FC; /* vertex colour level: time left */
extern s16 D_801046FE; /* duration */
extern s16 D_80104700; /* level at the beginning */
extern s16 D_80104702; /* ... at the end */
extern s16 D_80104704; /* ... now */
extern s32 D_800A0D84;

extern s32 func_80041E58(f32 x, f32 y, f32 z, f32 *screenX, f32 *screenY, s32 arg5); /* projects to the screen */
extern void func_800595F4(Gfx **dl, s32 count, M046Rect *rects, s32 arg3);         /* fills rectangles */
extern void func_80054EA0(s32 *width, s32 *height, Gfx **dl);
extern void func_80055B80(Gfx **dl);
extern s32 runlink_module_loaded(s32 module);
extern void func_m039_00540(s32 *r, s32 *g, s32 *b);
extern void func_m023_006B0(s32 *r, s32 *g, s32 *b);
extern void func_m029_00E98(s32 *r, s32 *g, s32 *b);
extern void func_m110_00320();

static Gfx sOverlaySetupDL[] = {
    { 0xE7000000, 0x00000000 },
    { 0xB6000000, 0x00010001 },
    { 0xFC71FEE3, 0xFFFFF7FB },
    { 0xEF082C0F, 0x00504340 },
    { 0xB8000000, 0x00000000 },
    { 0x00000000, 0x00000000 },
};

void func_m046_00000(s32 slot, s32 r, s32 g, s32 b) {
    M046Marker *m;
    s32 i;

    if (D_800A74E0 < 8) {
        m = D_801046A8;
        i = 8;
        while (i--) {
            if (m->timer == -1) {
                m->timer = 0;
                m->sizes[0] = 30;
                m->sizes[1] = 30;
                m->sizes[2] = 30;
                m->r = r;
                m->g = g;
                m->b = b;
                m->slot = slot;
                D_800A74E0++;
                return;
            }
            m++;
        }
    }
}

void func_m046_0008C(s32 slot) {
    M046Marker *m;
    s32 i;

    if (D_800A74E0 > 0) {
        m = D_801046A8;
        i = 8;
        while (i--) {
            if (m->timer != -1 && m->slot == slot) {
                m->timer = -1;
                D_800A74E0--;
            }
            m++;
        }
    }
}

void func_m046_000F8(s32 updateRate) {
    M046Marker *m;
    s32 i;
    M046Slot *s;
    Object *obj;

    m = D_801046A8;
    i = 8;
    while (i--) {
        if (m->timer != -1) {
            if (m->timer < 8) {
                m->timer++;
            }
            m->sizes[2] = m->sizes[1];
            m->sizes[1] = m->sizes[0];
            m->sizes[0] -= updateRate;
            if (m->sizes[0] < 0) {
                m->sizes[0] = 0;
            }
            s = D_801045B0[m->slot];
            if (s != NULL) {
                obj = s->obj;
                if (obj != NULL) {
                    if (m->timer & 1) {
                        obj->state[0x93 - 0x88] = 6;
                        obj->unk68 = &m->r;
                    } else {
                        obj->state[0x93 - 0x88] = 0;
                        obj->unk68 = NULL;
                    }
                }
            }
        }
        m++;
    }
}

/* Renders a w x h bracket box at (x, y) in the colour, with a black shadow under each side */
static void render_bracket_box(Gfx **dl, s32 x, s32 y, s32 w, s32 h, s32 r, s32 g, s32 b, s32 a) {
    s32 colour;
    s32 right;
    s32 bottom;
    M046Rect *rect;
    M046Rect rects[8];

    colour = (r << 24) | (g << 16) | (b << 8) | (a & 0xFF);
    right = x + w;
    bottom = y + h;
    rect = rects;
    rect->x0 = x - 2;
    rect->y0 = y - 2;
    rect->x1 = right + 2;
    rect->y1 = y + 3;
    rect->colour = 0;
    rect++;
    rect->x0 = x;
    rect->y0 = y;
    rect->x1 = right + 1;
    rect->y1 = y + 1;
    rect->colour = colour;
    rect++;
    rect->x0 = x - 2;
    rect->y0 = y - 2;
    rect->x1 = x + 3;
    rect->y1 = bottom + 2;
    rect->colour = 0;
    rect++;
    rect->x0 = x;
    rect->y0 = y + 1;
    rect->x1 = x + 1;
    rect->y1 = bottom;
    rect->colour = colour;
    rect++;
    rect->x0 = right - 2;
    rect->y0 = y - 2;
    rect->x1 = right + 3;
    rect->y1 = bottom + 2;
    rect->colour = 0;
    rect++;
    rect->x0 = right;
    rect->y0 = y + 1;
    rect->x1 = right + 1;
    rect->y1 = bottom;
    rect->colour = colour;
    rect++;
    rect->x0 = x - 2;
    rect->y0 = bottom - 2;
    rect->x1 = right + 2;
    rect->y1 = bottom + 3;
    rect->colour = 0;
    rect++;
    rect->x0 = x;
    rect->y0 = bottom;
    rect->x1 = right + 1;
    rect->y1 = bottom + 1;
    rect->colour = colour;
    func_800595F4(dl, 8, rects, 0);
}

void func_m046_00308(Gfx **dl) {
    M046Marker *m;
    s32 i;
    M046Slot *s;
    Object *obj;
    s8 *sizes;
    f32 screenX;
    f32 screenY;
    s32 alpha;
    s32 j;
    s32 size;
    s32 prev;

    m = D_801046A8;
    i = 8;
    while (i--) {
        if (m->timer != -1) {
            s = D_801045B0[m->slot];
            if (s != NULL) {
                obj = s->obj;
                if (obj != NULL && func_80041E58(obj->trans.x_position, obj->trans.y_position,
                                                 obj->trans.z_position, &screenX, &screenY, 1)) {
                    sizes = m->sizes;
                    alpha = 0xA0;
                    prev = -1;
                    j = 3;
                    while (j--) {
                        size = *sizes * 8 + 16;
                        if (size != prev) {
                            render_bracket_box(dl, (s32) screenX - size, (s32) screenY - size, size * 2, size * 2,
                                             m->r, m->g, m->b, alpha);
                            prev = size;
                        }
                        alpha -= 0x34;
                        sizes++;
                    }
                }
            }
        }
        m++;
    }
}

void func_m046_0049C(s32 r, s32 g, s32 b, s32 alpha, s32 duration) {
    D_801046E8 = r;
    D_801046EA = g;
    D_801046EC = b;
    D_801046F0 = D_801046EE;
    D_801046F2 = alpha;
    D_801046F6 = duration;
    D_801046F4 = duration;
}

void func_m046_004E4(s32 updateRate) {
    if (D_801046F4 != 0) {
        D_801046EE = D_801046F2;
        D_801046F4 -= updateRate;
        if (D_801046F4 > 0) {
            D_801046EE += ((D_801046F0 - D_801046F2) * D_801046F4) / D_801046F6;
        } else {
            D_801046F4 = 0;
        }
    }
}

void func_m046_00584(Gfx **dl) {
    s32 inv;
    s32 r;
    s32 g;
    s32 b;
    s32 width;
    s32 height;

    if (D_801046EE != 0) {
        func_80054EA0(&width, &height, dl);
        inv = 255 - D_801046EE;
        r = (D_801046E8 * inv) >> 8;
        g = (D_801046EA * inv) >> 8;
        b = (D_801046EC * inv) >> 8;
        gSPDisplayList((*dl)++, sOverlaySetupDL);
        gDPSetPrimColor((*dl)++, 0, 0, r, g, b, D_801046EE);
        gDPFillRectangle((*dl)++, 0, 0, width, height);
        func_80055B80(dl);
        gDPSetPrimColor((*dl)++, 0, 0, 255, 255, 255, 255);
    }
}

void func_m046_006EC(u8 *enabled, M046Model *model, s32 updateRate) {
    s32 i;
    M046Batch *batch;
    s32 r;
    s32 g;
    s32 b;
    s32 j;
    M046Colours *colours;
    s16 *flags;
    u8 *src;
    Vertex *v;

    r = D_80104702;
    if (D_801046FC != 0) {
        if (updateRate < D_801046FC) {
            D_801046FC -= updateRate;
            r += ((D_80104700 - D_80104702) * D_801046FC) / D_801046FE;
        } else {
            D_801046FC = 0;
        }
        D_80104704 = r;
    }
    if (runlink_module_loaded(39)) {
        func_m039_00540(&r, &g, &b);
    } else if (runlink_module_loaded(23)) {
        func_m023_006B0(&r, &g, &b);
    } else {
        g = r;
        b = r;
    }
    if (runlink_module_loaded(29)) {
        func_m029_00E98(&r, &g, &b);
    }
    i = model->numBatches;
    batch = model->batches;
    while (i--) {
        if (*enabled) {
            colours = batch->colours;
            if (colours != NULL) {
                flags = colours->flags;
                j = (batch->numVerts + 15) >> 4;
                while (j--) {
                    *flags++ = 0;
                }
                src = colours->rgb;
                v = batch->verts;
                j = batch->numVerts;
                while (j--) {
                    v->r = (src[0] * r) >> 8;
                    v->g = (src[1] * g) >> 8;
                    v->b = (src[2] * b) >> 8;
                    v++;
                    src += 3;
                }
                batch->unk32 = 0;
            }
        }
        batch++;
        enabled++;
    }
    if (D_800A0D84 > 0) {
        func_m110_00320();
    }
}
