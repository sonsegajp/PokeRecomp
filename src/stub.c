#ifdef PORTABLE
#include "global.h"
#include "main.h"
#include "malloc.h"
#include "gba/m4a_internal.h"

#include <stdio.h>

extern struct SoundInfo gSoundInfo;
extern u32 MidiKeyToFreq(struct WaveData *wav, u8 key, u8 fineAdjust);
extern void TrkVolPitSet(struct MusicPlayerInfo *mplayInfo, struct MusicPlayerTrack *track);
extern void FadeOutBody(struct MusicPlayerInfo *mplayInfo);
extern u32 MidiKeyToCgbFreq(u8 chanNum, u8 key, u8 fineAdjust);


#define STUB_FUNC(func) func { puts("STUB: " #func); }
#define STUB_FUNC_BLOCK(func, block) func { block }
#define STUB_FUNC_QUIET(func) func {}

ALIGNED(4) u8 gHeap[HEAP_SIZE] = {0};

const char RomHeaderGameCode[GAME_CODE_LENGTH] = "BPRE";
const char RomHeaderSoftwareVersion = 0;

char SoundMainRAM[0x400] = {0};

s32 Div(s32 num, s32 denom)
{
    if (denom == 0)
        return 0;
    return num / denom;
}

STUB_FUNC_QUIET(void RegisterRamReset(u32 resetFlags))
void intr_main(void) {}

STUB_FUNC_QUIET(void GameCubeMultiBoot_Main(void))
STUB_FUNC(void GameCubeMultiBoot_ExecuteProgram(void))
STUB_FUNC(void GameCubeMultiBoot_Init(void))
STUB_FUNC(void GameCubeMultiBoot_HandleSerialInterrupt(void))
STUB_FUNC(void GameCubeMultiBoot_Quit(void))
STUB_FUNC_BLOCK(int MultiBoot(struct MultiBootParam *mp), return 0;)

u32 umul3232H32(u32 a, u32 b)
{
    u64 result = (u64)a * (u64)b;
    return (u32)(result >> 32);
}

// ============================================================
// SoundMain — core PCM mixer (ported from pokeemerald PC port)
// ============================================================

#define FLOAT_BUF_SIZE 2048

static float sMixerBuffer[FLOAT_BUF_SIZE * 2];
static float sChannelFinePos[12];

extern const s8 gDeltaEncodingTable[];
static s8 sBDPCMBlockBuffer[64];
static u32 sBDPCMBlockIndex = 0xFFFFFFFF;
static struct WaveData *sBDPCMBlockWav = NULL;

static s8 DecompressBDPCM(struct WaveData *wav, u32 sampleIndex)
{
    u32 blockOffset = sampleIndex >> 6;
    if (wav != sBDPCMBlockWav || blockOffset != sBDPCMBlockIndex)
    {
        sBDPCMBlockWav = wav;
        sBDPCMBlockIndex = blockOffset;
        u8 *blockPtr = (u8 *)wav->data + blockOffset * 0x21;
        s32 s;
        sBDPCMBlockBuffer[0] = s = (s8)*blockPtr++;
        sBDPCMBlockBuffer[1] = s += gDeltaEncodingTable[*blockPtr++ & 0xF];
        for (int i = 2; i < 64; i += 2)
        {
            u8 temp = *blockPtr++;
            sBDPCMBlockBuffer[i] = s += gDeltaEncodingTable[temp >> 4];
            sBDPCMBlockBuffer[i + 1] = s += gDeltaEncodingTable[temp & 0xF];
        }
    }
    return sBDPCMBlockBuffer[sampleIndex & 63];
}

static int TickEnvelopePort(struct SoundChannel *chan, struct WaveData *wav)
{
    u8 status = chan->statusFlags;
    if ((status & 0xC7) == 0)
        return 0;

    u8 env = 0;
    if ((status & 0x80) == 0)
    {
        env = chan->envelopeVolume;
        if (status & 4)
        {
            --chan->pseudoEchoLength;
            if (chan->pseudoEchoLength <= 0) { chan->statusFlags = 0; return 0; }
            return 1;
        }
        else if (status & 0x40)
        {
            chan->envelopeVolume = env * chan->release / 256U;
            u8 echoVol = chan->pseudoEchoVolume;
            if (chan->envelopeVolume > echoVol) return 1;
            if (echoVol == 0) { chan->statusFlags = 0; return 0; }
            chan->statusFlags |= 4;
            return 1;
        }

        switch (status & 3)
        {
        u16 newEnv;
        case 2:
            chan->envelopeVolume = env * chan->decay / 256U;
            if (chan->envelopeVolume <= chan->sustain && chan->sustain == 0)
            {
                if (chan->pseudoEchoVolume == 0) { chan->statusFlags = 0; return 0; }
                chan->statusFlags |= 4; return 1;
            }
            else if (chan->envelopeVolume <= chan->sustain)
            {
                chan->envelopeVolume = chan->sustain;
                --chan->statusFlags;
            }
            break;
        case 3:
        attack:
            newEnv = env + chan->attack;
            if (newEnv > 0xFF) { chan->envelopeVolume = 0xFF; --chan->statusFlags; }
            else chan->envelopeVolume = newEnv;
            break;
        default:
            break;
        }
        return 1;
    }
    else if (status & 0x40)
    {
        chan->statusFlags = 0;
        return 0;
    }
    else
    {
        chan->statusFlags = 3;
        chan->currentPointer = wav->data + chan->count;
        chan->count = wav->size - chan->count;
        chan->fw = 0;
        chan->envelopeVolume = 0;
        {
            int chIdx = ((u8 *)chan - (u8 *)gSoundInfo.chans) / sizeof(struct SoundChannel);
            if (chIdx >= 0 && chIdx < 12) sChannelFinePos[chIdx] = 0.0f;
        }
        if (wav->status & 0xC000)
            chan->statusFlags |= 0x10;
        goto attack;
    }
}

float *SoundMainGetMixBuffer(void) { return sMixerBuffer; }
int SoundMainGetSamplesPerFrame(void) { return gSoundInfo.pcmSamplesPerVBlank; }

void SoundMain(void)
{
    struct SoundInfo *si = &gSoundInfo;
    if (si->ident != ID_NUMBER && si->ident != ID_NUMBER + 1)
        return;

    {
        extern const struct MusicPlayer gMPlayTable[];
        extern struct MusicPlayerInfo gPokemonCryMusicPlayers[];
        for (int i = 0; i < 4; i++)
        {
            struct MusicPlayerInfo *mp = gMPlayTable[i].info;
            if (mp != NULL && mp->ident == ID_NUMBER)
                MPlayMain(mp);
        }
        for (int i = 0; i < MAX_POKEMON_CRIES; i++)
        {
            struct MusicPlayerInfo *mp = &gPokemonCryMusicPlayers[i];
            if (mp->ident == ID_NUMBER)
                MPlayMain(mp);
        }
    }

    if (si->CgbSound != NULL)
        si->CgbSound();

    s32 nSamples = si->pcmSamplesPerVBlank;
    if (nSamples <= 0 || nSamples > FLOAT_BUF_SIZE)
    {
        si->ident = ID_NUMBER;
        return;
    }

    float *outBuf = sMixerBuffer;
    for (s32 i = 0; i < nSamples * 2; i++)
        outBuf[i] = 0.0f;

    float sampleRateReciprocal = 1.0f / (float)si->pcmFreq;

    struct SoundChannel *chan = si->chans;
    for (u8 ch = 0; ch < si->maxChans; ch++, chan++)
    {
        struct WaveData *wav = chan->wav;
        if (!TickEnvelopePort(chan, wav))
            continue;

        u8 v = chan->envelopeVolume * (si->masterVolume + 1) / 16U;
        s32 envR = chan->rightVolume * v / 256U;
        s32 envL = chan->leftVolume * v / 256U;

        if (wav == NULL || (u32)wav < 0x10000)
        {
            chan->statusFlags = 0;
            continue;
        }

        s32 loopLen = 0;
        s8 *loopStart = NULL;
        if (chan->statusFlags & 0x10)
        {
            loopStart = wav->data + wav->loopStart;
            loopLen = wav->size - wav->loopStart;
        }

        s32 samplesLeft = chan->count;
        s8 *current = chan->currentPointer;
        if (current == NULL) { chan->statusFlags = 0; continue; }

        if (wav->type != 0 && (chan->type & 0x20))
        {
            int chIdx = ch;
            float finePos = sChannelFinePos[chIdx];
            float romSamplesPerOut = (float)chan->frequency * sampleRateReciprocal;
            u32 samplePos = (u32)(current - wav->data);

            s16 b = DecompressBDPCM(wav, samplePos);
            s16 m = DecompressBDPCM(wav, samplePos + 1) - b;

            for (u16 i = 0; i < nSamples; i++)
            {
                float sample = (finePos * m) + b;
                outBuf[i * 2 + 1] += (sample * envR) / 32768.0f;
                outBuf[i * 2 + 0] += (sample * envL) / 32768.0f;

                finePos += romSamplesPerOut;
                u32 newCoarsePos = (u32)finePos;
                if (newCoarsePos != 0)
                {
                    finePos -= (int)finePos;
                    samplesLeft -= newCoarsePos;
                    samplePos += newCoarsePos;
                    if (samplesLeft <= 0)
                    {
                        chan->statusFlags = 0;
                        break;
                    }
                    else
                    {
                        b = DecompressBDPCM(wav, samplePos);
                        m = DecompressBDPCM(wav, samplePos + 1) - b;
                    }
                }
            }
            sChannelFinePos[chIdx] = finePos;
            chan->count = samplesLeft;
            chan->currentPointer = wav->data + samplePos;
        }
        else
        {
            int chIdx = ch;
            float finePos = sChannelFinePos[chIdx];
            float romSamplesPerOut = (float)chan->frequency * sampleRateReciprocal;

            s16 b = current[0];
            s16 m = current[1] - b;
            current += 1;

            for (u16 i = 0; i < nSamples; i++)
            {
                float sample = (finePos * m) + b;
                outBuf[i * 2 + 1] += (sample * envR) / 32768.0f;
                outBuf[i * 2 + 0] += (sample * envL) / 32768.0f;

                finePos += romSamplesPerOut;
                u32 newCoarsePos = (u32)finePos;
                if (newCoarsePos != 0)
                {
                    finePos -= (int)finePos;
                    samplesLeft -= newCoarsePos;
                    if (samplesLeft <= 0)
                    {
                        if (loopLen != 0)
                        {
                            current = loopStart;
                            u32 overshoot = -samplesLeft;
                            samplesLeft += loopLen;
                            while (samplesLeft <= 0)
                            {
                                overshoot -= loopLen;
                                samplesLeft += loopLen;
                            }
                            b = current[overshoot];
                            m = current[overshoot + 1] - b;
                            current += overshoot + 1;
                        }
                        else
                        {
                            chan->statusFlags = 0;
                            break;
                        }
                    }
                    else
                    {
                        b = current[newCoarsePos - 1];
                        m = current[newCoarsePos] - b;
                        current += newCoarsePos;
                    }
                }
            }
            sChannelFinePos[chIdx] = finePos;
            chan->count = samplesLeft;
            chan->currentPointer = current - 1;
        }
    }

    si->ident = ID_NUMBER;
}

void SoundMainBTM(void) {}

// MPlayMain, ply_note, TrackStop, and all ply_* event handlers
// are now in music_player_port.c (ported from pokeemerald PC port)

void MPlayJumpTableCopy(MPlayFunc *mplayJumpTable)
{
    extern void * const gMPlayJumpTableTemplate[];
    extern MPlayFunc gMPlayJumpTable[];
    int i;
    for (i = 0; i < 36; i++)
        gMPlayJumpTable[i] = gMPlayJumpTableTemplate[i];
}

void RealClearChain(void *x)
{
    if (x != NULL)
    {
        struct SoundChannel *chan = (struct SoundChannel *)x;
        chan->statusFlags = 0;
        chan->track = NULL;
    }
}

void m4aSoundVSync(void)
{
    if (gSoundInfo.ident >= ID_NUMBER && gSoundInfo.ident <= ID_NUMBER + 1)
    {
        gSoundInfo.pcmDmaCounter--;
        if (gSoundInfo.pcmDmaCounter <= 0)
            gSoundInfo.pcmDmaCounter = gSoundInfo.pcmDmaPeriod;
    }
}

void AGBAssert(const char *pFile, int nLine, const char *pExpression, int nStopProgram)
{
    printf("ASSERT FAILED: %s:%d: %s\n", pFile, nLine, pExpression);
}

#endif // PORTABLE
