/*
 * sprintf (glibc 1.09 style) and the on-screen debug text printer.
 *
 * From the Diddy Kong Racing decompilation (CC0 1.0), src/printf.c, plus
 * memset from its src/unused_string.c. Functions that still need their
 * .rodata (strings, jump tables) or differ in JFG are assembly for now.
 */
#include "printf.h"

/************ .data ************/

extern s32 gSprintfSpacingCode; // Used to determine if vsprintf should use fixed width mode or not

/************ .bss ************/

extern u16 gDebugTextX;
extern u16 gDebugTextY;
extern s32 gDebugBoundsX1;
extern s32 gDebugBoundsX2;
extern s32 gDebugBoundsY1;
extern s32 gDebugBoundsY2;
extern u16 gDebugScreenWidth;
extern u16 gDebugScreenHeight;
extern char *gDebugPrintBufferEnd;

/************ .rodata ************/

extern const char _itoa_lower_digits[]; // "0123456789abcdefghijklmnopqrstuvwxyz"
extern const char _itoa_upper_digits[]; // "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"

/******************************/

/**
 * memset(void *s, int c, size_t n)
 *
 * s: start of area to clear
 * c: char to fill with
 * n: size of area to clear
 */
void *memset(void *s, int c, size_t n) {
    unsigned char *ret = s;

    while (n-- > 0) {
        *ret++ = c;
    }
    return s;
}

/**
 * Standard C Library function that converts integers to strings.
 * Exact match to glibc 1.09 version.
 */
char *_itoa(unsigned long long int n, char *buflim, unsigned int base, int upper_case) {
    /* Base-36 digits for numbers. */
    const char *alphabet = upper_case ? _itoa_upper_digits : _itoa_lower_digits;
    register char *bp = buflim;

    while (n > 0) {
        *(--bp) = alphabet[n % base];
        n /= base;
    }

    return bp;
}

/**
 * Used to determine if vsprintf should use fixed width mode or not
 * Official Name: sprintfSetSpacingCodes
 */
void sprintfSetSpacingCodes(s32 setting) {
    gSprintfSpacingCode = setting;
}

/**
 * Exact match to glibc 1.09 version.
 * Official name: sprintf
 */
int sprintf(char *s, const char *format, ...) {
    va_list arg;
    int done;

    va_start(arg, format);
    done = vsprintf(s, format, arg);
    va_end(arg);

    return done;
}

INCLUDE_ASM("asm/nonmatchings/printf", func_80063A14);

INCLUDE_ASM("asm/nonmatchings/printf", func_80064D9C);

INCLUDE_ASM("asm/nonmatchings/printf", func_80064DF0);

INCLUDE_ASM("asm/nonmatchings/printf", func_80064E8C);

/**
 * Set the colour of the current debug text.
 * Official Name: diPrintfSetCol
 */
void set_render_printf_colour(u8 red, u8 green, u8 blue, u8 alpha) {
    RENDER_PRINTF_CMD_SET_COLOR(red, green, blue, alpha)
}

/**
 * Set the background colour of the current debug text.
 * Official Name: diPrintfSetBG
 */
void set_render_printf_background_colour(u8 red, u8 green, u8 blue, u8 alpha) {
    RENDER_PRINTF_CMD_SET_BACKGROUND_COLOR(red, green, blue, alpha)
}

/**
 * Sets the character position of further prints to the given coordinates.
 * Official name: diPrintfSetXY
 */
void set_render_printf_position(u16 x, u16 y){ RENDER_PRINTF_CMD_SET_POSITION(x, y) }

INCLUDE_ASM("asm/nonmatchings/printf", func_800652E4);

/**
 * Render the background for the debug text.
 */
void debug_text_background(Gfx **dList, u32 ulx, u32 uly, u32 lrx, u32 lry) {
    if (!((ulx == lrx) | (uly == lry))) {
        if (ulx >= 2) {
            ulx -= 2;
        }
        lrx += 2;
        gDPSetCombineMode((*dList)++, G_CC_PRIMITIVE, G_CC_PRIMITIVE);
        gDPFillRectangle((*dList)++, ulx, uly, lrx, lry);
    }
}

INCLUDE_ASM("asm/nonmatchings/printf", func_800657A4);

/**
 * Sets the upper and lower boundaries of the debug text.
 */
void debug_text_bounds(void) {
    if (gDebugScreenWidth <= 320) {
        gDebugBoundsX1 = 16;
        gDebugBoundsX2 = gDebugScreenWidth - 16;
    } else {
        gDebugBoundsX1 = 32;
        gDebugBoundsX2 = gDebugScreenWidth - 32;
    }
    if (gDebugScreenHeight <= 240) {
        gDebugBoundsY1 = 16;
        gDebugBoundsY2 = gDebugScreenHeight - 16;
    } else {
        gDebugBoundsY1 = 32;
        gDebugBoundsY2 = gDebugScreenHeight - 32;
    }
}

/**
 * Sets the print position of the text back to top left.
 */
void debug_text_origin(void) {
    gDebugTextX = gDebugBoundsX1;
    gDebugTextY = gDebugBoundsY1;
}

/**
 * Moves the print position of the text down one line.
 */
void debug_text_newline(void) {
    gDebugTextX = gDebugBoundsX1;
    gDebugTextY += 11;
}
