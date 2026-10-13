/* runner.c — PokeRecompGame.exe: runs a PokeRecomp game DLL in its own process.
 *
 *   PokeRecompGame.exe --game firered [game arguments...]
 *       load games/firered.dll at runtime and run it (the launcher starts
 *       this with --launcher-parent, --pkmn, --save-file, --data-dir ...)
 *   PokeRecompGame.exe --list
 *       print the installed games as JSON (one per games/*.dll)
 *   PokeRecompGame.exe --extract ROM [--out FILE.pkmn] [--game ID]
 *       ROM import: find the game the ROM belongs to and unpack it into its
 *       .pkmn (default: next to this program). Prints {"progress":N} lines
 *       as it goes, then the game as JSON; exit 0 on success.
 *
 * Travelling between regions: a game that exits with PC_EXIT_TRAVEL wrote
 * travel.dat in its data directory; the runner then starts the game named
 * there (a child runner with --chained, the same window arguments) with
 * --travel and the save in the file, and so on until a game exits otherwise.
 *
 * Games are found by name and loaded with LoadLibrary; nothing links them.
 * See docs/GAME_DLL.md. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <windows.h>
#include <shellapi.h>

#include "pc_travel.h"

struct PokeRecompGameInfo {
    uint32_t abi;
    const char *id, *title, *region;
    const wchar_t *pkmn;
};
typedef const struct PokeRecompGameInfo *(*InfoFn)(void);
typedef int (*ExtractFn)(const wchar_t *rom, const wchar_t *pkmn);
typedef int (*ExtractProgressFn)(const wchar_t *rom, const wchar_t *pkmn, void (*progress)(int, void *), void *ctx);
typedef int (*RunFn)(int argc, char **argv);
typedef int (*ExtractForeignFn)(const wchar_t *rom, const wchar_t *pkmn);

static wchar_t sDir[MAX_PATH];
static wchar_t sStartDir[MAX_PATH];   /* the games change directory to their data dir */

static HMODULE load_game(const wchar_t *id)
{
    wchar_t path[MAX_PATH];
    _snwprintf(path, MAX_PATH, L"%ls\\games\\%ls.dll", sDir, id);
    return LoadLibraryW(path);
}

static char *utf8(const wchar_t *w)
{
    int n = WideCharToMultiByte(CP_UTF8, 0, w, -1, NULL, 0, NULL, NULL);
    char *s = malloc(n > 0 ? n : 1);
    WideCharToMultiByte(CP_UTF8, 0, w, -1, s, n, NULL, NULL);
    return s;
}

static void json_string(const char *s)
{
    putchar('"');
    for (; *s; s++) {
        if (*s == '"' || *s == '\\') putchar('\\');
        putchar(*s);
    }
    putchar('"');
}

static void print_game(const struct PokeRecompGameInfo *info, const wchar_t *pkmn_path)
{
    char *pkmn = utf8(pkmn_path ? pkmn_path : info->pkmn);
    printf("{\"id\":"); json_string(info->id);
    printf(",\"title\":"); json_string(info->title);
    printf(",\"region\":"); json_string(info->region);
    printf(",\"pkmn\":"); json_string(pkmn);
    printf("}");
    free(pkmn);
}

/* Each installed game, in turn: id (from the file name) and its module. */
static int for_each_game(int (*fn)(const wchar_t *id, HMODULE m, void *ctx), void *ctx)
{
    wchar_t pattern[MAX_PATH];
    WIN32_FIND_DATAW fd;
    HANDLE h;
    int result = 0;

    _snwprintf(pattern, MAX_PATH, L"%ls\\games\\*.dll", sDir);
    if ((h = FindFirstFileW(pattern, &fd)) == INVALID_HANDLE_VALUE) return 0;
    do {
        wchar_t id[MAX_PATH], *dot;
        HMODULE m;
        wcsncpy(id, fd.cFileName, MAX_PATH - 1);
        id[MAX_PATH - 1] = 0;
        if ((dot = wcsrchr(id, L'.')) != NULL) *dot = 0;
        if ((m = load_game(id)) != NULL && GetProcAddress(m, "PokeRecomp_GameInfo"))
            result = fn(id, m, ctx);
    } while (!result && FindNextFileW(h, &fd));
    FindClose(h);
    return result;
}

static int list_one(const wchar_t *id, HMODULE m, void *ctx)
{
    int *first = ctx;
    if (!*first) putchar(',');
    *first = 0;
    print_game(((InfoFn)GetProcAddress(m, "PokeRecomp_GameInfo"))(), NULL);
    return 0;
}

struct ExtractJob { const wchar_t *rom, *out, *only; int code; };

/* {"progress":N} lines while a ROM is unpacked (the launcher's progress bar) */
static void print_progress(int percent, void *ctx)
{
    static int last = -1;
    if (percent == 0) last = -1;   /* a new attempt */
    (void)ctx;
    if (percent == last) return;
    last = percent;
    printf("{\"progress\":%d}\n", percent);
    fflush(stdout);
}

static int extract_one(const wchar_t *id, HMODULE m, void *ctx)
{
    struct ExtractJob *job = ctx;
    const struct PokeRecompGameInfo *info = ((InfoFn)GetProcAddress(m, "PokeRecomp_GameInfo"))();
    ExtractFn extract = (ExtractFn)GetProcAddress(m, "PokeRecomp_Extract");
    ExtractProgressFn extract_progress = (ExtractProgressFn)GetProcAddress(m, "PokeRecomp_ExtractProgress");
    wchar_t out[MAX_PATH];
    int code;

    if (!extract || (job->only && _wcsicmp(job->only, id)))
        return 0;
    if (job->out) wcsncpy(out, job->out, MAX_PATH - 1), out[MAX_PATH - 1] = 0;
    else _snwprintf(out, MAX_PATH, L"%ls\\%ls", sDir, info->pkmn);
    code = extract_progress ? extract_progress(job->rom, out, print_progress, NULL) : extract(job->rom, out);
    if (code == 0) {
        print_game(info, out);
        putchar('\n');
        job->code = 0;
        return 1;
    }
    if (code != 1 /* not this game's ROM */) job->code = code;
    return 0;
}

/* Every game that takes visitors' sprites from this ROM unpacks them too
 * (<game>_<region>.pkmn next to this program). */
static int extract_foreign(const wchar_t *id, HMODULE m, void *ctx)
{
    ExtractForeignFn fn = (ExtractForeignFn)GetProcAddress(m, "PokeRecomp_ExtractForeign");
    (void)id;
    if (fn) fn((const wchar_t *)ctx, NULL);
    return 0;
}

/* Append one argument to a command line, quoted for CommandLineToArgvW. */
static void append_arg(wchar_t *cmd, size_t size, const wchar_t *arg)
{
    size_t n = wcslen(cmd);
    if (n && n + 1 < size) cmd[n++] = L' ';
    if (n + 1 < size) cmd[n++] = L'"';
    for (; *arg && n + 3 < size; arg++) {
        if (*arg == L'"') cmd[n++] = L'\\';
        cmd[n++] = *arg;
    }
    if (n + 1 < size) cmd[n++] = L'"';
    cmd[n] = 0;
}

/* The next leg of a trip: the game travel.dat (in the data directory, the
 * current one now) names, in a child runner with the same window arguments.
 * Returns its exit code. */
static int travel_leg(int argc, wchar_t **wargv)
{
    struct PcTravelFile t;
    wchar_t path[MAX_PATH], exe[MAX_PATH], id[16], save[MAX_PATH];
    static wchar_t cmd[32768];
    STARTUPINFOW si = {sizeof si};
    PROCESS_INFORMATION pi;
    DWORD code = 2;
    FILE *f;
    int i;

    GetFullPathNameW(L"" PC_TRAVEL_FILE, MAX_PATH, path, NULL);
    if (!(f = _wfopen(path, L"rb"))) return 2;
    i = fread(&t, sizeof t, 1, f) == 1 && !memcmp(t.magic, PC_TRAVEL_MAGIC, 8);
    fclose(f);
    if (!i) return 2;
    t.homeId[15] = t.destId[15] = 0;
    t.homeSave[259] = t.destSave[259] = 0;
    MultiByteToWideChar(CP_ACP, 0, t.state == PC_TRAVEL_RETURNING ? t.homeId : t.destId, -1, id, 16);
    MultiByteToWideChar(CP_ACP, 0, t.state == PC_TRAVEL_RETURNING ? t.homeSave : t.destSave, -1, save, MAX_PATH);

    GetModuleFileNameW(NULL, exe, MAX_PATH);
    cmd[0] = 0;
    append_arg(cmd, 32768, exe);
    append_arg(cmd, 32768, L"--game");
    append_arg(cmd, 32768, id);
    append_arg(cmd, 32768, L"--chained");
    append_arg(cmd, 32768, L"--travel");
    append_arg(cmd, 32768, L"--save-file");
    append_arg(cmd, 32768, save);
    /* the window, data directory and options carry over; not this game's own
       save, asset file, or test scripts */
    for (i = 3; i < argc; i++) {
        if (!wcscmp(wargv[i], L"--save-file") || !wcscmp(wargv[i], L"--pkmn") || !wcscmp(wargv[i], L"--input")) { i++; continue; }
        if (!wcscmp(wargv[i], L"--warp")) { i += 4; continue; }
        if (!wcscmp(wargv[i], L"--give-mon") || !wcscmp(wargv[i], L"--give-item")) { i += 2; continue; }
        if (!wcscmp(wargv[i], L"--wild-battle")) { i += 3; continue; }
        if (!wcscmp(wargv[i], L"--travel") || !wcscmp(wargv[i], L"--chained") || !wcscmp(wargv[i], L"--net-travel")
         || !wcscmp(wargv[i], L"--net-autojoin")) continue;
        append_arg(cmd, 32768, wargv[i]);
    }
    fflush(stdout);
    if (!CreateProcessW(exe, cmd, NULL, NULL, TRUE, 0, NULL, sStartDir, &si, &pi)) return 2;
    WaitForSingleObject(pi.hProcess, INFINITE);
    GetExitCodeProcess(pi.hProcess, &code);
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    return (int)code;
}

int main(void)
{
    int argc, i;
    wchar_t **wargv = CommandLineToArgvW(GetCommandLineW(), &argc), *slash;

    GetModuleFileNameW(NULL, sDir, MAX_PATH);
    if ((slash = wcsrchr(sDir, L'\\')) != NULL) *slash = 0;
    GetCurrentDirectoryW(MAX_PATH, sStartDir);

    if (argc >= 2 && !wcscmp(wargv[1], L"--list")) {
        int first = 1;
        putchar('[');
        for_each_game(list_one, &first);
        printf("]\n");
        return 0;
    }
    if (argc >= 3 && !wcscmp(wargv[1], L"--extract")) {
        struct ExtractJob job = {wargv[2], NULL, NULL, 1};
        for (i = 3; i + 1 < argc; i++) {
            if (!wcscmp(wargv[i], L"--out")) job.out = wargv[++i];
            else if (!wcscmp(wargv[i], L"--game")) job.only = wargv[++i];
        }
        for_each_game(extract_one, &job);
        for_each_game(extract_foreign, (void *)job.rom);   /* visitors' sprites for the other games */
        if (job.code != 0)
            printf("{\"error\":%d}\n", job.code);   /* 1: no installed game knows this ROM */
        return job.code;
    }
    if (argc >= 3 && !wcscmp(wargv[1], L"--game")) {
        HMODULE m = load_game(wargv[2]);
        RunFn run = m ? (RunFn)GetProcAddress(m, "PokeRecomp_Run") : NULL;
        char **args;
        int n = 0, code, chained = 0;
        if (!run) {
            MessageBoxW(NULL, L"That game isn't installed (games\\<id>.dll).", L"PokeRecomp", MB_OK | MB_ICONERROR);
            return 2;
        }
        args = calloc(argc, sizeof *args);
        args[n++] = utf8(wargv[0]);
        for (i = 3; i < argc; i++) {
            if (!wcscmp(wargv[i], L"--chained")) { chained = 1; continue; }
            args[n++] = utf8(wargv[i]);
        }
        code = run(n, args);
        if (chained) return code;          /* the first runner starts the next leg */
        while (code == PC_EXIT_TRAVEL)
            code = travel_leg(argc, wargv);
        return code;
    }
    MessageBoxW(NULL, L"PokeRecompGame.exe --game <id> | --list | --extract <rom>", L"PokeRecomp", MB_OK);
    return 2;
}
