#include "common.h"

/* A sprite at a point that fades in while nothing is in the way and out otherwise (50 slots). Size: 0x50 bytes */
typedef struct Corona {
    /* 0x00 */ s16 rotation[3];
    /* 0x06 */ u8 active;
    /* 0x07 */ s8 opacity;
    /* 0x08 */ f32 scale;
    /* 0x0C */ f32 pos[3];
    /* 0x18 */ struct CoronaOwner *owner;
    /* 0x1C */ f32 offset[3];    // from the owner (or the position when it has none)
    /* 0x28 */ f32 direction[3];
    /* 0x34 */ s8 attachPoint;   // an attach point of the owner's model, or -1
    /* 0x35 */ u8 rotateOffset;  // the offset turns with the owner
    /* 0x36 */ u8 dirty;         // 1: the position, 2: the direction
    /* 0x37 */ s8 unk37;
    /* 0x38 */ u8 colour[4];
    /* 0x3C */ void *unk3C; // JFG: a sprite
    /* 0x40 */ f32 unk40;
    /* 0x44 */ f32 unk40Inv;
    /* 0x48 */ f32 unk48;
    /* 0x4C */ f32 unk48Inv;
} Corona;
extern Corona D_800F5B60[50];
extern s32 D_800A110C;

/* JFG: mark the 50 corona slots free (no sprite). */
void coronas_init(void) {
    Corona *corona = D_800F5B60;
    s32 i = 50;
    while (i--) {
        corona->active = 0;
        corona->unk3C = 0;
        corona++;
    }
    D_800A110C = 0;
}

void corona_free(Corona *light);

/* JFG: free every one of the 50 coronas. */
void coronas_free_all(void) {
    Corona *corona = D_800F5B60;
    s32 i = 50;
    while (i--) {
        corona_free(corona);
        corona++;
    }
}

extern s32 D_800A1108; // JFG: how many coronas are active
extern void *D_800A1110; // JFG: one sprite all coronas use

/* JFG: the description of one of an object's coronas (an object header's). Size: 0x20 bytes */
typedef struct CoronaHeader {
    /* 0x00 */ u8 active;
    /* 0x01 */ u8 size;
    /* 0x02 */ s16 rotation[3];
    /* 0x08 */ f32 offset[3];
    /* 0x14 */ u8 colour[4];
    /* 0x18 */ s16 spriteID; // -1: none
    /* 0x1A */ s16 unk1A;
    /* 0x1C */ s16 unk1C;
    /* 0x1E */ s8 attachPoint;
    /* 0x1F */ u8 visible;
} CoronaHeader;

extern f32 D_800ABAB0;
void corona_update_transform(Corona *corona);
void *tex_load_sprite(s32 spriteID, s32 arg1);

/* JFG: take a free corona slot (of 50) for an object (or none) from its description, placing it
   at once; the first one also loads the sprite all coronas share. */
Corona *corona_add(struct CoronaOwner *owner, CoronaHeader *header) {
    Corona *corona;

    if (D_800A1108 >= 50) {
        return NULL;
    }
    corona = D_800F5B60;
    while (corona->active) {
        corona++;
    }
    D_800A1108++;
    corona->rotation[0] = header->rotation[0];
    corona->rotation[1] = header->rotation[1];
    corona->rotation[2] = header->rotation[2];
    corona->active = header->active;
    if (header->visible == 0) {
        corona->opacity = 0;
    } else {
        corona->opacity = 16;
    }
    corona->scale = header->size * D_800ABAB0;
    corona->owner = owner;
    corona->offset[0] = header->offset[0];
    corona->offset[1] = header->offset[1];
    corona->offset[2] = header->offset[2];
    corona->attachPoint = header->attachPoint;
    corona->rotateOffset = 0;
    if (owner != NULL && (corona->offset[0] != 0.0f || corona->offset[1] != 0.0f || corona->offset[2] != 0.0f)) {
        corona->rotateOffset = 1;
    }
    corona->dirty = 3;
    corona->unk37 = -1;
    corona_update_transform(corona);
    corona->colour[0] = header->colour[0];
    corona->colour[1] = header->colour[1];
    corona->colour[2] = header->colour[2];
    corona->colour[3] = header->colour[3];
    if (header->spriteID >= 0) {
        corona->unk3C = tex_load_sprite(header->spriteID, 0);
    } else {
        corona->unk3C = NULL;
    }
    corona->unk40 = header->unk1A;
    corona->unk40Inv = 1.0f / corona->unk40;
    corona->unk48 = header->unk1C;
    corona->unk48Inv = 1.0f / corona->unk48;
    if (D_800A1108 == 1) {
        D_800A1110 = tex_load_sprite(0x49, 0);
    }
    return corona;
}
void sprite_free(void *sprite);

/* JFG: free an active corona and its sprite; freeing the last one also frees the common sprite. */
void corona_free(Corona *corona) {
    if (corona != NULL && corona->active) {
        corona->active = 0;
        if (corona->unk3C != NULL) {
            sprite_free(corona->unk3C);
            corona->unk3C = NULL;
        }
        if (--D_800A1108 == 0) {
            if (D_800A1110 != NULL) {
                sprite_free(D_800A1110);
                D_800A1110 = NULL;
            }
        }
    }
}

void corona_update_transform(Corona *corona);

/* JFG: run corona_update_transform on each active corona. */
void coronas_update(void) {
    Corona *corona;
    s32 i;

    if (D_800A1108 > 0) {
        corona = D_800F5B60;
        i = 49;
        do {
            if (corona->active) {
                corona_update_transform(corona);
            }
            corona++;
        } while (i--);
    }
}

/* JFG: a model's header, reduced */
typedef struct CoronaModelHeader {
    /* 0x00 */ u8 pad0[0x2D];
    /* 0x2D */ u8 numAttachPoints;
} CoronaModelHeader;

/* JFG: a model instance, reduced */
typedef struct CoronaModel {
    /* 0x00 */ CoronaModelHeader *header;
    /* 0x04 */ u8 pad4[0x68 - 0x4];
    /* 0x68 */ f32 (*attachPoints)[3];
} CoronaModel;

/* JFG: the object a corona is attached to, reduced */
typedef struct CoronaOwner {
    /* 0x00 */ s16 rotation[3];
    /* 0x06 */ u8 pad6[6];
    /* 0x0C */ f32 pos[3];
    /* 0x18 */ u8 pad18[0x3A - 0x18];
    /* 0x3A */ s8 modelIndex;
    /* 0x3B */ u8 pad3B[0x6C - 0x3B];
    /* 0x6C */ CoronaModel **models;
} CoronaOwner;

void func_800496B0(s32 arg0, s16 *rotation, f32 *in, f32 *out); // DKR: vec3f_rotate
void func_8004944C(s16 *rotation, f32 *vec);                     // DKR: vec3f_rotate_py

/* JFG: recompute a corona's position (from its owner: an attach point or the turned offset) and its
   direction (straight back, turned by the corona and its owner) when they are marked dirty or it has an owner. */
void corona_update_transform(Corona *corona) {
    CoronaOwner *owner;
    CoronaModel *model;
    UNUSED s32 pad;

    if (corona != NULL) {
        owner = corona->owner;
        if ((corona->dirty & 1) || owner != NULL) {
            if (owner == NULL) {
                corona->pos[0] = corona->offset[0];
                corona->pos[1] = corona->offset[1];
                corona->pos[2] = corona->offset[2];
            } else if (corona->attachPoint >= 0) {
                model = owner->models[owner->modelIndex];
                if (model != NULL && model->header != NULL && corona->attachPoint < model->header->numAttachPoints) {
                    corona->pos[0] = model->attachPoints[corona->attachPoint][0];
                    corona->pos[1] = model->attachPoints[corona->attachPoint][1];
                    corona->pos[2] = model->attachPoints[corona->attachPoint][2];
                }
            } else if (corona->rotateOffset) {
                func_800496B0(1, owner->rotation, corona->offset, corona->pos);
                corona->pos[0] += owner->pos[0];
                corona->pos[1] += owner->pos[1];
                corona->pos[2] += owner->pos[2];
            } else {
                corona->pos[0] = owner->pos[0];
                corona->pos[1] = owner->pos[1];
                corona->pos[2] = owner->pos[2];
            }
        }
        if ((corona->dirty & 2) || owner != NULL) {
            corona->direction[2] = -1.0f;
            func_8004944C(corona->rotation, corona->direction);
            if (owner != NULL) {
                func_800496B0(1, owner->rotation, corona->direction, corona->direction);
            }
            corona->direction[0] = -corona->direction[0];
            corona->direction[1] = -corona->direction[1];
            corona->direction[2] = -corona->direction[2];
        }
        corona->dirty = 0;
    }
}

/* JFG: what func_80018808 writes back */
typedef struct HitResult {
    /* 0x00 */ f32 unk0[11];
} HitResult;

s32 func_80018808(s32 arg0, f32 *from, f32 *to, HitResult *result, s32 flags, s32 arg5);

/* JFG: raise the corona's opacity by 5 (up to 16) when func_80018808 finds nothing between pos and it, else lower it (down to 0). */
void corona_visibility_update(f32 *pos, Corona *corona) {
    HitResult result;

    if (func_80018808(0, pos + 3, corona->pos, &result, 0x2404, 0) == 0) {
        corona->opacity += 5;
        if (corona->opacity > 16) {
            corona->opacity = 16;
        }
    } else {
        corona->opacity -= 5;
        if (corona->opacity < 0) {
            corona->opacity = 0;
        }
    }
}

/* JFG: the camera, reduced */
typedef struct CoronaCamera {
    /* 0x00 */ s16 rotation[3];
    /* 0x06 */ u8 pad6[6];
    /* 0x0C */ f32 x;
    /* 0x10 */ f32 y;
    /* 0x14 */ f32 z;
} CoronaCamera;

CoronaCamera *cam_get_active_camera(void);
s32 func_80045A14(s32 rect);
s8 screen_rect_queue(s32 x, s32 y, s32 size);
s32 func_80041E58(f32 x, f32 y, f32 z, f32 *outX, f32 *outY, u8 toView);

#ifdef NON_MATCHING
/* JFG: update the active coronas from where the last call stopped: screen-space ones (0x20) fade by
   whether their queued screen point was hidden and queue a new one along their direction; the others
   in range are lit, or (0x40) get up to four visibility tests a call.
   NON_MATCHING: the target keeps -1 and &screenY in saved registers (mine &dir and &screenX) and has
   two more words below dir. */
void func_80024044(void) {
    CoronaCamera *cam;
    Corona *corona;
    s32 next;
    f32 screenX;
    f32 screenY;
    s32 i;
    s32 budget;
    f32 dx;
    f32 dy;
    f32 dz;
    f32 dir[3];
    f32 radiusSq;
    f32 distSq;

    if (D_800A1108 > 0) {
        cam = cam_get_active_camera();
        budget = 4;
        i = D_800A110C;
        do {
            corona = &D_800F5B60[i];
            i++;
            if (i >= 50) {
                i = 0;
            }
            if (corona->active & 0x20) {
                if (func_80045A14(corona->unk37) == 0) {
                    corona->opacity += 8;
                    if (corona->opacity > 16) {
                        corona->opacity = 16;
                    }
                } else {
                    corona->opacity -= 8;
                    if (corona->opacity < 0) {
                        corona->opacity = 0;
                    }
                }
                dir[2] = 256.0f;
                func_8004944C(corona->rotation, dir);
                dir[0] += cam->x;
                dir[1] += cam->y;
                dir[2] += cam->z;
                if (func_80041E58(dir[0], dir[1], dir[2], &screenX, &screenY, 1)) {
                    corona->unk37 = screen_rect_queue(screenX, screenY, 3);
                } else {
                    corona->unk37 = -1;
                }
            } else if (corona->active) {
                radiusSq = corona->unk40 * corona->unk40;
                dx = corona->pos[0] - cam->x;
                dy = corona->pos[1] - cam->y;
                dz = corona->pos[2] - cam->z;
                distSq = dx * dx + dy * dy + dz * dz;
                if (distSq < radiusSq) {
                    if (corona->active & 0x40) {
                        if (budget) {
                            corona_visibility_update((f32 *) cam, corona);
                            budget--;
                            if (budget == 0) {
                                next = i;
                            }
                        }
                    } else {
                        corona->opacity = 16;
                    }
                } else {
                    corona->opacity = 0;
                }
            }
        } while (i != D_800A110C);
        if (budget == 0) {
            D_800A110C = next;
        }
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/245D0", func_80024044);
#endif

#ifdef NON_MATCHING
/**
 * JFG: of count RGBA colours, each channel of the one brightest in it (times alpha), and the top alpha.
 * NON_MATCHING: 22 of 64 words: the loop is one instruction short (the target copies the count in
 * the loop) and its registers differ.
 */
void func_80024330(s32 count, u8 *colours, u8 *outRed, u8 *outGreen, u8 *outBlue, u8 *outAlpha) {
    s32 blue, green, red;
    s32 maxBlue, maxGreen, maxRed;
    s32 maxAlpha;
    s32 value;
    s32 alpha;
    s32 product;

    blue = 0;
    green = 0;
    red = 0;
    maxBlue = 0;
    maxGreen = 0;
    maxRed = 0;
    maxAlpha = 0;
    while (count--) {
        value = colours[0];
        alpha = colours[3];
        product = value * alpha;
        if (maxRed < product) {
            maxRed = product;
            red = value;
        }
        value = colours[1];
        product = value * alpha;
        if (maxGreen < product) {
            maxGreen = product;
            green = value;
        }
        value = colours[2];
        colours += 4;
        product = value * alpha;
        if (maxBlue < product) {
            maxBlue = product;
            blue = value;
        }
        if (maxAlpha < alpha) {
            maxAlpha = alpha;
        }
    }
    *outRed = red;
    *outGreen = green;
    *outBlue = blue;
    *outAlpha = maxAlpha;
}
#else
INCLUDE_ASM("asm/nonmatchings/245D0", func_80024330);
#endif

/* JFG: one sprite of a flare (a list ends with size 0). Size: 0x10 bytes */
typedef struct CoronaFlare {
    /* 0x00 */ f32 t; // how far along the direction
    /* 0x04 */ f32 size;
    /* 0x08 */ u8 pad8;
    /* 0x09 */ u8 red;
    /* 0x0A */ u8 green;
    /* 0x0B */ u8 blue;
    /* 0x0C */ f32 alpha;
} CoronaFlare;

/* JFG: what the sprite draw takes for its position (DKR's ObjectTransform and more) */
typedef struct SpriteTransform {
    /* 0x00 */ s16 rotation[3];
    /* 0x06 */ s16 flags;
    /* 0x08 */ f32 scale;
    /* 0x0C */ f32 x;
    /* 0x10 */ f32 y;
    /* 0x14 */ f32 z;
    /* 0x18 */ u8 pad18[0x28 - 0x18];
    /* 0x28 */ f32 unk28;
} SpriteTransform;

extern Gfx *D_800F5B50;
extern Mtx *D_800F5B54;
extern void *D_800F5B58;
void render_sprite_billboard(Gfx **dList, Mtx **matrices, void *verts, SpriteTransform *trans, void *sprite, s32 flags, s32 arg6);

/* JFG: draw a flare's sprites along a direction from a point, each tinted and scaled. */
void corona_flare_draw(CoronaFlare *flare, f32 x, f32 y, f32 z, f32 dirX, f32 dirY, f32 dirZ, f32 scale, f32 opacity) {
    SpriteTransform trans;

    if (flare != NULL) {
        trans.rotation[0] = 0;
        trans.rotation[1] = 0;
        trans.rotation[2] = 0;
        trans.unk28 = 0.0f;
        while (flare->size != 0.0f) {
            trans.x = flare->t * dirX + x;
            trans.y = flare->t * dirY + y;
            trans.z = flare->t * dirZ + z;
            trans.scale = flare->size * scale;
            gDPSetPrimColor(D_800F5B50++, 0, 0, flare->red, flare->green, flare->blue, (s32) (flare->alpha * opacity));
            render_sprite_billboard(&D_800F5B50, &D_800F5B54, &D_800F5B58, &trans, D_800A1110, 4, 0);
            flare++;
        }
    }
}

INCLUDE_ASM("asm/nonmatchings/245D0", func_80024634);
