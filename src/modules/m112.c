/*
 * Runlink module 112: an explosive object, either placed in the level or
 * launched (then it flies off in the direction of its setup angles). It has
 * its own transform matrix for drawing (pushed by 013B4); 00000 stops its
 * sound and switches it to the finished state.
 */
#include "module.h"

/* What init gets */
typedef struct M112Setup {
    /* 0x00 */ s16 objectID;
    /* 0x02 */ u8 unk2;
    /* 0x03 */ u8 launched; /* 0: placed in the level */
    /* 0x04 */ s16 x;
    /* 0x06 */ s16 y;
    /* 0x08 */ s16 z;
    /* 0x0A */ u8 type;
    /* 0x0B */ u8 unkB;
    /* 0x0C */ s16 yaw;
    /* 0x0E */ s16 pitch;
    /* 0x10 */ f32 speed;
    /* 0x14 */ s32 unk14;
    /* 0x18 */ u8 unk18;
    /* 0x19 */ u8 delay; /* in tenths of a second */
} M112Setup;

/* What obj->unk68 points at */
typedef struct M112State {
    /* 0x00 */ u8 type;
    /* 0x01 */ u8 mode;
    /* 0x02 */ s8 unk2; /* a timer */
    /* 0x03 */ s8 unk3; /* a count */
    /* 0x04 */ u8 unk4;
    /* 0x05 */ u8 unk5;
    /* 0x06 */ s16 unk6;
    /* 0x08 */ MtxF rel;  /* its transform relative to the object it sticks to */
    /* 0x48 */ MtxF mtx;
    /* 0x88 */ f32 plane[4]; /* the surface it sticks to: normal and distance */
    /* 0x98 */ s16 unk98;
    /* 0x9A */ s16 unk9A;
    /* 0x9C */ Object *unk9C;
    /* 0xA0 */ Object *unkA0;
    /* 0xA4 */ Object *holder; /* what it sticks to */
    /* 0xA8 */ s32 sound;
    /* 0xAC */ u8 unkAC;
} M112State;

/* The joint matrices of the object it sticks to (obj->unk5C) */
typedef struct M112Joints {
    /* 0x000 */ MtxF mtxA[2];
    /* 0x080 */ MtxF mtxB[2];
    /* 0x100 */ u8 pad100[0x12B - 0x100];
    /* 0x12B */ u8 index;
    /* 0x12C */ s16 active;
} M112Joints;

/* What func_80018808 fills in, with the surface kind after it */
typedef struct M112Hit {
    /* 0x00 */ Object *obj;
    /* 0x04 */ Vec3f pos;
    /* 0x10 */ f32 normal[4];
    /* 0x20 */ u8 pad20[0x28 - 0x20];
    /* 0x28 */ u32 kind;
} M112Hit;

/* The level entry of the blast it spawns (size 0x18) */
typedef struct M112BlastEntry {
    /* 0x00 */ s16 objectID;
    /* 0x02 */ u8 pad2[2];
    /* 0x04 */ s16 x;
    /* 0x06 */ s16 y;
    /* 0x08 */ s16 z;
    /* 0x0A */ s16 yaw;
    /* 0x0C */ s16 pitch;
    /* 0x0E */ u8 padE[2];
    /* 0x10 */ u8 unk10;
    /* 0x11 */ u8 unk11;
    /* 0x12 */ u8 unk12;
    /* 0x13 */ u8 unk13;
    /* 0x14 */ Object *owner;
} M112BlastEntry;

typedef struct M112Model {
    /* 0x00 */ u8 pad0[0x11];
    /* 0x11 */ u8 unk11;
    /* 0x12 */ u8 pad12[0x4E - 0x12];
    /* 0x4E */ s8 unk4E;
} M112Model;

typedef struct M112Instance {
    /* 0x00 */ M112Model *model;
    /* 0x04 */ u8 pad4[0xA - 0x4];
    /* 0x0A */ s8 unkA;
    /* 0x0B */ s8 matrixIndex;
    /* 0x0C */ u8 padC[0x10 - 0xC];
    /* 0x10 */ MtxF *matrices[2];
} M112Instance;

/* The state kept in the object (OBJ_STATE) */
typedef struct M112ObjState {
    /* 0x0 */ s32 unk0;
    /* 0x4 */ s32 alpha;
} M112ObjState;

/* The model of the blast's sprite */
typedef struct M112BlastModel {
    /* 0x00 */ u8 pad0[0x78];
    /* 0x78 */ u32 dl;
} M112BlastModel;

/* obj->modelInstances[1]: the blast's sprite */
typedef struct M112Blast {
    /* 0x00 */ M112BlastModel *model;
    /* 0x04 */ u32 matrices;
    /* 0x08 */ u8 pad8[0x8C - 0x8];
    /* 0x8C */ s16 size;
} M112Blast;

/* Loads a model's matrices (the game's own command 0xBF) */
#define gSPPartMatrices(pkt, m)                    \
    {                                              \
        Gfx *_g = (Gfx *) (pkt);                   \
        _g->words.w0 = 0xBF000000;                 \
        _g->words.w1 = (u32) (m);                  \
    }

/* The game's own command 0x02 */
#define gSPCommand02(pkt, a)                       \
    {                                              \
        Gfx *_g = (Gfx *) (pkt);                   \
        _g->words.w0 = 0x02000050;                 \
        _g->words.w1 = (u32) (a);                  \
    }

extern s32 D_m112_013E0; /* the module's number */
extern u8 D_800A0838[];

extern void runlink_unload_timer_set(s32, s32, s32);
extern ObjectTransform *func_80041DE0(void); /* the camera */
extern void func_80041CA0(Gfx **dl, Mtx **mtx, ObjectTransform *trans, f32 scale, f32 arg4);
extern void func_80055B80(Gfx **dl); /* resets the render settings */
extern void mtx_pop(Gfx **dl);
extern void func_80003614(s32 handle); /* stops a positioned sound */
extern void func_80011D1C(Object *obj);
extern void func_80049DC0(Object *obj, MtxF *out);
extern void mtx_model_push(Gfx **dList, Mtx **mtx, MtxF *transform);
extern u8 rl_m112_013E4[]; /* per type */
extern void func_m105_01474(Object *obj, f32 *x, f32 *y, f32 *z, f32 *ox, f32 *oy, f32 *oz);
extern void func_80048B60(MtxF *mtx, f32 x, f32 y, f32 z, f32 *ox, f32 *oy, f32 *oz); /* DKR mathMtxXFMF */
extern s32 func_80009A04(Object *obj, f32 dx, f32 dy, f32 dz);                       /* moves the object */
extern f32 func_80049BEC(f32 base, s32 exponent);
extern s32 track_height_at(f32 x, f32 z, f32 *y, s32 flags); /* the floor height */
extern void func_8005F5AC(Object *obj, s32 updateRate);
extern void func_80048C7C(MtxF *a, MtxF *b, MtxF *out);
extern void func_8004A2FC(MtxF *mtx, f32 *in, f32 *out);
extern void func_800035E8(s32 handle, f32 x, f32 y, f32 z);
extern void spawn_object_from_list(s32 arg0, s16 x, s16 y, s16 z, s32 arg4, Object *owner);
extern s16 func_80049C84(f32 x, f32 z);
extern f32 func_80097140(f32 x); /* sqrtf */
extern Object *func_80005F14(void *entry, s32 arg1);
extern void zone_add(f32 x, f32 y, f32 z, f32 size, f32 value, f32 time, s32 arg6, Object *owner);
extern void enemy_module_notify(s32 arg0, s16 x, s16 y, s16 z, s32 arg4, s32 arg5, s32 arg6);
extern void func_8003D21C(M112Instance *inst, M112Model *model, Object *obj, f32 arg3, f32 arg4);
extern void func_800221F0(Object *obj, M112Instance *inst, ObjectShading *shading, MtxF *mtx);
extern void obj_tex_animate(Object *obj, M112Model *model, M112Instance *inst, s32 arg3);

void func_m112_00000(Object *obj) {
    M112State *s = obj->unk68;

    if (s->mode != 5) {
        if (s->sound != 0) {
            func_80003614(s->sound);
        }
        s->mode = 6;
        s->unkAC = 0;
    }
}

void func_m112_0005C(Object *obj, M112Setup *setup) {
    M112State *s = obj->unk68;
    f32 size;

    if (setup->launched == 0) {
        s->type = 0;
        s->mode = 2;
        s->unk4 = 1;
        s->unk5 = func_80048E2C(0, 0x7F);
        s->unk9C = 0;
        s->unkA0 = 0;
    } else {
        obj->velocity.z = -setup->speed;
        obj->trans.rotation.y_rotation = setup->yaw;
        obj->trans.rotation.x_rotation = setup->pitch;
        obj->trans.rotation.z_rotation = 0;
        func_8004944C(&obj->trans.rotation, &obj->velocity);
        s->type = setup->type;
        s->mode = 0;
        s->unk2 = 60;
        s->unk3 = setup->unkB;
        s->unk4 = setup->unk18;
        s->unk5 = 0;
        s->unk6 = func_80005064(setup->delay * 6);
        s->unk9C = (Object *) setup->unk14;
        s->unkA0 = (Object *) setup->unk14;
        obj->level_entry = NULL;
        if (s->type == 0 || s->type == 1) {
            func_80011D1C(obj);
        }
    }
    s->unk9A = 0;
    OBJ_STATE(obj, M112ObjState)->unk0 = 0;
    OBJ_STATE(obj, M112ObjState)->alpha = 0xFF;
    if (s->type == 0) {
        size = 80.0f;
    } else if (s->type == 1) {
        size = 107.0f;
    } else {
        size = 32.0f;
    }
    func_80079D1C(obj, 0x100, 1, 2, 0, 0, 0, size, size, -1.0f, 1.0f, 1);
    func_80049DC0(obj, &s->mtx);
}

/*
 * Control: follows what it sticks to; flies (bouncing off the floor) until
 * it hits something, then waits for a player to come close, counts down or
 * goes off: the blast (an object of its own), a sound and a shake.
 */
void func_m112_00220(Object *obj, s32 updateRate) {
    M112Hit hit;
    s32 rate;
    s32 found;
    s32 first;
    s32 count;
    M112State *s;
    f32 dx;
    f32 dy;
    f32 dz;
    Object **list;
    f32 floorY;
    Vec3f from;
    f32 fRate;
    f32 decay;
    s32 flags;
    f32 dir[3];
    MtxF objMtx;
    M112BlastEntry entry;
    Object *blast;
    Object *owner;
    Object *other;
    M112Model *model;
    Vec3f *relPos;
    M112Joints *joints;
    Object *holder;
    u8 *data;
    s32 pad4; /* (four unused words: the frame needs them) */
    s32 pad5;
    M112Instance *inst;
    s32 pad6;
    s32 pad7;
    s16 savedRot;
    f32 groundY;
    f32 savedY;

    s = obj->unk68;
    runlink_unload_timer_set(D_m112_013E0, 2, 2);
    relPos = (Vec3f *) s->rel[3];
    if (s->holder != NULL) {
        holder = s->holder;
        joints = (M112Joints *) holder->unk5C;
        if (holder->behaviorId == 0x37) {
            func_m105_01474(holder, &relPos->x, &relPos->y, &relPos->z, &obj->trans.x_position,
                            &obj->trans.y_position, &obj->trans.z_position);
        } else if (holder->behaviorId == 7) {
            data = holder->unk68;
            if (*(u16 *) (data + 0x3E) & 5) {
                s->mode = 5;
            }
        } else if (holder->behaviorId == 0x4E) {
            if (OBJ_STATE(holder, s32)[0] <= 0) {
                s->mode = 5;
            }
        } else if (joints->active != 0) {
            func_80048B60(&joints->mtxB[joints->index], relPos->x, relPos->y, relPos->z, &dx, &dy, &dz);
            func_80009A04(obj, dx - obj->trans.x_position, dy - obj->trans.y_position, dz - obj->trans.z_position);
        }
    }
    if (obj->interactObj != NULL) {
        if (s->unk9A != 0) {
            obj->interactObj->hitPoints -= updateRate * 16;
        }
        if (obj->interactObj->hitPoints <= 0) {
            s->mode = 5;
        }
    }
    s->unk9A = 0;
    switch (s->mode) {
        case 0: {
            from.x = obj->trans.x_position;
            from.y = obj->trans.y_position;
            from.z = obj->trans.z_position;
            rate = updateRate;
            if (updateRate >= s->unk6) {
                rate = s->unk6;
                s->unk6 = 0;
            } else {
                s->unk6 -= updateRate;
            }
            if (OBJ_STATE(obj, M112ObjState)->unk0 == 0) {
                obj->trans.rotation.x_rotation += updateRate << 10;
                obj->trans.rotation.y_rotation += updateRate << 11;
            }
            fRate = rate;
            if (OBJ_STATE(obj, M112ObjState)->unk0 != 0) {
                decay = func_80049BEC(0.9f, rate);
                obj->velocity.x *= decay;
                obj->velocity.y = func_80049BEC(0.7f, rate) * (-0.4f - obj->velocity.y) + obj->velocity.y;
                obj->velocity.z *= decay;
                dy = obj->velocity.y * fRate;
            } else {
                dy = obj->velocity.y * fRate + -0.075f * fRate * fRate;
                obj->velocity.y += -0.15f * fRate;
            }
            dx = obj->velocity.x * fRate;
            dz = obj->velocity.z * fRate;
            func_80009A04(obj, dx, dy, dz);
            if (OBJ_STATE(obj, M112ObjState)->unk0 == 0) {
                if (track_height_at(obj->trans.x_position, obj->trans.z_position, &groundY, 0x2000) & 0x2000) {
                    if (obj->trans.y_position < groundY) {
                        OBJ_STATE(obj, M112ObjState)->unk0 = 8;
                        savedY = obj->trans.y_position;
                        savedRot = obj->trans.rotation.x_rotation;
                        obj->trans.y_position = groundY;
                        obj->trans.rotation.x_rotation = 0;
                        obj->particleEmittersEnabled = 6;
                        func_8005F5AC(obj, 2);
                        func_80003410(0x9E, obj->trans.x_position, groundY, obj->trans.z_position, 4, NULL);
                        obj->trans.y_position = savedY;
                        obj->trans.rotation.x_rotation = savedRot;
                    }
                }
            }
            if (func_80018808(obj, &from, (Vec3f *) &obj->trans.x_position, (HitResult *) &hit, 0x2000, 0) != 0) {
                func_80009A04(obj, hit.pos.x - obj->trans.x_position, hit.pos.y - obj->trans.y_position,
                              hit.pos.z - obj->trans.z_position);
                if (hit.kind == 3) {
                    obj->velocity.x = hit.normal[0] * 8.0f;
                    obj->velocity.y = hit.normal[1];
                    obj->velocity.z = hit.normal[2] * 8.0f;
                } else {
                    if (s->type == 0) {
                        s->mode = 1;
                    } else if (s->type == 1) {
                        if (s->unk3 != 0) {
                            s->mode = 3;
                        } else {
                            s->mode = 5;
                        }
                    } else {
                        s->mode = 4;
                    }
                    if ((s->holder = hit.obj) != NULL) {
                        joints = (M112Joints *) s->holder->unk5C;
                        func_80049DC0(obj, &objMtx);
                        func_80048C7C(&objMtx, &joints->mtxA[joints->index], &s->rel);
                        func_8004A2FC(&joints->mtxB[joints->index], hit.normal, s->plane);
                    } else {
                        s->plane[0] = hit.normal[0];
                        s->plane[1] = hit.normal[1];
                        s->plane[2] = hit.normal[2];
                        s->plane[3] = hit.normal[3];
                    }
                    s->unk98 = 1;
                }
            } else if (s->unk6 == 0) {
                s->mode = 5;
            }
            obj->particleEmittersEnabled = OBJ_STATE(obj, M112ObjState)->unk0 | 1;
            func_8005F5AC(obj, updateRate);
            break;
        }
        case 1:
        case 2:
            list = func_80005E98(&first, &count);
            s->unk5 += updateRate * 4;
            found = 0;
            while (first < count && found == 0) {
                other = list[first++];
                if (other->behaviorId == 1 || other->behaviorId == 0x18 || other->behaviorId == 0x4D) {
                    dx = other->trans.x_position - obj->trans.x_position;
                    dy = other->trans.y_position - obj->trans.y_position;
                    dz = other->trans.z_position - obj->trans.z_position;
                    if (dx * dx + dy * dy + dz * dz < 6400.0f) {
                        found = 1;
                    }
                }
            }
            if (s->mode == 1 && found == 0) {
                func_80003410(0xCD, obj->trans.x_position, obj->trans.y_position, obj->trans.z_position, 4, NULL);
                s->mode = 2;
            } else if (s->mode == 2) {
                func_8001138C(obj, 0.0167f, updateRate);
                if (found != 0) {
                    s->mode = 5;
                } else if (s->sound == 0) {
                    s->unkAC = 1;
                } else if (s->sound != 0) {
                    func_800035E8(s->sound, obj->trans.x_position, obj->trans.y_position, obj->trans.z_position);
                }
            }
            break;
        case 3:
            s->unk2 -= updateRate;
            if (s->unk2 <= 0) {
                s->unk3--;
                if (s->unk3 != 0) {
                    func_80003410(0xD0, obj->trans.x_position, obj->trans.y_position, obj->trans.z_position, 4, NULL);
                    s->unk2 += 60;
                } else {
                    s->mode = 5;
                }
            }
            break;
        case 4:
            if (s->unk9C == NULL) {
                s->mode = 5;
            } else {
                s->unk2 -= updateRate;
                if (s->unk2 <= 0) {
                    func_80003410(0xCF, obj->trans.x_position, obj->trans.y_position, obj->trans.z_position, 4, NULL);
                    s->unk2 += 60;
                }
            }
            break;
        case 5:
            if (s->type == 2) {
                owner = s->unk9C;
                if (owner != NULL && owner->behaviorId == 1) {
                    if (*(Object **) ((u8 *) owner->unk68 + 0x1A0) == obj) {
                        *(Object **) ((u8 *) owner->unk68 + 0x1A0) = NULL;
                    }
                }
            }
            if (s->sound != 0) {
                func_80003614(s->sound);
            }
            s->unkAC = 0;
            flags = track_height_at(obj->trans.x_position, obj->trans.z_position, &floorY, 0x2000);
            if ((flags & 0x2000) && obj->trans.y_position < floorY) {
                floorY -= obj->trans.y_position;
                if (floorY < 10.0f) {
                    flags = 0;
                }
            } else {
                flags = 0;
            }
            owner = s->unkA0;
            if (owner != NULL && owner->behaviorId != 1) {
                owner = NULL;
            }
            if (flags & 0x2000) {
                spawn_object_from_list(6, obj->trans.x_position, obj->trans.y_position, obj->trans.z_position,
                              rl_m112_013E4[s->type], owner);
            } else {
                if (s->holder != NULL) {
                    joints = (M112Joints *) s->holder->unk5C;
                    func_8004A2FC(&joints->mtxA[joints->index], s->plane, dir);
                    dx = dir[0];
                    dy = dir[1];
                    dz = dir[2];
                } else {
                    dx = s->plane[0];
                    dy = s->plane[1];
                    dz = s->plane[2];
                }
                entry.objectID = 0x13C;
                entry.x = obj->trans.x_position;
                entry.y = obj->trans.y_position;
                entry.z = obj->trans.z_position;
                entry.yaw = func_80049C84(dx, dz) + 0x8000;
                entry.pitch = func_80049C84(dy, func_80097140(dx * dx + dz * dz));
                entry.unk10 = 2;
                entry.unk12 = 0;
                entry.unk13 = s->unk98;
                entry.unk11 = rl_m112_013E4[s->type];
                entry.owner = owner;
                blast = func_80005F14(&entry, 1);
                if (blast != NULL) {
                    blast->level_entry = NULL;
                }
                zone_add(obj->trans.x_position, obj->trans.y_position, obj->trans.z_position, 160.0f, 80.0f,
                              0.5f, rl_m112_013E4[s->type], owner);
            }
            func_80003410(0x22, obj->trans.x_position, obj->trans.y_position, obj->trans.z_position, 4, NULL);
            enemy_module_notify(0, obj->trans.x_position, obj->trans.y_position, obj->trans.z_position, 5, 1, 150);
            func_800079B8(obj);
            break;
        case 6:
            s->unk5 += updateRate * 4;
            OBJ_STATE(obj, M112ObjState)->alpha -= updateRate * 16;
            if (OBJ_STATE(obj, M112ObjState)->alpha <= 0) {
                OBJ_STATE(obj, M112ObjState)->alpha = 0;
                func_800079B8(obj);
            }
            break;
    }
    {
        if (s->holder != NULL && s->holder->behaviorId != 0x37) {
            joints = (M112Joints *) s->holder->unk5C;
            func_80048C7C(&s->rel, &joints->mtxB[joints->index], &s->mtx);
        } else {
            func_80049DC0(obj, &s->mtx);
        }
        inst = ((M112Instance **) obj->modelInstances)[obj->modelIndex];
        model = inst->model;
        if (model->unk4E != 0) {
            func_8003D21C(inst, model, obj, 1.0f, 0.0f);
            func_800221F0(obj, inst, obj->shading, inst->matrices[inst->matrixIndex]);
        } else {
            if (model->unk11 != 0) {
                inst->matrixIndex ^= 1;
            }
            func_800221F0(obj, inst, obj->shading, &s->mtx);
        }
    }
    if (model->unk11 != 0) {
        obj_tex_animate(obj, model, inst, inst->unkA);
    }
    inst->unkA = 0;
}

/* Draws the blast: a sprite facing the camera, twice (crossed), growing and fading */
void func_m112_01004(Gfx **dList, Mtx **mtx, Object *obj) {
    s32 alpha;
    ObjectTransform trans;
    M112Blast *blast;
    M112BlastModel *model;
    M112State *s;
    ObjectTransform *cam;

    runlink_unload_timer_set(D_m112_013E0, 2, 2);
    s = obj->unk68;
    if (s->type == 0 && s->unk5 < 128) {
        cam = func_80041DE0();
        blast = ((M112Blast **) obj->modelInstances)[1];
        if (blast != NULL) {
            model = blast->model;
            trans.scale = 1.0f - (u32) s->unk5 / 127.0f;
            trans.scale = 1.0f - trans.scale * trans.scale;
            trans.scale *= 80.0f / blast->size;
            trans.x_position = obj->trans.x_position;
            trans.y_position = obj->trans.y_position;
            trans.z_position = obj->trans.z_position;
            alpha = 128 - s->unk5;
            if (alpha > 64) {
                alpha = 64;
            }
            alpha = (OBJ_STATE(obj, M112ObjState)->alpha * alpha) >> 8;
            gDPSetEnvColor((*dList)++, 255, 255, 255, 255);
            if (s->mode != 2 && s->mode != 5) {
                gDPSetPrimColor((*dList)++, 0, 0, 0, 255, 0, alpha);
            } else {
                gDPSetPrimColor((*dList)++, 0, 0, 255, 0, 0, alpha);
            }
            trans.rotation.y_rotation = -0x8000 - cam->rotation.y_rotation;
            trans.rotation.x_rotation = 0;
            trans.rotation.z_rotation = 0;
            func_80041CA0(dList, mtx, &trans, 1.0f, 0.0f);
            gSPPartMatrices((*dList)++, blast->matrices + 0x80000000);
            gSPCommand02((*dList)++, D_800A0838 + 0x80000000);
            gSPDisplayList((*dList)++, model->dl + 0x80000000);
            func_80055B80(dList);
            trans.rotation.y_rotation = -0x8000 - cam->rotation.y_rotation;
            trans.rotation.x_rotation = 0x4000;
            trans.rotation.z_rotation = 0;
            func_80041CA0(dList, mtx, &trans, 1.0f, 0.0f);
            gSPPartMatrices((*dList)++, blast->matrices + 0x80000000);
            gSPCommand02((*dList)++, D_800A0838 + 0x80000000);
            gSPDisplayList((*dList)++, model->dl + 0x80000000);
            func_80055B80(dList);
            gSPPartMatrices((*dList)++, 0);
            mtx_pop(dList);
            func_80055B80(dList);
            gDPSetPrimColor((*dList)++, 0, 0, 255, 255, 255, 255);
        }
    }
}

void func_m112_013B4(Gfx **dList, Mtx **mtx, Object *obj) {
    M112State *s = obj->unk68;

    mtx_model_push(dList, mtx, &s->mtx);
}

INCLUDE_ASM("asm/nonmatchings/modules/m112", data);

