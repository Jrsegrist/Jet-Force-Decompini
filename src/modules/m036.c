/*
 * Runlink module 36: the game's start-up. Its first function initialises the
 * game's systems in turn; the others allocate the two display list heaps
 * (display list, matrices, vertices and triangles) for a number of players.
 * Those two are Diddy Kong Racing's alloc_displaylist_heap and
 * default_alloc_displaylist_heap (DKR decomp, src/thread3_main.c, CC0), and
 * the tables are DKR's gNumF3dCmdsPerPlayer etc., here with a second row.
 */
#include "module.h"
#include "gfx.h"

extern void check_reset_and_halt();
extern void func_m025_00308(void *sched);
extern void func_8005342C(s32 arg0);
extern void audspat_init(void);
extern s32 func_80042FEC(void);
extern void func_800554B0(void);
extern void func_8003B8AC(void);
extern void func_m034_00000(void);
extern void func_80029B78(void);
extern void func_800436C0(void *arg0, void *arg1);
extern void func_80064D9C(void);
extern void func_m026_002F8(void);
extern void func_8005AB00(void);
extern void func_8006D0D8(void);
extern void func_80068450(void);
extern void func_800239D0(void);
extern void func_m035_00000(void);
extern void func_8004C2BC(void);
extern void func_80058590(void);
extern void func_80076EA0(void);
extern void func_8006B8C4(void);
extern void func_80052678(void);
extern void func_80065CE0(void);
extern void func_80083A90(void);
extern void func_80096BB0(void *mq, void *msg, s32 count);                  /* osCreateMesgQueue */
extern void func_8004F85C(void *sched, void *client, void *mq, s32 id);      /* osScAddClient */
extern s32 func_8000E624(s32 arg0);
extern void func_80097F70(u64 arg0, Gfx **dl); /* a 64-bit first argument (0 passed as two halves) */
extern void func_80048E20(s32 arg0);
extern void func_80043750(void);
extern void func_800476F0(void *arg0);
extern void func_8004791C(void);
extern void *func_8004A57C(s32 size, s32 colour);                      /* DKR mempool_alloc_safe */
extern void *func_8004A828(s32 size, void *address, s32 colour);       /* DKR mempool_alloc_fixed */
extern void func_8004AA80(void *data);                                 /* DKR mempool_free */
extern s32 mempool_free_timer_get(void);
extern void func_8004AA08(s32 state);

extern s32 D_800A32A8;
extern s32 D_800A32B0;
extern Gfx *gDisplayLists[2];
extern Gfx *gCurrDisplayList;
extern Mtx *gMatrixHeap[2];
extern Mtx *gGameCurrMatrix;
extern Vertex *gVertexHeap[2];
extern Vertex *gGameCurrVertexList;
extern Triangle *gTriangleHeap[2];
extern Triangle *gGameCurrTriList;
extern u8 D_800FB440[];         /* the scheduler */
extern s32 D_800FD748;
extern u8 D_800FD7A8[];
extern s32 D_800FD7B8;
extern s32 gSPTaskNum;
extern s32 gPrevPlayerCount;
extern void *gSettingsPtr;
extern s32 gCurrNumF3dCmdsPerPlayer;
extern s32 gCurrNumHudMatPerPlayer;
extern s32 gCurrNumHudTrisPerPlayer;
extern s32 gCurrNumHudVertsPerPlayer;
extern u8 D_800FD7F0[];
extern u8 D_800FD7FC[];
extern u8 D_800FD800[];
extern s32 D_800FD818;

static s32 sNumF3dCmdsPerPlayer[2][4] = { { 4500, 7168, 10240, 12288 }, { 5120, 5632, 6144, 6656 } };
static s32 sNumVertsPerPlayer[2][4] = { { 800, 1024, 1280, 1536 }, { 1024, 1024, 1024, 1024 } };
static s32 sNumMatPerPlayer[2][4] = { { 280, 384, 512, 640 }, { 300, 300, 300, 300 } };
static s32 sNumTrisPerPlayer[2][4] = { { 20, 30, 40, 50 }, { 20, 20, 20, 20 } };
static u32 sM036Key[4] = { 0xEC3CB676, 0xB81DBB8F, 0x6B3A80EC, 0xEDEA5B02 };

static void func_m036_00250(void);
static void default_alloc_displaylist_heap(void);

void func_m036_00000(void) {
    check_reset_and_halt();
    func_m025_00308(D_800FB440);
    func_8005342C(0x19);
    check_reset_and_halt();
    audspat_init();
    D_800A32B0 = func_80042FEC();
    func_800554B0();
    check_reset_and_halt();
    func_8003B8AC();
    check_reset_and_halt();
    func_m034_00000();
    func_8005342C(0x22);
    check_reset_and_halt();
    func_80029B78();
    func_800436C0(sM036Key, D_800FD7A8);
    func_8005342C(0x2D);
    check_reset_and_halt();
    func_80064D9C();
    check_reset_and_halt();
    func_m026_002F8();
    func_8005342C(0x1A);
    check_reset_and_halt();
    func_8005AB00();
    check_reset_and_halt();
    func_m036_00250();
    check_reset_and_halt();
    func_8006D0D8();
    func_80068450();
    check_reset_and_halt();
    func_800239D0();
    check_reset_and_halt();
    func_m035_00000();
    func_8005342C(0x23);
    check_reset_and_halt();
    func_8004C2BC();
    check_reset_and_halt();
    func_80058590();
    check_reset_and_halt();
    func_80076EA0();
    func_8006B8C4();
    check_reset_and_halt();
    func_80052678();
    func_80065CE0();
    func_80083A90();
    default_alloc_displaylist_heap();
    func_80096BB0(D_800FD800, D_800FD7FC, 1);
    func_8004F85C(D_800FB440, D_800FD7F0, D_800FD800, 3);
    D_800FD818 = 0;
    check_reset_and_halt();
    D_800FD7B8 = func_8000E624(0x34);
    gSPTaskNum = 0;
    gCurrDisplayList = gDisplayLists[gSPTaskNum];
    gDPFullSync(gCurrDisplayList++);
    gSPEndDisplayList(gCurrDisplayList++);
    func_80097F70(0, &gCurrDisplayList);
    func_80048E20(0xD43E);
}

static void func_m036_00250(void) {
    func_80043750();
    D_800FD748 = 0;
    gSettingsPtr = func_8004A57C(0x538, -1);
    func_800476F0(gSettingsPtr);
    func_8004791C();
    D_800A32A8 = 0x04000000;
}

/* DKR alloc_displaylist_heap */
void func_m036_002A8(s32 numberOfPlayers) {
    s32 num;
    s32 totalSize;
    s32 state;

    if (numberOfPlayers != gPrevPlayerCount) {
        gPrevPlayerCount = numberOfPlayers;
        num = numberOfPlayers;
        state = mempool_free_timer_get();
        func_8004AA08(0);
        totalSize = (sNumF3dCmdsPerPlayer[0][num] * sizeof(Gfx)) + (sNumMatPerPlayer[0][num] * sizeof(Mtx)) +
                    (sNumVertsPerPlayer[0][num] * sizeof(Vertex)) + (sNumTrisPerPlayer[0][num] * sizeof(Triangle));
        func_8004AA80(gDisplayLists[0]);
        func_8004AA80(gDisplayLists[1]);
        gDisplayLists[0] = (Gfx *) func_8004A828(totalSize, gDisplayLists[0], 0xFF0000FF);
        gDisplayLists[1] = (Gfx *) func_8004A828(totalSize, gDisplayLists[1], 0xFFFF00FF);
        if (gDisplayLists[0] == NULL || gDisplayLists[1] == NULL) {
            if (gDisplayLists[0] != NULL) {
                func_8004AA80(gDisplayLists[0]);
                gDisplayLists[0] = NULL;
            }
            if (gDisplayLists[1] != NULL) {
                func_8004AA80(gDisplayLists[1]);
                gDisplayLists[1] = NULL;
            }
            default_alloc_displaylist_heap();
        }
        gMatrixHeap[0] = (Mtx *) ((u8 *) gDisplayLists[0] + (sNumF3dCmdsPerPlayer[0][num] * sizeof(Gfx)));
        gTriangleHeap[0] = (Triangle *) ((u8 *) gMatrixHeap[0] + (sNumMatPerPlayer[0][num] * sizeof(Mtx)));
        gVertexHeap[0] = (Vertex *) ((u8 *) gTriangleHeap[0] + (sNumTrisPerPlayer[0][num] * sizeof(Triangle)));
        gMatrixHeap[1] = (Mtx *) ((u8 *) gDisplayLists[1] + (sNumF3dCmdsPerPlayer[0][num] * sizeof(Gfx)));
        gTriangleHeap[1] = (Triangle *) ((u8 *) gMatrixHeap[1] + (sNumMatPerPlayer[0][num] * sizeof(Mtx)));
        gVertexHeap[1] = (Vertex *) ((u8 *) gTriangleHeap[1] + (sNumTrisPerPlayer[0][num] * sizeof(Triangle)));
        gCurrNumF3dCmdsPerPlayer = sNumF3dCmdsPerPlayer[0][num];
        gCurrNumHudMatPerPlayer = sNumMatPerPlayer[0][num];
        gCurrNumHudTrisPerPlayer = sNumTrisPerPlayer[0][num];
        gCurrNumHudVertsPerPlayer = sNumVertsPerPlayer[0][num];
        func_8004AA08(state);
    }
    gCurrDisplayList = gDisplayLists[gSPTaskNum];
    gGameCurrMatrix = gMatrixHeap[gSPTaskNum];
    gGameCurrTriList = gTriangleHeap[gSPTaskNum];
    gGameCurrVertexList = gVertexHeap[gSPTaskNum];
    gDPFullSync(gCurrDisplayList++);
    gSPEndDisplayList(gCurrDisplayList++);
}

/* DKR default_alloc_displaylist_heap (module m036, 0x00564). */
static void default_alloc_displaylist_heap(void) {
    s32 numberOfPlayers;
    s32 totalSize;

    numberOfPlayers = 3;
    gPrevPlayerCount = numberOfPlayers;
    totalSize = (sNumF3dCmdsPerPlayer[0][numberOfPlayers] * sizeof(Gfx)) +
                (sNumMatPerPlayer[0][numberOfPlayers] * sizeof(Mtx)) +
                (sNumVertsPerPlayer[0][numberOfPlayers] * sizeof(Vertex)) +
                (sNumTrisPerPlayer[0][numberOfPlayers] * sizeof(Triangle));

    gDisplayLists[0] = (Gfx *) func_8004A57C(totalSize, 0xFF0000FF);
    gMatrixHeap[0] = (Mtx *) ((u8 *) gDisplayLists[0] + (sNumF3dCmdsPerPlayer[0][numberOfPlayers] * sizeof(Gfx)));
    gVertexHeap[0] = (Vertex *) ((u8 *) gMatrixHeap[0] + (sNumMatPerPlayer[0][numberOfPlayers] * sizeof(Mtx)));
    gTriangleHeap[0] =
        (Triangle *) ((u8 *) gVertexHeap[0] + (sNumVertsPerPlayer[0][numberOfPlayers] * sizeof(Vertex)));

    gDisplayLists[1] = (Gfx *) func_8004A57C(totalSize, 0xFFFF00FF);
    gMatrixHeap[1] = (Mtx *) ((u8 *) gDisplayLists[1] + (sNumF3dCmdsPerPlayer[0][numberOfPlayers] * sizeof(Gfx)));
    gVertexHeap[1] = (Vertex *) ((u8 *) gMatrixHeap[1] + (sNumMatPerPlayer[0][numberOfPlayers] * sizeof(Mtx)));
    gTriangleHeap[1] =
        (Triangle *) ((u8 *) gVertexHeap[1] + (sNumVertsPerPlayer[0][numberOfPlayers] * sizeof(Vertex)));

    gCurrNumF3dCmdsPerPlayer = sNumF3dCmdsPerPlayer[0][numberOfPlayers];
    gCurrNumHudMatPerPlayer = sNumMatPerPlayer[0][numberOfPlayers];
    gCurrNumHudTrisPerPlayer = sNumTrisPerPlayer[0][numberOfPlayers];
    gCurrNumHudVertsPerPlayer = sNumVertsPerPlayer[0][numberOfPlayers];
}
