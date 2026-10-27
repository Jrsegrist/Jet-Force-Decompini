/*
 * Runlink module 2: an off-screen pass. It allocates two 16-bit frame
 * buffers and each frame builds a display list that clears one of them, sets
 * up fog, and draws into it only the objects (behaviour 0x18) whose linked
 * object has kind 0xE6; then it runs the list as a task and swaps buffers.
 */
#include "module.h"
#include "gfx.h"

/* What a behaviour-0x18 object's unk68 points at */
typedef struct M002Owner {
    /* 0x00 */ u8 pad0[0x24];
    /* 0x24 */ Object *linked;
} M002Owner;

/* What the linked object's unk68 points at */
typedef struct M002Linked {
    /* 0x00 */ u8 pad0[0x2C];
    /* 0x2C */ u16 kind;
} M002Linked;

extern void func_80054EA0(u32 *width, u32 *height);    /* the frame buffer size */
extern void *func_8004A57C(s32 size, s32 colour);      /* allocates */
extern void func_8004AA80(void *data);                 /* frees */
extern s32 func_80096910(OSMesgQueue *mq, OSMesg *msg, s32 flags);
extern void func_80050E90(Gfx **dl, s32 segment, s32 base);
extern void func_8004E40C(Gfx **dl);
extern void func_8004E3E8(Gfx **dl);
extern void func_80055B80(Gfx **dl);
extern void func_80056000();
extern void func_8003FC2C(s32 arg0);
extern void func_8004046C(Gfx **dl, Mtx **mtx);
extern void func_8001BC2C(s32 player, s16 *near, s16 *far, s16 *arg3, u8 *r, u8 *g, u8 *b, u8 *arg7);
extern void func_8000DB94(Gfx **dl, Mtx **mtx, Vertex **verts, Object *obj); /* DKR render_object */
extern void func_8004DA10(Gfx *start, Gfx *end, s32 arg2, u16 *frameBuffer);
extern void func_80075C40(void);

extern s32 D_800A360C;
extern s32 D_800A3334;
extern u16 *D_800FD820[2]; /* the two buffers */
extern u16 *D_800FD828;    /* the current buffer */
extern s32 D_800FD82C;
extern s32 D_800FD830; /* which buffer is drawn to */
extern s32 D_800FECBC;
extern OSMesgQueue D_800FE8E0;

static s32 sBufferBlock; /* the allocation holding both buffers */
static Gfx sDisplayLists[2][0x180];
static Mtx sMatrices[2][0x30];
static Vertex sVertices[2][0x60];
static Gfx *sDisplayListEnds[2];
static u8 sReady;
u8 D_m002_03FE8[0x1008];

void func_m002_00148(void);

void func_m002_00000(void) {
    u32 width;
    u32 height;
    u16 *buf;
    OSMesg msg = 0;

    if (!sReady) {
        D_800A360C = 0;
        D_800FD830 = 0;
        D_800FD82C = 2;
        sBufferBlock = 0;
        func_80054EA0(&width, &height);
        sBufferBlock = (s32) func_8004A57C(width * height * 4 + 0x40, -1);
        if (sBufferBlock != 0) {
            /* 64-byte aligned */
            if ((u32) sBufferBlock & 0x3F) {
                buf = (u16 *) (((u32) sBufferBlock & ~0x3F) + 0x40);
            } else {
                buf = (u16 *) sBufferBlock;
            }
            D_800FD820[0] = buf;
            D_800FD820[1] = (u16 *) ((u8 *) buf + width * height * 2);
            D_800FD828 = D_800FD820[D_800FD830];
            while (func_80096910(&D_800FE8E0, &msg, OS_MESG_NOBLOCK) == 0) {
            }
            sReady = TRUE;
        } else {
            func_m002_00148();
        }
    }
}

void func_m002_00148(void) {
    if (sBufferBlock != 0) {
        func_8004AA80((void *) sBufferBlock);
        sBufferBlock = 0;
    }
}

void func_m002_0017C(void) {
    func_m002_00148();
}
#pragma weak __rl_fini = func_m002_0017C

void func_m002_0019C(void) {
    Gfx *dl;
    Mtx *mtx;
    Vertex *verts;
    u32 width;
    u32 height;
    s32 first;
    s32 count;
    s32 i;
    Object **objs;
    Object *obj;
    s16 fogNear;
    s16 fogFar;
    s16 fogUnk;
    u8 r;
    u8 g;
    u8 b;
    u8 fogUnk2;
    M002Owner *owner;
    Object *linked;
    M002Linked *data;

    objs = func_80005E98(&first, &count);
    func_80054EA0(&width, &height);
    dl = sDisplayLists[D_800FD830];
    mtx = sMatrices[D_800FD830];
    verts = sVertices[D_800FD830];
    func_80050E90(&dl, 0, 0);
    func_80050E90(&dl, 1, (s32) D_800FD820[D_800FD830]);
    func_80050E90(&dl, 2, D_800FECBC);
    func_8004E40C(&dl);
    func_8004E3E8(&dl);
    gSPClearGeometryMode(dl++, G_ZBUFFER);
    gDPSetColorImage(dl++, G_IM_FMT_RGBA, G_IM_SIZ_16b, width, (u32) D_800FD820[D_800FD830] + 0x80000000);
    gDPSetScissor(dl++, G_SC_NON_INTERLACE, 0, 0, width, height);
    gMoveWd(dl++, G_MW_NUMLIGHT, 0, 0);
    gDPSetFillColor(dl++, 0);
    gDPSetCycleType(dl++, G_CYC_FILL);
    gDPFillRectangle(dl++, 0, 0, width - 1, height - 1);
    gDPPipeSync(dl++);
    func_80055B80(&dl);
    gMoveWd(dl++, G_MW_NUMLIGHT, 0, 0);
    gSPClearGeometryMode(dl++, G_CULL_FRONT);
    gDPSetBlendColor(dl++, 0, 0, 0, 100);
    gDPSetPrimColor(dl++, 0, 0, 255, 255, 255, 255);
    gDPSetEnvColor(dl++, 255, 255, 255, 0);
    gDPPipeSync(dl++);
    func_8003FC2C(0);
    func_8004046C(&dl, &mtx);
    gDPPipeSync(dl++);
    func_8001BC2C(0, &fogNear, &fogFar, &fogUnk, &r, &g, &b, &fogUnk2);
    gDPSetFogColor(dl++, r, g, b, 255);
    gSPFogPosition(dl++, fogNear, fogFar);
    func_80056000(0);
    for (i = first; i < count; i++) {
        obj = objs[i];
        if (obj->behaviorId == 0x18) {
            owner = obj->unk68;
            linked = owner->linked;
            if (linked != NULL) {
                data = linked->unk68;
                if (data->kind == 0xE6) {
                    D_800A3334 = 1;
                    func_8000DB94(&dl, &mtx, &verts, obj);
                    D_800A3334 = 0;
                }
            }
        }
    }
    gMoveWd(dl++, G_MW_LIGHTCOL, 0, 0);
    gDPFullSync(dl++);
    gSPEndDisplayList(dl++);
    sDisplayListEnds[D_800FD830] = dl;
    func_80056000(1);
}

void func_m002_00788(void) {
    func_m002_0019C();
    func_8004DA10(sDisplayLists[D_800FD830], sDisplayListEnds[D_800FD830], 4, D_800FD820[D_800FD830]);
    D_800FD830 ^= 1;
    D_800FD828 = D_800FD820[D_800FD830];
    D_800A360C = 1;
}

void func_m002_00824(void) {
    func_80075C40();
}
