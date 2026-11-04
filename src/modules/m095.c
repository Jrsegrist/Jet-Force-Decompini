/*
 * Runlink module 95: a flame weapon. Init clears a user's weapon data and
 * keeps it in a list of up to four. The fire function starts a flame for
 * every two steps it is given (moving the new flame on by the steps left
 * over); the update function ages, fades, grows and moves the flames, heats
 * what they hit and keeps a light at the user while flames are alive; the
 * limit function keeps each flame close behind the one before it; the hit
 * function hurts what a player's flames touch; the draw function draws the
 * flames as sprites; the fini function removes the light of each player user
 * and frees it.
 */
#include "module.h"

/* A flame (size 0x34) */
typedef struct M095Flame {
    /* 0x00 */ f32 x;
    /* 0x04 */ f32 y;
    /* 0x08 */ f32 z;
    /* 0x0C */ Vec3f vel;
    /* 0x18 */ f32 size;
    /* 0x1C */ f32 unk1C; /* the sprite's frame */
    /* 0x20 */ Vec3f base; /* the user's velocity when it started */
    /* 0x2C */ s16 life;
    /* 0x2E */ s16 fadeDelay;
    /* 0x30 */ u8 fadeStep;
    /* 0x31 */ u8 alpha;
    /* 0x32 */ u8 pad32[0x34 - 0x32];
} M095Flame;

/* The light at the user (what func_800210A4 returns) */
typedef struct M095LightObj {
    /* 0x00 */ u8 pad0[0x18];
    /* 0x18 */ f32 x;
    /* 0x1C */ f32 y;
    /* 0x20 */ f32 z;
    /* 0x24 */ u8 pad24[0x43 - 0x24];
    /* 0x43 */ u8 intensity;
    /* 0x44 */ f32 unk44;
} M095LightObj;

/* A user's weapon data (size 0x624) */
typedef struct M095Data {
    /* 0x000 */ Object *owner;
    /* 0x004 */ M095LightObj *light;
    /* 0x008 */ M095Flame flames[30];
    /* 0x620 */ u32 newest : 6; /* the newest flame + 1 */
    /* 0x620 */ u32 count : 6;
    /* 0x620 */ u32 pending : 6; /* steps not yet turned into flames */
    /* 0x620 */ u32 fade : 1;
} M095Data;

/* What a flame user's obj->header points at */
typedef struct M095Header {
    /* 0x00 */ u8 pad0[0xAC];
    /* 0xAC */ u8 **textures;
} M095Header;

/* The owner's light (in the player's data) */
typedef struct M095Light {
    /* 0x0 */ u8 pad0[0x4];
    /* 0x4 */ s32 light;
} M095Light;

/* What a player object keeps in unk68 */
typedef struct M095Player {
    /* 0x000 */ u8 pad0[0xC0];
    /* 0x0C0 */ s32 sound;
    /* 0x0C4 */ u8 padC4[0x53A - 0xC4];
    /* 0x53A */ u16 sounding;
    /* 0x53C */ u8 pad53C[0x564 - 0x53C];
    /* 0x564 */ M095Light *flameLight;
    /* 0x568 */ u8 pad568[0x56F - 0x568];
    /* 0x56F */ u8 hurt;
    /* 0x570 */ Object *hurtBy;
} M095Player;

/* What objects of behaviours 0xA6 and 0xA7 keep in unk68 */
typedef struct M095Burnable {
    /* 0x00 */ u8 pad0[0x9A];
    /* 0x9A */ s16 burnt;
} M095Burnable;

/* What obj->interactObj points at here */
typedef struct M095Interact {
    /* 0x00 */ u8 pad0[0x24];
    /* 0x24 */ f32 radius;
} M095Interact;

/* What objects of behaviour 0x4E have at unk5C */
typedef struct M095Heated {
    /* 0x000 */ u8 pad0[0x12A];
    /* 0x12A */ u8 unk12A;
    /* 0x12B */ u8 pad12B[0x130 - 0x12B];
    /* 0x130 */ s16 heat;
} M095Heated;

/* The level entry of the smoke the update spawns */
typedef struct M095Entry {
    /* 0x00 */ s16 objectID;
    /* 0x02 */ u8 unk2;
    /* 0x03 */ u8 unk3;
    /* 0x04 */ s16 x;
    /* 0x06 */ s16 y;
    /* 0x08 */ s16 z;
    /* 0x0A */ u8 padA[0xE - 0xA];
    /* 0x0E */ u8 unkE;
    /* 0x0F */ u8 padF;
    /* 0x10 */ f32 unk10;
} M095Entry;

/* The start of the fire function's sweep */
typedef struct M095From {
    /* 0x00 */ Vec3f pos;
    /* 0x0C */ f32 unkC;
} M095From;

extern void func_80098360(void *data, s32 size); /* clears */
extern void func_8002179C(s32 light);            /* DKR light_remove */
extern void func_8004AA80(void *data);           /* DKR mempool_free */
extern void func_80055B80(Gfx **dl);             /* resets the render settings */
extern void func_m003_10548(Object *obj, Object *attacker);
extern void func_m003_0CFA8(Object *obj);
extern void render_sprite_billboard(Gfx **dl, Mtx **mtx, struct Vertex **vtx, Object *obj, void *texture, s32 flags, s32 alpha);
extern void sound_stop(s32 handle);
extern void func_80001248(s32 sound, s32 arg1); /* DKR amSndPlay */
extern M095LightObj *func_800210A4(Object *obj, LightParams *params); /* DKR light_add_from_object_header */
extern s32 sprite_anim_advance(void *sprite, s32 *flags, s32 speed, f32 *frame, s32 updateRate);
extern Object *func_80005F14(void *entry, s32 arg1); /* DKR spawn_object */
extern f32 func_80097140(f32 x); /* square root */
extern u8 D_800A4FC4;

static LightParams sFlameLight = { 1, 3, 0x71, -1, 0, 0, 0, 0, 0x100, 100, 255, 255, 100, 150, 0x20, 0, 0 };
s32 D_m095_01848 = 95; /* the module's number */

static M095Data *sUsers[4];
static s32 sUserCount;

void func_m095_00000(Object *owner, M095Data *data) {
    func_80098360(data, 0x624);
    data->owner = owner;
    sUsers[sUserCount++] = data;
}

/* Fires: a new flame at (x, y, z) heading (yaw, pitch) for every two steps */
void func_m095_0005C(M095Data *data, f32 x, f32 y, f32 z, s16 yaw, s16 pitch, s32 steps) {
    Object *owner;
    HitResult hit;
    M095Player *player;
    u8 *texture;
    M095Flame *f;
    s32 n;
    Vec3s rot;
    s32 k;
    s32 flags;
    M095From from;
    UNUSED s32 pad[2];

    owner = data->owner;
    from.pos.x = x;
    from.pos.y = y;
    from.pos.z = z;
    from.unkC = 0.0f;
    flags = 9;
    if (owner->behaviorId == 1) {
        player = owner->unk68;
        if (player->sounding != 1) {
            if (player->sound != 0) {
                sound_stop(player->sound);
            }
            func_80001248(1, (s32) &player->sound);
            player->sounding = 1;
        }
        texture = ((M095Header *) owner->header)->textures[3];
    } else {
        texture = ((M095Header *) owner->header)->textures[0];
    }
    data->pending += steps;
    while (data->pending >= 2) {
        f = &data->flames[data->newest];
        if (f->life <= 0) {
            f->life = 60;
            f->size = func_80048E2C(0, 3000) * 0.0001f + 1.0f;
            f->fadeDelay = 45;
            f->alpha = 255;
            f->fadeStep = 0x11;
            f->x = x;
            f->y = y;
            f->z = z;
            f->unk1C = func_80048E2C(0, *texture);
            f->base.x = owner->velocity.x;
            f->base.y = owner->velocity.y;
            f->base.z = owner->velocity.z;
            rot.y_rotation = yaw;
            rot.x_rotation = pitch;
            rot.z_rotation = 0;
            f->vel.x = 0.0f;
            f->vel.y = 0.0f;
            f->vel.z = -2.0f;
            func_8004944C(&rot, &f->vel);
            n = data->pending - 2;
            if (n != 0) {
                f->life -= n;
                f->fadeDelay -= n;
                for (k = 0; k < n; k++) {
                    f->size *= 1.04f;
                }
                f->x += (f->vel.x + f->base.x) * n;
                f->y += (f->vel.y + f->base.y) * n;
                f->z += (f->vel.z + f->base.z) * n;
                if (func_80018808(NULL, &from.pos, (Vec3f *) f, &hit, 0, 0)) {
                    f->x = hit.pos.x;
                    f->y = hit.pos.y;
                    f->z = hit.pos.z;
                }
                sprite_anim_advance(texture, &flags, 0x14, &f->unk1C, steps);
            }
            if (data->newest == 29) {
                data->newest = 0;
            } else {
                data->newest++;
            }
            data->count++;
        }
        data->pending -= 2;
    }
}

/* Updates the flames (by updateRate) and the light at (x, y, z) */
void func_m095_0059C(M095Data *data, f32 x, f32 y, f32 z, s32 updateRate) {
    M095Flame *f;
    HitResult hit;
    s32 i;
    Object *owner;
    s32 k;
    s32 n;
    Object *o;
    f32 rateF;
    M095LightObj *light;
    u8 *texture;
    Vec3f from;
    s32 flags;
    M095Entry entry;

    owner = data->owner;
    flags = 9;
    if (data->count != 0) {
        runlink_unload_timer_set(D_m095_01848, 2, 2);
    }
    if (data->count != 0) {
        light = data->light;
        if (light == NULL) {
            light = data->light = func_800210A4(NULL, &sFlameLight);
        }
        if (light != NULL) {
            i = data->count << 4;
            if (i >= 256) {
                i = 255;
            }
            light->intensity = i;
            data->light->unk44 = i;
            data->light->x = x;
            data->light->y = y;
            data->light->z = z;
        }
    } else if ((light = data->light) != NULL) {
        func_8002179C((s32) light);
        data->light = NULL;
    }
    if (data->count == 0) {
        return;
    }
    if (data->fade) {
        f = data->flames;
        for (i = 0; i < 30; i++) {
            if (f->life > 0) {
                f->fadeDelay = 0;
                f->fadeStep = f->alpha / f->life;
            }
            f++;
        }
    }
    if (owner->behaviorId == 1) {
        texture = ((M095Header *) owner->header)->textures[3];
    } else {
        texture = ((M095Header *) owner->header)->textures[0];
    }
    f = data->flames;
    for (i = 0; i < 30; i++, f++) {
        if (f->life > 0) {
            f->life -= updateRate;
            if (f->life <= 0) {
                data->count--;
            } else {
                if (f->fadeDelay <= 0) {
                    n = f->fadeStep * updateRate;
                    if (n < f->alpha) {
                        f->alpha -= n;
                    } else {
                        f->alpha = 0;
                    }
                } else {
                    f->fadeDelay -= updateRate;
                }
                rateF = updateRate;
                for (k = 0; k < updateRate; k++) {
                    f->size *= 1.04f;
                }
                from.x = f->x;
                from.y = f->y;
                from.z = f->z;
                f->x += (f->vel.x + f->base.x) * rateF;
                f->y += (f->vel.y + f->base.y) * rateF;
                f->z += (f->vel.z + f->base.z) * rateF;
                if (func_80018808(NULL, &from, (Vec3f *) f, &hit, 0, 0)) {
                    stubbed_printf("Hit\n");
                    f->x = hit.pos.x;
                    f->y = hit.pos.y;
                    f->z = hit.pos.z;
                    o = hit.obj;
                    if (o != NULL) {
                        if (o->behaviorId == 0x5B) {
                            *(s32 *) o->state += updateRate << 4;
                        } else if (o->behaviorId == 0x4E && o->unk5C != NULL) {
                            ((M095Heated *) o->unk5C)->unk12A = 1;
                            ((M095Heated *) o->unk5C)->heat += updateRate << 4;
                        }
                    }
                }
                sprite_anim_advance(texture, &flags, 0x14, &f->unk1C, updateRate);
            }
        }
    }
    if (!D_800A4FC4 && func_80048E2C(0, 3) == 0) {
        i = data->newest - func_80048E2C(1, 4);
        if (i < 0) {
            i += 30;
        }
        f = &data->flames[i];
        if (f->life > 40) {
            entry.objectID = 0x142;
            entry.unk2 = 20;
            entry.unk3 = 0;
            entry.x = f->x;
            entry.y = f->y;
            entry.z = f->z;
            entry.unkE = 0;
            entry.unk10 = 0.0f;
            func_80005F14(&entry, 1)->level_entry = NULL;
        }
    }
}

/* Keeps each flame within reach of the one in front of it (newest first) */
void func_m095_00D38(M095Data *data) {
    M095Flame *first;
    M095Flame *prev;
    s32 j;
    s32 n;
    f32 gx;
    f32 gy;
    f32 gz;
    f32 dx;
    f32 dy;
    f32 dz;
    f32 px;
    f32 py;
    f32 pz;
    f32 maxDist;
    f32 len;
    M095Flame *cur;
    f32 t;
    f32 dist;
    s32 d;

    if (data->count == 0) {
        return;
    }
    j = data->newest - 1;
    if (j < 0) {
        j += 30;
    }
    maxDist = 2.0f;
    first = prev = &data->flames[j];
    j--;
    for (n = 1; n < data->count; n++) {
        if (j < 0) {
            j += 30;
        }
        cur = &data->flames[j];
        d = prev->life - data->flames[j].life;
        /* (an empty then-branch: with `if (d < 4)` IDO keeps maxDist in a register across the loop) */
        if (d >= 4) {
        } else {
            px = prev->x + first->vel.x * d;
            py = prev->y + first->vel.y * d;
            pz = prev->z + first->vel.z * d;
            gx = cur->x - px;
            gy = cur->y - py;
            gz = cur->z - pz;
            dist = func_80097140(gx * gx + gy * gy + gz * gz);
            if (maxDist < dist) {
                t = maxDist / dist;
                px = t * gx + px;
                py = t * gy + py;
                pz = t * gz + pz;
                px -= prev->x;
                py -= prev->y;
                pz -= prev->z;
                len = func_80097140(px * px + py * py + pz * pz);
                dx = cur->x - prev->x;
                dy = cur->y - prev->y;
                dz = cur->z - prev->z;
                dist = func_80097140(dx * dx + dy * dy + dz * dz);
                if (len == 0.0f) {
                    t = 0.0f;
                } else {
                    t = dist / len;
                }
                cur->x = prev->x + t * px;
                cur->y = prev->y + t * py;
                cur->z = prev->z + t * pz;
            }
            dx = cur->x - prev->x;
            dy = cur->y - prev->y;
            dz = cur->z - prev->z;
            dist = func_80097140(dx * dx + dy * dy + dz * dz);
            if (dist != 10.0f) {
                if (dist == 0.0f) {
                    t = 0.0f;
                } else {
                    t = 10.0f / dist;
                }
                cur->x = prev->x + t * dx;
                cur->y = prev->y + t * dy;
                cur->z = prev->z + t * dz;
            }
        }
        if (n >= 4) {
            maxDist += 0.3f;
        }
        prev = cur;
        j--;
    }
}

/* A player's flames: what they touch is hurt (enemies, other players, burnable objects) */
void func_m095_0110C(M095Data *data) {
    Object *owner = data->owner;
    f32 reach = 0.0f;
    M095Flame *far = NULL;
    s32 first;
    s32 last;
    Object **list;
    Object *o;
    M095Burnable *burnable;
    M095Player *player;
    M095Flame *f;
    s32 n;
    s32 i;
    s32 j;
    f32 dx;
    f32 dy;
    f32 dz;
    f32 d;
    f32 r;
    M095Interact *interact;

    if (owner->behaviorId != 1) {
        return;
    }
    i = 1;
    if (data->count == 0) {
        return;
    }
    for (j = data->newest - 1; i < data->count; i++) {
        if (j < 0) {
            j += 30;
        }
        f = &data->flames[j];
        dx = owner->trans.x_position - f->x;
        dy = owner->trans.y_position - f->y;
        dz = owner->trans.z_position - f->z;
        d = dx * dx + dy * dy + dz * dz;
        if (reach < d) {
            reach = d;
            far = f;
        }
        j--;
    }
    if (far == NULL) {
        return;
    }
    r = far->size * 4.0f;
    reach += r * r;
    list = func_80005E98(&first, &last);
    for (i = first; i < last; i++) {
        o = list[i];
        if (o->behaviorId != 0x18) {
            if (o->behaviorId == 1 && o != owner) {
                player = o->unk68;
            } else if (o->behaviorId == 0xA6 || o->behaviorId == 0xA7) {
                burnable = o->unk68;
            } else {
                o = NULL;
            }
        }
        if (o != NULL) {
            dx = owner->trans.x_position - o->trans.x_position;
            dy = owner->trans.y_position - o->trans.y_position;
            dz = owner->trans.z_position - o->trans.z_position;
            if (dx * dx + dy * dy + dz * dz <= reach) {
                for (n = 1, j = data->newest - 1; n < data->count; n++) {
                    if (j < 0) {
                        j += 30;
                    }
                    f = &data->flames[j];
                    dx = o->trans.x_position - f->x;
                    dy = o->trans.y_position - f->y;
                    dz = o->trans.z_position - f->z;
                    r = f->size * 4.0f;
                    interact = (M095Interact *) o->interactObj;
                    if (interact != NULL) {
                        r += interact->radius;
                    }
                    r *= r;
                    if (dx * dx + dy * dy + dz * dz <= r) {
                        if (o->behaviorId == 0x18) {
                            func_m003_10548(o, owner);
                            func_m003_0CFA8(o);
                        } else if (o->behaviorId == 1) {
                            player->hurt = 6;
                            if (data->owner != NULL && data->owner->behaviorId == 1) {
                                player->hurtBy = data->owner;
                            }
                        } else {
                            burnable->burnt = 1;
                        }
                        break;
                    }
                    j--;
                }
            }
        }
    }
}

/* Draws the flames as sprites, newest first, skipping ones that would overlap the last drawn */
void func_m095_0148C(Object *obj, M095Data *data, Gfx **dlp, Mtx **mtxp, struct Vertex **vtxp) {
    UNUSED s32 pad[4];
    Object sprite;
    s32 i;
    u32 n;
    M095Flame *f;
    f32 gap;
    Gfx *dl;
    Mtx *mtx;
    struct Vertex *vtx;
    void *texture;

    dl = *dlp;
    mtx = *mtxp;
    vtx = *vtxp;
    sprite.trans.rotation.y_rotation = 0;
    sprite.trans.rotation.x_rotation = 0;
    sprite.trans.rotation.z_rotation = 0;
    func_80055B80(&dl);
    gDPPipeSync(dl++);
    gDPSetEnvColor(dl++, 255, 255, 255, 255);
    if (obj->behaviorId == 1) {
        texture = ((M095Header *) obj->header)->textures[3];
    } else {
        texture = ((M095Header *) obj->header)->textures[0];
    }
    i = data->newest - 1;
    gap = 0.0f;
    for (n = 0; n < data->count; n++) {
        if (i < 0) {
            i += 30;
        }
        if (gap <= 0.0f) {
            f = &data->flames[i];
            if (f->life > 0) {
                sprite.unk28 = f->unk1C;
                sprite.trans.x_position = f->x;
                sprite.trans.y_position = f->y;
                sprite.trans.z_position = f->z;
                sprite.trans.scale = f->size;
                gDPPipeSync(dl++);
                gDPSetPrimColor(dl++, 0, 0, 255, 255, 255, f->alpha);
                render_sprite_billboard(&dl, &mtx, &vtx, &sprite, texture, 2, f->alpha);
            }
            gap += f->size;
        }
        gap -= 1.5f;
        i--;
    }
    gDPSetPrimColor(dl++, 0, 0, 255, 255, 255, 255);
    func_80055B80(&dl);
    *dlp = dl;
    *mtxp = mtx;
    *vtxp = vtx;
}

void func_m095_0175C(void) {
    s32 i;
    Object *owner;
    M095Data *data;
    M095Player *player;

    i = sUserCount;
    while (i--) {
        data = sUsers[i];
        owner = data->owner;
        if (owner->behaviorId == 1) {
            player = owner->unk68;
            if (player->flameLight->light != 0) {
                func_8002179C(player->flameLight->light);
            }
            func_8004AA80(player->flameLight);
            player->flameLight = NULL;
        }
    }
}
#pragma weak __rl_fini = func_m095_0175C
