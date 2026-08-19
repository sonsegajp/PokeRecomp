#ifdef _WIN32
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef min
#undef max
#endif

#include "imgui.h"
#include "backends/imgui_impl_sdl2.h"
#include "backends/imgui_impl_opengl3.h"
#include <SDL2/SDL.h>
#include <stdlib.h>

extern "C" {
#include "../../server/sha256.h"
}

extern "C" int Platform_GetShaderCount(void);
extern "C" const char *Platform_GetShaderName(int idx);
extern "C" unsigned char AddBagItem(unsigned short itemId, unsigned short count);
extern "C" unsigned char RemoveBagItem(unsigned short itemId, unsigned short count);
extern "C" void Platform_OpenGameController(int deviceIndex);
extern "C" int Platform_GetGameControllerCount(void);
extern "C" const char *Platform_GetGameControllerName(int idx);
extern "C" int Platform_PollControllerButton(void);
extern "C" void Debug_WarpToMap(int group, int map);
extern "C" void Debug_SetPartyPidgeot(void);
extern "C" void Debug_FillPCBoxes(void);
extern "C" void Debug_GiveAllHMs(void);
extern "C" void Debug_CompletePokedex(void);
extern "C" void Debug_AllBadgesE4(void);
extern "C" void *gSaveBlock1Ptr;
extern "C" int NetClient_IsConnected(void);
extern "C" int NetClient_Connect(const char *ip, int port, const char *username, unsigned char trainerSprite,
                                 unsigned short mapGroup, unsigned short mapNum, short x, short y, unsigned char direction, const unsigned char *passwordHash);
extern "C" void NetClient_Disconnect(void);
extern "C" void NetClient_SendChat(const char *msg);
extern "C" int NetClient_GetChatCount(void);
struct ChatEntry { char username[16]; char message[128]; int timer; };
extern "C" struct ChatEntry *NetClient_GetChatHistory(void);
struct RemotePlayer { int active; unsigned char id; char username[16]; unsigned char trainerSprite;
    unsigned short mapGroup; unsigned short mapNum; short x, y; unsigned char direction; int needsSpawn; int needsDespawn; };
extern "C" struct RemotePlayer *NetClient_GetRemotePlayers(void);

static bool sChatInputActive = false;

static int (*sShaderCountOverride)(void) = nullptr;
static const char *(*sShaderNameOverride)(int) = nullptr;

extern "C" void ImGuiBridge_SetShaderProvider(int (*getCount)(void), const char *(*getName)(int))
{
    sShaderCountOverride = getCount;
    sShaderNameOverride = getName;
}

enum KeybindAction {
    BIND_A, BIND_B, BIND_START, BIND_SELECT,
    BIND_L, BIND_R,
    BIND_UP, BIND_DOWN, BIND_LEFT, BIND_RIGHT,
    BIND_SPEED_UP,
    BIND_COUNT
};

struct RenderConfig {
    int scale;
    int resolution_index;
    int fullscreen;
    int filter_mode;
    int color_shader;
    int lcd_grid;
    int scanlines;
    int widescreen;
    int show_fps;
    float text_speed;
    float volume;
    int kb_binds[BIND_COUNT];
    int gp_binds[BIND_COUNT];
    float speed_multiplier;
    int cheats_always_shiny;
    int cheats_walk_through_walls;
    int cheats_infinite_money;
    int cheats_infinite_rare_candy;
    int cheats_instant_egg_hatch;
    int cheats_no_random_encounters;
    int cheats_catch_rate_100;
    int cheats_infinite_pp;
    int cheats_exp_multiplier;
    int cheats_shiny_charm;
    int enhance_b_select_run;
    int enhance_poke_radar;
};

struct ResolutionPreset {
    const char *label;
    int width;
    int height;
};

static const ResolutionPreset sResolutions[] = {
    { "1x (240x160)",     240,  160  },
    { "2x (480x320)",     480,  320  },
    { "3x (720x480)",     720,  480  },
    { "4x (960x640)",     960,  640  },
    { "5x (1200x800)",    1200, 800  },
    { "720p (1280x720)",  1280, 720  },
    { "1080p (1920x1080)",1920, 1080 },
    { "1440p (2560x1440)",2560, 1440 },
    { "4K (3840x2160)",   3840, 2160 },
};
static const int sNumResolutions = sizeof(sResolutions) / sizeof(sResolutions[0]);

static bool sOverlayVisible = false;
static bool sQuitDialogOpen = false;
static bool sQuitConfirmed = false;
// XInput button constants for default gamepad binds
#define GP_A       0x1000
#define GP_X       0x4000
#define GP_START   0x0010
#define GP_BACK    0x0020
#define GP_LB      0x0100
#define GP_RB      0x0200
#define GP_DUP     0x0001
#define GP_DDOWN   0x0002
#define GP_DLEFT   0x0004
#define GP_DRIGHT  0x0008
#define GP_RTRIGGER 0xF001 // sentinel for right trigger

static const char *sBindNames[BIND_COUNT] = {
    "A Button", "B Button", "Start", "Select",
    "L", "R",
    "D-Pad Up", "D-Pad Down", "D-Pad Left", "D-Pad Right",
    "Speed Up"
};

extern "C" void ImGuiBridge_SaveSettings(void);
extern "C" void ImGuiBridge_LoadSettings(void);

static int sRebindingAction = -1; // -1 = not rebinding, otherwise the action index
static bool sRebindingController = false;

static RenderConfig sConfig = {
    .scale = 3,
    .resolution_index = 2,
    .fullscreen = 0,
    .filter_mode = 0,
    .color_shader = 0,
    .lcd_grid = 0,
    .scanlines = 0,
    .widescreen = 0,
    .show_fps = 1,
    .text_speed = 1.0f,
    .volume = 0.8f,
    .kb_binds = {
        SDLK_z, SDLK_x, SDLK_RETURN, SDLK_BACKSLASH,
        SDLK_a, SDLK_s,
        SDLK_UP, SDLK_DOWN, SDLK_LEFT, SDLK_RIGHT,
        SDLK_SPACE
    },
    .gp_binds = {
        GP_A, GP_X, GP_START, GP_BACK,
        GP_LB, GP_RB,
        GP_DUP, GP_DDOWN, GP_DLEFT, GP_DRIGHT,
        GP_RTRIGGER
    },
    .speed_multiplier = 5.0f,
    .cheats_always_shiny = 0,
    .cheats_walk_through_walls = 0,
    .cheats_infinite_money = 0,
    .cheats_infinite_rare_candy = 0,
    .cheats_instant_egg_hatch = 0,
    .cheats_no_random_encounters = 0,
    .cheats_catch_rate_100 = 0,
    .cheats_infinite_pp = 0,
    .cheats_exp_multiplier = 1,
    .cheats_shiny_charm = 0,
    .enhance_b_select_run = 0,
    .enhance_poke_radar = 0,
};
static Uint64 sLastFrameTime = 0;
static float sFps = 0.0f;
static int sSelectedShader = 0; // 0=None, 1=LCD Grid, 2=Scanlines, 3=Sharp Bilinear, 4=CRT

extern "C" void ImGuiBridge_Init(SDL_Window *window, void *gl_context)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.IniFilename = NULL;

    ImGui::StyleColorsDark();
    ImGuiStyle &style = ImGui::GetStyle();
    style.WindowRounding = 0.0f;
    style.FrameRounding = 3.0f;
    style.Alpha = 0.95f;
    style.WindowPadding = ImVec2(6, 4);
    style.FramePadding = ImVec2(4, 2);
    style.ItemSpacing = ImVec2(6, 3);

    // DPI-aware font scaling: query the display DPI and scale relative to 96 DPI baseline.
    float dpiScale = 1.0f;
    int displayIndex = SDL_GetWindowDisplayIndex(window);
    float ddpi = 0.0f;
    if (displayIndex >= 0 && SDL_GetDisplayDPI(displayIndex, &ddpi, NULL, NULL) == 0 && ddpi > 0.0f)
        dpiScale = ddpi / 96.0f;

    // Clamp to a reasonable range to avoid extreme sizing.
    if (dpiScale < 1.0f)
        dpiScale = 1.0f;
    if (dpiScale > 3.0f)
        dpiScale = 3.0f;

    io.FontGlobalScale = dpiScale;

    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init("#version 130");
    ImGuiBridge_LoadSettings();
}

extern "C" void ImGuiBridge_Shutdown(void)
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
}

extern "C" void ImGuiBridge_NewFrame(void)
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    // FPS calculation
    Uint64 now = SDL_GetPerformanceCounter();
    if (sLastFrameTime > 0)
    {
        float dt = (float)(now - sLastFrameTime) / (float)SDL_GetPerformanceFrequency();
        sFps = sFps * 0.95f + (1.0f / dt) * 0.05f;
    }
    sLastFrameTime = now;
}

extern "C" int NetPlayerSprites_GetNametagCount(void);
extern "C" int NetPlayerSprites_GetNametag(int index, int *screenX, int *screenY, char *nameOut, int nameMax);
extern "C" void Platform_GetGameViewport(int *x, int *y, int *w, int *h);
extern "C" int NetClient_IsConnected(void);

extern "C" bool IsOverworldActive(void);

static void DrawPlayerNametags(void)
{
    if (!NetClient_IsConnected()) return;
    if (!IsOverworldActive()) return;

    int vpX, vpY, vpW, vpH;
    Platform_GetGameViewport(&vpX, &vpY, &vpW, &vpH);
    if (vpW <= 0 || vpH <= 0) return;

    float scaleX = (float)vpW / 240.0f;
    float scaleY = (float)vpH / 160.0f;

    int count = NetPlayerSprites_GetNametagCount();
    for (int i = 0; i < count; i++)
    {
        int gbaX, gbaY;
        char name[16];
        if (!NetPlayerSprites_GetNametag(i, &gbaX, &gbaY, name, sizeof(name)))
            continue;
        if (name[0] == '\0') continue;

        float wx = vpX + gbaX * scaleX;
        float wy = vpY + gbaY * scaleY;

        ImVec2 textSize = ImGui::CalcTextSize(name);
        float tx = wx - textSize.x * 0.5f;
        float ty = wy - textSize.y;

        ImDrawList *dl = ImGui::GetForegroundDrawList();
        dl->AddRectFilled(ImVec2(tx - 2, ty - 1), ImVec2(tx + textSize.x + 2, ty + textSize.y + 1), IM_COL32(0, 0, 0, 160), 3.0f);
        dl->AddText(ImVec2(tx, ty), IM_COL32(255, 255, 255, 255), name);
    }
}

extern "C" void ImGuiBridge_Render(void)
{
    if (!sOverlayVisible)
    {
        ImGui::EndFrame();
        return;
    }

    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("Display"))
        {
            if (ImGui::BeginCombo("Resolution", sResolutions[sConfig.resolution_index].label)) {
                for (int i = 0; i < sNumResolutions; i++) {
                    bool selected = (sConfig.resolution_index == i);
                    if (ImGui::Selectable(sResolutions[i].label, selected))
                        sConfig.resolution_index = i;
                    if (selected) ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }
            ImGui::Checkbox("Fullscreen", (bool *)&sConfig.fullscreen);
            ImGui::Checkbox("Widescreen", (bool *)&sConfig.widescreen);
            ImGui::Separator();
            const char *filters[] = {"Nearest Neighbor", "Linear"};
            ImGui::Combo("Display Filter", &sConfig.filter_mode, filters, 2);
            const char *colorShaders[] = {"None", "GBA Color (higan)", "GBA Color (Libretro)"};
            ImGui::Combo("Color", &sConfig.color_shader, colorShaders, 3);
            ImGui::Separator();
            {
                int count = sShaderCountOverride ? sShaderCountOverride() : Platform_GetShaderCount();
                const char *curName = sShaderNameOverride ? sShaderNameOverride(sSelectedShader) : Platform_GetShaderName(sSelectedShader);
                if (ImGui::BeginCombo("Shader", curName)) {
                    for (int i = 0; i < count; i++) {
                        bool selected = (sSelectedShader == i);
                        const char *name = sShaderNameOverride ? sShaderNameOverride(i) : Platform_GetShaderName(i);
                        if (ImGui::Selectable(name, selected))
                            sSelectedShader = i;
                        if (selected) ImGui::SetItemDefaultFocus();
                    }
                    ImGui::EndCombo();
                }
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Audio"))
        {
            float volPercent = sConfig.volume * 100.0f;
            if (ImGui::SliderFloat("Volume", &volPercent, 0.0f, 100.0f, "%.0f%%"))
                sConfig.volume = volPercent / 100.0f;
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Controls"))
        {
            ImGui::SetNextItemWidth(300);
            if (ImGui::CollapsingHeader("Keyboard", ImGuiTreeNodeFlags_DefaultOpen))
            {
                for (int i = 0; i < BIND_COUNT; i++) {
                    ImGui::PushID(i);
                    const char *label = (sRebindingAction == i && !sRebindingController) ? ">> Press a key <<" : SDL_GetKeyName((SDL_Keycode)sConfig.kb_binds[i]);
                    if (ImGui::Button(label, ImVec2(140, 0))) {
                        sRebindingAction = i;
                        sRebindingController = false;
                    }
                    ImGui::SameLine();
                    ImGui::Text("%s", sBindNames[i]);
                    ImGui::PopID();
                }
            }
            if (ImGui::CollapsingHeader("Controller"))
            {
                // Controller selection
                {
                    int count = Platform_GetGameControllerCount();
                    if (count == 0) {
                        ImGui::TextDisabled("No controller detected");
                    } else {
                        static int sSelectedController = -1;
                        const char *curName = sSelectedController >= 0 ? Platform_GetGameControllerName(sSelectedController) : "Select a controller...";
                        if (ImGui::BeginCombo("Device", curName)) {
                            for (int c = 0; c < count; c++) {
                                bool sel = (sSelectedController == c);
                                if (ImGui::Selectable(Platform_GetGameControllerName(c), sel)) {
                                    sSelectedController = c;
                                    Platform_OpenGameController(c);
                                }
                                if (sel) ImGui::SetItemDefaultFocus();
                            }
                            ImGui::EndCombo();
                        }
                    }
                }
                ImGui::Separator();
                for (int i = 0; i < BIND_COUNT; i++) {
                    ImGui::PushID(100 + i);
                    const char *gpName = "?";
                    int btn = sConfig.gp_binds[i];
                    if (btn == GP_A) gpName = "A";
                    else if (btn == GP_X) gpName = "X";
                    else if (btn == 0x2000) gpName = "B";
                    else if (btn == 0x8000) gpName = "Y";
                    else if (btn == GP_START) gpName = "Start";
                    else if (btn == GP_BACK) gpName = "Back";
                    else if (btn == GP_LB) gpName = "LB";
                    else if (btn == GP_RB) gpName = "RB";
                    else if (btn == GP_DUP) gpName = "D-Up";
                    else if (btn == GP_DDOWN) gpName = "D-Down";
                    else if (btn == GP_DLEFT) gpName = "D-Left";
                    else if (btn == GP_DRIGHT) gpName = "D-Right";
                    else if (btn == GP_RTRIGGER) gpName = "RT";
                    const char *label = (sRebindingAction == i && sRebindingController) ? ">> Press button <<" : gpName;
                    if (ImGui::Button(label, ImVec2(140, 0))) {
                        sRebindingAction = i;
                        sRebindingController = true;
                    }
                    ImGui::SameLine();
                    ImGui::Text("%s", sBindNames[i]);
                    ImGui::PopID();
                }
            }
            ImGui::Separator();
            if (ImGui::Button("Reset to Defaults")) {
                int kb_defaults[] = { SDLK_z, SDLK_x, SDLK_RETURN, SDLK_BACKSLASH, SDLK_a, SDLK_s, SDLK_UP, SDLK_DOWN, SDLK_LEFT, SDLK_RIGHT, SDLK_SPACE };
                int gp_defaults[] = { GP_A, GP_X, GP_START, GP_BACK, GP_LB, GP_RB, GP_DUP, GP_DDOWN, GP_DLEFT, GP_DRIGHT, GP_RTRIGGER };
                for (int i = 0; i < BIND_COUNT; i++) { sConfig.kb_binds[i] = kb_defaults[i]; sConfig.gp_binds[i] = gp_defaults[i]; }
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Enhancements"))
        {
            if (ImGui::CollapsingHeader("Optional Enhancements", ImGuiTreeNodeFlags_DefaultOpen))
            {
                ImGui::SliderFloat("Text Speed", &sConfig.text_speed, 0.5f, 4.0f, "%.1fx");
                ImGui::Checkbox("Shiny Charm (earned in postgame by defeating Prof. Oak)", (bool *)&sConfig.cheats_shiny_charm);
                ImGui::Checkbox("B -> Select Run", (bool *)&sConfig.enhance_b_select_run);
                ImGui::Checkbox("Poké Radar", (bool *)&sConfig.enhance_poke_radar);
            }
            if (ImGui::CollapsingHeader("Speed-ups"))
            {
                ImGui::SliderFloat("Fast Forward Speed", &sConfig.speed_multiplier, 2.0f, 10.0f, "%.0fx");
                ImGui::TextDisabled("Hold Space or RT to fast forward");
            }
            if (ImGui::CollapsingHeader("Cheats"))
            {
                bool online = NetClient_IsConnected();
                if (online)
                {
                    sConfig.cheats_always_shiny = 0;
                    sConfig.cheats_walk_through_walls = 0;
                    sConfig.cheats_infinite_money = 0;
                    sConfig.cheats_infinite_rare_candy = 0;
                    sConfig.cheats_instant_egg_hatch = 0;
                    sConfig.cheats_no_random_encounters = 0;
                    sConfig.cheats_catch_rate_100 = 0;
                    sConfig.cheats_infinite_pp = 0;
                    sConfig.cheats_exp_multiplier = 1;
                    ImGui::BeginDisabled();
                }
                ImGui::Checkbox("Always Shiny Pokemon", (bool *)&sConfig.cheats_always_shiny);
                ImGui::Checkbox("Walk Through Walls", (bool *)&sConfig.cheats_walk_through_walls);
                ImGui::Checkbox("Infinite Money", (bool *)&sConfig.cheats_infinite_money);
                ImGui::Checkbox("Infinite Rare Candy", (bool *)&sConfig.cheats_infinite_rare_candy);
                ImGui::Checkbox("Instant Egg Hatch", (bool *)&sConfig.cheats_instant_egg_hatch);
                ImGui::Checkbox("No Random Encounters", (bool *)&sConfig.cheats_no_random_encounters);
                ImGui::Checkbox("100% Catch Rate", (bool *)&sConfig.cheats_catch_rate_100);
                ImGui::Checkbox("Infinite PP", (bool *)&sConfig.cheats_infinite_pp);
                int expVals[] = {1, 2, 4, 8};
                const char *expLabels[] = {"1x", "2x", "4x", "8x"};
                int expIdx = 0;
                for (int i = 0; i < 4; i++) if (sConfig.cheats_exp_multiplier == expVals[i]) expIdx = i;
                if (ImGui::Combo("EXP Multiplier", &expIdx, expLabels, 4))
                    sConfig.cheats_exp_multiplier = expVals[expIdx];
                if (online)
                    ImGui::EndDisabled();
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Online"))
        {
            static char sIP[64] = "127.0.0.1";
            static int sPort = 27015;
            static char sUser[17] = "Player";
            static char sPassword[33] = "";

            static int sTrainerChoice = 0;
            static const char *sTrainerNames[] = { "Red", "Leaf" };

            if (!NetClient_IsConnected())
            {
                ImGui::InputText("IP", sIP, sizeof(sIP));
                ImGui::InputInt("Port", &sPort);
                ImGui::InputText("Name", sUser, sizeof(sUser));
                ImGui::InputText("Password", sPassword, sizeof(sPassword), ImGuiInputTextFlags_Password);
                ImGui::Combo("Trainer", &sTrainerChoice, sTrainerNames, 2);
                if (ImGui::Button("Connect"))
                {
                    unsigned char hash[32] = {0};
                    if (sPassword[0])
                        sha256(sPassword, strlen(sPassword), hash);
                    NetClient_Connect(sIP, sPort, sUser, (unsigned char)sTrainerChoice, 0, 0, 0, 0, 1, hash);
                }
            }
            else
            {
                ImGui::TextColored(ImVec4(0.4f,1,0.4f,1), "Online as %s", sUser);
                if (ImGui::Button("Disconnect"))
                    NetClient_Disconnect();
                ImGui::Separator();
                struct RemotePlayer *p = NetClient_GetRemotePlayers();
                for (int i = 0; i < 64; i++)
                    if (p[i].active)
                        ImGui::BulletText("%s", p[i].username);
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Debug"))
        {
            static const struct { const char *name; int group; int map; } sWarpLocations[] = {
                {"Pallet Town", 3, 0}, {"Viridian City", 3, 1}, {"Pewter City", 3, 2},
                {"Cerulean City", 3, 3}, {"Lavender Town", 3, 4}, {"Vermilion City", 3, 5},
                {"Celadon City", 3, 6}, {"Fuchsia City", 3, 7}, {"Saffron City", 3, 10},
                {"Cinnabar Island", 3, 8}, {"Indigo Plateau", 3, 9},
                {"One Island", 3, 12}, {"Two Island", 3, 13}, {"Three Island", 3, 14},
                {"Four Island", 3, 15}, {"Five Island", 3, 16}, {"Six Island", 3, 18},
                {"Seven Island", 3, 17},
                {"Oak's Lab", 4, 3}, {"Player's House", 4, 0},
                {"--- HOENN ---", 43, 9},
                {"Littleroot Town", 43, 9}, {"Oldale Town", 43, 10},
                {"Petalburg City", 43, 0}, {"Rustboro City", 43, 3},
                {"Dewford Town", 43, 11}, {"Slateport City", 43, 1},
                {"Mauville City", 43, 2}, {"Verdanturf Town", 43, 14},
                {"Fallarbor Town", 43, 13}, {"Lavaridge Town", 43, 12},
                {"Fortree City", 43, 4}, {"Lilycove City", 43, 5},
                {"Mossdeep City", 43, 6}, {"Sootopolis City", 43, 7},
                {"Ever Grande City", 43, 8}, {"Pacifidlog Town", 43, 15},
                {"Route 101", 43, 16}, {"Route 102", 43, 17},
            };
            static int sWarpIdx = 0;
            if (ImGui::BeginCombo("Warp To", sWarpLocations[sWarpIdx].name))
            {
                for (int i = 0; i < (int)(sizeof(sWarpLocations)/sizeof(sWarpLocations[0])); i++)
                {
                    bool sel = (sWarpIdx == i);
                    if (ImGui::Selectable(sWarpLocations[i].name, sel))
                        sWarpIdx = i;
                    if (sel) ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }
            if (ImGui::Button("Warp!"))
            {
                Debug_WarpToMap(sWarpLocations[sWarpIdx].group, sWarpLocations[sWarpIdx].map);
            }
            ImGui::Separator();
            if (ImGui::Button("Set Party: Pidgeot Lv75 w/ Fly"))
                Debug_SetPartyPidgeot();
            ImGui::Separator();
            ImGui::TextColored(ImVec4(1,1,0,1), "Cheats:");
            if (ImGui::Button("Fill PC Boxes (All 386 Pokemon)"))
            {
                Debug_FillPCBoxes();
            }
            if (ImGui::Button("Give All HMs"))
            {
                Debug_GiveAllHMs();
            }
            if (ImGui::Button("Complete Pokedex"))
            {
                Debug_CompletePokedex();
            }
            if (ImGui::Button("All Badges + E4 Clear"))
            {
                Debug_AllBadgesE4();
            }
            ImGui::EndMenu();
        }

        if (sConfig.show_fps)
        {
            char fpsText[32];
            snprintf(fpsText, sizeof(fpsText), "FPS: %.0f", sFps);
            float w = ImGui::CalcTextSize(fpsText).x;
            ImGui::SameLine(ImGui::GetWindowWidth() - w - 10);
            ImGui::Text("%s", fpsText);
        }

        ImGui::EndMainMenuBar();
    }

    // Chat overlay + input (press / to type, Enter to send)
    if (NetClient_IsConnected())
    {
        static char sChatBuf[128] = "";
        int chatCount = NetClient_GetChatCount();
        struct ChatEntry *history = NetClient_GetChatHistory();

        // Show recent messages bottom-left
        if (chatCount > 0)
        {
            ImGui::SetNextWindowPos(ImVec2(8, ImGui::GetIO().DisplaySize.y - (sChatInputActive ? 60 : 8)), ImGuiCond_Always, ImVec2(0, 1));
            ImGui::SetNextWindowBgAlpha(0.6f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 4);
            ImGui::Begin("##chat", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoInputs);
            for (int i = 0; i < chatCount; i++)
            {
                if (strstr(history[i].message, "SHINY"))
                {
                    ImGui::TextColored(ImVec4(1,0.9f,0.2f,1), "%s %s", history[i].username, history[i].message);
                }
                else if (strstr(history[i].message, "caught"))
                {
                    ImGui::TextColored(ImVec4(0.3f,1,0.3f,1), "%s %s", history[i].username, history[i].message);
                }
                else
                {
                    ImGui::TextColored(ImVec4(0.5f,1,0.5f,1), "%s:", history[i].username);
                    ImGui::SameLine();
                    ImGui::Text("%s", history[i].message);
                }
            }
            ImGui::End();
            ImGui::PopStyleVar();
        }

        // / key opens chat input
        if (!sChatInputActive && !ImGui::GetIO().WantCaptureKeyboard && ImGui::IsKeyPressed(ImGuiKey_Slash))
        {
            sChatInputActive = true;
            sChatBuf[0] = '\0';
        }
        if (sChatInputActive)
        {
            ImGui::SetNextWindowPos(ImVec2(8, ImGui::GetIO().DisplaySize.y - 8), ImGuiCond_Always, ImVec2(0, 1));
            ImGui::SetNextWindowSize(ImVec2(400, 0));
            ImGui::Begin("##chatinput", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
            ImGui::SetNextItemWidth(-1);
            if (ImGui::IsWindowAppearing()) ImGui::SetKeyboardFocusHere();
            if (ImGui::InputText("##m", sChatBuf, sizeof(sChatBuf), ImGuiInputTextFlags_EnterReturnsTrue))
            {
                if (sChatBuf[0]) NetClient_SendChat(sChatBuf);
                sChatBuf[0] = '\0';
                sChatInputActive = false;
            }
            if (ImGui::IsKeyPressed(ImGuiKey_Escape)) sChatInputActive = false;
            ImGui::End();
        }
    }
    else sChatInputActive = false;

    // Auto-save settings every ~5 seconds
    {
        static Uint64 sLastSave = 0;
        Uint64 now = SDL_GetTicks64();
        if (now - sLastSave > 5000) {
            sLastSave = now;
            ImGuiBridge_SaveSettings();
        }
    }

    if (sQuitDialogOpen)
    {
        ImGui::OpenPopup("Quit Game");
        ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        if (ImGui::BeginPopupModal("Quit Game", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove))
        {
            ImGui::Text("Return to launcher?");
            ImGui::Separator();
            if (ImGui::Button("Yes", ImVec2(120, 0)))
            {
                sQuitConfirmed = true;
                sQuitDialogOpen = false;
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button("No", ImVec2(120, 0)))
            {
                sQuitDialogOpen = false;
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

extern "C" int ImGuiBridge_ProcessEvent(SDL_Event *event)
{
    ImGui_ImplSDL2_ProcessEvent(event);

    if (sChatInputActive && (event->type == SDL_KEYDOWN || event->type == SDL_KEYUP || event->type == SDL_TEXTINPUT))
        return 1;

    // Rebinding capture — intercept key/button presses
    if (sRebindingAction >= 0) {
        if (!sRebindingController && event->type == SDL_KEYDOWN && event->key.repeat == 0) {
            SDL_Keycode key = event->key.keysym.sym;
            if (key != SDLK_ESCAPE && key != SDLK_F1) {
                sConfig.kb_binds[sRebindingAction] = (int)key;
                sRebindingAction = -1;
            } else if (key == SDLK_ESCAPE) {
                sRebindingAction = -1;
            }
            return 1;
        }
        if (sRebindingController) {
            if (event->type == SDL_KEYDOWN && event->key.keysym.sym == SDLK_ESCAPE) {
                sRebindingAction = -1;
                return 1;
            }
            int btn = Platform_PollControllerButton();
            if (btn >= 0) {
                sConfig.gp_binds[sRebindingAction] = btn;
                sRebindingAction = -1;
                return 1;
            }
        }
    }

    if (event->type == SDL_KEYDOWN && event->key.keysym.sym == SDLK_F1
        && event->key.repeat == 0)
    {
        sOverlayVisible = !sOverlayVisible;
        return 1;
    }

    if (event->type == SDL_KEYDOWN && event->key.keysym.sym == SDLK_ESCAPE
        && event->key.repeat == 0 && !sQuitDialogOpen)
    {
        sQuitDialogOpen = true;
        return 1;
    }

    if (sQuitDialogOpen)
        return 1;

    if (sOverlayVisible)
    {
        ImGuiIO &io = ImGui::GetIO();
        if (io.WantCaptureKeyboard || io.WantCaptureMouse)
            return 1;
    }

    return 0;
}

extern "C" int ImGuiBridge_IsOverlayVisible(void)
{
    return sOverlayVisible ? 1 : 0;
}

extern "C" void ImGuiBridge_ToggleOverlay(void)
{
    sOverlayVisible = !sOverlayVisible;
}

extern "C" struct RenderConfig *ImGuiBridge_GetConfig(void)
{
    return &sConfig;
}

extern "C" unsigned char CheckBagHasItem(unsigned short itemId, unsigned short count);
extern "C" unsigned char FlagGet(unsigned short id);

extern "C" void ImGuiBridge_SyncShinyCharm(void)
{
    if (!gSaveBlock1Ptr) return;
    bool hasItem = CheckBagHasItem(375, 1);
    // Shiny Charm is earned by defeating Prof. Oak (FLAG_OAK_BATTLE_DONE = 0x8E0)
    bool earnedCharm = FlagGet(0x8E0);
    // If toggle is OFF, or player hasn't earned it yet, remove the item
    if (hasItem && (!sConfig.cheats_shiny_charm || !earnedCharm)) {
        for (int i = 0; i < 10 && CheckBagHasItem(375, 1); i++)
            RemoveBagItem(375, 1);
    }
    // Poke Radar sync
    bool hasRadar = CheckBagHasItem(376, 1);
    if (sConfig.enhance_poke_radar && !hasRadar)
        AddBagItem(376, 1);
    else if (!sConfig.enhance_poke_radar && hasRadar) {
        for (int i = 0; i < 10 && CheckBagHasItem(376, 1); i++)
            RemoveBagItem(376, 1);
    }
}

extern "C" int ImGuiBridge_IsAlwaysShiny(void) { return sConfig.cheats_always_shiny; }
extern "C" int ImGuiBridge_IsWalkThroughWalls(void) { return sConfig.cheats_walk_through_walls; }
extern "C" int ImGuiBridge_IsInfiniteMoney(void) { return sConfig.cheats_infinite_money; }
extern "C" int ImGuiBridge_IsInfiniteRareCandy(void) { return sConfig.cheats_infinite_rare_candy; }
extern "C" int ImGuiBridge_IsInstantEggHatch(void) { return sConfig.cheats_instant_egg_hatch; }
extern "C" int ImGuiBridge_IsNoRandomEncounters(void) { return sConfig.cheats_no_random_encounters; }
extern "C" int ImGuiBridge_IsCatchRate100(void) { return sConfig.cheats_catch_rate_100; }
extern "C" int ImGuiBridge_IsInfinitePP(void) { return sConfig.cheats_infinite_pp; }
extern "C" int ImGuiBridge_GetExpMultiplier(void) { return sConfig.cheats_exp_multiplier; }
extern "C" float ImGuiBridge_GetTextSpeed(void) { return sConfig.text_speed; }
extern "C" int ImGuiBridge_IsShinyCharm(void) { return sConfig.cheats_shiny_charm; }
extern "C" int ImGuiBridge_IsBSelectRun(void) { return sConfig.enhance_b_select_run; }

#define SETTINGS_FILE "pokerecomp_settings.bin"
#define SETTINGS_MAGIC 0x504B5243 // "PKRC"

struct SettingsHeader { unsigned int magic; unsigned int size; };

extern "C" void ImGuiBridge_SaveSettings(void)
{
    FILE *f = fopen(SETTINGS_FILE, "wb");
    if (!f) return;
    struct SettingsHeader hdr = { SETTINGS_MAGIC, sizeof(sConfig) };
    fwrite(&hdr, sizeof(hdr), 1, f);
    fwrite(&sConfig, sizeof(sConfig), 1, f);
    fclose(f);
}

extern "C" void ImGuiBridge_LoadSettings(void)
{
    FILE *f = fopen(SETTINGS_FILE, "rb");
    if (!f) return;
    struct SettingsHeader hdr;
    if (fread(&hdr, sizeof(hdr), 1, f) != 1 || hdr.magic != SETTINGS_MAGIC || hdr.size != sizeof(sConfig)) {
        fclose(f);
        return;
    }
    fread(&sConfig, sizeof(sConfig), 1, f);
    fclose(f);
}

extern "C" int ImGuiBridge_GetMenuBarHeight(void)
{
    // Return the ImGui frame height (font size + padding), which is the main menu bar height.
    ImGuiIO &io = ImGui::GetIO();
    ImGuiStyle &style = ImGui::GetStyle();
    return (int)(io.FontGlobalScale * 13.0f + style.FramePadding.y * 2.0f + 1.0f);
}

extern "C" int ImGuiBridge_GetSelectedShader(void)
{
    return sSelectedShader;
}

extern "C" int ImGuiBridge_ShouldQuit(void)
{
    if (sQuitConfirmed) { sQuitConfirmed = false; return 1; }
    return 0;
}

extern "C" void ImGuiBridge_GetResolution(int *w, int *h)
{
    int idx = sConfig.resolution_index;
    if (idx < 0 || idx >= sNumResolutions) idx = 2;
    *w = sResolutions[idx].width;
    *h = sResolutions[idx].height;
}
