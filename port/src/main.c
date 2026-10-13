/* main.c — host entry point and the frame loop.
 *
 * The game's AgbMain() never returns: it loops forever, waiting for VBlank.
 * It runs on its own fiber; every wait for an interrupt switches back to the
 * host fiber, which produces one video frame (scanline by scanline,
 * delivering HBlank/VCount interrupts and HBlank DMA between lines), then
 * delivers VBlank, presents, reads input, paces to 59.73 Hz and resumes the
 * game. Interrupt handlers run on the host fiber, like an interrupt would. */
#include <SDL.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

#include "pc_hw.h"
#include "pc_port.h"
#include "pc_settings.h"
#include "pc_net.h"
#include "pc_chat.h"

#define REG_DISPSTAT 0x04
#define REG_VCOUNT   0x06
#define REG_KEYINPUT 0x130
#define REG_IE       0x200
#define REG_IF       0x202
#define REG_IME      0x208

#define INTR_VBLANK 0x0001
#define INTR_HBLANK 0x0002
#define INTR_VCOUNT 0x0004

#define FRAME_SECONDS (280896.0 / 16777216.0)

typedef void (*IntrFunc)(void);
extern IntrFunc gIntrTable[];   /* game: main.c */
void AgbMain(void);

static void *host_fiber, *game_fiber;
static int quit_requested;
static uint32_t frame[GBA_WIDTH * GBA_HEIGHT * 4];
static int frame_width = GBA_WIDTH;

/* Hang watchdog: if no frame completes for 5 s, sample where the game thread
 * is and append it to hang.log (exe-relative addresses; map with nm). Only
 * logs — a dragged window or a breakpoint can trip it harmlessly. */
static volatile LONG frames_done;
static HANDLE main_thread;

static DWORD WINAPI watchdog(void *arg)
{
    LONG last = -1;
    int still = 0;
    uintptr_t base = (uintptr_t)GetModuleHandleA(NULL);

    (void)arg;
    for (;;) {
        Sleep(1000);
        if (frames_done != last) { last = frames_done; still = 0; continue; }
        if (++still != 5) continue;
        FILE *f = fopen("hang.log", "a");
        if (!f) continue;
        fprintf(f, "no frame for 5 s after frame %ld; game thread at:\n", last);
        for (int i = 0; i < 8; i++) {
            CONTEXT c;
            c.ContextFlags = CONTEXT_CONTROL;
            SuspendThread(main_thread);
            if (GetThreadContext(main_thread, &c)) {
                fprintf(f, "  rva=%08lx", (unsigned long)(c.Eip - base));
                /* return addresses near the top of the stack (some will be stale) */
                const DWORD *sp = (const DWORD *)c.Esp;
                for (int k = 0; k < 64; k++)
                    if (sp[k] - base < 0x2000000 && sp[k] > base)
                        fprintf(f, " %08lx", (unsigned long)(sp[k] - base));
                fprintf(f, "\n");
            }
            ResumeThread(main_thread);
            Sleep(50);
        }
        fclose(f);
    }
    return 0;
}

uint16_t PcIoRead16(uint32_t off) { return *(uint16_t *)&gPcIo[off]; }
void PcIoWrite16(uint32_t off, uint16_t v) { *(uint16_t *)&gPcIo[off] = v; }

void PcFatal(const char *fmt, ...)
{
    char msg[1024];
    va_list ap;

    va_start(ap, fmt);
    vsnprintf(msg, sizeof msg, fmt, ap);
    va_end(ap);
    fprintf(stderr, "fatal: %s\n", msg);
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "FireRed PC", msg, NULL);
    exit(1);
}

/* IntrMain's dispatch order: index into gIntrTable for each IF bit. */
static const int intr_index[14] = {4, 3, 0, 5, 6, 7, 2, 1, 8, 9, 10, 11, 12, 13};

void PcRaiseInterrupt(uint16_t flag)
{
    PcIoWrite16(REG_IF, PcIoRead16(REG_IF) | flag);
    if (!(PcIoRead16(REG_IME) & 1) || !(PcIoRead16(REG_IE) & flag))
        return;
    for (int bit = 0; bit < 14; bit++) {
        if (flag & (1 << bit)) {
            IntrFunc f = gIntrTable[intr_index[bit]];
            if (f) f();
        }
    }
    PcIoWrite16(REG_IF, PcIoRead16(REG_IF) & ~flag);
}

void PcWaitForInterrupt(unsigned short mask)
{
    (void)mask;
    SwitchToFiber(host_fiber);
}

static int spin_count;
struct PcDebugWarp gPcDebugWarp;
struct PcDebugGiveMon gPcDebugGiveMon[6];
struct PcDebugGiveItem gPcDebugGiveItem;
struct PcDebugNet gPcDebugNet;

void PcSpinWait(void)
{
    if (++spin_count >= 256) {
        spin_count = 0;
        PcWaitForInterrupt(1); /* VBlank */
    }
}

static void CALLBACK game_entry(void *arg)
{
    (void)arg;
    AgbMain();
    PcFatal("AgbMain returned");
}

void PcRestartGame(void)
{
    char path[MAX_PATH];
    STARTUPINFOA si = {sizeof si};
    PROCESS_INFORMATION pi;

    PcSaveFlush();
    GetModuleFileNameA(NULL, path, sizeof path);
    if (CreateProcessA(path, GetCommandLineA(), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
    }
    exit(0);
}

/* After each visible line: HBlank interrupt + HBlank DMA, then the VCount
 * match for the next line. */
static void on_hblank(int line)
{
    uint16_t stat = PcIoRead16(REG_DISPSTAT);

    PcIoWrite16(REG_DISPSTAT, stat | 0x0002);
    if (stat & 0x0010)
        PcRaiseInterrupt(INTR_HBLANK);
    PcDmaOnHBlank();
    PcIoWrite16(REG_DISPSTAT, PcIoRead16(REG_DISPSTAT) & ~0x0002);

    int next = line + 1;
    PcIoWrite16(REG_VCOUNT, (uint16_t)next);
    stat = PcIoRead16(REG_DISPSTAT);
    if ((stat >> 8) == next) {
        PcIoWrite16(REG_DISPSTAT, stat | 0x0004);
        if (stat & 0x0020)
            PcRaiseInterrupt(INTR_VCOUNT);
    } else {
        PcIoWrite16(REG_DISPSTAT, stat & ~0x0004);
    }
}

static void run_vblank(void)
{
    uint16_t stat = PcIoRead16(REG_DISPSTAT) | 0x0001;

    PcIoWrite16(REG_VCOUNT, GBA_HEIGHT);
    PcIoWrite16(REG_DISPSTAT, stat);
    PcDmaOnVBlank();
    if (stat & 0x0008)
        PcRaiseInterrupt(INTR_VBLANK);

    /* The rest of VBlank: only VCount matches (the sound engine's timing). */
    for (int line = GBA_HEIGHT + 1; line < 228; line++) {
        stat = PcIoRead16(REG_DISPSTAT);
        PcIoWrite16(REG_VCOUNT, (uint16_t)line);
        if ((stat >> 8) == line && (stat & 0x0020))
            PcRaiseInterrupt(INTR_VCOUNT);
    }
    PcIoWrite16(REG_DISPSTAT, PcIoRead16(REG_DISPSTAT) & ~0x0001);
    PcIoWrite16(REG_VCOUNT, 0);
}

static void save_frame(const char *path)
{
    SDL_Surface *shot = SDL_CreateRGBSurfaceWithFormatFrom(frame, frame_width, GBA_HEIGHT, 32,
                                                           frame_width * 4, SDL_PIXELFORMAT_ABGR8888);
    if (shot) {
        SDL_SaveBMP(shot, path);
        SDL_FreeSurface(shot);
    }
}

int main(int argc, char *argv[])
{
    uint64_t freq, next_frame;
    int headless = 0, max_frames = 0;
    const char *dump_path = NULL;
    int scripted = 0, shot_every = 0, muted = 0;
    const char *gl_dump = NULL;
    const char *shot_dir = NULL;
    int test_width = 0;
    int paced = 0;

    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "--headless")) headless = 1;
        else if (!strcmp(argv[i], "--frames") && i + 1 < argc) max_frames = atoi(argv[++i]);
        else if (!strcmp(argv[i], "--dump") && i + 1 < argc) dump_path = argv[++i];
        else if (!strcmp(argv[i], "--input") && i + 1 < argc) {
            if (PcTestLoadInput(argv[++i]) == 0) scripted = 1;
        } else if (!strcmp(argv[i], "--wav") && i + 1 < argc) PcTestOpenWav(argv[++i]);
        else if (!strcmp(argv[i], "--mute")) muted = 1;
        else if (!strcmp(argv[i], "--gl-dump") && i + 1 < argc) gl_dump = argv[++i];
        else if (!strcmp(argv[i], "--warp") && i + 4 < argc) {
            gPcDebugWarp.group = atoi(argv[++i]);
            gPcDebugWarp.num = atoi(argv[++i]);
            gPcDebugWarp.x = atoi(argv[++i]);
            gPcDebugWarp.y = atoi(argv[++i]);
            gPcDebugWarp.pending = 1;
        }
        else if (!strcmp(argv[i], "--net-autojoin")) gPcDebugNet.autojoin = 1;
        else if (!strcmp(argv[i], "--net-accept")) gPcDebugNet.accept = 1;
        else if (!strcmp(argv[i], "--net-chat") && i + 1 < argc) snprintf(gPcDebugNet.chat, sizeof gPcDebugNet.chat, "%s", argv[++i]);
        else if (!strcmp(argv[i], "--net-request") && i + 1 < argc) gPcDebugNet.request_kind = atoi(argv[++i]);
        else if (!strcmp(argv[i], "--paced")) paced = 1; /* test: headless at real speed (online) */
        else if (!strcmp(argv[i], "--data-dir") && i + 1 < argc) {
            /* save, settings and online account live here (test: two clients) */
            CreateDirectoryA(argv[i + 1], NULL);
            SetCurrentDirectoryA(argv[++i]);
        }
        else if (!strcmp(argv[i], "--give-item") && i + 2 < argc) {
            gPcDebugGiveItem.item = atoi(argv[++i]);
            gPcDebugGiveItem.count = atoi(argv[++i]);
        }
        else if (!strcmp(argv[i], "--give-mon") && i + 2 < argc) {
            static int n;
            int species = atoi(argv[++i]), level = atoi(argv[++i]);
            if (n < 6) {
                gPcDebugGiveMon[n].species = species;
                gPcDebugGiveMon[n].level = level;
                n++;
            }
        }
        else if (!strcmp(argv[i], "--field-width") && i + 1 < argc) test_width = atoi(argv[++i]); /* test: widescreen field width */
        else if (!strcmp(argv[i], "--shots") && i + 2 < argc) {
            shot_every = atoi(argv[++i]);
            shot_dir = argv[++i];
        }
    }

    PcNullGuardInstall();
    PcSettingsLoad();
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER | SDL_INIT_EVENTS) != 0)
        PcFatal("SDL_Init: %s", SDL_GetError());
    if (!headless && PcVideoInit("FireRed PC") != 0)
        PcFatal("could not open the game window");
    /* headless runs are silent (use --wav to capture) */
    PcAudioInit(headless || muted);
    PcSaveLoad();
    PcIoWrite16(REG_KEYINPUT, 0x03FF);

    DuplicateHandle(GetCurrentProcess(), GetCurrentThread(), GetCurrentProcess(), &main_thread, 0, FALSE,
                    DUPLICATE_SAME_ACCESS);
    CreateThread(NULL, 0, watchdog, NULL, 0, NULL);
    host_fiber = ConvertThreadToFiber(NULL);
    game_fiber = CreateFiber(8 * 1024 * 1024, game_entry, NULL);
    if (!host_fiber || !game_fiber)
        PcFatal("could not create the game fiber");

    freq = SDL_GetPerformanceFrequency();
    next_frame = SDL_GetPerformanceCounter();
    for (int frames = 0; !quit_requested && (!max_frames || frames < max_frames); frames++) {
        SwitchToFiber(game_fiber);   /* game logic until it waits for VBlank */
        InterlockedIncrement(&frames_done);
        spin_count = 0;
        if (getenv("PC_TRACE_FRAMES") && frames % 60 == 0) fprintf(stderr, "frame %d\n", frames);

        /* Widescreen applies to the field; every other screen is the GBA's 240
         * pixels, pillarboxed. Field BGs 1-3 extend into the margins. */
        frame_width = gPcSettings.widescreen && PcFieldIsOnScreen() ? PcVideoDisplayWidth() : GBA_WIDTH;
        if (test_width && PcFieldIsOnScreen()) frame_width = test_width;
        PcRenderFrame(frame, frame_width, 0xE, on_hblank);
        if (shot_every && frames % shot_every == 0) {
            char path[512];
            snprintf(path, sizeof path, "%s/f%06d.bmp", shot_dir, frames);
            save_frame(path);
            snprintf(path, sizeof path, "%s/f%06d_chat.bmp", shot_dir, frames);
            PcChatDebugShot(frame, frame_width, GBA_HEIGHT, path);
        }
        run_vblank();
        PcAudioFrame();
        PcSaveFlush();
        PcNetPump();

        if (scripted)
            PcIoWrite16(REG_KEYINPUT, PcTestInput(frames + 1));
        if (!headless) {
            static int turbo, skip;
            /* While fast-forwarding, show only every Nth frame so vsync can't cap the speed. */
            if (!turbo || ++skip >= gPcSettings.fast_forward) {
                skip = 0;
                PcVideoPresent(frame, frame_width, GBA_HEIGHT);
            }
            uint16_t keys = PcInputPoll(&quit_requested, &turbo);
            if (!scripted) PcIoWrite16(REG_KEYINPUT, keys);

            if (gl_dump) {
                /* test capture: run unpaced */
            } else if (turbo || !PcVideoVsyncPaces()) {
                next_frame += (uint64_t)(FRAME_SECONDS * freq / (turbo ? gPcSettings.fast_forward : 1));
                uint64_t now = SDL_GetPerformanceCounter();
                if (now < next_frame) {
                    uint32_t ms = (uint32_t)((next_frame - now) * 1000 / freq);
                    if (ms > 1) SDL_Delay(ms - 1);
                    while (SDL_GetPerformanceCounter() < next_frame) {}
                } else if (now - next_frame > freq / 10) {
                    next_frame = now; /* fell far behind: don't try to catch up */
                }
            } else {
                next_frame = SDL_GetPerformanceCounter();
            }
        } else if (paced) {
            next_frame += (uint64_t)(FRAME_SECONDS * freq);
            while (SDL_GetPerformanceCounter() < next_frame)
                SDL_Delay(1);
            if (gl_dump && max_frames && frames == max_frames - 1)
                PcVideoDumpScreen(gl_dump);
        }
    }

    if (dump_path)
        save_frame(dump_path);
    PcTestCloseWav();
    PcSaveFlush();
    if (!headless) PcVideoShutdown();
    SDL_Quit();
    return 0;
}
