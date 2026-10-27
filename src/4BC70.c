/*
 * Rumble, EEPROM and controller pak access. Functions in C here are from the
 * Diddy Kong Racing decompilation (CC0 1.0), src/save_data.c; see CREDITS.md.
 */
#include "common.h"
#include "PR/os_flash.h"

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
void *mempool_alloc_safe(s32 size, u32 colourTag);
char *string_to_font_codes(char *inString, char *outString, s32 stringLength);

extern OSMesgQueue *sControllerMesgQueue;
extern OSPfs pfs[MAXCONTROLLERS];
extern s8 *gPakFileList;
extern u8 gN64FontCodes[];
extern u8 gRumblePresent; // Bits 0, 1, 2, and 3 of the bit pattern correspond to Controllers 1, 2, 3, and 4.

extern u8 D_800A3470;
extern u8 D_800A3464;
s8 func_800482F8(void);

int func_8004B070(void) {
    return D_800A3470 != 0 && func_800482F8() == 0;
}

extern u8 D_800A3470;

void func_8004B0AC(s32 arg0) {
    D_800A3470 = arg0;
}

extern u8 D_800A346C;

void func_8004B0B8(s32 on) {
    if (on != 0 && D_800A346C == 0) {
        D_800A3464 = 1;
        D_800A346C = 1;
    } else {
        D_800A346C = 0;
    }
}

/* JFG: a rumble pak's state (10 bytes per player) */
typedef struct RumbleData {
    /* 0x00 */ u16 state : 4;
    /* 0x00 */ u16 unk0 : 4;
    /* 0x01 */ u16 unk1 : 8;
    /* 0x02 */ s16 strength;
    /* 0x04 */ s16 strength2;
    /* 0x06 */ s16 time;
    /* 0x08 */ u8 pad8[2];
} RumbleData;
extern RumbleData D_800FDF58[];
u8 input_player_id(s32 player);
extern f32 D_800AC940;

/* JFG: start a rumble on a controller (strength squared, time in seconds). */
void func_8004B0F4(s32 pad, s32 strength, f32 time) {
    RumbleData *rumble;

    if (func_8004B070() != 0) {
        if (pad >= 0 && pad < 4) {
            rumble = &D_800FDF58[input_player_id(pad)];
            if (rumble->state != 2) {
                rumble->state = 1;
                rumble->strength = strength * strength * D_800AC940;
                rumble->strength2 = rumble->strength;
                rumble->time = time * 60.0f;
            }
        }
    }
}

void func_8004B204(s32 pad, s32 force) {
    s32 id;

    if (pad >= 0 && pad < 4) {
        id = input_player_id(pad);
        if (force != 0 || (D_800FDF58[id].state != 0 && D_800FDF58[id].state != 3)) {
            D_800FDF58[id].state = 3;
            D_800FDF58[id].unk1 = 1;
        }
    }
}

INCLUDE_ASM("asm/nonmatchings/4BC70", func_8004B29C);

INCLUDE_ASM("asm/nonmatchings/4BC70", func_8004B3D8);

void func_8004B204(s32 pad, s32 arg1);

void func_8004B518(s32 arg0) {
    s32 i = 4;
    while (i--) {
        func_8004B204(i, arg0);
    }
}

extern u8 D_800A3464;

void func_8004B55C(void) {
    D_800A3464 = 0x1;
}

INCLUDE_ASM("asm/nonmatchings/4BC70", func_8004B56C);

typedef struct RumbleState {
    s32 unk0;
    f32 unk4;
} RumbleState;
extern RumbleState D_800A344C[3];

void func_8004BA50(s32 pad, s32 *a, f32 *b) {
    *a = 0;
    *b = 0.0f;
    if (pad >= 0 && pad < 3) {
        *a = D_800A344C[pad].unk0;
        *b = D_800A344C[pad].unk4;
    }
}

s32 func_8004BA98(u8 *data, s32 size) {
    s32 sum = 7;
    while (size--) {
        sum += *data++;
    }
    return sum;
}

s32 func_8004BAC8(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
    return 1;
}

s32 func_8004BAE0(s32 arg0, s32 arg1, s32 arg2) {
    return 0x1;
}

s32 is_reset_pressed(void);
void func_80083EB0(s32 sector);
void func_8004BF98(u32 page, u8 *data);
void func_8004C008(u32 page, u8 *data);
extern s32 D_800A3474[]; // JFG: the first flash page of each save slot

INCLUDE_ASM("asm/nonmatchings/4BC70", func_8004BAF4);

INCLUDE_ASM("asm/nonmatchings/4BC70", func_8004BC10);

/* JFG: erase a save slot's flash sector (fill its pages with 0xFF). */
void func_8004BCB4(s32 slot, UNUSED s32 arg1) {
    s32 page = D_800A3474[slot];
    u8 *buf = mempool_alloc_safe(0x80, -1);
    s32 i;
    s32 j;

    i = 0;
    while (i < 0x80) {
        buf[i++] = 0xFF;
    }
    if (is_reset_pressed() == 0) {
        func_80083EB0(page);
        for (j = 0; j < 0x80; j++) {
            func_8004BF98(page++, buf);
        }
    }
    mempool_free(buf);
}

#ifdef NON_MATCHING
/* NON_MATCHING: buf and i swap saved registers (target buf s3, i s4) and 0xFF goes to a0 */
/* JFG: erase the first 8 flash sectors. */
void func_8004BD78(void) {
    s32 i;
    u8 *buf = mempool_alloc_safe(0x80, -1);
    u8 *p = buf;
    s32 j;
    s32 page;

    for (i = 0; i < 0x80; i++) {
        *p++ = 0xFF;
    }
    if (is_reset_pressed() == 0) {
        page = 0;
        for (i = 0; i < 8; i++) {
            func_80083EB0(page);
            for (j = 0; j < 0x80; j++) {
                func_8004BF98(page++, buf);
            }
        }
    }
    mempool_free(buf);
}
#else
INCLUDE_ASM("asm/nonmatchings/4BC70", func_8004BD78);
#endif

s32 func_8004BE44(u8 *data) {
    s32 sum = 5;
    s32 i = 126;
    while (i--) {
        sum += *data++;
    }
    return sum;
}

s32 func_8004BE44(u8 *data);
extern u8 D_800A34AC[0x7E];

s32 func_8004BE6C(u8 *data) {
    u8 *src;
    u8 *dst;
    s32 i;

    func_8004C008(0x380, data);
    if (func_8004BE44(data) != *(u16 *) (data + 0x7E)) {
        src = D_800A34AC;
        dst = data;
        i = 126;
        while (i--) {
            *dst++ = *src++;
        }
        *(u16 *) (data + 0x7E) = func_8004BE44(data);
    }
    return 1;
}

s32 func_8004BEE4(u8 *data) {
    *(s16 *) (data + 0x7E) = func_8004BE44(data);
    if (is_reset_pressed() == 0) {
        func_80083EB0(0x380);
        func_8004BF98(0x380, data);
    }
    return 1;
}

extern OSMesgQueue D_800FDF80;
extern OSMesg D_800FDF98[2];
extern OSIoMesg D_800FDFA0;
extern OSMesg D_800FDFB8[2];
extern OSMesgQueue D_800FDFC0;

void func_8004BF2C(void) {
    osCreateMesgQueue(&D_800FDFC0, D_800FDFB8, 1);
    osSetEventMesg(OS_EVENT_CART, &D_800FDFC0, (OSMesg) 1);
    osCreateMesgQueue(&D_800FDF80, D_800FDF98, 1);
    osCartRomInit();
    osFlashInit();
}

void func_8004BF98(u32 page, u8 *data) {
    if (page < 0x3FE) {
        osWritebackDCache(data, 0x80);
        osFlashWriteBuffer(&D_800FDFA0, OS_MESG_PRI_NORMAL, data, &D_800FDF80);
        osRecvMesg(&D_800FDF80, NULL, OS_MESG_BLOCK);
        osFlashWriteArray(page);
    }
}

void func_8004C008(u32 page, u8 *data) {
    if (page < 0x3FE) {
        osInvalDCache(data, 0x80);
        osFlashReadArray(&D_800FDFA0, OS_MESG_PRI_NORMAL, page, data, 1, &D_800FDF80);
        osRecvMesg(&D_800FDF80, NULL, OS_MESG_BLOCK);
    }
}

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

#define SAVE_FILE_BYTES 24
#define SAVE_FILE_TYPE_UNSET 0xFF
#define SAVE_FILE_TYPE_CPAK_OTHER 1
#define COLOUR_TAG_BLACK 0xFF

char *font_codes_to_string(char *inString, char *outString, s32 stringLength);
s32 func_8004D250(s32 controllerIndex, s32 fileNum); // DKR: get_file_type

/**
 * Lists the files on the controller pak.
 * JFG: the statuses are returned without the controller index.
 * Official name: packDirectory
 */
s32 get_controller_pak_file_list(s32 controllerIndex, s32 maxNumOfFilesToGet, char **fileNames, char **fileExtensions,
                                 u32 *fileSizes, u8 *fileTypes) {
    OSPfsState state;
    s32 ret;
    s32 maxNumOfFilesOnCpak;
    s32 files_used;
    s8 *list;
    s32 i;
    u32 gameCode;

    ret = get_si_device_status(controllerIndex);
    if (ret != CONTROLLER_PAK_GOOD) {
        start_reading_controller_data(controllerIndex);
        return ret;
    }

    if (osPfsNumFiles(&pfs[controllerIndex], &maxNumOfFilesOnCpak, &files_used) != 0) {
        start_reading_controller_data(controllerIndex);
        return CONTROLLER_PAK_BAD_DATA;
    }

    if (get_language() == LANGUAGE_JAPANESE) {
        gameCode = JPN_GAME_CODE;
    } else if (osTvType == OS_TV_TYPE_PAL) {
        gameCode = PAL_GAME_CODE;
    } else {
        gameCode = NTSC_GAME_CODE;
    }

    if (maxNumOfFilesToGet < maxNumOfFilesOnCpak) {
        maxNumOfFilesOnCpak = maxNumOfFilesToGet;
    }

    if (gPakFileList != NULL) {
        mempool_free(gPakFileList);
    }

    files_used = maxNumOfFilesOnCpak * SAVE_FILE_BYTES;

    gPakFileList = mempool_alloc_safe(files_used, COLOUR_TAG_BLACK);
    bzero(gPakFileList, files_used);
    list = gPakFileList;

    for (i = 0; i < maxNumOfFilesOnCpak; i++) {
        fileNames[i] = (char *) list;
        list += 0x12;
        fileExtensions[i] = (char *) list;
        fileSizes[i] = 0;
        fileTypes[i] = SAVE_FILE_TYPE_UNSET;
        list += 6;
    }

    while (i < maxNumOfFilesToGet) {
        fileExtensions[i] = 0;
        fileNames[i] = 0;
        fileSizes[i] = 0;
        fileTypes[i] = SAVE_FILE_TYPE_UNSET;
        i++;
    }

    for (i = 0; i < maxNumOfFilesOnCpak; i++) {
        ret = osPfsFileState(&pfs[controllerIndex], i, &state);
        if (ret == PFS_ERR_INVALID) {
            fileNames[i] = 0;
            continue;
        }

        if (ret != 0) {
            start_reading_controller_data(controllerIndex);
            return CONTROLLER_PAK_BAD_DATA;
        }

        font_codes_to_string((char *) &state.game_name, (char *) fileNames[i], PFS_FILE_NAME_LEN);
        font_codes_to_string((char *) &state.ext_name, (char *) fileExtensions[i], PFS_FILE_EXT_LEN);
        fileSizes[i] = state.file_size;
        fileTypes[i] = SAVE_FILE_TYPE_CPAK_OTHER;

        if ((state.game_code == gameCode) && (state.company_code == COMPANY_CODE)) {
            fileTypes[i] = func_8004D250(controllerIndex, i);
        }
    }

    start_reading_controller_data(controllerIndex);
    return CONTROLLER_PAK_GOOD;
}

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

#define MAX_CPAK_FILES 16

/**
 * Get the number of free bytes and free notes on the controller pak.
 * JFG: the status is returned without the controller index.
 * Official name: packFreeSpace
 */
s32 get_free_space(s32 controllerIndex, u32 *bytesFree, s32 *notesFree) {
    s32 ret;
    s32 bytesNotUsed;
    s32 maxNotes;
    s32 notesUsed;

    ret = get_si_device_status(controllerIndex);
    if (ret == CONTROLLER_PAK_GOOD) {
        if (bytesFree != 0) {
            ret = osPfsFreeBlocks(&pfs[controllerIndex], &bytesNotUsed);
            if (ret != 0) {
                start_reading_controller_data(controllerIndex);
                return CONTROLLER_PAK_BAD_DATA;
            }
            *bytesFree = bytesNotUsed;
        }
        if (notesFree != 0) {
            ret = osPfsNumFiles(&pfs[controllerIndex], &maxNotes, &notesUsed);
            if (ret != 0) {
                start_reading_controller_data(controllerIndex);
                return CONTROLLER_PAK_BAD_DATA;
            }
            if (notesUsed >= MAX_CPAK_FILES) {
                *notesFree = 0;
            } else {
                *notesFree = MAX_CPAK_FILES - notesUsed;
            }
        }
    }
    start_reading_controller_data(controllerIndex);
    return ret;
}

/**
 * Deletes a file from the controller pak.
 * JFG: the status is returned without the controller index.
 * Official name: packDeleteFile
 */
s32 delete_file(s32 controllerIndex, s32 fileNum) {
    OSPfsState state;
    s32 ret;

    ret = get_si_device_status(controllerIndex);
    if (ret != CONTROLLER_PAK_GOOD) {
        start_reading_controller_data(controllerIndex);
        return ret;
    }

    ret = CONTROLLER_PAK_BAD_DATA;

    if (osPfsFileState(&pfs[controllerIndex], fileNum, &state) == 0) {
        if (osPfsDeleteFile(&pfs[controllerIndex], state.company_code, state.game_code, (u8 *) &state.game_name,
                            (u8 *) &state.ext_name) == 0) {
            ret = CONTROLLER_PAK_GOOD;
        }
    }

    start_reading_controller_data(controllerIndex);

    return ret;
}

#define COLOUR_TAG_BLACK 0xFF
SIDeviceStatus read_data_from_controller_pak(s32 controllerIndex, s32 fileNum, u8 *data, s32 dataLength);
SIDeviceStatus write_controller_pak_file(s32 controllerIndex, s32 fileNumber, char *fileName, char *fileExt,
                                         u8 *dataToWrite, s32 fileSize);
char *font_codes_to_string(char *inString, char *outString, s32 stringLength);

/**
 * Copies a file from one controller pak to another.
 * JFG: the statuses are returned without the controller index.
 * Official name: packCopyFile
 */
s32 copy_controller_pak_data(s32 controllerIndex, s32 fileNumber, s32 secondControllerIndex) {
    UNUSED s32 pad;
    char fileName[PFS_FILE_NAME_LEN];
    UNUSED s32 pad2;
    char fileExt[PFS_FILE_EXT_LEN];
    OSPfsState state;
    s32 status;
    u8 *alloc;

    status = get_si_device_status(controllerIndex);
    if (status != CONTROLLER_PAK_GOOD) {
        start_reading_controller_data(controllerIndex);
        return status;
    }

    if (osPfsFileState(&pfs[controllerIndex], fileNumber, &state) != 0) {
        start_reading_controller_data(controllerIndex);
        return CONTROLLER_PAK_BAD_DATA;
    }

    alloc = mempool_alloc_safe(state.file_size, COLOUR_TAG_BLACK);

    status = read_data_from_controller_pak(controllerIndex, fileNumber, alloc, state.file_size);
    start_reading_controller_data(controllerIndex);
    if (status != CONTROLLER_PAK_GOOD) {
        mempool_free(alloc);
        return status;
    }

    font_codes_to_string((char *) state.game_name, fileName, PFS_FILE_NAME_LEN);
    font_codes_to_string((char *) state.ext_name, fileExt, PFS_FILE_EXT_LEN);

    status = write_controller_pak_file(secondControllerIndex, -1, fileName, fileExt, alloc, state.file_size);

    mempool_free(alloc);
    return status;
}

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

#define CHAR 0x43484152 // "CHAR"

/**
 * Returns the type of a game file on the controller pak.
 * DKR: get_file_type. JFG: only one type ("CHAR" files are 0, everything else 1).
 */
s32 func_8004D250(s32 controllerIndex, s32 fileNum) {
    s32 *data;
    UNUSED s32 pad;
    s32 ret;

    data = mempool_alloc_safe(0x100, COLOUR_TAG_BLACK);
    if (read_data_from_controller_pak(controllerIndex, fileNum, (u8 *) data, 0x100) == CONTROLLER_PAK_GOOD) {
        switch (*data) {
            case CHAR:
                ret = 0;
                break;
            default:
                ret = SAVE_FILE_TYPE_CPAK_OTHER;
                break;
        }
    } else {
        ret = SAVE_FILE_TYPE_CPAK_OTHER;
    }
    mempool_free(data);
    return ret;
}
