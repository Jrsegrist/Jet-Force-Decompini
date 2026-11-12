/*
 * n_audio (the game's n_audio library, built unoptimised with debug information): the compact MIDI sequence
 * reader (cf. libultra's cseq.c).
 */
#include "common.h"

/* JFG: a compact MIDI file's header */
typedef struct NCMidiHdr {
    /* 0x00 */ u32 trackOffset[16];
    /* 0x40 */ u32 division;
} NCMidiHdr;

/* JFG: a compact sequence (libultra ALCSeq) */
typedef struct NCSeq {
    /* 0x00 */ NCMidiHdr *base;
    /* 0x04 */ u32 validTracks;
    /* 0x08 */ f32 qnpt;
    /* 0x0C */ u32 lastTicks;
    /* 0x10 */ u32 lastDeltaTicks;
    /* 0x14 */ s32 deltaFlag;
    /* 0x18 */ u8 *curLoc[16];
    /* 0x58 */ u8 *curBUPtr[16];
    /* 0x98 */ u8 curBULen[16];
    /* 0xA8 */ u8 lastStatus[16];
    /* 0xB8 */ u32 evtDeltaTicks[16];
} NCSeq;

/* JFG: an n_audio sequence event (the parts the reader fills in) */
typedef struct NSeqEvent {
    /* 0x00 */ s16 type;
    /* 0x02 */ u8 pad2[2];
    /* 0x04 */ s32 ticks;
    /* 0x08 */ u8 status;
    /* 0x09 */ u8 byte1; // JFG: the meta type for meta events
    /* 0x0A */ u8 byte2;
    /* 0x0B */ u8 tempo1;
    /* 0x0C */ union {
        u32 duration; // JFG: note on
        u32 loopStart;
        struct {
            u8 tempo2;
            u8 tempo3;
        } t;
    } u;
} NSeqEvent;

#define N_SEQ_MIDI_EVT 1
#define N_TEMPO_EVT 3
#define N_SEQ_END_EVT 4
#define N_TRACK_END 18
#define N_CSP_LOOPSTART 19
#define N_CSP_LOOPEND 20

char func_80089CEC(NCSeq *seq, u32 track, NSeqEvent *event, s32 loop);
u8 func_8008A138(NCSeq *seq, u32 track);
u32 func_8008A32C(NCSeq *seq, u32 track);

/* JFG: start reading a compact sequence (libultra alCSeqNew). */
void func_800899F0(NCSeq *seq, u8 *ptr) {
    u32 i;
    u32 tmpOff;
    u32 flagTmp;

    seq->base = (NCMidiHdr *) ptr;
    seq->validTracks = 0;
    seq->lastDeltaTicks = 0;
    seq->lastTicks = 0;
    seq->deltaFlag = 1;
    for (i = 0; i < 16; i++) {
        seq->lastStatus[i] = 0;
        seq->curBUPtr[i] = 0;
        seq->curBULen[i] = 0;
        tmpOff = seq->base->trackOffset[i];
        if (tmpOff) {
            flagTmp = 1 << i;
            seq->validTracks |= flagTmp;
            seq->curLoc[i] = (u8 *) ((u32) ptr + tmpOff);
            seq->evtDeltaTicks[i] = func_8008A32C(seq, i);
        } else {
            seq->curLoc[i] = 0;
        }
    }
    seq->qnpt = 1.0f / (f32) seq->base->division;
}

/* JFG: the next event of the sequence, from the track whose event comes first (libultra alCSeqNextEvent). */
void func_80089B70(NCSeq *seq, NSeqEvent *evt, s32 loop) {
    u32 i;
    u32 firstTime = 0xFFFFFFFF;
    u32 firstTrack;
    u32 lastTicks = seq->lastDeltaTicks;

    for (i = 0; i < 16; i++) {
        if ((seq->validTracks >> i) & 1) {
            if (seq->deltaFlag) {
                seq->evtDeltaTicks[i] -= lastTicks;
            }
            if (seq->evtDeltaTicks[i] < firstTime) {
                firstTime = seq->evtDeltaTicks[i];
                firstTrack = i;
            }
        }
    }
    func_80089CEC(seq, firstTrack, evt, loop);
    evt->ticks = firstTime;
    seq->lastTicks += firstTime;
    seq->lastDeltaTicks = firstTime;
    if (evt->type != N_TRACK_END) {
        seq->evtDeltaTicks[firstTrack] += func_8008A32C(seq, firstTrack);
    }
    seq->deltaFlag = 1;
}

/* JFG: read a track's next event (libultra __alCSeqGetTrackEvent; loop ends repeat only while loop is set). */
char func_80089CEC(NCSeq *seq, u32 track, NSeqEvent *event, s32 loop) {
    u32 offset;
    u8 status;
    u8 loopCt;
    u8 curLpCt;
    u8 *tmpPtr;

    status = func_8008A138(seq, track);
    if (status == 0xFF) {
        u8 type = func_8008A138(seq, track);

        if (type == 0x51) {
            event->type = N_TEMPO_EVT;
            event->status = status;
            event->byte1 = type;
            event->tempo1 = func_8008A138(seq, track);
            event->u.t.tempo2 = func_8008A138(seq, track);
            event->u.t.tempo3 = func_8008A138(seq, track);
            seq->lastStatus[track] = 0;
        } else if (type == 0x2F) {
            u32 flagMask;

            flagMask = 1 << track;
            seq->validTracks = seq->validTracks ^ flagMask;
            if (seq->validTracks) {
                event->type = N_TRACK_END;
            } else {
                event->type = N_SEQ_END_EVT;
            }
        } else if (type == 0x2E) {
            status = func_8008A138(seq, track);
            event->u.loopStart = status << 8;
            status = func_8008A138(seq, track);
            event->u.loopStart += status;
            seq->lastStatus[track] = 0;
            event->type = N_CSP_LOOPSTART;
        } else if (type == 0x2D) {
            tmpPtr = seq->curLoc[track];
            loopCt = *tmpPtr++;
            curLpCt = *tmpPtr;
            if (curLpCt == 0 || !loop) {
                *tmpPtr = loopCt;
                seq->curLoc[track] = tmpPtr + 5;
            } else {
                if (curLpCt != 0xFF) {
                    *tmpPtr = curLpCt - 1;
                }
                tmpPtr++;
                offset = (*tmpPtr++) << 24;
                offset += (*tmpPtr++) << 16;
                offset += (*tmpPtr++) << 8;
                offset += *tmpPtr++;
                seq->curLoc[track] = tmpPtr - offset;
            }
            seq->lastStatus[track] = 0;
            event->type = N_CSP_LOOPEND;
        }
    } else {
        event->type = N_SEQ_MIDI_EVT;
        if (status & 0x80) {
            event->status = (status & 0xF0) | track; // JFG: the channel is the track
            event->byte1 = func_8008A138(seq, track);
            seq->lastStatus[track] = event->status;
        } else {
            event->status = seq->lastStatus[track];
            event->byte1 = status;
        }
        if ((event->status & 0xF0) != 0xC0 && (event->status & 0xF0) != 0xD0) {
            event->byte2 = func_8008A138(seq, track);
            if ((event->status & 0xF0) == 0x90) {
                event->u.duration = func_8008A32C(seq, track);
            }
        } else {
            event->byte2 = 0;
        }
    }
    return TRUE;
}

/* JFG: the ticks read so far (libultra alCSeqGetTicks). */
u32 func_8008A120(NCSeq *seq) {
    return seq->lastTicks;
}

/* JFG: a track's next byte, following the compact format's back references (libultra __getTrackByte). */
u8 func_8008A138(NCSeq *seq, u32 track) {
    u8 theByte;

    if (seq->curBULen[track]) {
        theByte = *seq->curBUPtr[track];
        seq->curBUPtr[track]++;
        seq->curBULen[track]--;
    } else {
        theByte = *seq->curLoc[track];
        seq->curLoc[track]++;
        if (theByte == 0xFE) {
            u8 loBackUp, hiBackUp, theLen, nextByte;
            u32 backup;

            nextByte = *seq->curLoc[track];
            seq->curLoc[track]++;
            if (nextByte != 0xFE) {
                hiBackUp = nextByte;
                loBackUp = *seq->curLoc[track];
                seq->curLoc[track]++;
                theLen = *seq->curLoc[track];
                seq->curLoc[track]++;
                backup = (u32) hiBackUp;
                backup = backup << 8;
                backup += loBackUp;
                seq->curBUPtr[track] = seq->curLoc[track] - (backup + 4);
                seq->curBULen[track] = (u32) theLen;
                theByte = *seq->curBUPtr[track];
                seq->curBUPtr[track]++;
                seq->curBULen[track]--;
            }
        }
    }
    return theByte;
}

/* JFG: a variable-length number from a track (libultra __readVarLen). */
u32 func_8008A32C(NCSeq *seq, u32 track) {
    u32 value;
    u32 c;

    value = (u32) func_8008A138(seq, track);
    if (value & 0x80) {
        value &= 0x7F;
        do {
            c = (u32) func_8008A138(seq, track);
            value = (value << 7) + (c & 0x7F);
        } while (c & 0x80);
    }
    return value;
}
