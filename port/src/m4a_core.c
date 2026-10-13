/* m4a_core.c — the core of the game's sound engine (MusicPlayer 2000 /
 * "m4a"), which ships as ARM assembly (m4a_1.s) and is rewritten here in C.
 *
 * Two halves:
 *  - the sequencer (MPlayMain and the track commands): exact, because the
 *    game watches when songs, fanfares and cries end;
 *  - SoundMain, run once per VBlank: sequencer chain, the PSG driver
 *    (CgbSound, already C in m4a.c), then each Direct Sound channel's
 *    envelope step. The envelope step is exact (it decides when a channel
 *    frees itself). Turning channels into audio is the mixer's job
 *    (mixer.c), at the host's sample rate instead of the console's 13 kHz. */
#include <string.h>

#include "global.h"
#include "gba/m4a_internal.h"
#include "pc_mixer.h"

/* Implemented in C by m4a.c. */
void ClearChain(void *x);
void Clear64byte(void *x);
void FadeOutBody(struct MusicPlayerInfo *mplayInfo);
void TrkVolPitSet(struct MusicPlayerInfo *mplayInfo, struct MusicPlayerTrack *track);
u32 MidiKeyToFreq(struct WaveData *wav, u8 key, u8 fineAdjust);
void SampleFreqSet(u32 freq);
void RealClearChain(void *x);
void SoundMainBTM(void *x);

extern const u8 gClockTable[];
extern void *const gMPlayJumpTableTemplate[];

typedef void (*TrackCmd)(struct MusicPlayerInfo *, struct MusicPlayerTrack *);

#define CHAN_NEXT(c) ((struct SoundChannel *)(c)->nextChannelPointer)
#define CHAN_PREV(c) ((struct SoundChannel *)(c)->prevChannelPointer)

/* ---- small helpers ----------------------------------------------------------- */

u32 umul3232H32(u32 a, u32 b)
{
    return (u32)(((u64)a * b) >> 32);
}

static u8 next_byte(struct MusicPlayerTrack *track)
{
    return *track->cmdPtr++;
}

/* Copies the template; the console's BIOS call does the same. */
void MPlayJumpTableCopy(MPlayFunc *table)
{
    for (int i = 0; i < 36; i++)
        table[i] = (MPlayFunc)gMPlayJumpTableTemplate[i];
}

/* 64 zero bytes (used to reset a track's head). */
void SoundMainBTM(void *x)
{
    memset(x, 0, 64);
}

/* Unlink a channel from its track's channel list. */
void RealClearChain(void *x)
{
    struct SoundChannel *chan = x;
    struct MusicPlayerTrack *track = chan->track;

    if (!track)
        return;
    if (CHAN_PREV(chan))
        CHAN_PREV(chan)->nextChannelPointer = chan->nextChannelPointer;
    else
        track->chan = CHAN_NEXT(chan);
    if (CHAN_NEXT(chan))
        CHAN_NEXT(chan)->prevChannelPointer = chan->prevChannelPointer;
    chan->track = NULL;
}

static void clear_modM(struct MusicPlayerTrack *track)
{
    track->modM = 0;
    track->lfoSpeedC = 0;
    track->flags |= track->modT == 0 ? MPT_FLG_PITCHG : MPT_FLG_VOLCHG;
}

/* Channel volume from note velocity, rhythm pan and the track's L/R volume. */
static void chn_vol_set(struct SoundChannel *chan, struct MusicPlayerTrack *track)
{
    s32 pan = (s8)chan->rhythmPan;
    u32 right = ((0x80 + pan) * chan->velocity * track->volMR) >> 14;
    u32 left = ((0x7F - pan) * chan->velocity * track->volML) >> 14;

    chan->rightVolume = right > 0xFF ? 0xFF : (u8)right;
    chan->leftVolume = left > 0xFF ? 0xFF : (u8)left;
}

/* ---- track commands (0xB1-0xCE, indexed from 0xB1) ------------------------------ */

void ply_fine(struct MusicPlayerInfo *mp, struct MusicPlayerTrack *track)
{
    (void)mp;
    for (struct SoundChannel *chan = track->chan; chan; chan = CHAN_NEXT(chan)) {
        if (chan->statusFlags & SOUND_CHANNEL_SF_ON)
            chan->statusFlags |= SOUND_CHANNEL_SF_STOP;
        RealClearChain(chan);
    }
    track->flags = 0;
}

static u8 *read_ptr(const u8 *p)
{
    return (u8 *)(uintptr_t)(p[0] | (p[1] << 8) | (p[2] << 16) | ((u32)p[3] << 24));
}

void ply_goto(struct MusicPlayerInfo *mp, struct MusicPlayerTrack *track)
{
    (void)mp;
    track->cmdPtr = read_ptr(track->cmdPtr);
}

void ply_patt(struct MusicPlayerInfo *mp, struct MusicPlayerTrack *track)
{
    if (track->patternLevel >= 3) {
        ply_fine(mp, track);
        return;
    }
    track->patternStack[track->patternLevel++] = track->cmdPtr + 4;
    ply_goto(mp, track);
}

void ply_pend(struct MusicPlayerInfo *mp, struct MusicPlayerTrack *track)
{
    (void)mp;
    if (track->patternLevel)
        track->cmdPtr = track->patternStack[--track->patternLevel];
}

void ply_rept(struct MusicPlayerInfo *mp, struct MusicPlayerTrack *track)
{
    u8 *p = track->cmdPtr;

    if (*p == 0) { /* repeat forever */
        track->cmdPtr = p + 1;
        ply_goto(mp, track);
        return;
    }
    track->repN++;
    track->cmdPtr = p + 1;
    if (track->repN < *p) {
        ply_goto(mp, track);
    } else {
        track->repN = 0;
        track->cmdPtr = p + 5;
    }
}

void ply_prio(struct MusicPlayerInfo *mp, struct MusicPlayerTrack *track)
{
    (void)mp;
    track->priority = next_byte(track);
}

void ply_tempo(struct MusicPlayerInfo *mp, struct MusicPlayerTrack *track)
{
    mp->tempoD = next_byte(track) * 2;
    mp->tempoI = (u16)((mp->tempoD * mp->tempoU) >> 8);
}

void ply_keysh(struct MusicPlayerInfo *mp, struct MusicPlayerTrack *track)
{
    (void)mp;
    track->keyShift = (s8)next_byte(track);
    track->flags |= MPT_FLG_PITCHG;
}

void ply_voice(struct MusicPlayerInfo *mp, struct MusicPlayerTrack *track)
{
    const struct ToneData *tone = &mp->tone[next_byte(track)];
    memcpy(&track->tone, tone, sizeof(struct ToneData));
}

void ply_vol(struct MusicPlayerInfo *mp, struct MusicPlayerTrack *track)
{
    (void)mp;
    track->vol = next_byte(track);
    track->flags |= MPT_FLG_VOLCHG;
}

void ply_pan(struct MusicPlayerInfo *mp, struct MusicPlayerTrack *track)
{
    (void)mp;
    track->pan = (s8)(next_byte(track) - C_V);
    track->flags |= MPT_FLG_VOLCHG;
}

void ply_bend(struct MusicPlayerInfo *mp, struct MusicPlayerTrack *track)
{
    (void)mp;
    track->bend = (s8)(next_byte(track) - C_V);
    track->flags |= MPT_FLG_PITCHG;
}

void ply_bendr(struct MusicPlayerInfo *mp, struct MusicPlayerTrack *track)
{
    (void)mp;
    track->bendRange = next_byte(track);
    track->flags |= MPT_FLG_PITCHG;
}

void ply_lfodl(struct MusicPlayerInfo *mp, struct MusicPlayerTrack *track)
{
    (void)mp;
    track->lfoDelay = next_byte(track);
}

void ply_modt(struct MusicPlayerInfo *mp, struct MusicPlayerTrack *track)
{
    u8 type = next_byte(track);

    (void)mp;
    if (track->modT != type) {
        track->modT = type;
        track->flags |= MPT_FLG_VOLCHG | MPT_FLG_PITCHG;
    }
}

void ply_tune(struct MusicPlayerInfo *mp, struct MusicPlayerTrack *track)
{
    (void)mp;
    track->tune = (s8)(next_byte(track) - C_V);
    track->flags |= MPT_FLG_PITCHG;
}

/* Direct write to a PSG register (offset from REG_SOUND1CNT_L). */
void ply_port(struct MusicPlayerInfo *mp, struct MusicPlayerTrack *track)
{
    u8 reg = next_byte(track);
    u8 value = next_byte(track);

    (void)mp;
    gPcIo[0x60 + reg] = value;
}

void ply_lfos(struct MusicPlayerInfo *mp, struct MusicPlayerTrack *track)
{
    (void)mp;
    track->lfoSpeed = next_byte(track);
    if (!track->lfoSpeed)
        clear_modM(track);
}

void ply_mod(struct MusicPlayerInfo *mp, struct MusicPlayerTrack *track)
{
    (void)mp;
    track->mod = next_byte(track);
    if (!track->mod)
        clear_modM(track);
}

/* Release the (most recent) channel playing this key. */
void ply_endtie(struct MusicPlayerInfo *mp, struct MusicPlayerTrack *track)
{
    u8 key;

    (void)mp;
    if (*track->cmdPtr < 0x80)
        track->key = key = next_byte(track);
    else
        key = track->key;
    for (struct SoundChannel *chan = track->chan; chan; chan = CHAN_NEXT(chan)) {
        u8 st = chan->statusFlags;
        if ((st & (SOUND_CHANNEL_SF_START | SOUND_CHANNEL_SF_ENV)) && !(st & SOUND_CHANNEL_SF_STOP)
            && chan->midiKey == key) {
            chan->statusFlags = st | SOUND_CHANNEL_SF_STOP;
            break;
        }
    }
}

/* ---- notes ------------------------------------------------------------------------ */

/* Note on: pick (or steal) a channel by priority, set it up from the track's
 * current voice, and start it. `clock` indexes the note-length table. */
void ply_note(u32 clock, struct MusicPlayerInfo *mp, struct MusicPlayerTrack *track)
{
    struct SoundInfo *si = SOUND_INFO_PTR;
    const struct ToneData *tone;
    struct SoundChannel *chan;
    u32 key, priority, cgb;
    s32 rhythm_pan = 0;

    track->gateTime = gClockTable[clock];
    if (*track->cmdPtr < 0x80) {
        track->key = next_byte(track);
        if (*track->cmdPtr < 0x80) {
            track->velocity = next_byte(track);
            if (*track->cmdPtr < 0x80)
                track->gateTime += next_byte(track);
        }
    }

    /* Resolve drum kits / key splits to the actual voice. */
    if (track->tone.type & (TONEDATA_TYPE_RHY | TONEDATA_TYPE_SPL)) {
        u32 index = track->key;
        if (track->tone.type & TONEDATA_TYPE_SPL)
            index = (*(const u8 **)&track->tone.attack)[track->key];
        tone = &((const struct ToneData *)track->tone.wav)[index];
        if (tone->type & (TONEDATA_TYPE_SPL | TONEDATA_TYPE_RHY))
            return;
        if (track->tone.type & TONEDATA_TYPE_RHY) {
            if (tone->pan_sweep & 0x80)
                rhythm_pan = ((s32)tone->pan_sweep - TONEDATA_P_S_PAN) << 1;
            key = tone->key;
        } else {
            key = track->key;
        }
    } else {
        tone = &track->tone;
        key = track->key;
    }

    priority = track->priority + mp->priority;
    if (priority > 0xFF) priority = 0xFF;

    cgb = tone->type & TONEDATA_TYPE_CGB;
    if (cgb) {
        if (!si->cgbChans)
            return;
        chan = (struct SoundChannel *)&si->cgbChans[cgb - 1];
        u8 st = chan->statusFlags;
        if ((st & SOUND_CHANNEL_SF_ON) && !(st & SOUND_CHANNEL_SF_STOP)) {
            if (chan->priority > priority)
                return;
            if (chan->priority == priority && (uintptr_t)chan->track < (uintptr_t)track)
                return;
        }
    } else {
        /* A free channel, else the lowest-priority one (releasing ones first;
         * among equals, the one belonging to the later track). */
        struct SoundChannel *c = si->chans, *best = NULL;
        u32 best_prio = priority;
        struct MusicPlayerTrack *best_track = track;
        int found_stopping = 0;

        for (int n = si->maxChans; n > 0; n--, c++) {
            u8 st = c->statusFlags;
            if (!(st & SOUND_CHANNEL_SF_ON)) {
                best = c;
                break;
            }
            if (st & SOUND_CHANNEL_SF_STOP) {
                if (!found_stopping) {
                    found_stopping = 1;
                    best_prio = c->priority;
                    best_track = c->track;
                    best = c;
                    continue;
                }
            } else if (found_stopping) {
                continue;
            }
            if (c->priority < best_prio) {
                best_prio = c->priority;
                best_track = c->track;
                best = c;
            } else if (c->priority == best_prio) {
                if ((uintptr_t)c->track > (uintptr_t)best_track) {
                    best_track = c->track;
                    best = c;
                } else if ((uintptr_t)c->track == (uintptr_t)best_track) {
                    best = c;
                }
            }
        }
        chan = best;
        if (!chan)
            return;
    }

    ClearChain(chan);
    chan->prevChannelPointer = NULL;
    chan->nextChannelPointer = track->chan;
    if (track->chan)
        track->chan->prevChannelPointer = chan;
    track->chan = chan;
    chan->track = track;

    track->lfoDelayC = track->lfoDelay;
    if (track->lfoDelay)
        clear_modM(track);
    TrkVolPitSet(mp, track);

    /* gateTime, key, velocity and the byte after them, as one word */
    memcpy(&chan->gateTime, &track->gateTime, 4);
    chan->priority = (u8)priority;
    chan->key = (u8)key;
    chan->rhythmPan = (u8)rhythm_pan;
    chan->type = tone->type;
    chan->wav = tone->wav;
    memcpy(&chan->attack, &tone->attack, 4);
    memcpy(&chan->pseudoEchoVolume, &track->pseudoEchoVolume, 2);
    chn_vol_set(chan, track);

    s32 k = (s32)chan->key + track->keyM;
    if (k < 0) k = 0;
    if (cgb) {
        struct CgbChannel *cc = (struct CgbChannel *)chan;
        u8 ps = tone->pan_sweep;
        cc->length = tone->length;
        cc->sweep = ((ps & 0x80) || !(ps & 0x70)) ? 8 : ps;
        cc->frequency = si->MidiKeyToCgbFreq(cgb, (u8)k, track->pitM);
    } else {
        chan->count = track->unk_3C;
        chan->frequency = MidiKeyToFreq(chan->wav, (u8)k, track->pitM);
    }
    chan->statusFlags = SOUND_CHANNEL_SF_START;
    track->flags &= 0xF0;
}

void TrackStop(struct MusicPlayerInfo *mp, struct MusicPlayerTrack *track)
{
    (void)mp;
    if (!(track->flags & MPT_FLG_EXIST))
        return;
    for (struct SoundChannel *chan = track->chan; chan; chan = CHAN_NEXT(chan)) {
        if (chan->statusFlags) {
            if (chan->type & TONEDATA_TYPE_CGB)
                SOUND_INFO_PTR->CgbOscOff(chan->type & TONEDATA_TYPE_CGB);
            chan->statusFlags = 0;
        }
        chan->track = NULL;
    }
    track->chan = NULL;
}

/* ---- the player -------------------------------------------------------------------- */

void MPlayMain(struct MusicPlayerInfo *mp)
{
    struct SoundInfo *si;

    if (mp->ident != ID_NUMBER)
        return;
    mp->ident++; /* busy */

    if (mp->MPlayMainNext)
        mp->MPlayMainNext((struct MusicPlayerInfo *)mp->musicPlayerNext);

    if ((s32)mp->status < 0)
        goto done;
    si = SOUND_INFO_PTR;
    FadeOutBody(mp);
    if ((s32)mp->status < 0)
        goto done;

    mp->tempoC += mp->tempoI;
    while (mp->tempoC >= 150) {
        struct MusicPlayerTrack *track = mp->tracks;
        u32 active = 0, bit = 1;

        for (int i = mp->trackCount; i > 0; i--, track++, bit <<= 1) {
            if (!(track->flags & MPT_FLG_EXIST))
                continue;
            active |= bit;

            for (struct SoundChannel *chan = track->chan; chan; chan = CHAN_NEXT(chan)) {
                if (chan->statusFlags & SOUND_CHANNEL_SF_ON) {
                    if (chan->gateTime && --chan->gateTime == 0)
                        chan->statusFlags |= SOUND_CHANNEL_SF_STOP;
                } else {
                    ClearChain(chan);
                }
            }

            if (track->flags & MPT_FLG_START) {
                Clear64byte(track);
                track->flags = MPT_FLG_EXIST;
                track->bendRange = 2;
                track->volX = 64;
                track->lfoSpeed = 22;
                track->tone.type = 1;
            }

            while (track->wait == 0) {
                u8 cmd = *track->cmdPtr;
                if (cmd < 0x80) {
                    cmd = track->runningStatus;
                } else {
                    track->cmdPtr++;
                    if (cmd >= 0xBD)
                        track->runningStatus = cmd;
                }
                if (cmd >= 0xCF) {
                    si->plynote(cmd - 0xCF, mp, track);
                } else if (cmd > 0xB0) {
                    mp->cmd = cmd - 0xB1;
                    ((TrackCmd)si->MPlayJumpTable[cmd - 0xB1])(mp, track);
                    if (track->flags == 0)
                        goto next_track; /* the command ended the track */
                } else {
                    track->wait = gClockTable[cmd - 0x80];
                }
            }
            track->wait--;

            /* LFO (vibrato / tremolo) */
            if (track->lfoSpeed && track->mod) {
                if (track->lfoDelayC) {
                    track->lfoDelayC--;
                } else {
                    s32 phase = track->lfoSpeedC + track->lfoSpeed;
                    s32 wave;
                    track->lfoSpeedC = (u8)phase;
                    if ((s8)(phase - 0x40) < 0)
                        wave = (s8)phase;
                    else
                        wave = 0x80 - phase;
                    wave = (track->mod * wave) >> 6;
                    if ((u8)(track->modM ^ wave)) {
                        track->modM = (s8)wave;
                        track->flags |= track->modT == 0 ? MPT_FLG_PITCHG : MPT_FLG_VOLCHG;
                    }
                }
            }
        next_track:;
        }

        mp->clock++;
        if (!active) {
            mp->status = MUSICPLAYER_STATUS_PAUSE;
            goto done;
        }
        mp->status = active;
        mp->tempoC -= 150;
    }

    /* Push volume / pitch changes to the channels. */
    {
        struct MusicPlayerTrack *track = mp->tracks;
        for (int i = mp->trackCount; i > 0; i--, track++) {
            if (!(track->flags & MPT_FLG_EXIST) || !(track->flags & (MPT_FLG_VOLCHG | MPT_FLG_PITCHG)))
                continue;
            TrkVolPitSet(mp, track);
            for (struct SoundChannel *chan = track->chan; chan; chan = CHAN_NEXT(chan)) {
                u32 cgb;
                if (!(chan->statusFlags & SOUND_CHANNEL_SF_ON)) {
                    ClearChain(chan);
                    continue;
                }
                cgb = chan->type & TONEDATA_TYPE_CGB;
                if (track->flags & MPT_FLG_VOLCHG) {
                    chn_vol_set(chan, track);
                    if (cgb)
                        ((struct CgbChannel *)chan)->modify |= CGB_CHANNEL_MO_VOL;
                }
                if (track->flags & MPT_FLG_PITCHG) {
                    s32 k = (s32)chan->key + track->keyM;
                    if (k < 0) k = 0;
                    if (cgb) {
                        ((struct CgbChannel *)chan)->frequency = si->MidiKeyToCgbFreq(cgb, (u8)k, track->pitM);
                        ((struct CgbChannel *)chan)->modify |= CGB_CHANNEL_MO_PIT;
                    } else {
                        chan->frequency = MidiKeyToFreq(chan->wav, (u8)k, track->pitM);
                    }
                }
            }
            track->flags &= 0xF0;
        }
    }

done:
    mp->ident = ID_NUMBER;
}

/* ---- per-VBlank sound update ----------------------------------------------------------- */

/* The PCM DMA bookkeeping the game reads back (gPcmDmaCounter). */
void m4aSoundVSync(void)
{
    struct SoundInfo *si = SOUND_INFO_PTR;

    if (si->ident - ID_NUMBER > 1)
        return;
    int counter = si->pcmDmaCounter - 1;
    si->pcmDmaCounter = (u8)counter;
    if (counter <= 0)
        si->pcmDmaCounter = si->pcmDmaPeriod;
}

/* One envelope step for a Direct Sound channel, exactly as the engine does
 * it each VBlank. Returns 0 if the channel is (now) silent. */
static int envelope_step(struct SoundInfo *si, struct SoundChannel *chan)
{
    u8 st = chan->statusFlags;
    u32 env = chan->envelopeVolume;
    const struct WaveData *wav = chan->wav;

    if (!(st & SOUND_CHANNEL_SF_ON))
        return 0;

    if (st & SOUND_CHANNEL_SF_START) {
        if (st & SOUND_CHANNEL_SF_STOP) {
            chan->statusFlags = 0;
            return 0;
        }
        st = SOUND_CHANNEL_SF_ENV_ATTACK;
        chan->statusFlags = st;
        chan->currentPointer = (s8 *)wav->data + chan->count;
        chan->count = wav->size - chan->count;
        env = 0;
        chan->envelopeVolume = 0;
        chan->fw = 0;
        if (wav->status & 0xC000) { /* WAVE_DATA_FLAG_LOOP */
            st |= SOUND_CHANNEL_SF_LOOP;
            chan->statusFlags = st;
        }
        goto attack;
    }

    if (st & SOUND_CHANNEL_SF_IEC) { /* pseudo echo tail */
        if (chan->pseudoEchoLength-- <= 1) {
            chan->statusFlags = 0;
            return 0;
        }
        goto set_volume;
    }
    if (st & SOUND_CHANNEL_SF_STOP) { /* release */
        env = (env * chan->release) >> 8;
        if (env > chan->pseudoEchoVolume)
            goto set_volume;
    echo:
        env = chan->pseudoEchoVolume;
        if (env == 0) {
            chan->statusFlags = 0;
            return 0;
        }
        chan->statusFlags = st | SOUND_CHANNEL_SF_IEC;
        goto set_volume;
    }
    switch (st & SOUND_CHANNEL_SF_ENV) {
    case SOUND_CHANNEL_SF_ENV_DECAY:
        env = (env * chan->decay) >> 8;
        if (env > chan->sustain)
            goto set_volume;
        env = chan->sustain;
        if (env == 0)
            goto echo;
        chan->statusFlags = st - 1;
        goto set_volume;
    case SOUND_CHANNEL_SF_ENV_ATTACK:
    attack:
        env += chan->attack;
        if (env >= 0xFF) {
            env = 0xFF;
            chan->statusFlags = st - 1;
        }
        break;
    default:
        break;
    }

set_volume:
    chan->envelopeVolume = (u8)env;
    env = ((si->masterVolume + 1) * env) >> 4;
    chan->envelopeVolumeRight = (u8)((chan->rightVolume * env) >> 8);
    chan->envelopeVolumeLeft = (u8)((chan->leftVolume * env) >> 8);
    return 1;
}

void SoundMain(void)
{
    struct SoundInfo *si = SOUND_INFO_PTR;

    if (si->ident != ID_NUMBER)
        return;
    si->ident++; /* busy */

    if (si->MPlayMainHead)
        si->MPlayMainHead(si->musicPlayerHead);
    si->CgbSound();

    for (int i = 0; i < si->maxChans; i++) {
        struct SoundChannel *chan = &si->chans[i];
        if (envelope_step(si, chan))
            PcMixerChannelActive(i, chan);
    }
    PcMixerVBlank(si);

    si->ident = ID_NUMBER;
}

/* The console copies SoundMain's inner loop to IWRAM; nothing to do here. */
char SoundMainRAM[4];
