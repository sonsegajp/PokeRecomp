/* mixer.c — turns the sound engine's channels into 48 kHz stereo.
 *
 * Direct Sound (sampled instruments, cries): the console mixes them at
 * ~13 kHz into an 8-bit buffer; we interpolate straight from the source
 * samples at 48 kHz in floating point, which removes most of the console's
 * aliasing and quantisation noise while keeping its timing — channels start,
 * loop and finish on the same schedule, so the engine sees the same state.
 * The engine's reverb is the same feedback echo (one frame and one DMA
 * buffer back), applied to the Direct Sound mix.
 *
 * PSG (the four Game Boy channels): synthesised from the sound registers the
 * engine's CgbSound driver writes each frame — duty squares with sweep and
 * envelopes, the 4-bit wave channel, and the LFSR noise channel. */
#include <math.h>
#include <string.h>

#include "global.h"
#include "gba/m4a_internal.h"
#include "pc_mixer.h"
#include "pc_hw.h"

#define FRAME_RATE 59.7275005696
#define MAX_FRAME_SAMPLES 1024

extern const s8 gDeltaEncodingTable[];

static float ds_left[MAX_FRAME_SAMPLES], ds_right[MAX_FRAME_SAMPLES];
static float out[MAX_FRAME_SAMPLES * 2];
static double sample_debt;

/* ---- Direct Sound ------------------------------------------------------------- */

/* Compressed (DPCM) waves: 64 samples per 33-byte block. */
static s8 dpcm_sample(const struct WaveData *wav, u32 index)
{
    const u8 *block = (const u8 *)wav->data + (index >> 6) * 0x21;
    u32 n = index & 63;
    s8 v = (s8)block[0];

    for (u32 i = 1; i <= n; i++) {
        u8 b = block[1 + ((i - 1) >> 1)];
        v += gDeltaEncodingTable[((i - 1) & 1) ? (b >> 4) : (b & 0xF)];
    }
    return v;
}

/* Sample `pos` of the channel's wave (forward order). */
static inline float wave_sample(const struct WaveData *wav, int compressed, s32 pos)
{
    if (pos < 0) pos = 0;
    if ((u32)pos >= wav->size) pos = wav->size - 1;
    return compressed ? dpcm_sample(wav, pos) : wav->data[pos];
}

/* Mix one channel for `n` output samples. Positions are tracked as the
 * engine does: `count` samples left, `fw` the 23-bit fraction. */
static void mix_channel(struct SoundInfo *si, struct SoundChannel *chan, int n)
{
    const struct WaveData *wav = chan->wav;
    int compressed = wav->type != 0;
    int reverse = (chan->type & 0x10) != 0;          /* TONEDATA_TYPE_REV */
    int looping = (chan->statusFlags & SOUND_CHANNEL_SF_LOOP) != 0;
    float vol_r = chan->envelopeVolumeRight / 256.0f, vol_l = chan->envelopeVolumeLeft / 256.0f;
    double step;
    double frac = chan->fw / 8388608.0;
    s32 pos;   /* index of the current sample in forward order */

    if (chan->type & TONEDATA_TYPE_FIX)
        step = (double)si->pcmFreq / PC_AUDIO_RATE;
    else
        step = (double)chan->frequency * si->divFreq * si->pcmFreq / (8388608.0 * PC_AUDIO_RATE);

    /* Forward: currentPointer is the next sample. Reverse: playback starts at
     * the last sample and `count` samples remain, so the position is count-1. */
    pos = reverse ? (s32)chan->count - 1 : (s32)(chan->currentPointer - (s8 *)wav->data);

    for (int i = 0; i < n; i++) {
        float a, b, s;
        if (reverse) {
            a = wave_sample(wav, compressed, pos);
            b = wave_sample(wav, compressed, pos - 1);
        } else {
            a = wave_sample(wav, compressed, pos);
            b = wave_sample(wav, compressed, pos + 1);
        }
        s = a + (b - a) * (float)frac;
        ds_left[i] += s * vol_l;
        ds_right[i] += s * vol_r;

        frac += step;
        if (frac >= 1.0) {
            u32 adv = (u32)frac;
            frac -= adv;
            if (chan->count > adv) {
                chan->count -= adv;
                pos += reverse ? -(s32)adv : (s32)adv;
            } else if (looping && !reverse) {
                /* wrap into the loop */
                u32 loop_len = wav->size - wav->loopStart;
                u32 over = adv - chan->count;
                if (loop_len == 0) { chan->statusFlags = 0; break; }
                over %= loop_len;
                pos = (s32)(wav->loopStart + over);
                chan->count = loop_len - over;
            } else {
                chan->statusFlags = 0;   /* sample finished: channel frees itself */
                break;
            }
        }
    }

    chan->fw = (u32)(frac * 8388608.0);
    if (!reverse)
        chan->currentPointer = (s8 *)wav->data + pos;
}

/* The engine's echo: each frame starts from (L+R) of one frame ago plus
 * (L+R) of one DMA buffer ago, scaled by reverb/512. */
#define REVERB_HISTORY (MAX_FRAME_SAMPLES * 16)
static float rev_hist[REVERB_HISTORY];
static int rev_pos;

static void apply_reverb(struct SoundInfo *si, int n)
{
    int frame_len = (int)(PC_AUDIO_RATE / FRAME_RATE);
    int period = frame_len * (si->pcmDmaPeriod ? si->pcmDmaPeriod : 7);
    float gain = si->reverb / 512.0f;

    if (period >= REVERB_HISTORY) period = REVERB_HISTORY - 1;
    for (int i = 0; i < n; i++) {
        int p = rev_pos;
        float echo = 0.0f;
        if (si->reverb) {
            float a = rev_hist[(p - frame_len + REVERB_HISTORY) % REVERB_HISTORY];
            float b = rev_hist[(p - period + REVERB_HISTORY) % REVERB_HISTORY];
            echo = (a + b) * gain;
        }
        ds_left[i] += echo;
        ds_right[i] += echo;
        rev_hist[p] = ds_left[i] + ds_right[i];
        rev_pos = (p + 1) % REVERB_HISTORY;
    }
}

/* ---- PSG ------------------------------------------------------------------------- */

#define IO8(off) gPcIo[off]

struct Psg {
    int on;
    double phase;          /* 0..1 */
    int volume;            /* 0-15 */
    int env_dir, env_period, env_timer;
    int length, length_on;
    int sweep_period, sweep_dir, sweep_shift, sweep_timer;
    int freq;              /* 11-bit */
    u16 lfsr;
    u8 wave[32];
};

static struct Psg psg[4];
static const float duty_table[4] = {0.125f, 0.25f, 0.5f, 0.75f};

static void psg_trigger(int ch)
{
    static const int nrx2[4] = {0x63, 0x69, 0x73, 0x79};
    struct Psg *p = &psg[ch];
    u8 env = IO8(nrx2[ch]);

    p->on = 1;
    p->phase = 0;
    if (ch == 2) {
        for (int i = 0; i < 16; i++) {
            p->wave[i * 2] = IO8(0x90 + i) >> 4;
            p->wave[i * 2 + 1] = IO8(0x90 + i) & 0xF;
        }
        p->on = (IO8(0x70) & 0x80) != 0;
    } else {
        p->volume = env >> 4;
        p->env_dir = (env & 8) ? 1 : -1;
        p->env_period = env & 7;
        p->env_timer = p->env_period;
        if ((env & 0xF8) == 0) p->on = 0; /* DAC off */
    }
    if (ch == 3) p->lfsr = 0x7FFF;
    if (ch == 0) {
        u8 sw = IO8(0x60);
        p->sweep_period = (sw >> 4) & 7;
        p->sweep_dir = (sw & 8) ? -1 : 1;
        p->sweep_shift = sw & 7;
        p->sweep_timer = p->sweep_period;
    }
}

/* Called once per frame: pick up triggers and register changes. */
static void psg_update_registers(void)
{
    static const int nrx4[4] = {0x65, 0x6D, 0x75, 0x7D};
    static const int nrx1[4] = {0x62, 0x68, 0x72, 0x78};

    for (int ch = 0; ch < 4; ch++) {
        u8 hi = IO8(nrx4[ch]);
        if (hi & 0x80) {
            u8 len = IO8(nrx1[ch]);
            psg[ch].length = ch == 2 ? 256 - len : 64 - (len & 0x3F);
            psg_trigger(ch);
            IO8(nrx4[ch]) = hi & 0x7F; /* the trigger bit reads back as 0 */
        }
        psg[ch].length_on = (hi & 0x40) != 0;
        if (ch < 3)
            psg[ch].freq = IO8(nrx4[ch] - 1) | ((hi & 7) << 8);
    }
    if (!(IO8(0x70) & 0x80)) psg[2].on = 0;
}

/* 256 Hz length, 128 Hz sweep, 64 Hz envelope clocks (frame sequencer). */
static void psg_clock(int step)
{
    for (int ch = 0; ch < 4; ch++) {
        struct Psg *p = &psg[ch];
        if (!p->on) continue;
        if ((step & 1) == 0 && p->length_on && p->length > 0 && --p->length == 0)
            p->on = 0;
        if (step == 7 && ch != 2 && p->env_period) {
            if (--p->env_timer <= 0) {
                p->env_timer = p->env_period;
                int v = p->volume + p->env_dir;
                if (v >= 0 && v <= 15) p->volume = v;
            }
        }
    }
    if ((step == 2 || step == 6) && psg[0].on && psg[0].sweep_period && psg[0].sweep_shift) {
        if (--psg[0].sweep_timer <= 0) {
            psg[0].sweep_timer = psg[0].sweep_period;
            int f = psg[0].freq + psg[0].sweep_dir * (psg[0].freq >> psg[0].sweep_shift);
            if (f > 2047) psg[0].on = 0;
            else if (f >= 0) psg[0].freq = f;
        }
    }
}

static double seq_phase;
static int seq_step;

/* Mix the PSG into out[] (already holding Direct Sound), in DAC units. */
static void mix_psg(int n, float *left, float *right)
{
    u8 nr50 = IO8(0x80), nr51 = IO8(0x81);
    u16 cnt_h = *(u16 *)&gPcIo[0x82];
    float ratio = (cnt_h & 3) == 0 ? 0.25f : (cnt_h & 3) == 1 ? 0.5f : 1.0f;
    float vol_r = ((nr50 & 7) + 1) * ratio, vol_l = (((nr50 >> 4) & 7) + 1) * ratio;

    if (!(IO8(0x84) & 0x80))
        return;
    for (int i = 0; i < n; i++) {
        float l = 0, r = 0;

        seq_phase += 512.0 / PC_AUDIO_RATE;
        while (seq_phase >= 1.0) {
            seq_phase -= 1.0;
            psg_clock(seq_step);
            seq_step = (seq_step + 1) & 7;
        }

        for (int ch = 0; ch < 4; ch++) {
            struct Psg *p = &psg[ch];
            float v = 0;
            if (!p->on) continue;
            if (ch < 2) {
                double hz = 131072.0 / (2048 - p->freq);
                int duty = IO8(ch == 0 ? 0x62 : 0x68) >> 6;
                /* 4x oversampled to soften aliasing */
                float acc = 0;
                for (int k = 0; k < 4; k++) {
                    p->phase += hz / (PC_AUDIO_RATE * 4.0);
                    p->phase -= floor(p->phase);
                    acc += p->phase < duty_table[duty] ? 1.0f : -1.0f;
                }
                v = acc * 0.25f * p->volume;
            } else if (ch == 2) {
                double hz = 2097152.0 / (2048 - p->freq);
                int shift = (IO8(0x73) >> 5) & 3;
                float level;
                p->phase += hz / PC_AUDIO_RATE / 32.0;
                p->phase -= floor(p->phase);
                level = (p->wave[(int)(p->phase * 32) & 31] - 7.5f) * 2.0f;
                v = IO8(0x73) & 0x80 ? level * 0.75f : shift == 0 ? 0 : level / (1 << (shift - 1));
            } else {
                u8 poly = IO8(0x7C);
                int div = poly & 7, s = poly >> 4;
                double hz = 524288.0 / (div ? div : 0.5) / (1 << (s + 1));
                p->phase += hz / PC_AUDIO_RATE;
                while (p->phase >= 1.0) {
                    p->phase -= 1.0;
                    int bit = (p->lfsr ^ (p->lfsr >> 1)) & 1;
                    p->lfsr = (u16)((p->lfsr >> 1) | (bit << 14));
                    if (poly & 8) p->lfsr = (u16)((p->lfsr & ~0x40) | (bit << 6));
                }
                v = (p->lfsr & 1) ? -(float)p->volume : (float)p->volume;
            }
            if (nr51 & (1 << ch)) r += v;
            if (nr51 & (0x10 << ch)) l += v;
        }
        left[i] += l * vol_l;
        right[i] += r * vol_r;
    }
}

/* ---- the frame --------------------------------------------------------------------- */

void PcMixerVBlank(struct SoundInfo *si)
{
    u16 cnt_h = *(u16 *)&gPcIo[0x82];
    /* The engine feeds the right mix to FIFO A, the left to FIFO B; each at 100% or 50%. */
    float gain_r = (cnt_h & 4) ? 4.0f : 2.0f, gain_l = (cnt_h & 8) ? 4.0f : 2.0f;
    float psg_l[MAX_FRAME_SAMPLES], psg_r[MAX_FRAME_SAMPLES];
    int n;

    sample_debt += PC_AUDIO_RATE / FRAME_RATE;
    n = (int)sample_debt;
    sample_debt -= n;
    if (n > MAX_FRAME_SAMPLES) n = MAX_FRAME_SAMPLES;

    memset(ds_left, 0, n * sizeof(float));
    memset(ds_right, 0, n * sizeof(float));
    for (int i = 0; i < si->maxChans; i++) {
        struct SoundChannel *chan = &si->chans[i];
        if ((chan->statusFlags & SOUND_CHANNEL_SF_ON) && chan->wav)
            mix_channel(si, chan, n);
    }
    apply_reverb(si, n);

    memset(psg_l, 0, n * sizeof(float));
    memset(psg_r, 0, n * sizeof(float));
    psg_update_registers();
    mix_psg(n, psg_l, psg_r);

    /* Both paths in the GBA's 10-bit DAC units (+-512), then soft-clip. */
    for (int i = 0; i < n; i++) {
        float l = ds_left[i] * gain_l + psg_l[i];
        float r = ds_right[i] * gain_r + psg_r[i];
        l /= 512.0f;
        r /= 512.0f;
        out[i * 2] = l > 1.0f || l < -1.0f ? tanhf(l) : l;
        out[i * 2 + 1] = r > 1.0f || r < -1.0f ? tanhf(r) : r;
    }
    PcAudioPush(out, n);
}
