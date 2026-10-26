#include "common.h"
#include "joypad.h"

/* From the Diddy Kong Racing decompilation (CC0 1.0); see CREDITS.md. */
extern OSMesgQueue gNMIMesgQueue;
extern s32 gGameMode;
extern s32 gNMIMesgBuf;          // Official Name: resetPressed;
extern s32 sControllerStatus;

INCLUDE_ASM("asm/nonmatchings/45750", func_80044B50);

INCLUDE_ASM("asm/nonmatchings/45750", func_80044C20);

INCLUDE_ASM("asm/nonmatchings/45750", func_80044CF0);

INCLUDE_ASM("asm/nonmatchings/45750", func_80044DFC);

INCLUDE_ASM("asm/nonmatchings/45750", func_80044F24);

INCLUDE_ASM("asm/nonmatchings/45750", func_80044FAC);

INCLUDE_ASM("asm/nonmatchings/45750", func_8004581C);

INCLUDE_ASM("asm/nonmatchings/45750", func_80045914);

INCLUDE_ASM("asm/nonmatchings/45750", func_80045A14);

INCLUDE_ASM("asm/nonmatchings/45750", func_80045A44);

INCLUDE_ASM("asm/nonmatchings/45750", func_80045B98);

INCLUDE_ASM("asm/nonmatchings/45750", func_80045C38);

INCLUDE_ASM("asm/nonmatchings/45750", func_80045DC0);

INCLUDE_ASM("asm/nonmatchings/45750", func_80045DD0);

INCLUDE_ASM("asm/nonmatchings/45750", func_80045E04);

INCLUDE_ASM("asm/nonmatchings/45750", func_80045F00);

INCLUDE_ASM("asm/nonmatchings/45750", func_80046334);

INCLUDE_ASM("asm/nonmatchings/45750", func_80046378);

INCLUDE_ASM("asm/nonmatchings/45750", func_80046550);

INCLUDE_ASM("asm/nonmatchings/45750", func_8004665C);

INCLUDE_ASM("asm/nonmatchings/45750", func_800468A4);

INCLUDE_ASM("asm/nonmatchings/45750", func_800468B0);

INCLUDE_ASM("asm/nonmatchings/45750", func_800468C0);

INCLUDE_ASM("asm/nonmatchings/45750", func_800468CC);

INCLUDE_ASM("asm/nonmatchings/45750", func_800468DC);

INCLUDE_ASM("asm/nonmatchings/45750", func_800468EC);

INCLUDE_ASM("asm/nonmatchings/45750", func_80047570);

/**
 *  Sets the current game mode.
 *  Official Name: mainSetMode?
 */
void set_game_mode(s32 changeTo) {
    gGameMode = changeTo;
}

INCLUDE_ASM("asm/nonmatchings/45750", func_8004758C);

INCLUDE_ASM("asm/nonmatchings/45750", func_80047608);

INCLUDE_ASM("asm/nonmatchings/45750", func_80047674);

INCLUDE_ASM("asm/nonmatchings/45750", func_800476F0);

INCLUDE_ASM("asm/nonmatchings/45750", func_80047868);

INCLUDE_ASM("asm/nonmatchings/45750", func_800478A4);

INCLUDE_ASM("asm/nonmatchings/45750", func_800478E0);

INCLUDE_ASM("asm/nonmatchings/45750", func_8004791C);

INCLUDE_ASM("asm/nonmatchings/45750", func_8004797C);

INCLUDE_ASM("asm/nonmatchings/45750", func_80047A20);

INCLUDE_ASM("asm/nonmatchings/45750", func_80047AE0);

INCLUDE_ASM("asm/nonmatchings/45750", func_80047C0C);

INCLUDE_ASM("asm/nonmatchings/45750", func_80047C1C);

INCLUDE_ASM("asm/nonmatchings/45750", func_80047C9C);

INCLUDE_ASM("asm/nonmatchings/45750", func_80047D0C);

INCLUDE_ASM("asm/nonmatchings/45750", func_80047D80);

INCLUDE_ASM("asm/nonmatchings/45750", func_80047DD8);

INCLUDE_ASM("asm/nonmatchings/45750", func_80047E44);

INCLUDE_ASM("asm/nonmatchings/45750", func_80047E8C);

INCLUDE_ASM("asm/nonmatchings/45750", func_80047ED0);

INCLUDE_ASM("asm/nonmatchings/45750", func_80047EFC);

INCLUDE_ASM("asm/nonmatchings/45750", func_80048130);

INCLUDE_ASM("asm/nonmatchings/45750", func_8004822C);

INCLUDE_ASM("asm/nonmatchings/45750", func_800482A0);

INCLUDE_ASM("asm/nonmatchings/45750", func_800482F8);

INCLUDE_ASM("asm/nonmatchings/45750", func_80048308);

/**
 * Sets and returns (nonzero) the message set when pressing the reset button.
 * Official name: mainResetPressed
 */
s32 is_reset_pressed(void) {
    if (gNMIMesgBuf == 0) {
        gNMIMesgBuf = (s32) ((osRecvMesg(&gNMIMesgQueue, NULL, OS_MESG_NOBLOCK) + 1) != 0);
    }
    return gNMIMesgBuf;
}

INCLUDE_ASM("asm/nonmatchings/45750", func_800483B8);

INCLUDE_ASM("asm/nonmatchings/45750", func_800483C8);

INCLUDE_ASM("asm/nonmatchings/45750", func_800483F8);

INCLUDE_ASM("asm/nonmatchings/45750", func_8004841C);

INCLUDE_ASM("asm/nonmatchings/45750", func_8004842C);

INCLUDE_ASM("asm/nonmatchings/45750", func_80048460);

INCLUDE_ASM("asm/nonmatchings/45750", func_800484A4);

INCLUDE_ASM("asm/nonmatchings/45750", func_800484C0);

INCLUDE_ASM("asm/nonmatchings/45750", func_800484D0);

INCLUDE_ASM("asm/nonmatchings/45750", func_800484E0);

INCLUDE_ASM("asm/nonmatchings/45750", func_800484FC);

INCLUDE_ASM("asm/nonmatchings/45750", func_80048518);

/**
 * Returns TRUE if the game doesn't detect any controllers.
 * Official name: mainDemoOnly
 */
s32 is_controller_missing(void) {
    if (sControllerStatus == CONTROLLER_MISSING) {
        return TRUE;
    } else {
        return FALSE;
    }
}

INCLUDE_ASM("asm/nonmatchings/45750", func_80048548);

INCLUDE_ASM("asm/nonmatchings/45750", func_80048558);

INCLUDE_ASM("asm/nonmatchings/45750", func_800485C8);

INCLUDE_ASM("asm/nonmatchings/45750", func_80048630);

INCLUDE_ASM("asm/nonmatchings/45750", func_800487B8);

INCLUDE_ASM("asm/nonmatchings/45750", func_8004883C);
