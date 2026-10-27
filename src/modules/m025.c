/*
 * Runlink module 25: the audio start-up. It loads the sound and sequence
 * banks and tables from the audio assets, relocates the banks' pointers
 * (libultra's bank file patching, which the module carries its own copy of),
 * creates the audio manager, the two sequence players and the sound player,
 * and starts the audio thread. This is Diddy Kong Racing's audio_init and
 * sound_seqplayer_init (DKR decomp, src/audio.c, CC0) in JFG's form.
 */
#include "module.h"

/* libultra's bank and sequence file formats (libaudio.h), the fields used here */
typedef struct M025WaveTable {
    /* 0x00 */ u8 *base;
    /* 0x04 */ s32 len;
    /* 0x08 */ u8 type; /* 0: ADPCM, 1: raw 16-bit */
    /* 0x09 */ u8 flags;
    /* 0x0C */ void *loop;
    /* 0x10 */ void *book;
} M025WaveTable;

typedef struct M025Sound {
    /* 0x00 */ void *envelope;
    /* 0x04 */ void *keyMap;
    /* 0x08 */ M025WaveTable *wavetable;
    /* 0x0C */ u8 samplePan;
    /* 0x0D */ u8 sampleVolume;
    /* 0x0E */ u8 flags;
} M025Sound;

typedef struct M025Instrument {
    /* 0x00 */ u8 volume;
    /* 0x01 */ u8 pan;
    /* 0x02 */ u8 priority;
    /* 0x03 */ u8 flags;
    /* 0x04 */ u8 pad4[0xA];
    /* 0x0E */ s16 soundCount;
    /* 0x10 */ M025Sound *soundArray[1];
} M025Instrument;

typedef struct M025Bank {
    /* 0x00 */ s16 instCount;
    /* 0x02 */ u8 flags;
    /* 0x03 */ u8 pad3;
    /* 0x04 */ s32 sampleRate;
    /* 0x08 */ M025Instrument *percussion;
    /* 0x0C */ M025Instrument *instArray[1];
} M025Bank;

typedef struct M025BankFile {
    /* 0x00 */ s16 revision;
    /* 0x02 */ s16 bankCount;
    /* 0x04 */ M025Bank *bankArray[1];
} M025BankFile;

typedef struct M025SeqData {
    u8 *offset;
    s32 len;
} M025SeqData;

typedef struct M025SeqFile {
    /* 0x00 */ s16 revision;
    /* 0x02 */ s16 seqCount;
    /* 0x04 */ M025SeqData seqArray[1];
} M025SeqFile;

/* libultra's ALSynConfig */
typedef struct M025SynConfig {
    /* 0x00 */ s32 maxVVoices;
    /* 0x04 */ s32 maxPVoices;
    /* 0x08 */ s32 maxUpdates;
    /* 0x0C */ s32 maxFXbusses;
    /* 0x10 */ void *dmaproc;
    /* 0x14 */ void *heap;
    /* 0x18 */ s32 outputRate;
    /* 0x1C */ u8 fxType;
    /* 0x20 */ s32 unk20;
} M025SynConfig;

/* the sequence player configuration (DKR's ALSeqpConfig, with its voiceLimit) */
typedef struct M025SeqpConfig {
    /* 0x00 */ s32 maxVoices;
    /* 0x04 */ s32 maxEvents;
    /* 0x08 */ u8 maxChannels;
    /* 0x0C */ void *heap;
    /* 0x10 */ void *initOsc;
    /* 0x14 */ void *updateOsc;
    /* 0x18 */ void *stopOsc;
    /* 0x1C */ u8 voiceLimit;
} M025SeqpConfig;

/* the sound player configuration */
typedef struct M025SndpConfig {
    /* 0x00 */ s32 maxSounds;
    /* 0x04 */ s32 maxEvents;
    /* 0x08 */ s32 maxChannels;
    /* 0x0C */ void *heap;
    /* 0x10 */ s16 numGroups;
} M025SndpConfig;

/* libultra's ALEvent, as a bank change */
typedef struct M025Event {
    /* 0x00 */ s16 type;
    /* 0x04 */ void *bank;
    /* 0x08 */ u8 pad8[8];
} M025Event;

typedef struct M025SeqPlayer {
    /* 0x00 */ u8 pad0[0x48];
    /* 0x48 */ u8 evtq[1];
} M025SeqPlayer;

extern void func_8008A600(void *hp, u8 *base, s32 len);                      /* alHeapInit */
extern void *func_8008A624(u8 *file, s32 line, void *hp, s32 num, s32 size); /* alHeapDBAlloc */
extern void func_8008A490(void *seqp, void *mq);
extern void func_80086640(void *seqp, M025SeqpConfig *config);              /* DKR alCSPNew */
extern void func_8008B610(void *evtq, M025Event *evt, s32 delta);           /* alEvtqPostEvent */
extern s32 *func_8004D37C(s32 asset);                                        /* DKR asset_table_load */
extern s32 func_8004D530(s32 asset, void *dst, s32 offset, s32 size);        /* DKR asset_load (returns a value) */
extern s32 func_8004D5B0(s32 asset, s32 offset);                             /* DKR asset_rom_offset */
extern void *func_8004A57C(s32 size, s32 colour);                            /* DKR mempool_alloc_safe */
extern void func_8004AA80(void *data);                                       /* DKR mempool_free */
extern void func_80001A10(M025SynConfig *c, s32 pri, void *sched);           /* DKR amCreateAudioMgr */
extern void func_80084200(M025SndpConfig *config);                           /* DKR sndp_init_player */
extern void func_80001EA0(void);                                             /* DKR audioStartThread */
extern void func_80000450(s32 behaviour);                                    /* DKR sound_volume_change */
extern void func_80003DB0(void);
extern void func_8008B2A0(s32 arg0);
extern void func_8008B374(s32 arg0, s32 arg1);
extern void func_80096BB0(void *mq, void *msg, s32 count); /* osCreateMesgQueue */
extern void func_80003E40(void);
extern void func_80004164(void);
extern void func_8000474C(void);

extern void *D_800A0660; /* DKR gMusicPlayer */
extern void *D_800A0664; /* DKR gJinglePlayer */
extern u8 D_800B9B50[];  /* DKR gAudioHeapStack */
extern u8 D_800E94E0[];  /* DKR gALHeap */
extern M025SeqFile *D_800E94F0;  /* DKR gSequenceTable */
extern u32 *D_800E94FC;          /* DKR gSeqLengthTable */
extern M025BankFile *D_800E9500; /* DKR gSequenceBank */
extern M025BankFile *D_800E9504; /* DKR gSoundBank */
extern void *D_800E9508;         /* DKR gSoundTable */
extern void *gSeqSoundTable;
extern u32 D_800E9510;           /* DKR gSoundCount */
extern u32 D_800E9514;           /* DKR gSeqSoundCount */
extern u32 D_800E9518;           /* DKR gSoundTableSize */
extern u32 D_800E951C;           /* DKR gSeqSoundTableSize */
extern u8 D_800E9578[];
extern void *D_800E9670;
extern u8 D_800E9778[];
extern u8 D_800E9780[];

#define PATCH(src, base, type) src = (type) ((s32) src + (s32) base)

static void M025SeqFileNew(M025SeqFile *file, s32 base) {
    s32 i;

    for (i = 0; i < file->seqCount; i++) {
        PATCH(file->seqArray[i].offset, base, u8 *);
    }
}

static void M025PatchBank(M025Bank *bank, s32 offset, s32 table);
static void M025PatchInst(M025Instrument *inst, s32 offset, s32 table);
static void M025PatchSound(M025Sound *sound, s32 offset, s32 table);
static void M025PatchWaveTable(M025WaveTable *w, s32 offset, s32 table);

static void M025BnkfNew(M025BankFile *file, s32 table) {
    s32 offset = (s32) file;
    s32 i;

    if (file->revision != 0x4231) {
        return;
    }
    for (i = 0; i < file->bankCount; i++) {
        PATCH(file->bankArray[i], offset, M025Bank *);
        if (file->bankArray[i] != NULL) {
            M025PatchBank(file->bankArray[i], offset, table);
        }
    }
}

static void M025PatchBank(M025Bank *bank, s32 offset, s32 table) {
    s32 i;

    if (bank->flags) {
        return;
    }
    bank->flags = 1;
    if (bank->percussion != NULL) {
        PATCH(bank->percussion, offset, M025Instrument *);
        M025PatchInst(bank->percussion, offset, table);
    }
    for (i = 0; i < bank->instCount; i++) {
        PATCH(bank->instArray[i], offset, M025Instrument *);
        if (bank->instArray[i] != NULL) {
            M025PatchInst(bank->instArray[i], offset, table);
        }
    }
}

static void M025PatchInst(M025Instrument *inst, s32 offset, s32 table) {
    s32 i;

    if (inst->flags) {
        return;
    }
    inst->flags = 1;
    for (i = 0; i < inst->soundCount; i++) {
        PATCH(inst->soundArray[i], offset, M025Sound *);
        M025PatchSound(inst->soundArray[i], offset, table);
    }
}

static void M025PatchSound(M025Sound *sound, s32 offset, s32 table) {
    if (sound->flags) {
        return;
    }
    sound->flags = 1;
    PATCH(sound->envelope, offset, void *);
    PATCH(sound->keyMap, offset, void *);
    PATCH(sound->wavetable, offset, M025WaveTable *);
    M025PatchWaveTable(sound->wavetable, offset, table);
}

static void M025PatchWaveTable(M025WaveTable *w, s32 offset, s32 table) {
    if (w->flags) {
        return;
    }
    w->flags = 1;
    PATCH(w->base, table, u8 *);
    if (w->type == 0) {
        PATCH(w->book, offset, void *);
        if (w->loop != NULL) {
            PATCH(w->loop, offset, void *);
        }
    } else if (w->type == 1) {
        if (w->loop != NULL) {
            PATCH(w->loop, offset, void *);
        }
    }
}

/* libultra's alCSPSetBank */
static void M025SetBank(M025SeqPlayer *seqp, void *bank) {
    M025Event evt;

    evt.type = 14;
    evt.bank = bank;
    func_8008B610(&seqp->evtq, &evt, 0);
}

static void *M025SeqPlayerInit(s32 maxVoices, s32 maxEvents);

void func_m025_00308(void *sched) {
    s32 i;
    M025SynConfig synthConfig;
    s32 *addrPtr;
    u32 seqfSize;
    u32 seqLength;
    u32 pad; /* unused, as in DKR's audio_init */
    M025SndpConfig sndpConfig;

    seqLength = 0;
    D_800E9670 = D_800E9578;
    func_8008A600(D_800E94E0, D_800B9B50, 0x2F990);
    addrPtr = func_8004D37C(0x33);
    D_800E9504 = func_8004A57C(addrPtr[2] - addrPtr[1], 0x00FFFFFF);
    func_8004D530(0x34, D_800E9504, addrPtr[1], addrPtr[2] - addrPtr[1]);
    M025BnkfNew(D_800E9504, func_8004D5B0(0x34, addrPtr[2]));

    D_800E9518 = addrPtr[6] - addrPtr[5];
    D_800E9508 = func_8004A57C(D_800E9518, 0x00FFFFFF);
    func_8004D530(0x34, D_800E9508, addrPtr[5], D_800E9518);
    D_800E9510 = D_800E9518 / 10;

    D_800E951C = addrPtr[5] - addrPtr[4];
    gSeqSoundTable = func_8004A57C(D_800E951C, 0x00FFFFFF);
    func_8004D530(0x34, gSeqSoundTable, addrPtr[4], D_800E951C);
    D_800E9514 = D_800E951C / 3;

    D_800E9500 = func_8004A57C(addrPtr[0], 0x00FFFFFF);
    func_8004D530(0x34, D_800E9500, 0, addrPtr[0]);
    M025BnkfNew(D_800E9500, func_8004D5B0(0x34, addrPtr[0]));
    D_800E94F0 = func_8008A624(NULL, 0, D_800E94E0, 1, 4);
    func_8004D530(0x34, D_800E94F0, addrPtr[3], 4);

    seqfSize = D_800E94F0->seqCount * 8 + 4;
    D_800E94F0 = func_8004A57C(seqfSize, 0x00FFFFFF);
    func_8004D530(0x34, D_800E94F0, addrPtr[3], seqfSize);
    M025SeqFileNew(D_800E94F0, func_8004D5B0(0x34, addrPtr[3]));
    D_800E94FC = func_8004A57C(D_800E94F0->seqCount * 4, 0x00FFFFFF);

    for (i = 0; i < D_800E94F0->seqCount; i++) {
        D_800E94FC[i] = D_800E94F0->seqArray[i].len;
        if (D_800E94FC[i] & 1) {
            D_800E94FC[i]++;
        }
        if (seqLength < D_800E94FC[i]) {
            seqLength = D_800E94FC[i];
        }
    }

    synthConfig.maxVVoices = 44;
    synthConfig.maxPVoices = 40;
    synthConfig.maxUpdates = 128;
    synthConfig.dmaproc = NULL;
    synthConfig.fxType = 6;
    synthConfig.maxFXbusses = 1;
    synthConfig.outputRate = 0;
    synthConfig.heap = D_800E94E0;
    func_80001A10(&synthConfig, 12, sched);
    D_800A0660 = M025SeqPlayerInit(32, 150);
    D_800A0664 = M025SeqPlayerInit(16, 50);
    sndpConfig.maxEvents = 200;
    sndpConfig.maxSounds = 32;
    sndpConfig.maxChannels = 16;
    sndpConfig.numGroups = 5;
    sndpConfig.heap = D_800E94E0;
    func_80084200(&sndpConfig);
    func_80001EA0();
    func_80000450(0);
    func_8004AA80(addrPtr);
    func_80003DB0();
    func_8008B2A0(4);
    func_8008B374(0, 3);
    func_80096BB0(D_800E9780, D_800E9778, 1);
    func_8008A490(D_800A0660, D_800E9780);
}

static void *M025SeqPlayerInit(s32 maxVoices, s32 maxEvents) {
    void *cseqp;
    M025SeqpConfig config;

    config.maxVoices = maxVoices;
    config.maxEvents = maxEvents;
    config.voiceLimit = maxVoices;
    config.maxChannels = 16;
    config.heap = D_800E94E0;
    config.initOsc = func_80003E40;
    config.stopOsc = func_8000474C;
    config.updateOsc = func_80004164;
    cseqp = func_8008A624(NULL, 0, D_800E94E0, 1, 0x90);
    func_80086640(cseqp, &config);
    M025SetBank(cseqp, D_800E9500->bankArray[0]);
    return cseqp;
}
