/* runner.c — PokeRecompGame.exe: runs a PokeRecomp game DLL in its own process.
 *
 *   PokeRecompGame.exe --game firered [game arguments...]
 *       load games/firered.dll at runtime and run it (the launcher starts
 *       this with --launcher-parent, --pkmn, --save-file, --data-dir ...)
 *   PokeRecompGame.exe --list
 *       print the installed games as JSON (one per games/*.dll)
 *   PokeRecompGame.exe --extract ROM [--out FILE.pkmn] [--game ID]
 *       ROM import: find the game the ROM belongs to and unpack it into its
 *       .pkmn (default: next to this program). Prints JSON, exit 0 on success.
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

struct PokeRecompGameInfo {
    uint32_t abi;
    const char *id, *title, *region;
    const wchar_t *pkmn;
};
typedef const struct PokeRecompGameInfo *(*InfoFn)(void);
typedef int (*ExtractFn)(const wchar_t *rom, const wchar_t *pkmn);
typedef int (*RunFn)(int argc, char **argv);

static wchar_t sDir[MAX_PATH];

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

static int extract_one(const wchar_t *id, HMODULE m, void *ctx)
{
    struct ExtractJob *job = ctx;
    const struct PokeRecompGameInfo *info = ((InfoFn)GetProcAddress(m, "PokeRecomp_GameInfo"))();
    ExtractFn extract = (ExtractFn)GetProcAddress(m, "PokeRecomp_Extract");
    wchar_t out[MAX_PATH];
    int code;

    if (!extract || (job->only && _wcsicmp(job->only, id)))
        return 0;
    if (job->out) wcsncpy(out, job->out, MAX_PATH - 1), out[MAX_PATH - 1] = 0;
    else _snwprintf(out, MAX_PATH, L"%ls\\%ls", sDir, info->pkmn);
    code = extract(job->rom, out);
    if (code == 0) {
        print_game(info, out);
        putchar('\n');
        job->code = 0;
        return 1;
    }
    if (code != 1 /* not this game's ROM */) job->code = code;
    return 0;
}

int main(void)
{
    int argc, i;
    wchar_t **wargv = CommandLineToArgvW(GetCommandLineW(), &argc), *slash;

    GetModuleFileNameW(NULL, sDir, MAX_PATH);
    if ((slash = wcsrchr(sDir, L'\\')) != NULL) *slash = 0;

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
        if (job.code != 0)
            printf("{\"error\":%d}\n", job.code);   /* 1: no installed game knows this ROM */
        return job.code;
    }
    if (argc >= 3 && !wcscmp(wargv[1], L"--game")) {
        HMODULE m = load_game(wargv[2]);
        RunFn run = m ? (RunFn)GetProcAddress(m, "PokeRecomp_Run") : NULL;
        char **args;
        int n = 0;
        if (!run) {
            MessageBoxW(NULL, L"That game isn't installed (games\\<id>.dll).", L"PokeRecomp", MB_OK | MB_ICONERROR);
            return 2;
        }
        args = calloc(argc, sizeof *args);
        args[n++] = utf8(wargv[0]);
        for (i = 3; i < argc; i++) args[n++] = utf8(wargv[i]);
        return run(n, args);
    }
    MessageBoxW(NULL, L"PokeRecompGame.exe --game <id> | --list | --extract <rom>", L"PokeRecomp", MB_OK);
    return 2;
}
