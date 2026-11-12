/*
 * Runlink module 87: a breakable object whose damage settings are looked up
 * by object ID in the module's data (init 001DC, control 00510), and the
 * small tumbling pieces it throws (init 01060, control 010DC), which fly
 * under gravity until their timer runs out. 011B0 changes one setting of a
 * type's entry, 011DC resets an object's state from its entry.
 */
#include "module.h"

/* An entry of the settings table (the entries have different sizes) */
typedef struct M087Entry {
    /* 0x00 */ u8 unk0; /* 0x01, 0x02: sounds; 0x10: no longer hit; 0x20 */
    /* 0x01 */ u8 unk1; /* how many pieces */
    /* 0x02 */ u8 unk2; /* bit n: pieces from point n */
    /* 0x03 */ u8 unk3; /* the pieces' kind */
    /* 0x04 */ s16 unk4; /* the pieces' object ID */
    /* 0x06 */ s8 unk6; /* a particle effect, or -1 */
    /* 0x07 */ u8 unk7; /* its point */
    /* 0x08 */ s32 unk8; /* particle emitters */
    /* 0x0C */ s16 unkC;
    /* 0x0E */ u16 unkE;
    /* 0x10 */ f32 unk10;
    /* 0x14 */ f32 unk14;
    /* 0x18 */ f32 unk18;
    /* 0x1C */ s16 unk1C;
    /* 0x1E */ s16 unk1E;
} M087Entry;

/* What the pieces' init gets */
typedef struct M087PieceSetup {
    LevelObjectEntryCommon common;
    /* 0x0A */ s16 yaw;
    /* 0x0C */ s16 pitch;
    /* 0x0E */ u8 lifetime;
    /* 0x0F */ u8 emitters;
    /* 0x10 */ f32 speed;
    /* 0x14 */ f32 gravity;
} M087PieceSetup;

typedef struct M087PieceState {
    /* 0x0 */ f32 gravity;
    /* 0x4 */ s32 timer;
} M087PieceState;

extern s32 get_level_segment_index_from_position(f32 xPos, f32 yPos, f32 zPos); /* DKR */
extern void func_8005F5AC(Object *obj, s32 updateRate);

/* The settings entries in the module's .data (still assembly), by module offset */
extern M087Entry rl_m087_01224;
extern M087Entry rl_m087_01264;
extern M087Entry rl_m087_012A4;
extern M087Entry rl_m087_01324;
extern M087Entry rl_m087_01364; /* the default */
extern M087Entry rl_m087_01390;
extern M087Entry rl_m087_013F0;
extern M087Entry rl_m087_01450;
extern M087Entry rl_m087_01470;

/* The settings entry for an object ID */
static M087Entry *func_m087_00000(s32 id) {
    switch (id) {
        case 0x135:
            return &rl_m087_01224;
        case 0x168:
            return &rl_m087_01264;
        case 0x171:
            return &rl_m087_012A4;
        case 0x1A8:
            return &rl_m087_01324;
        case 0x1C3:
            return &rl_m087_01390;
        case 0x2DE:
        case 0x2DF:
            return &rl_m087_013F0;
        case 0x2CC:
            return &rl_m087_01450;
        case 0x58:
            return &rl_m087_01470;
        default:
            return &rl_m087_01364;
    }
}

/* Moves `count` points from the object's space into the world (its yaw, scale and position) */
void func_m087_000D4(Object *obj, s32 count, Vec3f *in, Vec3f *out) {
    f32 ix;
    f32 iy;
    f32 iz;
    f32 x;
    f32 y;
    f32 z;
    f32 scale;
    f32 sine;
    f32 cosine;

    scale = obj->trans.scale;
    x = obj->trans.x_position;
    y = obj->trans.y_position;
    z = obj->trans.z_position;
    sine = func_80049C34(obj->trans.rotation.y_rotation) * scale;
    cosine = func_80049C30(obj->trans.rotation.y_rotation) * scale;
    while (count--) {
        ix = in->x;
        iz = in->z;
        iy = in->y;
        out->x = ix * cosine + iz * sine + x;
        out->y = iy * scale + y;
        out->z = iz * cosine - ix * sine + z;
        in++;
        out++;
    }
}

/* What the breakable object's init gets */
typedef struct M087Setup {
    LevelObjectEntryCommon common;
    /* 0x0A */ u8 unkA;
    /* 0x0B */ u8 scale; /* in 64ths */
    /* 0x0C */ u8 yaw;
} M087Setup;

/* What the breakable object keeps in obj->unk68 */
typedef struct M087State {
    /* 0x00 */ s16 unk0; /* from its settings entry */
    /* 0x02 */ s8 unk2;
    /* 0x03 */ s8 unk3;
    /* 0x04 */ s32 unk4;
    /* 0x08 */ s8 unk8;
    /* 0x09 */ s8 unk9;
    /* 0x0A */ s8 unkA;
    /* 0x0B */ s8 unkB;
    /* 0x0C */ f32 floorY;
    /* 0x10 */ f32 shadowScale;
    /* 0x14 */ Object *unk14; /* what it stands on */
    /* 0x18 */ s32 flag; /* its object flag (set once broken) */
    /* 0x1C */ s32 unk1C;
} M087State;

extern s32 objflags_reserve_ids(s32 count);
extern s32 objflag_get(s32 flag);
extern s32 func_8001B078(f32 x, f32 z, s32 flags, f32 ***heights);

#ifdef NON_MATCHING
/* The 0.1 limit is an extern while the .rodata is asm, so IDO loads it late; heights is one word high */
void func_m087_001DC(Object *obj, M087Setup *setup, s32 arg2) {
    f32 scale;
    s32 count;
    s32 i;
    s32 found;
    M087State *state = obj->unk68;
    f32 **heights;

    scale = setup->scale * (1.0f / 64);
    if (scale < 0.1f) {
        scale = 0.1f;
    }
    obj->trans.scale = obj->header->scale * scale;
    state->shadowScale = obj->header->shadowScale * scale;
    if (obj->shadow != NULL) {
        *obj->shadow = 0.0f;
    }
    obj->trans.rotation.y_rotation = setup->yaw << 8;
    if (!arg2) {
        state->flag = objflags_reserve_ids(1);
    }
    obj->modelIndex = 0;
    func_80079D1C(obj, -1, 0, 8, 0, 0, 0, 32.0f, 32.0f, -1.0f, 1.0f, 1);
    state->unk0 = func_m087_00000(setup->common.objectID)->unk1C;
    state->unk2 = 0x20;
    state->unk3 = 0;
    state->floorY = obj->trans.y_position;
    count = func_8001B078(obj->trans.x_position, obj->trans.z_position, 0x800, &heights);
    if (count == 0 || state->floorY < *heights[count - 1]) {
        state->unk9 = 1;
        state->unkA = 0;
    } else {
        found = -1;
        for (i = 0; i < count; i++) {
            if (state->floorY - 5.0f <= *heights[i] && *heights[i] <= state->floorY + 5.0f) {
                found = i;
                i = count;
            }
        }
        if (found == -1) {
            state->unk9 = 0;
            state->unkA = 2;
        } else {
            state->floorY = *heights[found] + 1.0f;
            state->unk9 = -15;
            state->unkA = 0;
        }
    }
    obj->velocity.y = 0.0f;
    state->unk14 = 0;
    state->unkB = 0;
    OBJ_STATE(obj, s32)[0] = 0;
    if (objflag_get(state->flag)) {
        switch (setup->common.objectID) {
            case 0x2CC:
            case 0x2DE:
            case 0x2DF:
                break;
            default:
                obj->modelIndex = obj->header->numberOfModelIds - 1;
                obj->interactObj->flags &= ~1;
                if ((state->unk9 & 0xF0) && obj->shadow != NULL) {
                    *obj->shadow = state->shadowScale;
                }
                break;
        }
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m087/ro", func_m087_001DC);
#endif

#ifdef NON_MATCHING
extern s32 D_m087_01220;  /* the module's number (.data, still assembly) */
extern Vec3f rl_m087_01384; /* where the default kind's particles come from, in its own space */
extern void runlink_unload_timer_set(s32, s32, s32);
extern void objflag_set(s32 flag, s32 value);
extern void enemy_module_notify(Object *obj, s16 x, s16 y, s16 z, s32, s32, s32);
extern void spawn_object_from_list(s16 id, s16 x, s16 y, s16 z, s32 arg4, s32 arg5);
extern void zone_add(f32 x, f32 y, f32 z, f32 arg3, f32 arg4, f32 arg5, s32 arg6, s32 arg7); /* shakes the camera */
extern void func_m008_001CC(f32 x, f32 y, f32 z, f32 nx, f32 ny, f32 nz, s32 alpha, s32 arg7, f32 arg8);
extern Object *func_80005F14(void *entry, s32 arg1); /* DKR spawn_object */

/* The breakable object's control: takes the damage, breaks a stage (pieces, sounds, particles), falls and wobbles */
void func_m087_00510(Object *obj, s32 updateRate) {
    M087State *state;
    M087Entry *entry;
    s32 flags;
    s32 pieceKind;
    s32 particle;
    s32 point;
    s32 emitters;
    s32 sound;
    f32 maxA;
    f32 maxB;
    f32 maxC;
    f32 height;
    Vec3f from;
    Vec3f to;
    s32 dirs;
    HitResult hit;
    M087PieceSetup piece;
    s32 count;
    Vec3f points[5];
    s32 i;
    Vec3f vel;
    Vec3s rot;

    state = obj->unk68;
    runlink_unload_timer_set(D_m087_01220, 2, 2);
    if ((obj->interactObj->flags & 1) || state->unk1C != 0) {
        entry = func_m087_00000(obj->objectID);
        if (entry == &rl_m087_01364) {
            func_m087_000D4(obj, 1, &rl_m087_01384, &points[1]);
        }
        entry += obj->modelIndex;
        flags = 0;
        count = 0;
        if (obj->unk5C->damage < entry->unk1E) {
            obj->unk5C->damage = 0;
        }
        state->unk0 = state->unk0 - obj->unk5C->damage - OBJ_STATE(obj, s32)[0];
        OBJ_STATE(obj, s32)[0] = 0;
        state->unk1C = 0;
        entry--;
        dirs = 0;
        particle = -1;
        if (obj->interactObj->hitPoints <= 0) {
            points[0].x = obj->trans.x_position;
            points[0].y = obj->trans.y_position;
            points[0].z = obj->trans.z_position;
            maxA = 0.0f;
            maxB = 0.0f;
            maxC = 0.0f;
            emitters = 0;
            sound = -1;
            if (state->unk0 <= 0) {
                while (obj->modelIndex < obj->header->numberOfModelIds - 1) {
                    obj->modelIndex++;
                    state->unk0 += entry[1].unk1C;
                    entry++;
                    flags |= entry->unk0;
                    if (count < entry->unk1) {
                        count = entry->unk1;
                        pieceKind = entry->unk3;
                        piece.common.objectID = entry->unk4;
                    }
                    dirs |= entry->unk2;
                    if (entry->unk6 != -1) {
                        particle = entry->unk6;
                        point = entry->unk7;
                    }
                    emitters |= entry->unk8;
                    if (state->unk3 < entry->unkC) {
                        state->unk3 = entry->unkC;
                    }
                    state->unk4 |= entry->unkE;
                    if (maxA < entry->unk10) {
                        maxA = entry->unk10;
                    }
                    if (maxB < entry->unk14) {
                        maxB = entry->unk14;
                    }
                    if (maxC < entry->unk18) {
                        maxC = entry->unk18;
                    }
                    if (state->unk0 > 0) {
                        break;
                    }
                }
            }
            if (flags & 0x10) {
                obj->interactObj->flags &= ~1;
                objflag_set(state->flag, -1);
            }
            if ((flags & 0x20) && state->unk2 == 0x20) {
                state->unk2 = 0;
            }
            if (flags & 1) {
                sound = 0x22;
            }
            if (flags & 2) {
                sound = 0x195;
            }
            if (sound != -1) {
                func_80003410(sound, obj->trans.x_position, obj->trans.y_position, obj->trans.z_position, 4, NULL);
                enemy_module_notify(NULL, obj->trans.x_position, obj->trans.y_position, obj->trans.z_position, 5, 1,
                                    0x22);
            }
            if (particle >= 0) {
                spawn_object_from_list(particle, points[point].x, points[point].y, points[point].z, 100, 0);
            }
            if (emitters != 0) {
                if ((state->unk9 & 0xF0) && obj->shadow != NULL) {
                    *obj->shadow = state->shadowScale;
                }
                obj->particleEmittersEnabled = emitters;
                func_8005F5AC(obj, 2);
            }
            if (count > 0) {
                if (pieceKind == 0) {
                    piece.lifetime = 60;
                    piece.emitters = 1;
                    piece.gravity = -0.3f;
                    piece.yaw = func_80048E2C(0, 0x10000);
                }
                i = 0;
                if ((dirs & 7) && !(dirs & 1)) {
                    do {
                        i = (i + 1) & 7;
                    } while (!((1 << i) & dirs));
                }
                while (count--) {
                    if (pieceKind == 1) {
                        rot.y_rotation = func_80048E2C(0, 0x10000);
                        rot.x_rotation = func_80048E2C(0x1000, 0x3000);
                        vel.z = func_80048E2C(450, 600) * -0.01f;
                        func_8004944C(&rot, &vel);
                        func_m008_001CC(points[i].x, points[i].y, points[i].z, vel.x, vel.y, vel.z,
                                        obj->shading->unk0 * 255.0f, 1, 1.0f);
                    } else {
                        piece.common.x = points[i].x;
                        piece.common.y = points[i].y;
                        piece.common.z = points[i].z;
                        piece.pitch = func_80048E2C(0x2800, 0x3800);
                        piece.speed = func_80048E2C(750, 900) * 0.01f;
                        entry = (M087Entry *) func_80005F14(&piece, 1);
                        if (entry != NULL) {
                            ((Object *) entry)->level_entry = NULL;
                        }
                    }
                    piece.yaw += func_80048E2C(0x2000, 0x6000);
                    if (dirs & 7) {
                        do {
                            i = (i + 1) & 7;
                        } while (!((1 << i) & dirs));
                    }
                }
            }
            if (maxA > 0.0f) {
                zone_add(obj->trans.x_position, obj->trans.y_position, obj->trans.z_position, maxB, maxA, maxC,
                              100, 0);
            }
        }
    }
    if (state->unk3 != 0) {
        state->unk3 -= updateRate;
        obj->particleEmittersEnabled = state->unk4;
        func_8005F5AC(obj, updateRate);
        if (state->unk3 < 0) {
            state->unk3 = 0;
            state->unk4 = 0;
        }
    }
    if (state->unk2 < 0x20) {
        state->unk2 += updateRate;
        if (state->unk2 >= 0x21) {
            state->unk2 = 0x20;
        }
        obj->trans.rotation.z_rotation = (s16) (func_80049C34(state->unk2 << 11) * 2048.0f) & 0xFFF0;
        obj->trans.y_position = func_80049C34(state->unk2 << 10) * 16.0f + state->floorY;
        state->unkB = 1;
    } else {
        obj->trans.y_position = state->floorY;
        state->unkB = 0;
    }
    if (state->unk14 != NULL) {
        if ((((M087State *) state->unk14->unk68)->unkA & 3) || ((M087State *) state->unk14->unk68)->unkB != 0) {
            state->unk8 = 1;
        } else if (state->unkA & 3) {
            state->unk8 = 1;
        } else {
            state->unk8 = 0;
        }
    } else {
        state->unk8 = 1;
    }
    if (state->unk9 == 0 && state->unk8 != 0) {
        height = obj->trans.scale * 20.0f;
        obj->velocity.y -= 0.5f * (f32) updateRate;
        from.x = obj->trans.x_position;
        from.y = state->floorY + height;
        from.z = obj->trans.z_position;
        to.x = obj->trans.x_position;
        to.y = state->floorY + height + obj->velocity.y * (f32) updateRate;
        to.z = obj->trans.z_position;
        func_8001CE10(1, &from, &to, &height, 0xCE000000, 0, 0);
        if (func_80016208(obj, &from, &to, height, 0x20, &hit) != 0) {
            if (((u8 *) &hit.flags)[1] & 2) {
                state->unk9 = 1;
                state->unkA = 0;
                obj->velocity.y = 0.0f;
            } else if (((u8 *) &hit.flags)[1] & 0x10) {
                state->unk14 = hit.obj;
                obj->velocity.y = (to.y - from.y) / (f32) updateRate;
            }
        } else {
            state->unk14 = NULL;
            obj->velocity.y = (to.y - from.y) / (f32) updateRate;
        }
        if (obj->velocity.y > 0.0f) {
            state->unkA = 1;
        } else if (obj->velocity.y < 0.0f) {
            state->unkA = 2;
        } else if (obj->velocity.y == 0.0f) {
            if (state->unkA & 1) {
                state->unkA = 2;
            } else if (state->unkA & 2) {
                state->unkA = 0;
            }
        }
        obj->trans.x_position = to.x;
        state->floorY = to.y - height;
        obj->trans.z_position = to.z;
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m087/ro", func_m087_00510);
#endif

void func_m087_01060(Object *obj, M087PieceSetup *setup) {
    obj->trans.rotation.y_rotation = setup->yaw;
    obj->trans.rotation.x_rotation = setup->pitch;
    obj->velocity.z = -setup->speed;
    func_8004944C(&obj->trans.rotation, &obj->velocity);
    obj->particleEmittersEnabled = setup->emitters;
    OBJ_STATE(obj, M087PieceState)->gravity = setup->gravity;
    OBJ_STATE(obj, M087PieceState)->timer = setup->lifetime * 8;
}

void func_m087_010DC(Object *obj, s32 updateRate) {
    s32 i;

    OBJ_STATE(obj, M087PieceState)->timer -= updateRate * 8;
    if (OBJ_STATE(obj, M087PieceState)->timer <= 0) {
        func_800079B8(obj);
        return;
    }
    i = updateRate;
    while (i--) {
        obj->trans.x_position += obj->velocity.x;
        obj->trans.y_position += obj->velocity.y;
        obj->trans.z_position += obj->velocity.z;
        obj->velocity.y += OBJ_STATE(obj, M087PieceState)->gravity;
    }
    obj->segmentID = get_level_segment_index_from_position(obj->trans.x_position, obj->trans.y_position,
                                                           obj->trans.z_position);
    obj->trans.rotation.x_rotation += updateRate << 11;
    obj->trans.rotation.z_rotation += updateRate << 11;
    func_8005F5AC(obj, updateRate);
}

void func_m087_011B0(s32 id, s16 value) {
    func_m087_00000(id)->unk1E = value;
}

void func_m087_011DC(Object *obj) {
    s16 *state = obj->unk68;

    *state = func_m087_00000(*(s16 *) obj->pad2C)->unk1C;
}

INCLUDE_ASM("asm/nonmatchings/modules/m087", data);

/* (.rodata: C, and the assembly functions' own from ro/) */
