/*
 * Runlink module 67: an enemy and its missiles (still mostly assembly).
 */
#include "module.h"
#include "gfx.h"

/* A missile's state (obj->unk68), as far as used here */
typedef struct M067Missile {
    /* 0x0 */ u8 pad0[4];
    /* 0x4 */ f32 thrust;
    /* 0x8 */ void *flame;
} M067Missile;

/* A jump in progress */
typedef struct M067Jump {
    /* 0x00 */ u8 pad0;
    /* 0x01 */ s8 kind;
    /* 0x02 */ s8 target;
    /* 0x03 */ u8 pad3[0x10 - 0x3];
    /* 0x10 */ Vec3f start;
    /* 0x1C */ s16 yRotation;
    /* 0x1E */ s16 xRotation;
    /* 0x20 */ Vec3f dest;
    /* 0x2C */ s16 yRotation2;
    /* 0x2E */ s16 xRotation2;
} M067Jump;

/* An entry of the jump table (size 8) */
typedef struct M067JumpKind {
    /* 0x0 */ s32 anim;
    /* 0x4 */ s32 unk4;
} M067JumpKind;

/* The set-up of a shot (a bolt or a rock) */
typedef struct M067ShotSetup {
    LevelObjectEntryCommon common;
    /* 0x0A */ u8 strength;
    /* 0x0B */ u8 time;
    /* 0x0C */ s16 yRotation;
    /* 0x0E */ s16 xRotation;
    /* 0x10 */ f32 scale;
    /* 0x14 */ f32 speed;
    /* 0x18 */ s32 unk18;
} M067ShotSetup;

typedef struct M067Shot {
    /* 0x0 */ s32 unk0;
    /* 0x4 */ s16 timer;
    /* 0x6 */ u8 strength;
    /* 0x7 */ u8 unk7;
} M067Shot;

/* A jump point's data (the unk68 of an object with behaviour 0x68) */
typedef struct M067JumpPoint {
    /* 0x00 */ u8 pad0[0x14];
    /* 0x14 */ s16 id;
} M067JumpPoint;

extern M067JumpKind rl_m067_04430[];
extern s8 rl_m067_04580[][8]; /* the jump kind between two jump points (-1: none) */
extern s16 func_80049D80(s16 angle, s16 target);

/* The enemy's state (obj->unk68), as far as used here */
typedef struct M067State {
    /* 0x00 */ s8 side;
    /* 0x01 */ s8 kind;
    /* 0x02 */ s8 target;
    /* 0x03 */ u8 unk3;
    /* 0x04 */ s32 unk4; /* from the pair's table (0025C) */
    /* 0x08 */ u8 unk8;
    /* 0x09 */ u8 unk9;
    /* 0x0A */ u8 unkA;
    /* 0x0B */ u8 unkB;
    /* 0x0C */ u8 unkC;
    /* 0x0D */ u8 unkD;
    /* 0x0E */ u8 unkE;
    /* 0x0F */ s8 unkF; /* a countdown; -1: stopped */
    /* 0x10 */ u8 pad10[0x34 - 0x10];
    /* 0x34 */ s32 unk34;
    /* 0x38 */ s32 unk38;
    /* 0x3C */ u8 unk3C[6];
} M067State;

typedef struct M067Setup {
    LevelObjectEntryCommon common;
    /* 0x0A */ s8 side;
    /* 0x0B */ s8 yRotation; /* in 256ths of a turn */
} M067Setup;

/* The hit box numbers in the object's header */
typedef struct M067Header {
    /* 0x00 */ u8 pad0[0xB4];
    /* 0xB4 */ s16 hitX;
    /* 0xB6 */ s16 hitY;
    /* 0xB8 */ s16 hitZ;
    /* 0xBA */ u8 padBA[0xBC - 0xBA];
    /* 0xBC */ f32 hitRadius;
    /* 0xC0 */ f32 hitHeight;
} M067Header;

/* The level entry of the dust the enemy raises when it lands */
typedef struct M067DustEntry {
    /* 0x00 */ s16 objectID;
    /* 0x02 */ u8 pad2[0x4 - 0x2];
    /* 0x04 */ s16 x;
    /* 0x06 */ s16 y;
    /* 0x08 */ s16 z;
    /* 0x0A */ s16 yRotation;
    /* 0x0C */ s16 xRotation;
    /* 0x0E */ s16 unkE;
    /* 0x10 */ f32 scale;
} M067DustEntry;

/* What obj->interactObj points at here */
typedef struct M067HitShape {
    /* 0x00 */ u8 pad0[0x24];
    /* 0x24 */ f32 radius;
} M067HitShape;

extern void spawn_object_from_list(s32 kind, s16 x, s16 y, s16 z, s32 arg4, s32 arg5); /* a burst */
extern void func_80009A04(Object *obj, f32 dx, f32 dy, f32 dz); /* DKR objMoveXYZ */

/* The level's fog */
extern void func_8001BC2C(s32 arg0, s16 *fogNear, s16 *fogFar, s16 *arg3, u8 *r, u8 *g, u8 *b, u8 *arg7);

extern Object *rl_m067_04574[2]; /* the two enemies of a pair */
extern s32 rl_m067_0456C;
extern void rl_m067_0025C(void);
extern s32 runlink_module_loaded(s32 arg0);
extern void func_m039_00000(void);
extern f32 get_distance_to_active_camera(f32 x, f32 y, f32 z); /* distance to the camera */
extern void cam_shake_start(s32 arg0, f32 arg1, f32 arg2, f32 arg3, s32 strength); /* shakes the camera */
extern Object *func_80005F14(void *entry, s32 arg1); /* DKR spawn_object */

extern void func_80041CA0(Gfx **dl, s32 arg1, Object *obj, f32 scale, f32 arg4);
extern void func_80069BE8(Gfx **dl, void *flame, s32 arg2, s32 alpha);
extern void mtx_pop(Gfx **dl);
extern void effect_texture_pair_free(void *flame);

void func_m067_00000(UNUSED s32 arg0) {
}

/*
 * Finds the jump point with the given id (its position and rotation go to the
 * pointers) and returns the jump kind from point `from` to it (2 with no `from`)
 */
static s32 func_m067_00008(s32 from, s32 to, f32 *x, f32 *y, f32 *z, s16 *yRotation, s16 *xRotation) {
    s32 first;
    s32 count;
    Object **objs;
    Object *o;
    M067JumpPoint *p;

    objs = func_80005E98(&first, &count);
    count -= first;
    objs = first + objs;
    while (count--) {
        o = *objs++;
        if (o->behaviorId == 0x68 && (p = o->unk68)->id == to) {
            *x = o->trans.x_position;
            *y = o->trans.y_position;
            *z = o->trans.z_position;
            *yRotation = o->trans.rotation.y_rotation;
            *xRotation = o->trans.rotation.x_rotation;
        }
    }
    if (from != -1) {
        return rl_m067_04580[from][to];
    }
    return 2;
}

/* Starts a jump to target (an index into the jump table); returns whether there was one */
s32 func_m067_00130(Object *obj, M067Jump *j, s32 target) {
    s32 kind;

    kind = func_m067_00008(j->target, target, &j->dest.x, &j->dest.y, &j->dest.z, &j->yRotation2, &j->xRotation2);
    if (kind != -1) {
        j->kind = kind;
        j->target = target;
        obj_anim_set(obj, rl_m067_04430[j->kind].anim, 0.0f);
        j->start.x = obj->trans.x_position;
        j->dest.x -= obj->trans.x_position;
        j->start.y = obj->trans.y_position;
        j->dest.y -= obj->trans.y_position;
        j->start.z = obj->trans.z_position;
        j->dest.z -= obj->trans.z_position;
        j->yRotation = obj->trans.rotation.y_rotation;
        j->yRotation2 = func_80049D80(obj->trans.rotation.y_rotation, j->yRotation2);
        j->xRotation = obj->trans.rotation.x_rotation;
        j->xRotation2 = func_80049D80(obj->trans.rotation.x_rotation, j->xRotation2);
        return TRUE;
    }
    return FALSE;
}

extern s32 rl_m067_046C0[]; /* .data (still assembly): the pair's values, two per step */

/* Gives both enemies of the pair their next values from the table and moves the table on */
void func_m067_0025C(void) {
    M067State *s;

    if (rl_m067_04574[0] != NULL) {
        s = rl_m067_04574[0]->unk68;
        s->unk4 = rl_m067_046C0[rl_m067_0456C];
        s->unk8 = 0;
        if (s->unkF == -1) {
            s->unkF = 12;
        }
    }
    if (rl_m067_04574[1] != NULL) {
        s = rl_m067_04574[1]->unk68;
        s->unk4 = rl_m067_046C0[rl_m067_0456C + 1];
        s->unk8 = 0;
        if (s->unkF == -1) {
            s->unkF = 12;
        }
    }
    rl_m067_0456C += 2;
}

INCLUDE_ASM("asm/nonmatchings/modules/m067/ro", func_m067_00300);

extern void rl_m067_00300(Object *obj, M067State *s, s32 step);

/*
 * Runs the next step of the enemy's step list (s->unk4): -1 goes back to the
 * start and on to the first -19, a -19 and a step equal to the target are
 * skipped
 */
#ifdef NON_MATCHING
/* The first block's addu and beq operand order and one store's slot */
void func_m067_0064C(Object *obj, M067State *s) {
    s8 step;
    s8 *seq;

    step = (seq = (s8 *) s->unk4)[s->unk8];
    if (step == -1) {
        s->unk8 = 0;
        step = seq[s->unk8];
        while (step != -19) {
            s->unk8++;
            step = seq[s->unk8];
        }
    }
    if (step == -19) {
        s->unk8++;
        step = ((s8 *) s->unk4)[s->unk8];
    }
    if (step == s->target) {
        s->unk8++;
        step = ((s8 *) s->unk4)[s->unk8];
    }
    rl_m067_00300(obj, s, step);
    s->unk8++;
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m067", func_m067_0064C);
#endif

/* Landing: shakes the camera by how near it is, plays the thud and raises dust */
void func_m067_00728(Object *obj, s32 updateRate) {
    f32 dist;
    s32 strength;
    M067DustEntry entry;
    Object *dust;

    dist = get_distance_to_active_camera(obj->trans.x_position, obj->trans.y_position, obj->trans.z_position);
    if (dist < 2000.0f) {
        if (dist < 800.0f) {
            strength = 20;
        } else {
            strength = (2000.0f - dist) * 20.0f / 1200.0f;
        }
        cam_shake_start(0, 0.0f, 0.0f, 2.0f, strength);
    }
    func_80003410(0x1B9, obj->trans.x_position, obj->trans.y_position, obj->trans.z_position, 4, NULL);
    entry.objectID = 0x13B;
    entry.x = obj->trans.x_position;
    entry.y = obj->trans.y_position;
    entry.z = obj->trans.z_position;
    entry.yRotation = 0;
    entry.xRotation = -0x4000;
    entry.unkE = 45;
    entry.scale = 1.0f;
    dust = func_80005F14(&entry, 1);
    if (dust != NULL) {
        dust->level_entry = NULL;
    }
}

INCLUDE_ASM("asm/nonmatchings/modules/m067/ro", func_m067_008C4);

#ifdef NON_MATCHING
/* Registers: the ROM keeps the state in v1, the pair in v0 and the header in t0 (t0/t1 taken) */
void func_m067_00D70(Object *obj, M067Setup *setup) {
    M067Header *header;
    M067State *s = obj->unk68;

    obj->trans.rotation.y_rotation = setup->yRotation << 8;
    s->side = setup->side;
    rl_m067_04574[s->side & 1] = obj;
    s->kind = 0;
    s->target = -1;
    s->unk3 = 0;
    if (rl_m067_04574[0] != NULL && rl_m067_04574[1] != NULL) {
        rl_m067_0456C = 0;
        rl_m067_0025C();
    }
    s->unkF = 60;
    s->unk9 = 0;
    s->unkA = 0;
    s->unkB = 0;
    s->unkC = 0;
    s->unkD = 0;
    s->unkE = 0;
    obj_anim_set(obj, rl_m067_04430[s->kind].anim, 0.0f);
    s->unk34 = 0;
    s->unk38 = 0;
    s->unk3C[0] = 255;
    s->unk3C[1] = 255;
    s->unk3C[2] = 255;
    s->unk3C[3] = 255;
    s->unk3C[4] = 255;
    s->unk3C[5] = 255;
    header = (M067Header *) obj->header;
    func_80079D1C(obj, 0xF00, 1, 1, header->hitX, header->hitY, header->hitZ, header->hitRadius,
                  header->hitHeight, -1.0f, 1.0f, 1);
    ((u8 *) obj->interactObj)[0x31] = 255;
    if (!runlink_module_loaded(0x27)) {
        func_m039_00000();
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m067", func_m067_00D70);
#endif

INCLUDE_ASM("asm/nonmatchings/modules/m067/ro", func_m067_00EF4);

/* Sets the fog for drawing one of the enemy's parts: a flash colour, or the level's fog */
void func_m067_02BF0(Gfx **dl, Object *obj, s32 part) {
    s16 fogNear;
    s16 fogFar;
    s16 unused;
    u8 r;
    u8 g;
    u8 b;
    u8 unused2;
    s32 flags;
    M067State *s;

    if (part >= 0 && part < 6) {
        s = obj->unk68;
        flags = (&s->unk9)[part];
        if (flags & 0x40) {
            gDPSetFogColor((*dl)++, 255, 255, 255, 255);
            gSPFogFactor((*dl)++, 0x500, 0x500);
        } else if (flags & 0x10) {
            gDPSetFogColor((*dl)++, 0x32, 0xFF, 0x32, 0xFF);
            gSPFogFactor((*dl)++, 0x500, 0x500);
        } else if (flags & 4) {
            gDPSetFogColor((*dl)++, 0xFF, 0x32, 0x32, 0xFF);
            gSPFogFactor((*dl)++, 0x500, 0x500);
        } else {
            func_8001BC2C(0, &fogNear, &fogFar, &unused, &r, &g, &b, &unused2);
            gDPSetFogColor((*dl)++, r, g, b, 255);
            gSPFogPosition((*dl)++, fogNear, fogFar);
        }
    }
}

/* A missile's set-up */
typedef struct M067MissileSetup {
    LevelObjectEntryCommon common;
    /* 0x0A */ s16 yRotation;
    /* 0x0C */ s16 xRotation;
    /* 0x0E */ s16 zRotation;
    /* 0x10 */ u8 scale;
    /* 0x11 */ s8 speedY;
    /* 0x12 */ u16 unk12;
    /* 0x14 */ s16 unk14;
    /* 0x16 */ s16 unk16;
    /* 0x18 */ s16 unk18;
    /* 0x1A */ s16 unk1A;
    /* 0x1C */ s32 unk1C;
} M067MissileSetup;

/* A missile's whole state (obj->unk68) */
typedef struct M067MissileState {
    /* 0x00 */ u8 unk0;
    /* 0x01 */ s8 unk1;
    /* 0x02 */ u8 unk2;
    /* 0x03 */ u8 unk3;
    /* 0x04 */ f32 thrust;
    /* 0x08 */ void *flame;
    /* 0x0C */ f32 unkC;
    /* 0x10 */ f32 unk10;
    /* 0x14 */ s16 unk14;
    /* 0x16 */ s16 unk16;
    /* 0x18 */ s16 unk18;
    /* 0x1A */ s16 unk1A;
    /* 0x1C */ s16 unk1C;
    /* 0x1E */ s16 unk1E;
    /* 0x20 */ s32 unk20;
} M067MissileState;

/* A model vertex (size 0xA) */
typedef struct M067Vertex {
    /* 0x0 */ s16 x;
    /* 0x2 */ s16 y;
    /* 0x4 */ s16 z;
    /* 0x6 */ u8 pad6[4];
} M067Vertex;

/* The missile's model */
typedef struct M067Model {
    /* 0x00 */ u8 pad0[0x1C];
    /* 0x1C */ M067Vertex *vertices;
    /* 0x20 */ u8 pad20[0x30 - 0x20];
    /* 0x30 */ u16 *flameVertex; /* the vertex the flame comes from */
} M067Model;

extern void *effect_mesh_create(s16 x, s16 y, s16 z, s32 arg3, s32 arg4, f32 arg5, f32 arg6, s32 arg7, s32 arg8, s32 arg9);

#ifdef NON_MATCHING
/* Scheduling: from the speedY conversion on, every t-register is one lower than the ROM's */
void func_m067_02E38(Object *obj, M067MissileSetup *setup) {
    f32 k;
    M067Model *model;
    M067Vertex *v;
    M067MissileState *m;

    k = 0.1f;
    obj->trans.rotation.y_rotation = setup->yRotation;
    obj->trans.rotation.x_rotation = setup->xRotation;
    m = obj->unk68;
    obj->trans.rotation.z_rotation = setup->zRotation;
    obj->trans.scale = setup->scale * k;
    obj->velocity.x = 0.0f;
    obj->velocity.y = setup->speedY * k;
    obj->velocity.z = 0.0f;
    m->unkC = 0.0f;
    m->unk10 = setup->unk12 * k;
    m->unk1E = 0;
    m->unk14 = setup->unk14;
    m->unk16 = setup->unk16;
    m->unk18 = setup->unk18;
    m->unk1A = 0;
    m->unk1C = 0;
    m->unk1E = 0;
    m->unk20 = setup->unk1C;
    model = *(M067Model **) obj->modelInstances[0];
    v = &model->vertices[*model->flameVertex];
    m->thrust = 0.0f;
    m->unk2 = 0;
    m->unk3 = 0;
    m->flame = effect_mesh_create(v->x, v->y, v->z, -0x8000, 0, 20.0f, 200.0f, 8, 16, -1);
    m->unk0 = 0;
    m->unk1 = setup->unk1A * 6;
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m067/ro", func_m067_02E38);
#endif

INCLUDE_ASM("asm/nonmatchings/modules/m067/ro", func_m067_02FDC);

/* Draws a missile's flame */
void func_m067_03808(Gfx **dl, s32 arg1, Object *obj) {
    M067Missile *m = obj->unk68;

    if (m->flame != NULL && m->thrust > 0.0f) {
        func_80041CA0(dl, arg1, obj, 1.0f, 0.0f);
        func_80069BE8(dl, m->flame, 6, 255);
        mtx_pop(dl);
    }
}

/* Frees a missile's flame */
void func_m067_03888(Object *obj) {
    M067Missile *m = obj->unk68;

    if (m->flame != NULL) {
        effect_texture_pair_free(m->flame);
    }
}

void func_m067_038C0(Object *obj, M067ShotSetup *setup) {
    M067Shot *s = obj->unk68;

    obj->trans.rotation.y_rotation = setup->yRotation;
    obj->trans.rotation.x_rotation = setup->xRotation;
    obj->velocity.z = -setup->speed;
    func_8004944C(&obj->trans.rotation, &obj->velocity);
    obj->trans.scale = setup->scale;
    s->timer = setup->time * 6;
    s->unk0 = setup->unk18;
    s->strength = setup->strength;
    s->unk7 = 0;
    func_80079D1C(obj, s->strength << 8, 1, 4, 0, 0, 0, 80.0f, 80.0f, 100.0f, 1.0f, 0x11);
}

/* A bolt: spins and flies until its time is up, and bursts when it hits something */
void func_m067_0399C(Object *obj, s32 updateRate) {
    f32 rate;
    f32 radius;
    Vec3f from;
    HitResult hit;
    M067Shot *s = obj->unk68;

    rate = updateRate;
    if (obj->interactObj->hitPoints <= 0 || s->unk7) {
        spawn_object_from_list(3, obj->trans.x_position, obj->trans.y_position, obj->trans.z_position, 100, 0);
        func_800079B8(obj);
        return;
    }
    s->timer -= updateRate;
    if (s->timer < 0) {
        func_800079B8(obj);
        return;
    }
    obj->trans.rotation.z_rotation += updateRate << 12;
    from.x = obj->trans.x_position;
    from.y = obj->trans.y_position;
    from.z = obj->trans.z_position;
    func_80009A04(obj, obj->velocity.x * rate, obj->velocity.y * rate, obj->velocity.z * rate);
    radius = ((M067HitShape *) obj->interactObj)->radius;
    func_8001CE10(1, &from, (Vec3f *) &obj->trans.x_position, &radius, 0xCE002000, 0, 0);
    if (func_80016208(obj, &from, (Vec3f *) &obj->trans.x_position, radius, 1, &hit)) {
        s->unk7 = 1;
    }
}

extern void func_80055C00(Gfx **dl, void *tex, s32 flags, s32 offset); /* sets a texture */
extern void func_80041DC0(Gfx **dl);
extern Vertex rl_m067_04CC8[];   /* the bolt's vertices (.data) */
extern Triangle rl_m067_04C88[]; /* its triangles (.data) */

/* Draws a bolt: five copies along its path, growing and fading */
void func_m067_03B7C(Gfx **dl, s32 arg1, Object *obj) {
    M067Shot *s;
    s32 kind;
    s32 i;
    f32 scale;
    f32 savedScale;
    s32 savedRot;
    f32 savedX;
    f32 savedY;
    f32 savedZ;
    s32 b;
    s32 g;
    s32 a;

    s = obj->unk68;
    if (s->strength == 2) {
        kind = 0;
    } else {
        kind = 1;
    }
    savedRot = obj->trans.rotation.z_rotation;
    savedX = obj->trans.x_position;
    savedY = obj->trans.y_position;
    savedZ = obj->trans.z_position;
    savedScale = obj->trans.scale;
    obj->trans.x_position -= 4.0f * obj->velocity.x;
    obj->trans.y_position -= 4.0f * obj->velocity.y;
    obj->trans.z_position -= 4.0f * obj->velocity.z;
    scale = 0.5f;
    b = 0xFF;
    g = 0;
    a = 0x33;
    i = 5;
    func_80055C00(dl, obj->modelInstances[kind], 0x207, 0);
    while (i--) {
        obj->trans.scale = scale * savedScale;
        func_80041CA0(dl, arg1, obj, 1.0f, 0.0f);
        if (kind == 0) {
            gDPSetPrimColor((*dl)++, 0, 0, 255, 0, b, a);
            gDPSetEnvColor((*dl)++, 255, 255, 255, 255);
        } else {
            gDPSetPrimColor((*dl)++, 0, 0, 0, b, g, a);
            gDPSetEnvColor((*dl)++, 255, 255, 255, 255);
        }
        gSPVertexJFG((*dl)++, OS_K0_TO_PHYSICAL(rl_m067_04CC8), 6, 0);
        gSPPolygon((*dl)++, OS_K0_TO_PHYSICAL(rl_m067_04C88), 4, 1);
        gDPPipeSync((*dl)++);
        func_80041DC0(dl);
        obj->trans.x_position += obj->velocity.x;
        obj->trans.y_position += obj->velocity.y;
        obj->trans.z_position += obj->velocity.z;
        obj->trans.rotation.z_rotation += 0x800;
        scale += 0.125f;
        b -= 0x3D;
        g += 0x3D;
        a += 0x33;
    }
    gDPSetPrimColor((*dl)++, 0, 0, 255, 255, 255, 255);
    obj->trans.rotation.z_rotation = savedRot;
    obj->trans.x_position = savedX;
    obj->trans.y_position = savedY;
    obj->trans.z_position = savedZ;
    obj->trans.scale = savedScale;
}

void func_m067_03F08(Object *obj, M067ShotSetup *setup) {
    M067Shot *s;

    obj->trans.rotation.y_rotation = setup->yRotation;
    obj->trans.rotation.x_rotation = setup->xRotation;
    obj->trans.rotation.z_rotation = 0;
    s = obj->unk68;
    obj->trans.scale *= setup->scale;
    obj->velocity.z = -setup->speed;
    func_8004944C(&obj->trans.rotation, &obj->velocity);
    s->unk0 = setup->unk18;
    s->timer = setup->time * 6;
    s->strength = setup->strength;
    s->unk7 = 0;
    func_80079D1C(obj, s->strength << 8, 1, 4, 0, 0, 0, 10.0f, 10.0f, 100.0f, 1.0f, 0x11);
}

INCLUDE_ASM("asm/nonmatchings/modules/m067/ro", func_m067_03FF0);

INCLUDE_ASM("asm/nonmatchings/modules/m067", data);

/* (.rodata: C, and the assembly functions' own from ro/) */

