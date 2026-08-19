#ifdef PORTABLE
// ==========================================================================
// music_player_port.c
// Ported from pokeemerald PC port (Kurausukun/pokeemerald:pc_port)
// Adapted to pokefirered struct/field names.
// ==========================================================================

#include "global.h"
#include "main.h"
#include "gba/m4a_internal.h"
#include <stdio.h>
#include <string.h>

// ---------------------------------------------------------------------------
// External declarations
// ---------------------------------------------------------------------------
extern struct SoundInfo gSoundInfo;
extern u32 MidiKeyToFreq(struct WaveData *wav, u8 key, u8 fineAdjust);
extern void TrkVolPitSet(struct MusicPlayerInfo *mplayInfo, struct MusicPlayerTrack *track);
extern void FadeOutBody(struct MusicPlayerInfo *mplayInfo);
extern u32 MidiKeyToCgbFreq(u8 chanNum, u8 key, u8 fineAdjust);
extern void ClearModM(struct MusicPlayerTrack *track);
extern const u8 gClockTable[];
extern void *const gMPlayJumpTableTemplate[];

extern void MPlayJumpTableCopy(MPlayFunc *mplayJumpTable);

// Removes chan from the doubly-linked list of channels associated with chan->track.
void ClearChain(void *ptr)
{
    struct SoundChannel *chan = (struct SoundChannel *)ptr;
    if (chan->track == NULL)
        return;

    struct MusicPlayerTrack *track = chan->track;
    struct SoundChannel *next = chan->nextChannelPointer;
    struct SoundChannel *prev = chan->prevChannelPointer;

    if (prev != NULL)
        prev->nextChannelPointer = next;
    else
        track->chan = next;

    if (next != NULL)
        next->prevChannelPointer = prev;

    chan->track = NULL;
}

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

// Signed floor-division by 64: matches pokeemerald's FLOOR_DIV_POW2(x, 64)
#define FLOOR_DIV_POW2_64(x) ((x) >= 0 ? ((x) >> 6) : -(-(x) >> 6))

// Safety limit for linked list traversal to prevent infinite loops from corrupted lists
#define CHAN_LIST_LIMIT 16

// ---------------------------------------------------------------------------
// ChnVolSetAsm  (static)
// ---------------------------------------------------------------------------
static void ChnVolSetAsm(struct SoundChannel *chan, struct MusicPlayerTrack *track)
{
    s8 forcedPan = chan->rhythmPan;
    u32 rightVolume = (u8)(forcedPan + 128) * chan->velocity * track->volMR / 128 / 128;
    if (rightVolume > 0xFF)
        rightVolume = 0xFF;
    chan->rightVolume = rightVolume;

    u32 leftVolume = (u8)(127 - forcedPan) * chan->velocity * track->volML / 128 / 128;
    if (leftVolume > 0xFF)
        leftVolume = 0xFF;
    chan->leftVolume = leftVolume;
}

// ---------------------------------------------------------------------------
// TrackStop
// ---------------------------------------------------------------------------
void TrackStop(struct MusicPlayerInfo *mplayInfo, struct MusicPlayerTrack *track)
{
    if (track->flags & MPT_FLG_EXIST) {
        struct SoundChannel *chan = track->chan;
        for (int _lim = 0; chan != NULL && _lim < CHAN_LIST_LIMIT; _lim++) {
            struct SoundChannel *next = (struct SoundChannel *)chan->nextChannelPointer;
            if (chan->statusFlags != 0) {
                u8 cgbType = chan->type & TONEDATA_TYPE_CGB;
                if (cgbType != 0) {
                    gSoundInfo.CgbOscOff(cgbType);
                }
                chan->statusFlags = 0;
            }
            chan->track = NULL;
            chan = next;
        }
        track->chan = NULL;
        track->flags &= MPT_FLG_EXIST;
    }
}

// ---------------------------------------------------------------------------
// ply_fine  --  ends the current track
// ---------------------------------------------------------------------------
void ply_fine(struct MusicPlayerInfo *mplayInfo, struct MusicPlayerTrack *track)
{
    struct SoundChannel *chan = track->chan;
    for (int _lim = 0; chan != NULL && _lim < CHAN_LIST_LIMIT; _lim++) {
        struct SoundChannel *next = (struct SoundChannel *)chan->nextChannelPointer;
        if (chan->statusFlags & SOUND_CHANNEL_SF_ON) {
            chan->statusFlags |= SOUND_CHANNEL_SF_STOP;
        }
        ClearChain(chan);
        chan = next;
    }
    track->flags = 0;
}

// ---------------------------------------------------------------------------
// ply_goto  --  sets cmdPtr to the specified address
// ---------------------------------------------------------------------------
void ply_goto(struct MusicPlayerInfo *mplayInfo, struct MusicPlayerTrack *track)
{
    u8 *cmdPtr = track->cmdPtr;
    uintptr_t addr = 0;
    // Read little-endian pointer from the command stream
    addr  = cmdPtr[0];
    addr |= (uintptr_t)cmdPtr[1] << 8;
    addr |= (uintptr_t)cmdPtr[2] << 16;
    addr |= (uintptr_t)cmdPtr[3] << 24;
#if UINTPTR_MAX > 0xFFFFFFFFUL
    // On 64-bit platforms, read 8 bytes for a full pointer
    addr |= (uintptr_t)cmdPtr[4] << 32;
    addr |= (uintptr_t)cmdPtr[5] << 40;
    addr |= (uintptr_t)cmdPtr[6] << 48;
    addr |= (uintptr_t)cmdPtr[7] << 56;
#endif
    track->cmdPtr = (u8 *)addr;
}

// ---------------------------------------------------------------------------
// ply_patt  --  saves position then jumps (pattern call)
// ---------------------------------------------------------------------------
void ply_patt(struct MusicPlayerInfo *mplayInfo, struct MusicPlayerTrack *track)
{
    u8 level = track->patternLevel;
    if (level < 3) {
        track->patternStack[level] = track->cmdPtr + sizeof(u8 *);
        track->patternLevel++;
        ply_goto(mplayInfo, track);
    } else {
        // Too deeply nested -- stop track as an error signal
        ply_fine(mplayInfo, track);
    }
}

// ---------------------------------------------------------------------------
// ply_pend  --  return from pattern
// ---------------------------------------------------------------------------
void ply_pend(struct MusicPlayerInfo *mplayInfo, struct MusicPlayerTrack *track)
{
    if (track->patternLevel != 0) {
        u8 index = --track->patternLevel;
        track->cmdPtr = track->patternStack[index];
    }
}

// ---------------------------------------------------------------------------
// ply_rept  --  loop back N times (0 = infinite)
// ---------------------------------------------------------------------------
void ply_rept(struct MusicPlayerInfo *mplayInfo, struct MusicPlayerTrack *track)
{
    if (*track->cmdPtr == 0) {
        // "Repeat 0 times" == loop forever
        track->cmdPtr++;
        ply_goto(mplayInfo, track);
    } else {
        u8 repeatCount = ++track->repN;
        u8 target = *track->cmdPtr++;
        if (repeatCount < target) {
            ply_goto(mplayInfo, track);
        } else {
            track->repN = 0;
            track->cmdPtr += sizeof(u8 *);
        }
    }
}

// ---------------------------------------------------------------------------
// ply_prio  --  set track priority
// ---------------------------------------------------------------------------
void ply_prio(struct MusicPlayerInfo *mplayInfo, struct MusicPlayerTrack *track)
{
    track->priority = *track->cmdPtr++;
}

// ---------------------------------------------------------------------------
// ply_tempo  --  set BPM (in half-minutes)
// ---------------------------------------------------------------------------
void ply_tempo(struct MusicPlayerInfo *mplayInfo, struct MusicPlayerTrack *track)
{
    u16 bpm = *track->cmdPtr++;
    bpm *= 2;
    mplayInfo->tempoD = bpm;
    mplayInfo->tempoI = (bpm * mplayInfo->tempoU) >> 8;
}

// ---------------------------------------------------------------------------
// ply_keysh  --  set key shift (transpose)
// ---------------------------------------------------------------------------
void ply_keysh(struct MusicPlayerInfo *mplayInfo, struct MusicPlayerTrack *track)
{
    track->keyShift = *track->cmdPtr++;
    track->flags |= MPT_FLG_PITCHG;
}

// ---------------------------------------------------------------------------
// ply_voice  --  set voice / instrument
// ---------------------------------------------------------------------------
void ply_voice(struct MusicPlayerInfo *mplayInfo, struct MusicPlayerTrack *track)
{
    u8 voice = *track->cmdPtr++;
    struct ToneData *tonePtr = &mplayInfo->tone[voice];
    track->tone = *tonePtr;
}

// ---------------------------------------------------------------------------
// ply_vol  --  set volume
// ---------------------------------------------------------------------------
void ply_vol(struct MusicPlayerInfo *mplayInfo, struct MusicPlayerTrack *track)
{
    track->vol = *track->cmdPtr++;
    track->flags |= MPT_FLG_VOLCHG;
}

// ---------------------------------------------------------------------------
// ply_pan  --  set panning
// ---------------------------------------------------------------------------
void ply_pan(struct MusicPlayerInfo *mplayInfo, struct MusicPlayerTrack *track)
{
    track->pan = *track->cmdPtr++ - C_V;
    track->flags |= MPT_FLG_VOLCHG;
}

// ---------------------------------------------------------------------------
// ply_bend  --  set pitch bend
// ---------------------------------------------------------------------------
void ply_bend(struct MusicPlayerInfo *mplayInfo, struct MusicPlayerTrack *track)
{
    track->bend = *track->cmdPtr++ - C_V;
    track->flags |= MPT_FLG_PITCHG;
}

// ---------------------------------------------------------------------------
// ply_bendr  --  set pitch bend range
// ---------------------------------------------------------------------------
void ply_bendr(struct MusicPlayerInfo *mplayInfo, struct MusicPlayerTrack *track)
{
    track->bendRange = *track->cmdPtr++;
    track->flags |= MPT_FLG_PITCHG;
}

// ---------------------------------------------------------------------------
// ply_lfos  --  set LFO speed
// ---------------------------------------------------------------------------
void ply_lfos(struct MusicPlayerInfo *mplayInfo, struct MusicPlayerTrack *track)
{
    track->lfoSpeed = *track->cmdPtr++;
    if (track->lfoSpeed == 0) {
        ClearModM(track);
    }
}

// ---------------------------------------------------------------------------
// ply_lfodl  --  set LFO delay
// ---------------------------------------------------------------------------
void ply_lfodl(struct MusicPlayerInfo *mplayInfo, struct MusicPlayerTrack *track)
{
    track->lfoDelay = *track->cmdPtr++;
}

// ---------------------------------------------------------------------------
// ply_mod  --  set modulation depth
// ---------------------------------------------------------------------------
void ply_mod(struct MusicPlayerInfo *mplayInfo, struct MusicPlayerTrack *track)
{
    track->mod = *track->cmdPtr++;
    if (track->mod == 0) {
        ClearModM(track);
    }
}

// ---------------------------------------------------------------------------
// ply_modt  --  set modulation type
// ---------------------------------------------------------------------------
void ply_modt(struct MusicPlayerInfo *mplayInfo, struct MusicPlayerTrack *track)
{
    u8 type = *track->cmdPtr++;
    if (type != track->modT) {
        track->modT = type;
        track->flags |= 0xF;
    }
}

// ---------------------------------------------------------------------------
// ply_tune  --  set fine tune
// ---------------------------------------------------------------------------
void ply_tune(struct MusicPlayerInfo *mplayInfo, struct MusicPlayerTrack *track)
{
    track->tune = *track->cmdPtr++ - C_V;
    track->flags |= MPT_FLG_PITCHG;
}

// ---------------------------------------------------------------------------
// ply_port  --  direct register write (CGB port; skipped on PC)
// ---------------------------------------------------------------------------
void ply_port(struct MusicPlayerInfo *mplayInfo, struct MusicPlayerTrack *track)
{
    // On a real GBA this writes to NR10+offset. On PC we just skip 2 bytes.
    track->cmdPtr += 2;
}

// ---------------------------------------------------------------------------
// ply_endtie  --  end a tied note
// ---------------------------------------------------------------------------
void ply_endtie(struct MusicPlayerInfo *mplayInfo, struct MusicPlayerTrack *track)
{
    u8 key = *track->cmdPtr;
    if (key < 0x80) {
        track->key = key;
        track->cmdPtr++;
    } else {
        key = track->key;
    }

    struct SoundChannel *chan = track->chan;
    for (int _lim = 0; chan != NULL && _lim < CHAN_LIST_LIMIT; _lim++) {
        if ((chan->statusFlags & 0x83) && !(chan->statusFlags & SOUND_CHANNEL_SF_STOP)
            && chan->midiKey == key) {
            chan->statusFlags |= SOUND_CHANNEL_SF_STOP;
            return;
        }
        chan = (struct SoundChannel *)chan->nextChannelPointer;
    }
}

// ---------------------------------------------------------------------------
// ply_note  --  play a note (event >= 0xCF)
// ---------------------------------------------------------------------------
void ply_note(u32 note_cmd, struct MusicPlayerInfo *mplayInfo, struct MusicPlayerTrack *track)
{
    struct SoundInfo *si = &gSoundInfo;

    // Read note length from clock table
    track->gateTime = gClockTable[note_cmd];
    if (*track->cmdPtr < 0x80) {
        track->key = *track->cmdPtr++;
        if (*track->cmdPtr < 0x80) {
            track->velocity = *track->cmdPtr++;
            if (*track->cmdPtr < 0x80) {
                track->gateTime += *track->cmdPtr++;
            }
        }
    }

    s8 forcedPan = 0;
    struct ToneData tone = track->tone;
    u8 key = track->key;
    u8 type = tone.type;

    if (type & (TONEDATA_TYPE_RHY | TONEDATA_TYPE_SPL)) {
        u8 instrumentIndex;
        if (type & TONEDATA_TYPE_SPL) {
            u8 *keySplitTable = *(u8 **)&tone.attack;
            if (keySplitTable == NULL)
                return;
            instrumentIndex = keySplitTable[track->key];
        } else {
            instrumentIndex = track->key;
        }

        struct ToneData *subGroup = (struct ToneData *)tone.wav;
        if (subGroup == NULL)
            return;
        tone = subGroup[instrumentIndex];

        if (tone.type & (TONEDATA_TYPE_RHY | TONEDATA_TYPE_SPL))
            return;

        if (type & TONEDATA_TYPE_RHY) {
            if (tone.pan_sweep & 0x80) {
                forcedPan = ((s8)(tone.pan_sweep & 0x7F) - 0x40) * 2;
            }
            key = tone.key;
        }
    }

    // Calculate effective priority
    u16 priority = mplayInfo->priority + track->priority;
    if (priority > 0xFF) {
        priority = 0xFF;
    }

    u8 cgbType = tone.type & TONEDATA_TYPE_CGB;
    struct SoundChannel *chan;

    if (cgbType != 0) {
        if (si->cgbChans == NULL)
            return;
        chan = (struct SoundChannel *)&si->cgbChans[cgbType - 1];

        if ((chan->statusFlags & SOUND_CHANNEL_SF_ON)
         && !(chan->statusFlags & SOUND_CHANNEL_SF_STOP)) {
            if (chan->priority > (u8)priority
             || (chan->priority == (u8)priority && chan->track < track)) {
                return;
            }
        }
    } else {
        u16 p = priority;
        struct MusicPlayerTrack *t = track;
        bool32 foundStoppingChannel = FALSE;
        chan = NULL;
        u8 maxChans = si->maxChans;
        struct SoundChannel *currChan = si->chans;

        for (u8 i = 0; i < maxChans; i++, currChan++) {
            if (!(currChan->statusFlags & SOUND_CHANNEL_SF_ON)) {
                // Found a completely inactive channel
                chan = currChan;
                break;
            }

            if ((currChan->statusFlags & SOUND_CHANNEL_SF_STOP) && !foundStoppingChannel) {
                // Found a stopping channel -- better than any playing channel
                foundStoppingChannel = TRUE;
                p = currChan->priority;
                t = currChan->track;
                chan = currChan;
            } else if (((currChan->statusFlags & SOUND_CHANNEL_SF_STOP) && foundStoppingChannel)
                    || (!(currChan->statusFlags & SOUND_CHANNEL_SF_STOP) && !foundStoppingChannel)) {
                // Same tier -- check priority and track order
                if (currChan->priority < p) {
                    p = currChan->priority;
                    t = currChan->track;
                    chan = currChan;
                } else if (currChan->priority == p && currChan->track > t) {
                    t = currChan->track;
                    chan = currChan;
                } else if (currChan->priority == p && currChan->track == t) {
                    chan = currChan;
                }
            }
        }
    }

    if (chan == NULL) {
        return;
    }
    ClearChain(chan);

    // Insert channel at head of track's channel list
    chan->prevChannelPointer = NULL;
    chan->nextChannelPointer = track->chan;
    if (track->chan != NULL) {
        track->chan->prevChannelPointer = chan;
    }
    track->chan = chan;
    chan->track = track;

    track->lfoDelayC = track->lfoDelay;
    if (track->lfoDelay != 0) {
        ClearModM(track);
    }
    TrkVolPitSet(mplayInfo, track);

    chan->gateTime = track->gateTime;
    chan->midiKey = track->key;
    chan->velocity = track->velocity;
    chan->priority = (u8)priority;
    chan->key = key;
    chan->rhythmPan = forcedPan;
    chan->type = tone.type;
    chan->wav = (struct WaveData *)tone.wav;
    chan->attack = tone.attack;
    chan->decay = tone.decay;
    chan->sustain = tone.sustain;
    chan->release = tone.release;
    chan->pseudoEchoVolume = track->pseudoEchoVolume;
    chan->pseudoEchoLength = track->pseudoEchoLength;
    ChnVolSetAsm(chan, track);

    // Compute transposed key
    s16 transposedKey = chan->key;
    transposedKey += track->keyM;
    if (transposedKey < 0) {
        transposedKey = 0;
    }

    if (cgbType != 0) {
        struct CgbChannel *cgbChan = (struct CgbChannel *)chan;
        cgbChan->length = tone.length;
        if ((tone.pan_sweep & 0x80) || (tone.pan_sweep & 0x70) == 0) {
            cgbChan->sweep = 8;
        } else {
            cgbChan->sweep = tone.pan_sweep;
        }

        cgbChan->frequency = si->MidiKeyToCgbFreq(cgbType, (u8)transposedKey, track->pitM);
    } else {
        chan->count = track->unk_3C;
        chan->frequency = MidiKeyToFreq(chan->wav, (u8)transposedKey, track->pitM);
    }

    chan->statusFlags = SOUND_CHANNEL_SF_START;
    track->flags &= ~0xF;

}

// ---------------------------------------------------------------------------
// MPlayMain  --  main music player tick function
// ---------------------------------------------------------------------------
void MPlayMain(struct MusicPlayerInfo *mplayInfo)
{
    struct SoundInfo *si = &gSoundInfo;
    extern struct MusicPlayerInfo gMPlayInfo_BGM;
    extern struct MusicPlayerInfo gMPlayInfo_SE1;
    extern struct MusicPlayerInfo gMPlayInfo_SE2;
    static int se2LogCount = 0;

    if (mplayInfo->ident != ID_NUMBER) {
        return;
    }
    mplayInfo->ident = ID_NUMBER + 1;

    if (0 && mplayInfo->MPlayMainNext != NULL) {
        mplayInfo->MPlayMainNext(mplayInfo->musicPlayerNext);
    }

    if (mplayInfo->status & MUSICPLAYER_STATUS_PAUSE) {
        goto returnEarly;
    }
    FadeOutBody(mplayInfo);
    if (mplayInfo->status & MUSICPLAYER_STATUS_PAUSE) {
        goto returnEarly;
    }

    mplayInfo->tempoC += mplayInfo->tempoI;
    (void)se2LogCount;
    while (mplayInfo->tempoC >= 150) {
        u16 trackBits = 0;

        for (u32 i = 0; i < mplayInfo->trackCount; i++) {
            struct MusicPlayerTrack *currentTrack = mplayInfo->tracks + i;
            struct SoundChannel *chan;
            if (!(currentTrack->flags & MPT_FLG_EXIST)) {
                continue;
            }
            trackBits |= (1 << i);

            chan = currentTrack->chan;
            for (int _lim = 0; chan != NULL && _lim < CHAN_LIST_LIMIT; _lim++) {
                struct SoundChannel *next = (struct SoundChannel *)chan->nextChannelPointer;
                if (!(chan->statusFlags & SOUND_CHANNEL_SF_ON)) {
                    ClearChain(chan);
                } else if (chan->gateTime != 0 && --chan->gateTime == 0) {
                    chan->statusFlags |= SOUND_CHANNEL_SF_STOP;
                }
                chan = next;
            }

            if (currentTrack->flags & MPT_FLG_START) {
                memset(currentTrack, 0, 0x40);
                currentTrack->flags = MPT_FLG_EXIST;
                currentTrack->bendRange = 2;
                currentTrack->volX = 64;
                currentTrack->lfoSpeed = 22;
                currentTrack->tone.type = 1;
            }

            for (int _cmdLimit = 0; currentTrack->wait == 0 && _cmdLimit < 256; _cmdLimit++) {
                u8 event = *currentTrack->cmdPtr;
                if (event < 0x80) {
                    event = currentTrack->runningStatus;
                    if (event == 0) break;
                } else {
                    currentTrack->cmdPtr++;
                    if (event >= 0xBD) {
                        currentTrack->runningStatus = event;
                    }
                }

                if (event >= 0xCF) {
                    si->plynote(event - 0xCF, mplayInfo, currentTrack);
                } else if (event >= 0xB1) {
                    u8 cmd = event - 0xB1;
                    void (*eventFunc)(struct MusicPlayerInfo *, struct MusicPlayerTrack *);
                    eventFunc = (void (*)(struct MusicPlayerInfo *, struct MusicPlayerTrack *))si->MPlayJumpTable[cmd];
                    eventFunc(mplayInfo, currentTrack);

                    if (currentTrack->flags == 0) {
                        goto nextTrack;
                    }
                } else {
                    currentTrack->wait = gClockTable[event - 0x80];
                }
            }

            currentTrack->wait--;

            if (currentTrack->lfoSpeed != 0 && currentTrack->mod != 0) {
                if (currentTrack->lfoDelayC != 0) {
                    currentTrack->lfoDelayC--;
                    goto nextTrack;
                }

                currentTrack->lfoSpeedC += currentTrack->lfoSpeed;

                s8 r;
                if (currentTrack->lfoSpeedC >= 0x40U && currentTrack->lfoSpeedC < 0xC0U) {
                    r = 128 - currentTrack->lfoSpeedC;
                } else if (currentTrack->lfoSpeedC >= 0xC0U) {
                    r = currentTrack->lfoSpeedC - 256;
                } else {
                    r = currentTrack->lfoSpeedC;
                }
                r = FLOOR_DIV_POW2_64(currentTrack->mod * r);

                if (r != currentTrack->modM) {
                    currentTrack->modM = r;
                    if (currentTrack->modT == 0) {
                        currentTrack->flags |= MPT_FLG_PITCHG;
                    } else {
                        currentTrack->flags |= MPT_FLG_VOLCHG;
                    }
                }
            }

            nextTrack:;
        }

        mplayInfo->clock++;
        if (trackBits == 0) {
            mplayInfo->status = MUSICPLAYER_STATUS_PAUSE;
            goto returnEarly;
        }
        mplayInfo->status = trackBits;
        mplayInfo->tempoC -= 150;
    }

    // Volume/pitch update pass
    {
        u32 i = 0;
        do {
            struct MusicPlayerTrack *track = mplayInfo->tracks + i;

            if (!(track->flags & MPT_FLG_EXIST) || !(track->flags & 0xF)) {
                continue;
            }
            TrkVolPitSet(mplayInfo, track);
            struct SoundChannel *chan = track->chan;
            for (int _lim = 0; chan != NULL && _lim < CHAN_LIST_LIMIT; _lim++) {
                struct SoundChannel *next = (struct SoundChannel *)chan->nextChannelPointer;
                if (!(chan->statusFlags & SOUND_CHANNEL_SF_ON)) {
                    ClearChain(chan);
                    chan = next;
                    continue;
                }
                u8 cgbType = chan->type & TONEDATA_TYPE_CGB;
                if (track->flags & MPT_FLG_VOLCHG) {
                    ChnVolSetAsm(chan, track);
                    if (cgbType != 0) {
                        ((struct CgbChannel *)chan)->modify |= CGB_CHANNEL_MO_VOL;
                    }
                }
                if (track->flags & MPT_FLG_PITCHG) {
                    s32 adjKey = chan->key + track->keyM;
                    if (adjKey < 0) {
                        adjKey = 0;
                    }
                    if (cgbType != 0) {
                        ((struct CgbChannel *)chan)->frequency = si->MidiKeyToCgbFreq(cgbType, (u8)adjKey, track->pitM);
                        ((struct CgbChannel *)chan)->modify |= CGB_CHANNEL_MO_PIT;
                    } else {
                        chan->frequency = MidiKeyToFreq(chan->wav, (u8)adjKey, track->pitM);
                    }
                }
                chan = next;
            }
            track->flags &= ~0xF;
        } while (++i < mplayInfo->trackCount);
    }

returnEarly:
    mplayInfo->ident = ID_NUMBER;
}

#endif // PORTABLE
