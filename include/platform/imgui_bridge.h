#ifndef GUARD_PLATFORM_IMGUI_BRIDGE_H
#define GUARD_PLATFORM_IMGUI_BRIDGE_H

#include <SDL2/SDL.h>

#ifdef __cplusplus
extern "C" {
#endif

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
    // Keybinds (SDL_Keycode for keyboard, XInput button mask for controller)
    int kb_binds[BIND_COUNT];
    int gp_binds[BIND_COUNT];
    // Enhancements - Speed
    float speed_multiplier;
    // Enhancements - Cheats
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

void ImGuiBridge_Init(SDL_Window *window, void *gl_context);
void ImGuiBridge_Shutdown(void);
void ImGuiBridge_NewFrame(void);
void ImGuiBridge_Render(void);
int  ImGuiBridge_ProcessEvent(SDL_Event *event);
int  ImGuiBridge_IsOverlayVisible(void);
void ImGuiBridge_ToggleOverlay(void);
struct RenderConfig *ImGuiBridge_GetConfig(void);
int  ImGuiBridge_IsAlwaysShiny(void);
int  ImGuiBridge_GetMenuBarHeight(void);
int  ImGuiBridge_GetSelectedShader(void);
int  ImGuiBridge_ShouldQuit(void);
void ImGuiBridge_GetResolution(int *w, int *h);

#ifdef __cplusplus
}
#endif

#endif // GUARD_PLATFORM_IMGUI_BRIDGE_H
