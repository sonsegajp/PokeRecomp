/* audio.c — the SDL audio device, fed from a ring buffer.
 *
 * The game produces one frame of samples per video frame. The device pulls
 * at its own clock, so the read rate is nudged (+-0.5%) to keep the buffer
 * near its target fill: no drift, no crackle, ~40 ms of latency. */
#include <SDL.h>
#include <string.h>

#include "pc_mixer.h"
#include "pc_port.h"

#define RING_FRAMES 16384           /* stereo frames */
#define TARGET_FILL 1920            /* ~40 ms */

static SDL_AudioDeviceID device;
static float ring[RING_FRAMES * 2];
static volatile unsigned write_pos, read_pos;   /* in stereo frames, wrapping */
static double read_frac;

static unsigned fill(void) { return (write_pos - read_pos) % RING_FRAMES; }

static void SDLCALL callback(void *user, Uint8 *stream, int len)
{
    float *dst = (float *)stream;
    int frames = len / (int)(2 * sizeof(float));
    unsigned avail = fill();
    double rate = 1.0 + ((double)avail - TARGET_FILL) / TARGET_FILL * 0.005;

    (void)user;
    if (rate < 0.995) rate = 0.995;
    if (rate > 1.005) rate = 1.005;
    for (int i = 0; i < frames; i++) {
        if (fill() < 2) { /* underrun: hold silence */
            dst[i * 2] = dst[i * 2 + 1] = 0.0f;
            continue;
        }
        unsigned a = read_pos % RING_FRAMES, b = (read_pos + 1) % RING_FRAMES;
        float t = (float)read_frac;
        dst[i * 2] = ring[a * 2] + (ring[b * 2] - ring[a * 2]) * t;
        dst[i * 2 + 1] = ring[a * 2 + 1] + (ring[b * 2 + 1] - ring[a * 2 + 1]) * t;
        read_frac += rate;
        while (read_frac >= 1.0) {
            read_frac -= 1.0;
            read_pos++;
        }
    }
}

void PcAudioInit(void)
{
    SDL_AudioSpec want = {0}, have;

    want.freq = PC_AUDIO_RATE;
    want.format = AUDIO_F32SYS;
    want.channels = 2;
    want.samples = 512;
    want.callback = callback;
    device = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);
    if (device)
        SDL_PauseAudioDevice(device, 0);
}

void PcAudioPush(const float *stereo, int frames)
{
    if (!device)
        return;
    SDL_LockAudioDevice(device);
    for (int i = 0; i < frames; i++) {
        if (fill() >= RING_FRAMES - 1) break; /* overrun: drop */
        unsigned w = write_pos % RING_FRAMES;
        ring[w * 2] = stereo[i * 2];
        ring[w * 2 + 1] = stereo[i * 2 + 1];
        write_pos++;
    }
    SDL_UnlockAudioDevice(device);
}

void PcAudioFrame(void) {}
