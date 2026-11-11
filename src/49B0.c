#include "common.h"

/* JFG: the oscillators (vibrato and tremolo) of the sound player: a separate object from 3430.c (its own
 * .rodata at 0x800AA180). The two reports are stubbed out; their strings stay, in this order. */
const char D_800AA180[] = "am: Unknown OSCILLATOR type %d\n";
const char D_800AA1A0[] = "am: Out of oscillator states\n";

/* JFG: a node of the D_800F2B10 free list */
typedef struct FreeNode {
    /* 0x00 */ struct FreeNode *next;
    /* 0x04 */ u8 pad4[0x10];
} FreeNode;

extern FreeNode *D_800F2B10;
extern FreeNode D_800F2B18[16];

/* JFG: link all the nodes into the free list. */
void func_80003DB0(void) {
    s32 i;
    FreeNode *node;

    D_800F2B10 = node = D_800F2B18;
    for (i = 1; i < 16; i++) {
        node->next = &D_800F2B18[i];
        node = node->next;
    }
    node->next = NULL;
}

/* JFG: an oscillator (vibrato/tremolo) state, taken from the D_800F2B10 free list */
typedef struct OscState {
    /* 0x00 */ struct OscState *next;
    /* 0x04 */ u8 type;
    /* 0x05 */ u8 stateFlags;
    /* 0x06 */ u16 cycleSpan;
    /* 0x08 */ u16 curCount;
    /* 0x0A */ u8 padA[2];
    union {
        struct {
            /* 0x0C */ u8 halfDepth;
            /* 0x0D */ u8 baseVol;
            /* 0x0E */ u8 loVal;
        } tsin;
        struct {
            /* 0x0C */ f32 loRatio;
            /* 0x10 */ f32 hiRatio;
        } vsqr;
        struct {
            /* 0x0C */ s32 hiCents;
            /* 0x10 */ s32 detuneScale;
        } vsaw;
        /* 0x0C */ f32 depthcents;
    } data;
} OscState;

f32 func_80004768(u8 depth);
f32 alCents2Ratio(s32 cents);

#ifdef NON_MATCHING
/* NON_MATCHING: temporaries only: the target keeps the shared 127 in $v0 (mine $v1) and the cycle spans in
   t-registers (mine $v0), so the t-register numbering in the cases is shifted */
/* JFG: start an oscillator of the given type (1-4: tremolo shapes, 0x80-0x83: vibrato shapes); returns its delay
 * (in 1/0x4000 units), 0 when none is free. */
s32 func_80003E40(void **oscState, f32 *initVal, u8 oscType, u8 oscRate, u8 oscDepth, u8 oscDelay) {
    OscState *osc;
    s32 retVal = 0;
    s32 cents;

    if (D_800F2B10 != NULL) {
        osc = (OscState *) D_800F2B10;
        D_800F2B10 = (FreeNode *) osc->next;
        osc->type = oscType;
        *oscState = osc;
        retVal = oscDelay << 14;
        switch (oscType) {
            case 1:
                osc->curCount = 0;
                osc->cycleSpan = 259 - oscRate;
                osc->data.tsin.halfDepth = oscDepth >> 1;
                osc->data.tsin.baseVol = 127 - osc->data.tsin.halfDepth;
                *initVal = osc->data.tsin.baseVol;
                break;
            case 2:
                osc->cycleSpan = 256 - oscRate;
                osc->curCount = 256 - oscRate;
                osc->stateFlags = 0;
                osc->data.tsin.loVal = 127 - oscDepth;
                osc->data.tsin.baseVol = 127;
                osc->data.tsin.halfDepth = 127;
                *initVal = 127.0f;
                break;
            case 3:
                osc->cycleSpan = 256 - oscRate;
                osc->curCount = 0;
                osc->data.tsin.halfDepth = 127;
                osc->data.tsin.baseVol = oscDepth;
                *initVal = osc->data.tsin.halfDepth;
                break;
            case 4:
                osc->cycleSpan = 256 - oscRate;
                osc->curCount = 0;
                osc->data.tsin.halfDepth = 127 - oscDepth;
                osc->data.tsin.baseVol = oscDepth;
                *initVal = osc->data.tsin.halfDepth;
                break;
            case 0x80:
                osc->data.depthcents = func_80004768(oscDepth);
                osc->curCount = 0;
                osc->cycleSpan = 259 - oscRate;
                *initVal = 1.0f;
                break;
            case 0x81:
                osc->cycleSpan = 256 - oscRate;
                osc->curCount = 256 - oscRate;
                osc->stateFlags = 0;
                cents = func_80004768(oscDepth);
                osc->data.vsqr.loRatio = alCents2Ratio(-cents);
                osc->data.vsqr.hiRatio = alCents2Ratio(cents);
                *initVal = osc->data.vsqr.hiRatio;
                break;
            case 0x82:
                osc->cycleSpan = 256 - oscRate;
                osc->curCount = 256 - oscRate;
                cents = func_80004768(oscDepth);
                osc->data.vsaw.hiCents = cents;
                osc->data.vsaw.detuneScale = cents * 2;
                *initVal = alCents2Ratio(cents);
                break;
            case 0x83:
                osc->cycleSpan = 256 - oscRate;
                osc->curCount = 256 - oscRate;
                cents = func_80004768(oscDepth);
                osc->data.vsaw.hiCents = -cents;
                osc->data.vsaw.detuneScale = cents * 2;
                *initVal = alCents2Ratio(-cents);
                break;
        }
    }
    return retVal;
}
#else
INCLUDE_ASM("asm/nonmatchings/49B0", func_80003E40);
#endif


/* JFG: step an oscillator and write its new value (a volume for the tremolos, a pitch ratio for the vibratos);
 * returns the time until the next update (one frame, or the half period for the square waves) */
s32 osc_update(void *oscState, f32 *updateVal) {
    f32 t;
    OscState *osc = (OscState *) oscState;
    s32 deltaTime = 16000;

    switch (osc->type) {
        case 1:
            if (++osc->curCount >= osc->cycleSpan) {
                osc->curCount = 0;
            }
            t = (f32) osc->curCount / (f32) osc->cycleSpan;
            t = sinf(t * 6.2831855f);
            t = (f32) osc->data.tsin.halfDepth * t;
            *updateVal = (f32) osc->data.tsin.baseVol + t;
            break;
        case 2:
            if (osc->stateFlags == 0) {
                *updateVal = (f32) osc->data.tsin.loVal;
                osc->stateFlags = 1;
            } else {
                *updateVal = (f32) osc->data.tsin.baseVol;
                osc->stateFlags = 0;
            }
            deltaTime = osc->cycleSpan * 16000;
            break;
        case 3:
            if (++osc->curCount > osc->cycleSpan) {
                osc->curCount = 0;
            }
            t = (f32) osc->curCount / (f32) osc->cycleSpan;
            t *= (f32) osc->data.tsin.baseVol;
            *updateVal = (f32) osc->data.tsin.halfDepth - t;
            break;
        case 4:
            if (++osc->curCount > osc->cycleSpan) {
                osc->curCount = 0;
            }
            t = (f32) osc->curCount / (f32) osc->cycleSpan;
            t *= (f32) osc->data.tsin.baseVol;
            *updateVal = (f32) osc->data.tsin.halfDepth + t;
            break;
        case 0x80:
            if (++osc->curCount >= osc->cycleSpan) {
                osc->curCount = 0;
            }
            t = (f32) osc->curCount / (f32) osc->cycleSpan;
            t = sinf(t * 6.2831855f) * osc->data.depthcents;
            *updateVal = alCents2Ratio(t);
            break;
        case 0x81:
            if (osc->stateFlags == 0) {
                osc->stateFlags = 1;
                *updateVal = osc->data.vsqr.loRatio;
            } else {
                osc->stateFlags = 0;
                *updateVal = osc->data.vsqr.hiRatio;
            }
            deltaTime = osc->cycleSpan * 16000;
            break;
        case 0x82:
            if (++osc->curCount > osc->cycleSpan) {
                osc->curCount = 0;
            }
            t = (f32) osc->curCount / (f32) osc->cycleSpan;
            t *= (f32) osc->data.vsaw.detuneScale;
            t = (f32) osc->data.vsaw.hiCents - t;
            *updateVal = alCents2Ratio(t);
            break;
        case 0x83:
            if (++osc->curCount > osc->cycleSpan) {
                osc->curCount = 0;
            }
            t = (f32) osc->curCount / (f32) osc->cycleSpan;
            t *= (f32) osc->data.vsaw.detuneScale;
            t += (f32) osc->data.vsaw.hiCents;
            *updateVal = alCents2Ratio(t);
            break;
    }
    return deltaTime;
}

/* JFG: push a node onto a free list. */
void func_8000474C(FreeNode *node) {
    node->next = D_800F2B10;
    D_800F2B10 = node;
}

/* JFG: 1.030993 (the ratio of about 53 cents) raised to the power of depth (by squaring). */
f32 func_80004768(u8 depth) {
    f32 base = 1.030993f;
    f32 result = 1.0f;

    while (depth) {
        if (depth & 1) {
            result *= base;
        }
        base *= base;
        depth >>= 1;
    }
    return result;
}
