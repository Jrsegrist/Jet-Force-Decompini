/*
 * Rumble, EEPROM and controller pak access. Functions in C here are from the
 * Diddy Kong Racing decompilation (CC0 1.0), src/save_data.c; see CREDITS.md.
 */
#include "common.h"

/* DKR src/save_data.h, with Jet Force Gemini's values */
typedef enum SIDeviceStatus {
    CONTROLLER_PAK_GOOD = 0,
    CONTROLLER_PAK_NOT_FOUND = 1,
    CONTROLLER_PAK_INCONSISTENT = 2,
    CONTROLLER_PAK_WITH_BAD_ID = 3,
    CONTROLLER_PAK_FULL = 4,
    CONTROLLER_PAK_CHANGED = 5,
    CONTROLLER_PAK_BAD_DATA = 6,       // DKR: 9
    CONTROLLER_PAK_RUMBLE_PAK_FOUND = 8 // DKR: 7
} SIDeviceStatus;

#define COMPANY_CODE 0x3459 // "4Y", Rare
#define GHSS_SIZE 0x100
/* The controller pak game codes are still Diddy Kong Racing's. */
#define JPN_GAME_CODE 0x4E44594A  // "NDYJ"
#define PAL_GAME_CODE 0x4E445950  // "NDYP"
#define NTSC_GAME_CODE 0x4E445945 // "NDYE"
#define LANGUAGE_JAPANESE 4       // DKR: 5
#define OS_TV_TYPE_PAL OS_TV_PAL
#define REGION_JP 3
#define REGION 1

s32 get_language(void);
char *string_to_font_codes(char *inString, char *outString, s32 stringLength);

extern OSMesgQueue *sControllerMesgQueue;
extern OSPfs pfs[MAXCONTROLLERS];
extern s8 *gPakFileList;
extern u8 gN64FontCodes[];
extern u8 gRumblePresent; // Bits 0, 1, 2, and 3 of the bit pattern correspond to Controllers 1, 2, 3, and 4.

INCLUDE_ASM("asm/nonmatchings/4BC70", func_8004B070);

extern u8 D_800A3470;

void func_8004B0AC(s32 arg0) {
    D_800A3470 = arg0;
}

INCLUDE_ASM("asm/nonmatchings/4BC70", func_8004B0B8);

INCLUDE_ASM("asm/nonmatchings/4BC70", func_8004B0F4);

INCLUDE_ASM("asm/nonmatchings/4BC70", func_8004B204);

INCLUDE_ASM("asm/nonmatchings/4BC70", func_8004B29C);

INCLUDE_ASM("asm/nonmatchings/4BC70", func_8004B3D8);

INCLUDE_ASM("asm/nonmatchings/4BC70", func_8004B518);

extern u8 D_800A3464;

void func_8004B55C(void) {
    D_800A3464 = 0x1;
}

INCLUDE_ASM("asm/nonmatchings/4BC70", func_8004B56C);

INCLUDE_ASM("asm/nonmatchings/4BC70", func_8004BA50);

INCLUDE_ASM("asm/nonmatchings/4BC70", func_8004BA98);

s32 func_8004BAC8(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
    return 1;
}

s32 func_8004BAE0(s32 arg0, s32 arg1, s32 arg2) {
    return 0x1;
}

INCLUDE_ASM("asm/nonmatchings/4BC70", func_8004BAF4);

INCLUDE_ASM("asm/nonmatchings/4BC70", func_8004BC10);

INCLUDE_ASM("asm/nonmatchings/4BC70", func_8004BCB4);

INCLUDE_ASM("asm/nonmatchings/4BC70", func_8004BD78);

INCLUDE_ASM("asm/nonmatchings/4BC70", func_8004BE44);

INCLUDE_ASM("asm/nonmatchings/4BC70", func_8004BE6C);

INCLUDE_ASM("asm/nonmatchings/4BC70", func_8004BEE4);

INCLUDE_ASM("asm/nonmatchings/4BC70", func_8004BF2C);

INCLUDE_ASM("asm/nonmatchings/4BC70", func_8004BF98);

INCLUDE_ASM("asm/nonmatchings/4BC70", func_8004C008);

SIDeviceStatus get_si_device_status(s32 controllerIndex) {
    OSMesg unusedMsg;
    s32 ret;
    s32 bytes_not_used;
    s32 i;

    if (sControllerMesgQueue->validCount == 0) {
        if (osMotorInit(sControllerMesgQueue, &pfs[controllerIndex], controllerIndex) == 0) {
            return CONTROLLER_PAK_RUMBLE_PAK_FOUND;
        }
    }

    // Couldn't get a for loop to match this
    i = 0;
    while (sControllerMesgQueue->validCount != 0 && i < 10) {
        osRecvMesg(sControllerMesgQueue, &unusedMsg, OS_MESG_NOBLOCK);
        i++;
    }

    for (i = 0; i <= 4; i++) {
        ret = osPfsFreeBlocks(&pfs[controllerIndex], &bytes_not_used);
        if (ret == PFS_ERR_INVALID) {
            ret = osPfsInit(sControllerMesgQueue, &pfs[controllerIndex], controllerIndex);
        }
        if (ret == PFS_ERR_ID_FATAL) {
            if (osMotorInit(sControllerMesgQueue, &pfs[controllerIndex], controllerIndex) == 0) {
                return CONTROLLER_PAK_RUMBLE_PAK_FOUND;
            }
        }
        if (ret == PFS_ERR_NEW_PACK) {
            if ((osPfsInit(sControllerMesgQueue, &pfs[controllerIndex], controllerIndex) == PFS_ERR_ID_FATAL) &&
                (osMotorInit(sControllerMesgQueue, &pfs[controllerIndex], controllerIndex) == 0)) {
                return CONTROLLER_PAK_RUMBLE_PAK_FOUND;
            }
            return CONTROLLER_PAK_CHANGED;
        }
        if (ret == PFS_ERR_NOPACK || ret == PFS_ERR_DEVICE) {
            return CONTROLLER_PAK_NOT_FOUND;
        }
        if (ret == PFS_ERR_BAD_DATA) {
            return CONTROLLER_PAK_BAD_DATA;
        }
        if (ret == PFS_ERR_ID_FATAL) {
            return CONTROLLER_PAK_WITH_BAD_ID;
        }
        if (ret == PFS_ERR_INCONSISTENT) {
            return CONTROLLER_PAK_INCONSISTENT;
        }
        if (ret == 0) {
            return CONTROLLER_PAK_GOOD;
        }
    }

    return CONTROLLER_PAK_NOT_FOUND;
}

s32 start_reading_controller_data(UNUSED s32 controllerIndex) {
    osContStartReadData(sControllerMesgQueue);
    return 0;
}

INCLUDE_ASM("asm/nonmatchings/4BC70", func_8004C2BC);

SIDeviceStatus check_for_rumble_pak(s32 controllerIndex) {
    s32 ret;

    ret = get_si_device_status(controllerIndex);
    start_reading_controller_data(controllerIndex);

    if (ret == CONTROLLER_PAK_RUMBLE_PAK_FOUND) {
        gRumblePresent |= 1 << controllerIndex;
    }

    return ret;
}

SIDeviceStatus repair_controller_pak(s32 controllerIndex) {
    s32 ret;
    s32 status = get_si_device_status(controllerIndex);
    if (status == CONTROLLER_PAK_GOOD || status == CONTROLLER_PAK_INCONSISTENT) {
        status = osPfsChecker(&pfs[controllerIndex]);
        if (status == 0) {
            ret = CONTROLLER_PAK_GOOD;
        } else if (status == PFS_ERR_NEW_PACK) {
            ret = CONTROLLER_PAK_CHANGED;
        } else {
            ret = CONTROLLER_PAK_INCONSISTENT;
        }
    } else {
        ret = CONTROLLER_PAK_GOOD;
    }
    start_reading_controller_data(controllerIndex);
    return ret;
}

SIDeviceStatus reformat_controller_pak(s32 controllerIndex) {
    s32 ret;
    s32 status = get_si_device_status(controllerIndex);
    if (status == CONTROLLER_PAK_GOOD || status == CONTROLLER_PAK_INCONSISTENT ||
        status == CONTROLLER_PAK_WITH_BAD_ID) {
        status = osPfsReFormat(&pfs[controllerIndex], sControllerMesgQueue, controllerIndex);
        if (status == 0) {
            ret = CONTROLLER_PAK_GOOD;
        } else if (status == PFS_ERR_NEW_PACK) {
            ret = CONTROLLER_PAK_CHANGED;
        } else {
            ret = CONTROLLER_PAK_INCONSISTENT;
        }
    } else {
        ret = CONTROLLER_PAK_GOOD;
    }
    start_reading_controller_data(controllerIndex);
    return ret;
}

INCLUDE_ASM("asm/nonmatchings/4BC70", func_8004C618);

/**
 * Free the controller pak file list from memory.
 * Official Name: pakDirectoryFree
 */
void cpak_free_files(void) {
    if (gPakFileList) {
        mempool_free(gPakFileList);
    }
    gPakFileList = NULL;
}

INCLUDE_ASM("asm/nonmatchings/4BC70", func_8004C940);

INCLUDE_ASM("asm/nonmatchings/4BC70", func_8004CA58);

INCLUDE_ASM("asm/nonmatchings/4BC70", func_8004CB10);

SIDeviceStatus get_file_number(s32 controllerIndex, char *fileName, char *fileExt, s32 *fileNumber) {
    u32 gameCode;
    char fileNameAsFontCodes[PFS_FILE_NAME_LEN];
    UNUSED s32 pad;
    char fileExtAsFontCodes[PFS_FILE_EXT_LEN];
    UNUSED s32 pad2;
    s32 ret;

    string_to_font_codes(fileName, fileNameAsFontCodes, PFS_FILE_NAME_LEN);
    string_to_font_codes(fileExt, fileExtAsFontCodes, PFS_FILE_EXT_LEN);

    if (get_language() == LANGUAGE_JAPANESE) {
        gameCode = JPN_GAME_CODE;
    } else if (osTvType == OS_TV_TYPE_PAL) {
        gameCode = PAL_GAME_CODE;
    } else {
        gameCode = NTSC_GAME_CODE;
    }

    ret = osPfsFindFile(&pfs[controllerIndex], COMPANY_CODE, gameCode, (u8 *) fileNameAsFontCodes,
                        (u8 *) fileExtAsFontCodes, fileNumber);
    if (ret == 0) {
        return CONTROLLER_PAK_GOOD;
    }
    if ((ret == PFS_ERR_NOPACK) || (ret == PFS_ERR_DEVICE)) {
        return CONTROLLER_PAK_NOT_FOUND;
    }
    if (ret == PFS_ERR_INCONSISTENT) {
        return CONTROLLER_PAK_INCONSISTENT;
    }
    if (ret == PFS_ERR_ID_FATAL) {
        return CONTROLLER_PAK_WITH_BAD_ID;
    }
    if (ret == PFS_ERR_INVALID) {
        return CONTROLLER_PAK_CHANGED;
    }

    return CONTROLLER_PAK_BAD_DATA;
}

SIDeviceStatus read_data_from_controller_pak(s32 controllerIndex, s32 fileNum, u8 *data, s32 dataLength) {
    s32 readResult = osPfsReadWriteFile(&pfs[controllerIndex], fileNum, PFS_READ, 0, dataLength, data);

    if (readResult == 0) {
        return CONTROLLER_PAK_GOOD;
    }
    if ((readResult == PFS_ERR_NOPACK) || (readResult == PFS_ERR_DEVICE)) {
        return CONTROLLER_PAK_NOT_FOUND;
    }
    if (readResult == PFS_ERR_INCONSISTENT) {
        return CONTROLLER_PAK_INCONSISTENT;
    }
    if (readResult == PFS_ERR_ID_FATAL) {
        return CONTROLLER_PAK_WITH_BAD_ID;
    }
    if (readResult == PFS_ERR_INVALID) {
        return CONTROLLER_PAK_CHANGED;
    }

    return CONTROLLER_PAK_BAD_DATA;
}

SIDeviceStatus write_controller_pak_file(s32 controllerIndex, s32 fileNumber, char *fileName, char *fileExt,
                                         u8 *dataToWrite, s32 fileSize) {
    s32 temp;
    u8 fileNameAsFontCodes[PFS_FILE_NAME_LEN];
    UNUSED s32 temp2;
    u8 fileExtAsFontCodes[PFS_FILE_EXT_LEN];
    SIDeviceStatus ret;
    s32 file_number;
    s32 bytesToSave;
    u32 game_code;

    ret = get_si_device_status(controllerIndex);
    if (ret != CONTROLLER_PAK_GOOD) {
        start_reading_controller_data(controllerIndex);
        return ret;
    }

    bytesToSave = fileSize;
    temp = fileSize & 0xFF;
    if (temp != 0) {
        bytesToSave = (fileSize - temp) + GHSS_SIZE;
    }

    string_to_font_codes(fileName, (char *) fileNameAsFontCodes, PFS_FILE_NAME_LEN);
    string_to_font_codes(fileExt, (char *) fileExtAsFontCodes, PFS_FILE_EXT_LEN);

    if (get_language() == LANGUAGE_JAPANESE) {
        game_code = JPN_GAME_CODE;
    } else if (osTvType == OS_TV_TYPE_PAL) {
        game_code = PAL_GAME_CODE;
    } else {
        game_code = NTSC_GAME_CODE;
    }

    ret = get_file_number(controllerIndex, fileName, fileExt, &file_number);
    if (ret == CONTROLLER_PAK_GOOD) {
        if (fileNumber != -1 && fileNumber != file_number) {
            ret = CONTROLLER_PAK_BAD_DATA;
        }
    } else if (ret == CONTROLLER_PAK_CHANGED) {
        if (fileNumber != -1) {
            ret = CONTROLLER_PAK_BAD_DATA;
        } else {
            temp = osPfsAllocateFile(&pfs[controllerIndex], COMPANY_CODE, game_code, fileNameAsFontCodes,
                                     fileExtAsFontCodes, bytesToSave, &file_number);
            if (temp == 0) {
                ret = CONTROLLER_PAK_GOOD;
            } else if (temp == PFS_DATA_FULL || temp == PFS_DIR_FULL) {
                ret = CONTROLLER_PAK_FULL;
            } else {
                ret = CONTROLLER_PAK_BAD_DATA;
            }
        }
    }

    if (ret == CONTROLLER_PAK_GOOD) {
        temp = osPfsReadWriteFile(&pfs[controllerIndex], file_number, PFS_WRITE, 0, bytesToSave, dataToWrite);
        if (temp == 0) {
            ret = CONTROLLER_PAK_GOOD;
        } else if ((temp == PFS_ERR_NOPACK) || (temp == PFS_ERR_DEVICE)) {
            ret = CONTROLLER_PAK_NOT_FOUND;
        } else if (temp == PFS_ERR_INCONSISTENT) {
            ret = CONTROLLER_PAK_INCONSISTENT;
        } else if (temp == PFS_ERR_ID_FATAL) {
            ret = CONTROLLER_PAK_WITH_BAD_ID;
        } else {
            ret = CONTROLLER_PAK_BAD_DATA;
        }
    }

    start_reading_controller_data(controllerIndex);
    return ret;
}

/**
 * Get File size for given controller's Control Pak
 * Official name: packFileSize
 */
SIDeviceStatus get_file_size(s32 controllerIndex, s32 fileNum, s32 *fileSize) {
    OSPfsState state;

    *fileSize = 0;
    if (osPfsFileState(&pfs[controllerIndex], fileNum, &state) == 0) {
        *fileSize = state.file_size;
        return CONTROLLER_PAK_GOOD;
    }
    return CONTROLLER_PAK_BAD_DATA;
}

/**
 * Converts N64 Font codes used in controller pak file names, into C ASCII a coded string
 */
char *font_codes_to_string(char *inString, char *outString, s32 stringLength) {
    s32 index = *inString;
    char *ret = outString;

    while (index != 0 && stringLength != 0) {
        // Less than sizeof(gN64FontCodes) - 3.
        // So basically make sure it's a valid font code value
        if (index <= 65) {
            *outString = gN64FontCodes[index];
            outString++;
        } else {
#if REGION == REGION_JP
            *outString++ = 0x80;
            *outString++ = *inString;
#else
            // Replace invalid characters with a hyphen
            *outString++ = '-';
#endif
        }

        inString++;
        stringLength--;
        index = *inString;
    }

    // Pad any extra characters with NULL font code
    while (stringLength != 0) {
        *outString = 0;
        stringLength--;
        outString++;
    }

    *outString = 0;
    return ret;
}

/**
 * Converts a C ASCII string into N64 Font codes for controller pak file names
 */
char *string_to_font_codes(char *inString, char *outString, s32 stringLength) {
    s32 i;
    char currentChar;
    char *ret = outString;

    while (*inString != 0 && stringLength != 0) {
        *outString = 0;
#if REGION == REGION_JP
        if (*inString & 0x80) {
            *outString++ = inString[1];
            inString += 2;
        } else {
            for (i = 0; i < 65; i++) {
                if (*inString == gN64FontCodes[i]) {
                    *outString++ = i;
                    break;
                }
            }
        }
#else
        for (i = 0; i < 65; i++) {
            currentChar = *inString;
            if (currentChar == gN64FontCodes[i]) {
                *outString++ = i;
                break;
            }
        }
#endif
        inString++;
        stringLength--;
    }

    while (stringLength != 0) {
        *outString = 0;
        stringLength--;
        outString++;
    }

    *outString = 0;
    return ret;
}

INCLUDE_ASM("asm/nonmatchings/4BC70", func_8004D250);
