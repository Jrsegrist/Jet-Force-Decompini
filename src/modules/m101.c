/*
 * Runlink module 101: a breakable pane. It is drawn as one textured quad,
 * its texture scrolling with the camera, until it has taken enough damage.
 * Then it breaks into 40 triangular shards that spin, fly apart and fall
 * (each point of the pane keeps its texture coordinate) for a few seconds,
 * after which the object frees itself or, if its level entry says so, just
 * stops drawing.
 */
#include "module.h"
#include "gfx.h"

/*
 * gSPVertexJFG with the count given as the last index (n - 1) and the length
 * field as the code computes it (m064)
 */
#define gSPVertexJFGLast(pkt, v, last, v0) \
    gDma1p(pkt, G_VTX, v, ((last) << 3) + ((last) << 1) + 8, (last) << 3 | ((u32) (v) & 6) | (v0))

/* The start of a texture: its size in pixels */
typedef struct M101Texture {
    /* 0x00 */ u8 width;
    /* 0x01 */ u8 height;
} M101Texture;

/* What obj->unk68 points at */
typedef struct M101Pane {
    /* 0x00 */ M101Texture *texture;
    /* 0x04 */ Triangle *quadTris[2]; /* double-buffered */
    /* 0x0C */ Vertex *quadVerts;
    /* 0x10 */ struct M101Shard *shards;
    /* 0x14 */ Triangle *shardTris;
    /* 0x18 */ Vertex *shardVerts[2]; /* double-buffered */
    /* 0x20 */ Triangle tris[2][2];
} M101Pane;

/* The pane's state in the object */
typedef struct M101State {
    /* 0x00 */ s32 strength; /* then, once broken, the time since */
    /* 0x04 */ s32 buffer;
} M101State;

#define PANE_STATE(obj) OBJ_STATE(obj, M101State)

extern void func_80041CA0(Gfx **dl, s32 arg1, Object *obj, f32 scale, f32 arg4);
extern void func_80055C00(Gfx **dl, void *tex, s32 flags, s32 offset); /* sets a texture */
extern void mtx_pop(Gfx **dl);

extern s32 D_m101_01130; /* the module's number */

/* A point of a shard: where it is from the shard's centre, and its texture coordinate (size 0xC) */
typedef struct M101ShardPoint {
    /* 0x00 */ f32 x;
    /* 0x04 */ f32 y;
    /* 0x08 */ s16 u;
    /* 0x0A */ s16 v;
} M101ShardPoint;

/* A shard (size 0x48) */
typedef struct M101Shard {
    /* 0x00 */ s16 rotation[3];
    /* 0x06 */ s16 spin[3];
    /* 0x0C */ f32 x;
    /* 0x10 */ f32 y;
    /* 0x14 */ f32 z;
    /* 0x18 */ f32 vx;
    /* 0x1C */ f32 vy;
    /* 0x20 */ f32 vz;
    /* 0x24 */ M101ShardPoint points[3];
} M101Shard;

extern u8 rl_m101_01040[40][6]; /* .data: the shards' corners on the pane, in 32nds of its size */

static void func_m101_0056C(Object *obj, M101Pane *pane, s32 updateRate);

/* Breaks the pane: makes the 40 shards (their corners, texture coordinates, spin and speed) and their triangles */
static void func_m101_00000(Object *obj, M101Pane *pane) {
    Triangle *tris;
    M101Texture *tex;
    M101Shard *s;
    u8 *corner;
    Vertex *v;
    s32 i;
    s32 j;
    u32 w16;
    s32 w8;
    u32 h16;
    s32 h8;
    s32 angle;
    f32 speed;
    s32 k;

    if (pane->shardTris == NULL) {
        pane->shardTris = func_8004A57C(0x1720, 0xFFFF);
    }
    if (pane->shardTris != NULL) {
        pane->shards = (M101Shard *) (pane->shardTris + 40);
        pane->shardVerts[0] = (Vertex *) (pane->shards + 40);
        pane->shardVerts[1] = pane->shardVerts[0] + 120;
        tex = pane->texture;
        if (tex != NULL) {
            w16 = tex->width << 4;
            w8 = tex->width << 3;
            h16 = tex->height << 4;
            h8 = tex->height << 3;
        } else {
            h16 = 0;
            w16 = 0;
            h8 = 0;
            w8 = 0;
        }
        s = pane->shards;
        corner = rl_m101_01040[0];
        for (i = 0; i < 40; i++) {
            s->points[0].x = ((corner[0] << 9) >> 7) + -256.0f;
            s->points[0].y = 256.0f - ((corner[1] << 9) >> 7);
            s->points[0].u = ((s32) (corner[0] * w16) >> 7) + w8;
            s->points[0].v = h8 - ((s32) (corner[1] * h16) >> 7);
            s->points[1].x = ((corner[2] << 9) >> 7) + -256.0f;
            s->points[1].y = 256.0f - ((corner[3] << 9) >> 7);
            s->points[1].u = ((s32) (corner[2] * w16) >> 7) + w8;
            s->points[1].v = h8 - ((s32) (corner[3] * h16) >> 7);
            s->points[2].x = ((corner[4] << 9) >> 7) + -256.0f;
            s->points[2].y = 256.0f - ((corner[5] << 9) >> 7);
            s->points[2].u = ((s32) (corner[4] * w16) >> 7) + w8;
            s->points[2].v = h8 - ((s32) (corner[5] * h16) >> 7);
            s->rotation[0] = 0;
            s->rotation[1] = 0;
            s->spin[0] = func_80048E2C(-0x800, 0x800);
            if (s->spin[0] > 0) {
                if (s->spin[0] < 0x100) {
                    s->spin[0] = 0x100;
                }
            } else if (s->spin[0] >= -0xFF) {
                s->spin[0] = -0x100;
            } else {
                /* (empty: an extra block here keeps -0x100 out of a saved register, as in the ROM) */
            }
            s->spin[1] = func_80048E2C(-0x800, 0x800);
            if (s->spin[1] > 0) {
                if (s->spin[1] < 0x100) {
                    s->spin[1] = 0x100;
                }
            } else if (s->spin[1] >= -0xFF) {
                s->spin[1] = -0x100;
            }
            s->spin[2] = func_80048E2C(-0x800, 0x800);
            if (s->spin[2] > 0) {
                if (s->spin[2] < 0x100) {
                    s->spin[2] = 0x100;
                }
            } else if (s->spin[2] >= -0xFF) {
                s->spin[2] = -0x100;
            }
            s->x = (s->points[2].x + (s->points[1].x + s->points[0].x)) * 0.3334f;
            s->y = (s->points[2].y + (s->points[1].y + s->points[0].y)) * 0.3334f;
            s->z = 0.0f;
            s->points[0].x -= s->x;
            s->points[1].x -= s->x;
            s->points[2].x -= s->x;
            s->points[0].y -= s->y;
            s->points[1].y -= s->y;
            s->points[2].y -= s->y;
            speed = func_80048E2C(5, 15) * 0.1f;
            angle = func_80048E2C(-0x8000, 0x7FFF);
            s->vx = func_80049C34(angle) * speed;
            s->vy = func_80048E2C(0, 20) * 0.1f;
            s->vz = func_80049C30(angle) * speed;
            s++;
            corner += 6;
        }
        i = 0;
        tris = pane->shardTris;
        k = 0;
        while (TRUE) {
            tris->vi0 = k++;
            tris->vi1 = k++;
            tris->vi2 = k++;
            tris->flags = 0x40;
            tris++;
            if (k + 3 > 24) {
                k = 0;
            }
            i++;
            if (i < 40) {
                continue;
            }
            break;
        }
        for (j = 0; j < 2; j++) {
            v = pane->shardVerts[j];
            for (k = 0; k < 120; k++) {
                v->r = 255;
                v->g = 255;
                v->b = 255;
                v->a = 255;
                v++;
            }
        }
        func_m101_0056C(obj, pane, 1);
    }
}

extern ObjectTransform *func_80041DE0(void); /* the camera */
extern u8 D_800A4FC4;

/*
 * Moves the shards on by `updateRate` frames (falling, and spinning), places their vertices and scrolls their
 * texture coordinates with the camera's angles.
 */
static void func_m101_0056C(Object *obj, M101Pane *pane, s32 updateRate) {
    ObjectTransform *cam;
    M101Texture *tex;
    Triangle *tris;
    Vertex *v;
    s32 w16;
    s32 h16;
    s32 shiftU;
    s32 shiftV;
    s32 camYaw;
    s32 camPitch;
    f32 gx;
    f32 gy;
    f32 gz;
    M101Shard *s;
    M101ShardPoint *pt;
    s32 i;
    s32 j;
    s32 du;
    s32 dv;
    f32 a;
    f32 b;
    f32 sinX;
    f32 cosX;
    f32 sinY;
    f32 cosY;
    f32 sinZ;
    f32 cosZ;

    cam = func_80041DE0();
    tex = pane->texture;
    tris = pane->shardTris;
    v = pane->shardVerts[PANE_STATE(obj)->buffer];
    s = pane->shards;
    if (tex != NULL) {
        w16 = tex->width << 4;
        h16 = tex->height << 4;
        for (shiftU = 0; tex->width >> shiftU; shiftU++) {
        }
        shiftU = 11 - shiftU;
        for (shiftV = 0; tex->width >> shiftV; shiftV++) {
        }
        shiftV = 11 - shiftV;
    } else {
        w16 = 0;
        h16 = 0;
        shiftU = 0;
        shiftV = 0;
    }
    sinY = func_80049C34(-obj->trans.rotation.x_rotation);
    cosY = func_80049C30(-obj->trans.rotation.x_rotation);
    sinZ = func_80049C34(-obj->trans.rotation.z_rotation);
    cosZ = func_80049C30(-obj->trans.rotation.z_rotation);
    gx = sinZ * cosY * 0.15f;
    gy = cosZ * cosY * -0.15f;
    gz = sinY * -0.15f;
    if (D_800A4FC4) {
        camYaw = 0;
        camPitch = 0;
    } else {
        camYaw = cam->rotation.y_rotation;
        camPitch = cam->rotation.x_rotation;
    }
    for (i = 0; i < 40; i++) {
        j = updateRate;
        while (j--) {
            s->x += s->vx;
            s->y += s->vy;
            s->z += s->vz;
            s->rotation[0] += s->spin[0];
            s->rotation[1] += s->spin[1];
            s->rotation[2] += s->spin[2];
            s->vx += gx;
            s->vy += gy;
            s->vz += gz;
        }
        sinX = func_80049C34(s->rotation[0]);
        cosX = func_80049C30(s->rotation[0]);
        sinY = func_80049C34(s->rotation[1]);
        cosY = func_80049C30(s->rotation[1]);
        sinZ = func_80049C34(s->rotation[2]);
        cosZ = func_80049C30(s->rotation[2]);
        for (j = 0, pt = s->points; j < 3; j++) {
            a = pt->x * cosZ - pt->y * sinZ;
            b = pt->x * sinZ + pt->y * cosZ;
            v->x = s->x + a * cosX + b * sinY * sinX;
            v->y = s->y + b * cosY;
            v->z = s->z + b * sinY * cosX - a * sinX;
            pt++;
            v++;
        }
        du = ((camYaw - s->rotation[0] - obj->trans.rotation.y_rotation) >> shiftU) + w16;
        dv = ((-camPitch - s->rotation[1] - obj->trans.rotation.x_rotation) >> shiftV) + h16;
        tris->uv[0][0] = s->points[0].u + du;
        tris->uv[0][1] = s->points[0].v + dv;
        tris->uv[1][0] = s->points[1].u + du;
        tris->uv[1][1] = s->points[1].v + dv;
        tris->uv[2][0] = s->points[2].u + du;
        tris->uv[2][1] = s->points[2].v + dv;
        tris++;
        s++;
    }
}

/* The pane's level entry */
typedef struct M101Setup {
    LevelObjectEntryCommon common;
    /* 0x0A */ s16 yRotation;
    /* 0x0C */ s16 xRotation;
    /* 0x0E */ s16 zRotation;
    /* 0x10 */ u8 scale;
    /* 0x11 */ u8 pad11;
    /* 0x12 */ u8 strength;
} M101Setup;

/* The model the pane's texture comes from */
typedef struct M101Model {
    /* 0x00 */ u8 pad0[0x18];
    /* 0x18 */ M101Texture **textures;
} M101Model;

extern Vertex rl_m101_01134[];  /* the quad's vertices (.data) */

/* Sets the pane up from its level entry: rotation, scale, strength, the quad's triangles and its hit box */
void func_m101_009F8(Object *obj, M101Setup *setup) {
    M101Pane *pane;
    Triangle *t;
    s32 i;
    void *inst;
    M101Model *model;

    obj->trans.rotation.y_rotation = setup->yRotation;
    pane = obj->unk68;
    obj->trans.rotation.x_rotation = setup->xRotation;
    obj->trans.rotation.z_rotation = setup->zRotation;
    t = pane->tris[0];
    obj->trans.scale = obj->header->scale * setup->scale * 0.01f;
    PANE_STATE(obj)->strength = setup->strength << 8;
    if (PANE_STATE(obj)->strength <= 0) {
        PANE_STATE(obj)->strength = 0x100;
    }
    PANE_STATE(obj)->buffer = 1;
    inst = obj->modelInstances[0];
    model = *(M101Model **) inst;
    pane->texture = model->textures[0];
    pane->quadTris[0] = t;
    pane->quadTris[1] = t + 2;
    pane->quadVerts = rl_m101_01134;
    for (i = 0; i < 2; i++) {
        t = pane->quadTris[i];
        t->flags = 0x40;
        t->vi0 = 0;
        t->vi1 = 1;
        t->vi2 = 2;
        t++;
        t->flags = 0x40;
        t->vi0 = 0;
        t->vi1 = 2;
        t->vi2 = 3;
    }
    func_80079D1C(obj, -1, 0, 8, 0, 0, 0, 32.0f, 32.0f, -1.0f, 1.0f, 1);
}

/* What obj->unk5C points at for the pane */
typedef struct M101Hits {
    /* 0x000 */ u8 pad0[0x12A];
    /* 0x12A */ s8 hit;
    /* 0x12B */ u8 pad12B[0x130 - 0x12B];
    /* 0x130 */ s16 damage;
} M101Hits;


/*
 * Before it breaks: scrolls the quad's texture with the pane's and the camera's angles and takes
 * damage. Once broken: moves the shards and, after three seconds, frees itself or stops drawing.
 */
void func_m101_00B64(Object *obj, s32 updateRate) {
    s32 u;
    s32 v;
    s32 w;
    s32 h;
    ObjectTransform *cam;
    M101Pane *pane;
    M101Hits *hits;
    Triangle *t;
    s16 yaw;
    s16 pitch;

    runlink_unload_timer_set(D_m101_01130, 2, 2);
    if (obj->trans.flags & 0x400) {
        return;
    }
    pane = obj->unk68;
    hits = (M101Hits *) obj->unk5C;
    cam = func_80041DE0();
    PANE_STATE(obj)->buffer = 1 - PANE_STATE(obj)->buffer;
    if (PANE_STATE(obj)->strength <= 0) {
        PANE_STATE(obj)->strength -= updateRate;
        if (-PANE_STATE(obj)->strength >= 181) {
            if (obj->level_entry != NULL && (((u8 *) obj->level_entry)[3] & 1)) {
                obj->trans.flags |= 0x400;
            } else {
                func_800079B8(obj);
            }
        } else {
            func_m101_0056C(obj, pane, updateRate);
        }
        return;
    }
    t = pane->quadTris[PANE_STATE(obj)->buffer];
    u = pane->texture->width;
    v = pane->texture->height;
    yaw = -obj->trans.rotation.y_rotation;
    w = u << 4;
    h = v << 4;
    pitch = -obj->trans.rotation.x_rotation;
    if (!D_800A4FC4) {
        yaw += cam->rotation.y_rotation;
        pitch -= cam->rotation.x_rotation;
    }
    u = (u << 4) + ((u * yaw) >> 11);
    v = (v << 4) + ((v * pitch) >> 11);
    t->uv[0][0] = u + w;
    t->uv[0][1] = v - h;
    t->uv[1][0] = u + w;
    t->uv[1][1] = v + h;
    t->uv[2][0] = u - w;
    t->uv[2][1] = v + h;
    t++;
    t->uv[0][0] = u + w;
    t->uv[0][1] = v - h;
    t->uv[1][0] = u - w;
    t->uv[1][1] = v + h;
    t->uv[2][0] = u - w;
    t->uv[2][1] = v - h;
    if (hits->hit != 0) {
        PANE_STATE(obj)->strength -= hits->damage;
        if (PANE_STATE(obj)->strength <= 0) {
            PANE_STATE(obj)->strength = 0;
            obj->interactObj->flags &= ~1;
            func_m101_00000(obj, pane);
            func_80003410(0xD3, obj->trans.x_position, obj->trans.y_position, obj->trans.z_position, 4, NULL);
            if (pane->shards == NULL) {
                func_800079B8(obj);
            }
        }
    }
}

void func_m101_00DCC(Gfx **dl, s32 arg1, Object *obj) {
    M101Pane *pane;
    u8 *tris;
    Vertex *verts;
    s32 left;
    s32 numVerts;
    s32 numTris;

    runlink_unload_timer_set(D_m101_01130, 2, 2);
    pane = obj->unk68;
    func_80041CA0(dl, arg1, obj, 1.0f, 0.0f);
    func_80055C00(dl, pane->texture, 0xF, 0);
    gDPSetPrimColor((*dl)++, 0, 0, 255, 255, 255, 255);
    gDPSetEnvColor((*dl)++, 255, 255, 255, 255);
    left = 120;
    if (PANE_STATE(obj)->strength > 0) {
        gSPVertexJFG((*dl)++, OS_K0_TO_PHYSICAL(pane->quadVerts), 5, 0);
        gSPPolygon((*dl)++, OS_K0_TO_PHYSICAL(pane->quadTris[PANE_STATE(obj)->buffer]), 2, 1);
    } else {
        verts = pane->shardVerts[PANE_STATE(obj)->buffer];
        tris = (u8 *) pane->shardTris;
        do {
            if (left > 24) {
                numVerts = 24;
                numTris = 8;
                left -= 24;
            } else {
                numVerts = left;
                numTris = left / 3;
                left = 0;
            }
            gSPVertexJFGLast((*dl)++, OS_K0_TO_PHYSICAL(verts), numVerts, 0);
            gSPPolygon((*dl)++, OS_K0_TO_PHYSICAL(tris), numTris, 1);
            verts += numVerts;
            tris += numTris * 16; /* sizeof (Triangle) */
        } while (left != 0);
    }
    mtx_pop(dl);
}

INCLUDE_ASM("asm/nonmatchings/modules/m101", data);
