/* testing.c — headless test helpers: scripted input and audio capture.
 *
 * --input FILE: a key timeline. Each line "FRAME KEYS [HOLD]" presses KEYS
 *   (e.g. A, START, UP+B) at FRAME for HOLD frames (default 4). '#' comments.
 * --wav FILE: everything the mixer produces, as 16-bit stereo 48 kHz WAV. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pc_mixer.h"
#include "pc_port.h"

struct Press { int frame, hold; unsigned short keys; };

static struct Press *presses;
static int press_count;
static FILE *wav;
static unsigned wav_frames;

static unsigned short parse_keys(const char *s)
{
    static const struct { const char *name; unsigned short bit; } names[] = {
        {"A", 0x001}, {"B", 0x002}, {"SELECT", 0x004}, {"START", 0x008}, {"RIGHT", 0x010},
        {"LEFT", 0x020}, {"UP", 0x040}, {"DOWN", 0x080}, {"R", 0x100}, {"L", 0x200},
    };
    unsigned short keys = 0;
    char buf[128], *tok;

    snprintf(buf, sizeof buf, "%s", s);
    for (tok = strtok(buf, "+"); tok; tok = strtok(NULL, "+"))
        for (size_t i = 0; i < sizeof names / sizeof names[0]; i++)
            if (!strcmp(tok, names[i].name)) keys |= names[i].bit;
    return keys;
}

int PcTestLoadInput(const char *path)
{
    FILE *f = fopen(path, "r");
    char line[256];

    if (!f) return -1;
    while (fgets(line, sizeof line, f)) {
        char keys[128];
        int frame, hold = 4, n;
        char *hash = strchr(line, '#');
        if (hash) *hash = 0;
        n = sscanf(line, "%d %127s %d", &frame, keys, &hold);
        if (n < 2) continue;
        presses = realloc(presses, (press_count + 1) * sizeof *presses);
        presses[press_count++] = (struct Press){frame, hold, parse_keys(keys)};
    }
    fclose(f);
    return 0;
}

/* KEYINPUT for this frame (active-low). */
uint16_t PcTestInput(int frame)
{
    unsigned short held = 0;

    for (int i = 0; i < press_count; i++)
        if (frame >= presses[i].frame && frame < presses[i].frame + presses[i].hold)
            held |= presses[i].keys;
    return (uint16_t)(~held & 0x03FF);
}

static void put32(unsigned v) { fwrite(&v, 4, 1, wav); }
static void put16(unsigned short v) { fwrite(&v, 2, 1, wav); }

int PcTestOpenWav(const char *path)
{
    wav = fopen(path, "wb");
    if (!wav) return -1;
    fwrite("RIFF\0\0\0\0WAVEfmt ", 1, 16, wav);
    put32(16); put16(1); put16(2); put32(PC_AUDIO_RATE); put32(PC_AUDIO_RATE * 4); put16(4); put16(16);
    fwrite("data\0\0\0\0", 1, 8, wav);
    return 0;
}

void PcTestCaptureAudio(const float *stereo, int frames)
{
    if (!wav) return;
    for (int i = 0; i < frames * 2; i++) {
        float v = stereo[i] * 32767.0f;
        short s = (short)(v > 32767 ? 32767 : v < -32768 ? -32768 : v);
        fwrite(&s, 2, 1, wav);
    }
    wav_frames += frames;
}

void PcTestCloseWav(void)
{
    if (!wav) return;
    fseek(wav, 4, SEEK_SET);
    put32(36 + wav_frames * 4);
    fseek(wav, 40, SEEK_SET);
    put32(wav_frames * 4);
    fclose(wav);
    wav = NULL;
}
