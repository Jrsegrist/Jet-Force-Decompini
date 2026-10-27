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

extern void func_80044C20();
extern void func_m025_00308(void *sched);
extern void func_8005342C(s32 arg0);
extern void func_80002830(void);
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
extern s32 func_8004AEAC(void);
extern void func_8004AA08(s32 state);

extern s32 D_800A32A8;
extern s32 D_800A32B0;
extern Gfx *D_800FB400[2];      /* DKR gDisplayLists */
extern Gfx *D_800FB408;         /* DKR gCurrDisplayList */
extern Mtx *D_800FB410[2];      /* DKR gMatrixHeap */
extern Mtx *D_800FB418;         /* DKR gGameCurrMatrix */
extern Vertex *D_800FB420[2];   /* DKR gVertexHeap */
extern Vertex *D_800FB428;      /* DKR gGameCurrVertexList */
extern Triangle *D_800FB430[2]; /* DKR gTriangleHeap */
extern Triangle *D_800FB438;    /* DKR gGameCurrTriList */
extern u8 D_800FB440[];         /* the scheduler */
extern s32 D_800FD748;
extern u8 D_800FD7A8[];
extern s32 D_800FD7B8;
extern s32 D_800FD7C0; /* DKR gSPTaskNum */
extern s32 D_800FD7CC; /* DKR gPrevPlayerCount */
extern void *D_800FD7D4;
extern s32 D_800FD7DC; /* DKR gCurrNumF3dCmdsPerPlayer */
extern s32 D_800FD7E0; /* DKR gCurrNumHudMatPerPlayer */
extern s32 D_800FD7E4; /* DKR gCurrNumHudTrisPerPlayer */
extern s32 D_800FD7E8; /* DKR gCurrNumHudVertsPerPlayer */
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
static void func_m036_00564(void);

void func_m036_00000(void) {
    func_80044C20();
    func_m025_00308(D_800FB440);
    func_8005342C(0x19);
    func_80044C20();
    func_80002830();
    D_800A32B0 = func_80042FEC();
    func_800554B0();
    func_80044C20();
    func_8003B8AC();
    func_80044C20();
    func_m034_00000();
    func_8005342C(0x22);
    func_80044C20();
    func_80029B78();
    func_800436C0(sM036Key, D_800FD7A8);
    func_8005342C(0x2D);
    func_80044C20();
    func_80064D9C();
    func_80044C20();
    func_m026_002F8();
    func_8005342C(0x1A);
    func_80044C20();
    func_8005AB00();
    func_80044C20();
    func_m036_00250();
    func_80044C20();
    func_8006D0D8();
    func_80068450();
    func_80044C20();
    func_800239D0();
    func_80044C20();
    func_m035_00000();
    func_8005342C(0x23);
    func_80044C20();
    func_8004C2BC();
    func_80044C20();
    func_80058590();
    func_80044C20();
    func_80076EA0();
    func_8006B8C4();
    func_80044C20();
    func_80052678();
    func_80065CE0();
    func_80083A90();
    func_m036_00564();
    func_80096BB0(D_800FD800, D_800FD7FC, 1);
    func_8004F85C(D_800FB440, D_800FD7F0, D_800FD800, 3);
    D_800FD818 = 0;
    func_80044C20();
    D_800FD7B8 = func_8000E624(0x34);
    D_800FD7C0 = 0;
    D_800FB408 = D_800FB400[D_800FD7C0];
    gDPFullSync(D_800FB408++);
    gSPEndDisplayList(D_800FB408++);
    func_80097F70(0, &D_800FB408);
    func_80048E20(0xD43E);
}

static void func_m036_00250(void) {
    func_80043750();
    D_800FD748 = 0;
    D_800FD7D4 = func_8004A57C(0x538, -1);
    func_800476F0(D_800FD7D4);
    func_8004791C();
    D_800A32A8 = 0x04000000;
}

/* DKR alloc_displaylist_heap */
void func_m036_002A8(s32 numberOfPlayers) {
    s32 num;
    s32 totalSize;
    s32 state;

    if (numberOfPlayers != D_800FD7CC) {
        D_800FD7CC = numberOfPlayers;
        num = numberOfPlayers;
        state = func_8004AEAC();
        func_8004AA08(0);
        totalSize = (sNumF3dCmdsPerPlayer[0][num] * sizeof(Gfx)) + (sNumMatPerPlayer[0][num] * sizeof(Mtx)) +
                    (sNumVertsPerPlayer[0][num] * sizeof(Vertex)) + (sNumTrisPerPlayer[0][num] * sizeof(Triangle));
        func_8004AA80(D_800FB400[0]);
        func_8004AA80(D_800FB400[1]);
        D_800FB400[0] = (Gfx *) func_8004A828(totalSize, D_800FB400[0], 0xFF0000FF);
        D_800FB400[1] = (Gfx *) func_8004A828(totalSize, D_800FB400[1], 0xFFFF00FF);
        if (D_800FB400[0] == NULL || D_800FB400[1] == NULL) {
            if (D_800FB400[0] != NULL) {
                func_8004AA80(D_800FB400[0]);
                D_800FB400[0] = NULL;
            }
            if (D_800FB400[1] != NULL) {
                func_8004AA80(D_800FB400[1]);
                D_800FB400[1] = NULL;
            }
            func_m036_00564();
        }
        D_800FB410[0] = (Mtx *) ((u8 *) D_800FB400[0] + (sNumF3dCmdsPerPlayer[0][num] * sizeof(Gfx)));
        D_800FB430[0] = (Triangle *) ((u8 *) D_800FB410[0] + (sNumMatPerPlayer[0][num] * sizeof(Mtx)));
        D_800FB420[0] = (Vertex *) ((u8 *) D_800FB430[0] + (sNumTrisPerPlayer[0][num] * sizeof(Triangle)));
        D_800FB410[1] = (Mtx *) ((u8 *) D_800FB400[1] + (sNumF3dCmdsPerPlayer[0][num] * sizeof(Gfx)));
        D_800FB430[1] = (Triangle *) ((u8 *) D_800FB410[1] + (sNumMatPerPlayer[0][num] * sizeof(Mtx)));
        D_800FB420[1] = (Vertex *) ((u8 *) D_800FB430[1] + (sNumTrisPerPlayer[0][num] * sizeof(Triangle)));
        D_800FD7DC = sNumF3dCmdsPerPlayer[0][num];
        D_800FD7E0 = sNumMatPerPlayer[0][num];
        D_800FD7E4 = sNumTrisPerPlayer[0][num];
        D_800FD7E8 = sNumVertsPerPlayer[0][num];
        func_8004AA08(state);
    }
    D_800FB408 = D_800FB400[D_800FD7C0];
    D_800FB418 = D_800FB410[D_800FD7C0];
    D_800FB438 = D_800FB430[D_800FD7C0];
    D_800FB428 = D_800FB420[D_800FD7C0];
    gDPFullSync(D_800FB408++);
    gSPEndDisplayList(D_800FB408++);
}

/* DKR default_alloc_displaylist_heap */
static void func_m036_00564(void) {
    s32 numberOfPlayers;
    s32 totalSize;

    numberOfPlayers = 3;
    D_800FD7CC = numberOfPlayers;
    totalSize = (sNumF3dCmdsPerPlayer[0][numberOfPlayers] * sizeof(Gfx)) +
                (sNumMatPerPlayer[0][numberOfPlayers] * sizeof(Mtx)) +
                (sNumVertsPerPlayer[0][numberOfPlayers] * sizeof(Vertex)) +
                (sNumTrisPerPlayer[0][numberOfPlayers] * sizeof(Triangle));

    D_800FB400[0] = (Gfx *) func_8004A57C(totalSize, 0xFF0000FF);
    D_800FB410[0] = (Mtx *) ((u8 *) D_800FB400[0] + (sNumF3dCmdsPerPlayer[0][numberOfPlayers] * sizeof(Gfx)));
    D_800FB420[0] = (Vertex *) ((u8 *) D_800FB410[0] + (sNumMatPerPlayer[0][numberOfPlayers] * sizeof(Mtx)));
    D_800FB430[0] =
        (Triangle *) ((u8 *) D_800FB420[0] + (sNumVertsPerPlayer[0][numberOfPlayers] * sizeof(Vertex)));

    D_800FB400[1] = (Gfx *) func_8004A57C(totalSize, 0xFFFF00FF);
    D_800FB410[1] = (Mtx *) ((u8 *) D_800FB400[1] + (sNumF3dCmdsPerPlayer[0][numberOfPlayers] * sizeof(Gfx)));
    D_800FB420[1] = (Vertex *) ((u8 *) D_800FB410[1] + (sNumMatPerPlayer[0][numberOfPlayers] * sizeof(Mtx)));
    D_800FB430[1] =
        (Triangle *) ((u8 *) D_800FB420[1] + (sNumVertsPerPlayer[0][numberOfPlayers] * sizeof(Vertex)));

    D_800FD7DC = sNumF3dCmdsPerPlayer[0][numberOfPlayers];
    D_800FD7E0 = sNumMatPerPlayer[0][numberOfPlayers];
    D_800FD7E4 = sNumTrisPerPlayer[0][numberOfPlayers];
    D_800FD7E8 = sNumVertsPerPlayer[0][numberOfPlayers];
}
