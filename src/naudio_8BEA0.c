#include "common.h"

/*
 * n_audio (built -g -mips2, see the Makefile): JFG's surround output settings. The output type picks which
 * reverb routings are on; each of the two reverbs then has its routing flags set from its mode.
 */

extern u8 D_80105010;    // JFG: output type 4
extern u8 D_80105011;    // JFG: output type 1
extern u8 D_80105012;    // JFG: output type 3
extern u8 D_80105014[2]; // JFG: per reverb: routing flag A
extern u8 D_80105016[2]; // JFG: per reverb: routing flag B
extern u8 D_80105018[2]; // JFG: per reverb: its mode

void func_8008B374(s32 reverb, s32 mode);

/* JFG: set the output type, then redo both reverbs' routing in their current modes */
void func_8008B2A0(u8 type) {
    s32 reverb;

    D_80105010 = 0;
    D_80105011 = 0;
    D_80105012 = 0;
    switch (type) {
        case 1:
            D_80105011 = 1;
            break;
        case 3:
            D_80105012 = 1;
            break;
        case 4:
            D_80105010 = 1;
            break;
    }
    for (reverb = 0; reverb < 2; reverb++) {
        func_8008B374(reverb, 0);
    }
}

/* JFG: set a reverb's mode (0 keeps the current one) and its routing flags for the output type */
void func_8008B374(s32 reverb, s32 mode) {
    if (mode == 0) {
        mode = D_80105018[reverb];
    }
    D_80105014[reverb] = 0;
    D_80105016[reverb] = 0;
    switch (mode) {
        case 2:
            if (D_80105010) {
                D_80105016[reverb] = 1;
            }
            break;
        case 3:
            if (D_80105010) {
                D_80105014[reverb] = 1;
            }
            break;
        case 4:
            if (!D_80105011) {
                D_80105014[reverb] = 1;
            }
            break;
        case 5:
            if (!D_80105011) {
                D_80105014[reverb] = 1;
                D_80105016[reverb] = 1;
            }
            break;
    }
    D_80105018[reverb] = mode;
}
