/*
 * Runlink module 22: the control plate (a level object that hands the player
 * control of a helper character) and the helper's task: its states, its goals,
 * the pause screen with its letterbox, and the plate's camera.
 * Family: other object behaviours (docs/runlink.md, "Module families").
 */
#include "module.h"

/* The plate's behaviour data */
typedef struct M022Plate {
    /* 0x00 */ f32 radius;
    /* 0x04 */ f32 yMin;
    /* 0x08 */ f32 yMax;
    /* 0x0C */ s32 unkC;
    /* 0x10 */ s32 sound; /* a positioned sound's handle, or 0 */
    /* 0x14 */ u8 unk14;
    /* 0x15 */ u8 unk15;
    /* 0x16 */ u8 unk16;
    /* 0x17 */ u8 unk17;
    /* 0x18 */ u8 bob; /* the angle of the bobbing seen by the player it carries */
    /* 0x19 */ u8 unk19;
    /* 0x1A */ s8 unk1A;
    /* 0x1B */ u8 unk1B;
    /* 0x1C */ Object *marker; /* the marker shown over the plate (00000) */
} M022Plate;

/* The plate's level entry */
typedef struct M022PlateEntry {
    /* 0x00 */ LevelObjectEntryCommon common;
    /* 0x0A */ u8 radius;
    /* 0x0B */ s8 yMin;
    /* 0x0C */ s8 yMax;
    /* 0x0D */ u8 unkD;
} M022PlateEntry;

/* The object header as the plate reads it */
typedef struct M022Header {
    /* 0x00 */ u8 pad0[0xAC];
    /* 0x0AC */ s32 *unkAC;
} M022Header;

/* The helper (the player-like object the task runs on) */
typedef struct M022Helper {
    /* 0x000 */ u8 pad0[0x10D];
    /* 0x10D */ u8 state;
    /* 0x10E */ u8 pad10E[0x50C - 0x10E];
    /* 0x50C */ Object *obj;
} M022Helper;

/* The helper object's behaviour data, as far as the task uses it */
typedef struct M022HelperState {
    /* 0x00 */ f32 x; /* where its task starts */
    /* 0x04 */ f32 y;
    /* 0x08 */ f32 z;
    /* 0x0C */ u8 padC[0x1C - 0xC];
    /* 0x1C */ u16 unk1C;
    /* 0x1E */ s16 unk1E;
    /* 0x20 */ u8 pad20[0x60 - 0x20];
    /* 0x60 */ s16 yaw;
    /* 0x62 */ s16 pitch;
    /* 0x64 */ s16 roll;
    /* 0x66 */ u8 pad66[0x90 - 0x66];
    /* 0x90 */ s16 unk90;
} M022HelperState;

/* A level object of behaviour 0x68's data, as 02310 uses it */
typedef struct M022Start {
    /* 0x00 */ u8 pad0[0x16];
    /* 0x16 */ s16 unk16;
} M022Start;

extern void func_80003614(s32 handle);             /* stops a positioned sound */
extern ObjectTransform *func_80041DE0(void);       /* the camera */
extern void transition_values_start(s32 x0, s32 y0, s32 x1, s32 y1, s32 x2, s32 y2, s32 x3, s32 y3, s32 speed); /* letterbox */
extern void set_game_paused(s32 paused);
extern f32 func_80097140(f32 x);                   /* sqrtf */
extern s32 func_80049C84(f32 a, f32 b);            /* the angle of (a, b) */

extern s32 D_800A320C;
extern s32 D_800A3210[3];
extern u8 D_800A3208;
extern s16 D_801046EE;
extern s16 D_801046F4;

/* .data, still assembly */
extern Object *rl_m022_03418; /* the helper's plate object */
extern u8 rl_m022_03424;
extern s16 rl_m022_03428;
extern s16 rl_m022_0342C;
extern s16 rl_m022_03430;
extern s8 rl_m022_03434;
extern u8 rl_m022_03438;
extern u8 rl_m022_0343C;
extern u8 rl_m022_03440;

/* .bss, still assembly: the plate camera */
extern f32 rl_m022_035B0;
extern f32 rl_m022_035B4;
extern f32 rl_m022_035B8;
extern f32 rl_m022_035BC;
extern f32 rl_m022_035C0;
extern s16 rl_m022_035C4;
extern s16 rl_m022_035C6;
extern s16 rl_m022_035C8;
#define sPlateCamX rl_m022_035B0
#define sPlateCamY rl_m022_035B4
#define sPlateCamZ rl_m022_035B8
#define sPlateCamDist rl_m022_035BC /* the distance to the camera, flat */
#define sPlateCamDy rl_m022_035C0
#define sPlateCamYaw rl_m022_035C4
#define sPlateCamUnk16 rl_m022_035C6
#define sPlateCamUnk18 rl_m022_035C8

extern LevelObjectEntryCommon rl_m022_03444; /* .data (still assembly): the marker's level entry */
extern Object *func_80005F14(void *entry, s32 arg1); /* DKR spawn_object */

/* The marker's data, as 00000 uses it */
typedef struct M022Marker {
    /* 0x0 */ s16 unk0;
    /* 0x2 */ s16 alpha;
} M022Marker;

/* Spawns the plate's marker when it is due, and fades it in while unk17 is set, out otherwise */
void func_m022_00000(Object *obj, M022Plate *plate, s32 updateRate) {
    M022Marker *m;

    if (plate->unk1B) {
        rl_m022_03444.x = obj->trans.x_position;
        rl_m022_03444.y = obj->trans.y_position + 70.0f;
        rl_m022_03444.z = obj->trans.z_position;
        plate->unk1B = 0;
        plate->marker = func_80005F14(&rl_m022_03444, 1);
    }
    if (plate->marker != NULL) {
        m = plate->marker->unk68;
        if (plate->unk17) {
            m->alpha += updateRate * 4;
            if (m->alpha >= 256) {
                m->alpha = 255;
            }
        } else {
            m->alpha -= updateRate * 16;
            if (m->alpha < 0) {
                m->alpha = 0;
            }
        }
    }
}

void func_m022_00154(Object *obj, M022PlateEntry *entry) {
    M022Plate *plate = obj->unk68;

    plate->radius = entry->radius;
    plate->yMin = obj->trans.y_position + entry->yMin;
    plate->yMax = obj->trans.y_position + entry->yMax;
    plate->unkC = 0;
    plate->sound = 0;
    plate->unk14 = 0;
    plate->unk15 = entry->unkD;
    plate->unk16 = 0;
    plate->unk17 = 0;
    plate->unk19 = 0;
    plate->unk1A = -1;
    plate->unk1B = 1;
    plate->marker = NULL;
    D_800A320C = ((M022Header *) obj->header)->unkAC[0];
    D_800A3210[0] = ((M022Header *) obj->header)->unkAC[1];
    D_800A3210[1] = ((M022Header *) obj->header)->unkAC[2];
    D_800A3210[2] = ((M022Header *) obj->header)->unkAC[3];
    obj_collision_spheres_init(obj, -1, 0, 8, 0, 0, 0, 32.0f, 32.0f, -1.0f, 1.0f, 1);
    OBJ_STATE(obj, s32)[0] = 0;
}

INCLUDE_ASM("asm/nonmatchings/modules/m022/ro", func_m022_002A4);

void func_m022_00EA0(Object *obj) {
    M022Plate *plate;

    if (rl_m022_03418 != NULL) {
        func_800079B8(rl_m022_03418);
        rl_m022_03418 = NULL;
    }
    D_800A320C = 0;
    D_800A3210[0] = 0;
    D_800A3210[1] = 0;
    D_800A3210[2] = 0;
    plate = obj->unk68;
    if (plate->sound != 0) {
        func_80003614(plate->sound);
    }
}

extern void func_80054EA0(u32 *width, u32 *height); /* the frame buffer size */
extern void transition_values_get(s32 *x1, s32 *y1, s32 *x2, s32 *y2); /* the viewport */
extern void func_800408C0(Gfx **dl);                 /* sets the viewport's scissor */
extern Gfx rl_m022_034C0[]; /* .data (still assembly): a display list */

/* The pause screen's frame: the whole screen's scissor, then black right of and below the viewport (called
   from 018AC) */
static void func_m022_00F18(Gfx **dl, UNUSED s32 arg1) {
    s32 x1;
    s32 y1;
    s32 x2;
    s32 y2;
    u32 width;
    u32 height;

    func_80054EA0(&width, &height);
    transition_values_get(&x1, &y1, &x2, &y2);
    gSPDisplayList((*dl)++, rl_m022_034C0);
    gDPSetScissor((*dl)++, G_SC_NON_INTERLACE, 0, 0, width - 1, height - 1);
    if (x2 < (s32) width) {
        gDPFillRectangle((*dl)++, x2, 0, width, y2);
    }
    if (y2 < (s32) height) {
        gDPFillRectangle((*dl)++, 0, y2, width, height);
    }
    gDPPipeSync((*dl)++);
    gDPSetPrimColor((*dl)++, 0, 0, 255, 255, 255, 255);
    gDPSetEnvColor((*dl)++, 255, 255, 255, 255);
    func_800408C0(dl);
}

/* A model as 0114C draws it (59060.c's MenuDrawModel) */
typedef struct M022ModelHeader {
    /* 0x00 */ u8 pad0[0x4E];
    /* 0x4E */ s8 noMatrix; /* draw without pushing the transform */
    /* 0x4F */ u8 pad4F[0x74 - 0x4F];
    /* 0x74 */ Gfx *dl;
    /* 0x78 */ Gfx *dlFaint; /* drawn for the trail copies */
} M022ModelHeader;

typedef struct M022Model {
    /* 0x00 */ M022ModelHeader *header;
    /* 0x04 */ u8 *vertices;
    /* 0x08 */ u8 pad8[3];
    /* 0x0B */ s8 frame;
    /* 0x0C */ u8 padC[4];
    /* 0x10 */ u8 *frames[1];
} M022Model;

/* The rank object's last two yaws (in the object at 0x88) */
typedef struct M022Spin {
    /* 0x88 */ s32 prev;
    /* 0x8C */ s32 prev2;
} M022Spin;

extern f32 func_8003F7C4(void);              /* the field of view */
extern void func_8003F7F4(f32 fov, s32 arg1); /* sets the field of view */
extern void cam_fov_scale_set(f32 x, f32 y);
extern void set_active_camera(s32 camera);
extern void viewport_apply(Gfx **dl, Mtx **mtx);
extern void func_80040FE4(Gfx **dl);              /* viewport_reset */
extern void func_80040E64(Gfx **dl, Mtx **mtx);   /* mtx_perspective */
extern void zbuffer_clear_rect(Gfx **dList, u32 width, u32 height, s32 ulx, s32 uly, s32 lrx, s32 lry);
extern void func_80041CA0(Gfx **dl, Mtx **mtx, ObjectTransform *trans, f32 scale, f32 arg4);
extern s32 obj_anim_frame_advance(Object *obj, f32 speed, f32 rate);
extern void func_8003D21C(M022Model *inst, M022ModelHeader *model, Object *obj, f32 arg3, f32 arg4);
extern void func_800221F0(Object *obj, M022Model *inst, ObjectShading *shading, void *arg3);
extern void mtx_pop(Gfx **dl);
extern void func_80055B80(Gfx **dl); /* resets the render settings */
extern u8 D_800A0838[];
extern s8 rl_m022_03414;

#ifdef NON_MATCHING
/*
 * The task screen's models, in their own viewport: the plate's model, then, when a rank is shown, the rank's
 * object, spinning (and growing in), with a two-copy fading trail of its last yaws for rank 5.
 * NON_MATCHING: ROM length and frame; about 170 words differ, nearly all t-register numbers (the ROM keeps t0-t2 for 0x80000000, the trail count and the list).
 */
void func_m022_0114C(Gfx **dl, Mtx **mtx, s32 updateRate) {
    s32 n;
    s32 x1;
    s32 y1;
    s32 x2;
    s32 y2;
    s32 width;
    s32 height;
    f32 ratio;
    f32 fov;
    ObjectTransform trans;
    Gfx *list;
    Object *obj;
    M022Model *model;
    M022ModelHeader *header;


    model = (M022Model *) D_800A320C;
    if (model == NULL) {
        return;
    }
    header = model->header;
    if (header == NULL) {
        return;
    }
    fov = func_8003F7C4();
    func_80054EA0((u32 *) &width, (u32 *) &height);
    transition_values_get(&x1, &y1, &x2, &y2);
    ratio = (f32) (x2 - x1 - 144) / (f32) (width - 144);
    trans.rotation.y_rotation = -0x8000;
    trans.rotation.x_rotation = 0;
    trans.rotation.z_rotation = 0;
    trans.scale = 1.2222f * ratio + 1.0f;
    trans.x_position = 360.0f * ratio;
    trans.y_position = -240.0f * ratio;
    trans.z_position = -520.0f;
    set_active_camera(0);
    cam_fov_scale_set(1.0f, 1.0f);
    func_8003F7F4(45.0f, 1);
    viewport_apply(dl, NULL);
    func_80040FE4(dl);
    func_80040E64(dl, mtx);
    zbuffer_clear_rect(dl, width, height, x1, y1, x2, y2);
    gDPSetPrimColor((*dl)++, 0, 0, 255, 255, 255, 255);
    func_80041CA0(dl, mtx, &trans, 1.0f, 0.0f);
    {
        Gfx *_g = (*dl)++;
        _g->words.w0 = (((u32) model->frames[model->frame] + 0x80000000) & 0xFFFFFF) | 0xBF000000;
        _g->words.w1 = (u32) model->vertices + 0x80000000;
    }
    {
        Gfx *_g = (*dl)++;
        _g->words.w0 = 0x02000050;
        _g->words.w1 = (u32) (D_800A0838 + 0x80000000);
    }
    gSPDisplayList((*dl)++, (u32) header->dl + 0x80000000);
    if (rl_m022_03414 >= 0 && (obj = rl_m022_03418) != NULL) {
        model = *(M022Model **) obj->modelInstances;
        header = model->header;

        OBJ_STATE(obj, M022Spin)->prev2 = OBJ_STATE(obj, M022Spin)->prev;
        OBJ_STATE(obj, M022Spin)->prev = obj->trans.rotation.y_rotation;
        if (rl_m022_03414 == 10) {
            obj->trans.scale += updateRate * 0.002f;
            if (0.2f < obj->trans.scale) {
                obj->trans.scale = 0.2f;
            }
            obj_anim_frame_advance(obj, 0.01f, updateRate);
            obj->trans.x_position = -80.0f;
            obj->trans.y_position = 10.0f;
            obj->trans.z_position = -106.0f;
            func_8003D21C(model, header, obj, 1.0f, 0.0f);
        } else if (rl_m022_03414 == 8) {
            obj->trans.scale += updateRate * 0.002f;
            if (0.2f < obj->trans.scale) {
                obj->trans.scale = 0.2f;
            }
            trans.scale = obj->trans.scale;
            trans.x_position = -168.0f;
            trans.y_position = 102.0f;
            trans.z_position = -500.0f;
        } else if (rl_m022_03414 == 7) {
            obj->trans.scale += updateRate * 0.002f;
            if (0.2f < obj->trans.scale) {
                obj->trans.scale = 0.2f;
            }
            obj_anim_frame_advance(obj, 0.01f, updateRate);
            obj->trans.x_position = -132.0f;
            obj->trans.y_position = 34.0f;
            obj->trans.z_position = -350.0f;
            func_8003D21C(model, header, obj, 1.0f, 0.0f);
        } else if (rl_m022_03414 == 6 || rl_m022_03414 == 9) {
            obj->trans.scale += updateRate * 0.002f;
            if (0.2f < obj->trans.scale) {
                obj->trans.scale = 0.2f;
            }
            trans.scale = obj->trans.scale;
            trans.x_position = -42.0f;
            trans.y_position = 6.0f;
            trans.z_position = 88.0f;
        } else {
            obj->trans.scale += updateRate * 0.01f;
            if (1.25f < obj->trans.scale) {
                obj->trans.scale = 1.25f;
            }
            trans.scale = obj->trans.scale;
            trans.x_position = -168.0f;
            trans.y_position = 102.0f;
            trans.z_position = -500.0f;
        }
        func_800221F0(obj, model, obj->shading, NULL);
        if (rl_m022_03414 == 5) {
            n = 3;
            obj->trans.rotation.y_rotation += updateRate << 9;
        } else {
            n = 1;
            obj->trans.rotation.y_rotation += updateRate << 8;
        }
        gDPPipeSync((*dl)++);
        {
            Gfx *_g = (*dl)++;
            _g->words.w0 = (((u32) model->frames[model->frame] + 0x80000000) & 0xFFFFFF) | 0xBF000000;
            _g->words.w1 = (u32) model->vertices + 0x80000000;
        }
        while (n-- != 0) {
            if (n == 2) {
                trans.rotation.y_rotation = OBJ_STATE(obj, M022Spin)->prev2;
                list = header->dlFaint;
                gDPSetPrimColor((*dl)++, 0, 0, 255, 85, 85, 85);
            } else if (n == 1) {
                trans.rotation.y_rotation = OBJ_STATE(obj, M022Spin)->prev;
                list = header->dlFaint;
                gDPSetPrimColor((*dl)++, 0, 0, 255, 170, 170, 170);
            } else {
                trans.rotation.y_rotation = obj->trans.rotation.y_rotation;
                list = header->dl;
                gDPSetPrimColor((*dl)++, 0, 0, 255, 255, 255, 255);
            }
            if (header->noMatrix == 0) {
                func_80041CA0(dl, mtx, &trans, 1.0f, 0.0f);
            }
            gSPDisplayList((*dl)++, (u32) list + 0x80000000);
        }
    }
    {
        Gfx *_g = (*dl)++;
        _g->words.w0 = 0xBF000000;
        _g->words.w1 = 0;
    }
    mtx_pop(dl);
    func_80055B80(dl);
    func_8003F7F4(fov, 1);
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m022/ro", func_m022_0114C);
#endif

/* A task goal: a count to reach, or flags to have */
typedef struct M022Goal {
    /* 0x0 */ u8 text; /* its text, by rl_m022_031EC */
    /* 0x1 */ u8 kind; /* 0, 1: the counters; else the flags */
    /* 0x2 */ u16 value;
} M022Goal;

/* The task's goals: two lists */
typedef struct M022Goals {
    /* 0x00 */ u8 pad0[2];
    /* 0x02 */ u8 count1;
    /* 0x03 */ u8 count2;
    /* 0x04 */ M022Goal list1[3];
    /* 0x10 */ M022Goal list2[1];
    /* 0x14 */ u8 pad14[0x1C - 0x14];
    /* 0x1C */ struct M022Rank {
        u16 time; /* in seconds: to beat */
        u8 count1; /* the counters to reach */
        u8 count2;
        u16 flags; /* to have */
    } ranks[5];
} M022Goals;

/* A textured rectangle for func_8004EDBC (4E610.c's TexturedRect) */
typedef struct M022Rect {
    /* 0x00 */ void *tex;
    /* 0x04 */ s32 frame;
    /* 0x08 */ s16 x;
    /* 0x0A */ s16 y;
} M022Rect;

extern void func_8006FC8C(s32 r, s32 g, s32 b, s32 a);
extern void func_8006FC68(s32 r, s32 g, s32 b, s32 a, s32 opacity);
extern void func_8006FCA8(Gfx **dl, s32 x, s32 y, char *text, s32 flags);
extern void func_8006FC54(s32 font);
extern s32 func_800639E8(char *buf, char *fmt, ...); /* sprintf */
extern void func_8004EDBC(Gfx **dList, M022Rect *rects, s32 x, s32 y, u8 r, u8 g, u8 b, u8 alpha);
extern char **gMenuText; /* the game's texts */
extern s32 D_800A3204; /* the task's time, in frames */
extern u8 D_800A3208;  /* a task is running */
extern u8 D_800FD790;
extern u8 D_800FD791;
extern s32 rl_m022_0340C; /* the task */
extern M022Goals *rl_m022_03410;
extern u8 rl_m022_03420;
extern s8 rl_m022_03414; /* the rank shown, -1: none */
extern s16 rl_m022_031E0[]; /* (.data) per task: its name's text */
extern s16 rl_m022_031EC[]; /* (.data) per goal: its text */
extern s16 rl_m022_03214[]; /* (.data) per rank: its text */
extern s16 rl_m022_0322C[]; /* (.data) the item texts */
extern u8 rl_m022_033D8[];  /* (.data) per rank: its colour */
extern u8 rl_m022_033E4[];
extern u8 rl_m022_033F0[];
extern s32 rl_m022_03500[]; /* (.data) how the time is shown: divisors, -1 a colon, -2 a point, 0 the end */
extern void rl_m022_0114C(Gfx **dl, s32 arg1, s32 updateRate); /* still assembly */

/*
 * The task screen: fades in and out, then draws the items picked up, the goals (or the two counters), the rank,
 * the task's name and its time. `mode` holds first the item list's first text, then which goals are shown
 * (2: the two counters instead).
 */
void func_m022_018AC(Gfx **dl, s32 arg1, s32 updateRate) {
    s32 mode;
    s32 i;
    s32 y;
    s32 bit;
    s32 t;
    s32 d;
    M022Goals *goals;
    M022Goal *goal;
    s32 *f;
    char *p;
    char text[12];
    M022Rect rects[2];
    char time[20];
    UNUSED s32 pad[6]; /* the ROM's frame has 24 more bytes below `time` */

    if (rl_m022_03430 < 0) {
        rl_m022_0342C -= updateRate * 8;
        if (rl_m022_0342C < 0) {
            rl_m022_0342C = 0;
        }
    } else if (rl_m022_03430 > 0) {
        rl_m022_0342C += updateRate * 8;
        if (rl_m022_0342C >= 256) {
            rl_m022_0342C = 255;
        }
    }
    if (rl_m022_0342C == 0) {
        return;
    }
    func_8006FC8C(0, 0, 0, 0);
    func_m022_00F18(dl, updateRate);
    if (*(s8 *) &rl_m022_0343C > 0) {
        mode = *(s8 *) &rl_m022_03438;
        i = 0;
        y = 0x5C - *(s8 *) &rl_m022_0343C * 10;
        rects[0].frame = 0;
        rects[0].x = -8;
        rects[0].y = -8;
        rects[1].tex = NULL;
        func_8006FC54(2);
        func_8006FC68(255, 255, 255, 255, rl_m022_0342C);
        for (; i < *(s8 *) &rl_m022_0343C; i++) {
            rects[0].tex = (void *) (D_800A3210 + i)[*(s8 *) &rl_m022_03440];
            func_8004EDBC(dl, rects, 200, y, 255, 255, 255, rl_m022_0342C);
            func_8006FCA8(dl, 212, y, gMenuText[rl_m022_0322C[mode++]], 8);
            y += 20;
        }
    }
    goals = rl_m022_03410;
    bit = 1 << goals->count1;
    mode = *(u8 *) &rl_m022_03434;
    if (rl_m022_03424 != 0) {
        if (rl_m022_03424 < bit) {
            mode = 0;
        } else {
            mode = 1;
        }
    }
    if (mode == 2) {
        func_8006FC54(2);
        func_8006FC68(255, 255, 32, 255, rl_m022_0342C);
        func_8006FCA8(dl, 160, 158, gMenuText[515], 4);
        func_8006FCA8(dl, 160, 183, gMenuText[516], 4);
        func_8006FC68(255, 255, 255, 255, rl_m022_0342C);
        func_800639E8(text, "%d", D_800FD790);
        func_8006FCA8(dl, 160, 168, text, 4);
        func_800639E8(text, "%d", D_800FD791);
        func_8006FCA8(dl, 160, 193, text, 4);
    } else {
        if (mode == 0) {
            y = 0x1D6;
            i = goals->count1;
            goal = goals->list1;
            bit = 1;
        } else {
            y = 0x1D7;
            i = goals->count2;
            goal = goals->list2;
        }
        func_8006FC54(2);
        func_8006FC68(255, 255, 32, 255, rl_m022_0342C);
        func_8006FCA8(dl, 160, 158, gMenuText[y], 4);
        y = 170; if (i == 0) {
            func_8006FC68(255, 255, 255, 255, rl_m022_0342C);
            func_8006FCA8(dl, 160, 170, gMenuText[343], 4);
        } else {
            /**/
            while (i--) {
                if (rl_m022_03424 & bit) {
                    func_8006FC68(255, 255, 255, 255, rl_m022_0342C);
                } else if (rl_m022_03420 & bit) {
                    func_8006FC68(64, 255, 0, 255, rl_m022_0342C);
                } else {
                    func_8006FC68(255, 64, 0, 255, rl_m022_0342C);
                }
                func_8006FCA8(dl, 160, y, gMenuText[rl_m022_031EC[goal->text]], 4);
                goal++;
                y += 10;
                bit <<= 1;
            }
        }
    }
    if (rl_m022_03414 >= 0) {
        if (rl_m022_03414 == 0) {
            func_8006FC54(2);
            y = 0x54;
        } else {
            func_8006FC54(2);
            y = 0x7A;
        }
        func_8006FC68(0, 0, 0, 255, rl_m022_0342C);
        func_8006FCA8(dl, 0x61, y + 1, gMenuText[rl_m022_03214[rl_m022_03414]], 12);
        func_8006FC68(rl_m022_033D8[rl_m022_03414], rl_m022_033E4[rl_m022_03414], rl_m022_033F0[rl_m022_03414], 255,
                      rl_m022_0342C);
        func_8006FCA8(dl, 0x60, y, gMenuText[rl_m022_03214[rl_m022_03414]], 12);
    }
    rl_m022_0114C(dl, arg1, updateRate);
    func_8006FC54(2);
    func_8006FC68(0, 0, 64, 255, rl_m022_0342C);
    func_8006FCA8(dl, 160, 26, gMenuText[rl_m022_031E0[rl_m022_0340C]], 4);
    func_8006FC68(0, 0, 64, 255, rl_m022_0342C);
    func_8006FCA8(dl, 160, 134, gMenuText[469], 4);
    if (D_800A3208 != 0) {
        t = D_800A3204 * 100 / 60;
        /**/
        f = rl_m022_03500; p = time;
        do {
            d = *f++;
            if (d == -1) {
                *p++ = ':';
            } else if (d == -2) {
                *p++ = '.';
            } else if (d > 0) {
                *p++ = t / d + '0';
                t -= t / d * d;
            }
        } while (d != 0);
        *p = '\0';
        func_8006FC54(2);
        func_8006FC68(0, 0, 0, 255, rl_m022_0342C);
        func_8006FCA8(dl, 0x62, 0x2C, time, 4);
        func_8006FC68(0xE0, 0xE0, 255, 255, rl_m022_0342C);
        func_8006FCA8(dl, 0x60, 0x2A, time, 4);
    }
}

struct M022Goals;
extern struct M022Goals *rl_m022_03410;
extern u8 D_800FD790; /* counters the goals count */
extern u8 D_800FD791;
extern u16 D_800FD792; /* flags */
extern s32 D_800A3204;
extern u8 rl_m022_03420; /* a bit per goal reached */
extern s32 rl_m022_0340C; /* the task */
extern u8 rl_m022_03240[][0x40]; /* .data (still assembly): the goals of each task */
extern s8 rl_m022_03414;
extern void func_m118_0007C(Object *obj, LevelObjectEntryCommon *entry);
extern void func_m118_003DC(Object *obj, LevelObjectEntryCommon *entry);
extern void func_m074_00000(Object *obj, LevelObjectEntryCommon *entry);
extern void func_m101_009F8(Object *obj, LevelObjectEntryCommon *entry);

/* Starts task `task`: clears the goal counters and sets up again the objects of kinds 0x66, 0x67, 0x60
   and 0x4E (called from 002A4) */
void func_m022_020EC(s32 task) {
    s32 first;
    s32 count;
    Object **objs;
    Object *obj;

    D_800FD790 = 0;
    D_800FD791 = 0;
    D_800FD792 = 0;
    D_800A3204 = 0;
    rl_m022_03420 = 0;
    rl_m022_03424 = 0;
    rl_m022_03434 = 0;
    rl_m022_0340C = task;
    rl_m022_03410 = (struct M022Goals *) rl_m022_03240[task];
    rl_m022_03414 = -1;
    objs = func_80005E98(&first, &count);
    while (first < count) {
        obj = objs[first++];
        if (obj->behaviorId == 0x66) {
            if (obj->level_entry != NULL) {
                func_m118_0007C(obj, obj->level_entry);
            }
        } else if (obj->behaviorId == 0x67) {
            if (obj->level_entry != NULL) {
                func_m118_003DC(obj, obj->level_entry);
            }
        } else if (obj->behaviorId == 0x60) {
            if (obj->level_entry != NULL) {
                func_m074_00000(obj, obj->level_entry);
            }
        } else if (obj->behaviorId == 0x4E && *(s32 *) obj->state <= 0) {
            if (obj->level_entry != NULL) {
                func_m101_009F8(obj, obj->level_entry);
                obj->trans.flags &= ~0x400;
            }
        }
    }
}
extern void rl_m022_02310(M022HelperState *s);

/* Puts the helper in state 1 and starts its task (called from 002A4) */
void func_m022_022C4(M022Helper *helper) {
    M022HelperState *s;

    D_800A3208 = 1;
    helper->state = 1;
    s = helper->obj->unk68;
    s->unk1C = *(u16 *) rl_m022_03410;
    s->unk1E = 6;
    rl_m022_02310(s);
}

/* Puts the helper's start where the (last) behaviour-0x68 object with unk16 0 is, facing away */
void func_m022_02310(M022HelperState *s) {
    s32 first;
    s32 count;
    Object **objs;
    Object *obj;
    M022Start *start;

    objs = func_80005E98(&first, &count);
    while (first < count) {
        obj = objs[first++];
        if (obj->behaviorId == 0x68 && (start = obj->unk68)->unk16 == 0) {
            s->x = obj->trans.x_position;
            s->y = obj->trans.y_position;
            s->z = obj->trans.z_position;
            s->yaw = obj->trans.rotation.y_rotation + 0x8000;
            s->pitch = -obj->trans.rotation.x_rotation;
            s->roll = 0;
        }
    }
    s->unk90 = 0;
}

void func_m022_023FC(M022Helper *helper) {
    helper->state = 3;
    rl_m022_03424 = 0;
}

/* Puts the helper in state 4 (called from 002A4) */
void func_m022_02410(M022Helper *helper) {
    M022HelperState *s;

    D_800A3208 = 0;
    helper->state = 4;
    s = helper->obj->unk68;
    s->unk90 = 0;
}

typedef struct M022Rank M022Rank;

extern u8 D_800FD790;  /* counters the goals count */
extern u8 D_800FD791;
extern u16 D_800FD792; /* flags */
extern M022Goals *rl_m022_03410;
extern u8 rl_m022_03420; /* a bit per goal reached */

/* Counts a thing done (kind 0 or 1) or sets flag `bit`, then sees which goals are reached */
void func_m022_02430(s32 bit, s32 kind) {
    M022Goals *g;
    M022Goal *e;
    s32 i;
    s32 mask;

    switch (kind) {
        case 0:
            D_800FD790++;
            break;
        case 1:
            D_800FD791++;
            break;
    }
    if (bit >= 0) {
        D_800FD792 |= 1 << bit;
    }
    rl_m022_03420 = 0;
    mask = 1;
    g = rl_m022_03410;
    bit = g->count1; /* the goals left in the list */
    e = g->list1;
    i = 2;
    while (i--) {
        while (bit--) {
            switch (e->kind) {
                case 0:
                    if (D_800FD790 >= e->value) {
                        rl_m022_03420 |= mask;
                    }
                    break;
                case 1:
                    if (D_800FD791 >= e->value) {
                        rl_m022_03420 |= mask;
                    }
                    break;
                default:
                    if ((D_800FD792 & e->value) == e->value) {
                        rl_m022_03420 |= mask;
                    }
                    break;
            }
            e++;
            mask <<= 1;
        }
        bit = g->count2;
        e = g->list2;
    }
}

/* The game's save data, as used here */
typedef struct M022Save {
    /* 0x00 */ u8 pad0[0x64];
    /* 0x64 */ u32 unk64; /* flags */
    /* 0x68 */ u8 pad68[0x6B - 0x68];
    /* 0x6B */ u8 unk6B;
} M022Save;

/* The game's settings (what get_settings returns), as used here */
typedef struct M022Record {
    /* 0x00 */ u8 pad0[0x28];
    /* 0x28 */ u8 rank[4]; /* the best rank of each task */
} M022Record;

extern M022Save D_800FF318;
extern M022Record *get_settings(void);
extern void character_item_give(s32 player, s32 character, s32 item);
extern s32 player_chars_have_item(s32 item);
extern s32 game_flag_get(s32 flag);
extern void func_80047EFC(s32 flag, s32 set);
extern u8 rl_m022_0341C; /* .bss (still assembly): 2 a new best rank, 1 a rank, 0 none */
extern s16 rl_m022_033C0[]; /* .data (still assembly): the object shown for each rank, or -1 */

/*
 * The end of the task: its rank (the ranks it meets in turn), the record, the rewards, and the
 * object that shows the rank
 */
void func_m022_025B8(s8 *player) {
    LevelObjectEntryCommon entry;
    M022Rank *r;
    M022Record *rec;

    rl_m022_03414 = 0;
    r = rl_m022_03410->ranks;
    while (rl_m022_03414 < 5 && D_800A3204 < r->time * 60 && D_800FD790 >= r->count1 &&
           D_800FD791 >= r->count2 && (D_800FD792 & r->flags) == r->flags) {
        rl_m022_03414++;
        r++;
    }
    rec = get_settings();
    if (rec->rank[rl_m022_0340C] < rl_m022_03414) {
        rec->rank[rl_m022_0340C] = rl_m022_03414;
        rl_m022_0341C = 2;
    } else if (rl_m022_03414 > 0) {
        rl_m022_0341C = 1;
    } else {
        rl_m022_0341C = 0;
    }
    if (rl_m022_0340C == 0 && rl_m022_03414 >= 4 && !(D_800FF318.unk6B & 1)) {
        rl_m022_03414 = 6;
        func_80047EFC(0x49, 1);
    } else if (rl_m022_0340C == 1 && rl_m022_03414 >= 4 && !player_chars_have_item(0x1A) && !game_flag_get(0x20)) {
        character_item_give(player[0], player[1], 0x1A);
        rl_m022_03414 = 7;
    } else if (rl_m022_0340C == 2 && rl_m022_03414 >= 4 && !player_chars_have_item(0x1B)) {
        character_item_give(0, 0, 0x1B);
        character_item_give(0, 1, 0x1B);
        character_item_give(0, 2, 0x1B);
        rl_m022_03414 = 8;
    } else if (rl_m022_0340C == 3 && rl_m022_03414 >= 4 && !(D_800FF318.unk6B & 4)) {
        rl_m022_03414 = 9;
        func_80047EFC(0x4B, 1);
    } else if (rec->rank[0] == 5 && rec->rank[1] == 5 && rec->rank[2] == 5 && rec->rank[3] == 5 &&
               !(D_800FF318.unk64 & 0x20000)) {
        func_80047EFC(0x11, 1);
        rl_m022_03414 = 10;
    }
    entry.objectID = rl_m022_033C0[rl_m022_03414];
    rl_m022_03418 = NULL;
    if (entry.objectID >= 0) {
        entry.x = 0;
        entry.y = 0;
        entry.z = 0;
        rl_m022_03418 = func_80005F14(&entry, 0);
    }
}

/* Pauses the task: brings the letterbox in */
void func_m022_02990(void) {
    rl_m022_03434 = 2;
    rl_m022_03424 = 0;
    rl_m022_03430 = 0;
    rl_m022_0342C = 1;
    rl_m022_03438 = 2;
    rl_m022_0343C = 3;
    rl_m022_03440 = 0;
    transition_values_start(0, 0, 320, 240, 24, 30, 168, 138, 32);
    set_game_paused(1);
}

extern s32 transition_values_time_left(void);
extern s32 func_80043510(s32 player); /* the buttons pressed */
extern void func_80001248(s32 sound, s32 arg1);
extern void transition_begin(void *transition);
extern u8 D_800A4FC8; /* a second player plays */
extern u8 rl_m022_033FC[];

/* The task's pause screen: A/start leave it, B and Z move between its pages */
void func_m022_02A34(void) {
    Object *player;
    s32 buttons;
    u8 *data;

    if (transition_values_time_left()) {
        return;
    }
    rl_m022_03430 = 1;
    buttons = func_80043510(0);
    if (D_800A4FC8) {
        buttons |= func_80043510(1);
    }
    if (*(s8 *) &rl_m022_03438 == 2) {
        if (buttons & 0x9000) {
            func_80001248(0x16B, 0);
            transition_begin(rl_m022_033FC);
            transition_values_start(24, 30, 168, 138, 0, 0, 320, 240, -1);
            rl_m022_03430 = 0;
            rl_m022_0342C = 0;
            set_game_paused(0);
        } else if (buttons & 0x4000) {
            func_80001248(0x16B, 0);
            rl_m022_03438 = 5;
            rl_m022_0343C = 2;
            rl_m022_03440 = 0;
        } else if (buttons & 0x2000) {
            func_80001248(0x16C, 0);
            if (++*(u8 *) &rl_m022_03434 >= 3) {
                *(u8 *) &rl_m022_03434 = 0;
            }
        }
    } else if (buttons & 0x9000) {
        func_80001248(0x16B, 0);
        player = func_8000E4D0(0);
        if (player != NULL && (data = player->unk68) != NULL) {
            data[0x10D] = 5;
        }
        set_game_paused(0);
    } else if (buttons & 0x4000) {
        func_80001248(0x16B, 0);
        rl_m022_03438 = 2;
        rl_m022_0343C = 3;
        rl_m022_03440 = 0;
    }
}

/* Places the plate camera above the plate, turned towards the camera (useAngle) or along `yaw` */
void func_m022_02C0C(Object *obj, s32 yaw, s32 useAngle) {
    f32 dx;
    f32 dz;
    ObjectTransform *cam;

    cam = func_80041DE0();
    sPlateCamX = obj->trans.x_position;
    sPlateCamY = obj->trans.y_position + 50.0f;
    sPlateCamZ = obj->trans.z_position;
    dx = cam->x_position - obj->trans.x_position;
    dz = cam->z_position - obj->trans.z_position;
    sPlateCamDist = func_80097140(dx * dx + dz * dz);
    sPlateCamDy = cam->y_position - sPlateCamY;
    if (useAngle) {
        sPlateCamYaw = 0x8000 - func_80049C84(dx, dz);
    } else {
        sPlateCamYaw = yaw;
    }
    sPlateCamUnk16 = 0;
    rl_m022_03428 = 300;
    sPlateCamUnk18 = useAngle;
}

extern f32 func_80049BEC(f32 base, s32 exponent);
extern s16 angle_lerp(s16 from, s16 to, f32 t);

/* The plate camera: circles the plate, settling at 200 units out and 75 up */
void func_m022_02D14(Object *obj, s32 updateRate) {
    s16 pitch;
    s16 yaw;
    f32 x;
    f32 z;
    f32 f;

    if (sPlateCamUnk18) {
        f = 1.0f - func_80049BEC(0.95f, updateRate);
    } else {
        f = 1.0f;
    }
    sPlateCamUnk16 += updateRate * 12;
    sPlateCamDist += (200.0f - sPlateCamDist) * f;
    sPlateCamDy += (75.0f - sPlateCamDy) * f;
    if (sPlateCamUnk16 > 192) {
        sPlateCamUnk16 = 192;
    }
    sPlateCamYaw += sPlateCamUnk16 * updateRate;
    x = func_80049C34(sPlateCamYaw) * sPlateCamDist;
    z = -func_80049C30(sPlateCamYaw) * sPlateCamDist;
    obj->trans.x_position = sPlateCamX + x;
    obj->trans.y_position = sPlateCamY + sPlateCamDy;
    obj->trans.z_position = sPlateCamZ + z;
    yaw = 0x8000 - func_80049C84(x, z);
    pitch = func_80049C84(sPlateCamDy, sPlateCamDist);
    if (sPlateCamUnk18) {
        f = 1.0f - func_80049BEC(0.875f, updateRate);
    } else {
        f = 1.0f;
    }
    obj->trans.rotation.y_rotation = angle_lerp(obj->trans.rotation.y_rotation, yaw, f);
    obj->trans.rotation.x_rotation = angle_lerp(obj->trans.rotation.x_rotation, pitch, f);
    sPlateCamUnk18 = 1;
}

/* What the player object's unk68 points at, as used here */
typedef struct M022PlayerData {
    /* 0x000 */ u8 pad0;
    /* 0x001 */ s8 character;
    /* 0x002 */ u8 pad2[0x594 - 0x2];
    /* 0x594 */ f32 unk594;
} M022PlayerData;

extern void func_80009A04(Object *obj, f32 dx, f32 dy, f32 dz); /* DKR objMoveXYZ */

/* Moves the player onto the plate (behaviour 0x5D), at a height that depends on the character */
void func_m022_02F38(Object *obj, M022PlayerData *player, s32 updateRate) {
    f32 dx;
    f32 dy;
    f32 dz;
    Object **objs;
    M022Plate *data;
    s32 first;
    s32 count;
    Object *plate;
    f32 f;
    f32 max;

    objs = func_80005E98(&first, &count);
    plate = NULL;
    while (first < count) {
        if (objs[first++]->behaviorId == 0x5D) {
            plate = objs[first - 1];
        }
    }
    if (plate != NULL) {
        data = plate->unk68;
        data->bob += updateRate;
        dx = plate->trans.x_position;
        dy = func_80049C34(data->bob << 8) * 8.0f + plate->trans.y_position;
        dz = plate->trans.z_position;
        switch (player->character) {
            default:
                dy += 47.5f;
                break;
            case 1:
                dy += 45.0f;
                break;
            case 2:
                dy += 55.0f;
                break;
        }
        dx -= obj->trans.x_position;
        dy -= obj->trans.y_position;
        dz -= obj->trans.z_position;
        f = func_80049BEC(0.925f, updateRate);
        obj->velocity.y *= f;
        player->unk594 += (1.0f - player->unk594) * (1.0f - f);
        f = 1.0f - func_80049BEC(0.98f, updateRate);
        dx *= f;
        dy *= f;
        dz *= f;
        max = updateRate * 0.075f;
        if (max < dy) {
            dy = max;
        }
        func_80009A04(obj, dx, dy, dz);
    }
}

/* Ends the task: frees the plate object and takes the letterbox out */
void func_m022_03150(void) {
    D_800A3208 = 0;
    D_801046EE = 0;
    D_801046F4 = 0;
    rl_m022_03430 = 0;
    rl_m022_0342C = 0;
    if (rl_m022_03418 != NULL) {
        func_800079B8(rl_m022_03418);
    }
    transition_values_start(24, 30, 168, 138, 0, 0, 320, 240, -1);
}

INCLUDE_ASM("asm/nonmatchings/modules/m022", data);

/* (.rodata: C, and the assembly functions' own from ro/) */

INCLUDE_ASM("asm/nonmatchings/modules/m022", bss);
