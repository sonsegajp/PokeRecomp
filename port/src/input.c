/* input.c — keyboard and gamepad to the GBA keypad (KEYINPUT, active-low). */
#include <SDL.h>

#include "pc_port.h"
#include "pc_settings.h"
#include "pc_chat.h"
#include "pc_net.h"

enum {
    KEY_A = 0x001, KEY_B = 0x002, KEY_SELECT = 0x004, KEY_START = 0x008,
    KEY_RIGHT = 0x010, KEY_LEFT = 0x020, KEY_UP = 0x040, KEY_DOWN = 0x080,
    KEY_R = 0x100, KEY_L = 0x200,
};

static const struct { SDL_Scancode key; uint16_t bit; } keymap[] = {
    {SDL_SCANCODE_X, KEY_A},          {SDL_SCANCODE_Z, KEY_B},
    {SDL_SCANCODE_BACKSPACE, KEY_SELECT}, {SDL_SCANCODE_RETURN, KEY_START},
    {SDL_SCANCODE_RIGHT, KEY_RIGHT},  {SDL_SCANCODE_LEFT, KEY_LEFT},
    {SDL_SCANCODE_UP, KEY_UP},        {SDL_SCANCODE_DOWN, KEY_DOWN},
    {SDL_SCANCODE_S, KEY_R},          {SDL_SCANCODE_A, KEY_L},
};

static const struct { SDL_GameControllerButton button; uint16_t bit; } padmap[] = {
    {SDL_CONTROLLER_BUTTON_A, KEY_A},          {SDL_CONTROLLER_BUTTON_B, KEY_B},
    {SDL_CONTROLLER_BUTTON_BACK, KEY_SELECT},  {SDL_CONTROLLER_BUTTON_START, KEY_START},
    {SDL_CONTROLLER_BUTTON_DPAD_RIGHT, KEY_RIGHT}, {SDL_CONTROLLER_BUTTON_DPAD_LEFT, KEY_LEFT},
    {SDL_CONTROLLER_BUTTON_DPAD_UP, KEY_UP},   {SDL_CONTROLLER_BUTTON_DPAD_DOWN, KEY_DOWN},
    {SDL_CONTROLLER_BUTTON_RIGHTSHOULDER, KEY_R}, {SDL_CONTROLLER_BUTTON_LEFTSHOULDER, KEY_L},
};

static SDL_GameController *pad;

uint16_t PcInputPoll(int *quit, int *fast_forward)
{
    SDL_Event e;
    uint16_t held = 0;
    const Uint8 *keys;

    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) *quit = 1;
        else if (PcChatActive() && (e.type == SDL_TEXTINPUT || e.type == SDL_KEYDOWN)) {
            /* online chat input line: the keyboard types, the game sees nothing */
            if (e.type == SDL_TEXTINPUT) PcChatTypeUtf8(e.text.text);
            else if (e.key.keysym.scancode == SDL_SCANCODE_BACKSPACE) PcChatBackspace();
            else if (e.key.keysym.scancode == SDL_SCANCODE_RETURN || e.key.keysym.scancode == SDL_SCANCODE_KP_ENTER) {
                PcChatSubmit();
                SDL_StopTextInput();
            } else if (e.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
                PcChatCancel();
                SDL_StopTextInput();
            }
        }
        else if (e.type == SDL_KEYDOWN && !e.key.repeat && e.key.keysym.scancode == SDL_SCANCODE_T && PcNetConnected()) {
            PcChatOpen();
            SDL_StartTextInput();
        }
        else if (e.type == SDL_KEYDOWN && !e.key.repeat) {
            if (e.key.keysym.scancode == SDL_SCANCODE_ESCAPE && PcVideoIsEmbedded()) {
                PcVideoRequestLauncherExit();
            } else if (e.key.keysym.scancode == SDL_SCANCODE_F11) {
                gPcSettings.fullscreen ^= 1;
                PcSettingsApplyVideo();
                PcSettingsSave();
            } else if (e.key.keysym.scancode == SDL_SCANCODE_F2) {
                gPcSettings.filter = (gPcSettings.filter + 1) % PC_FILTER_COUNT;
                PcSettingsSave();
            }
        }
        else if (e.type == SDL_CONTROLLERDEVICEADDED && !pad) pad = SDL_GameControllerOpen(e.cdevice.which);
        else if (e.type == SDL_CONTROLLERDEVICEREMOVED && pad) { SDL_GameControllerClose(pad); pad = NULL; }
    }

    keys = SDL_GetKeyboardState(NULL);
    if (PcChatActive()) {   /* typing: the game gets no keyboard input */
        *fast_forward = 0;
        return 0x03FF;
    }
    *fast_forward = keys[SDL_SCANCODE_TAB] ||
                    (pad && SDL_GameControllerGetAxis(pad, SDL_CONTROLLER_AXIS_TRIGGERRIGHT) > 16000);
    for (size_t i = 0; i < sizeof keymap / sizeof keymap[0]; i++)
        if (keys[keymap[i].key]) held |= keymap[i].bit;
    if (pad) {
        for (size_t i = 0; i < sizeof padmap / sizeof padmap[0]; i++)
            if (SDL_GameControllerGetButton(pad, padmap[i].button)) held |= padmap[i].bit;
        int lx = SDL_GameControllerGetAxis(pad, SDL_CONTROLLER_AXIS_LEFTX);
        int ly = SDL_GameControllerGetAxis(pad, SDL_CONTROLLER_AXIS_LEFTY);
        if (lx > 16000) held |= KEY_RIGHT;
        if (lx < -16000) held |= KEY_LEFT;
        if (ly > 16000) held |= KEY_DOWN;
        if (ly < -16000) held |= KEY_UP;
    }
    /* A real d-pad can't press opposite directions at once. */
    if ((held & (KEY_LEFT | KEY_RIGHT)) == (KEY_LEFT | KEY_RIGHT)) held &= ~(KEY_LEFT | KEY_RIGHT);
    if ((held & (KEY_UP | KEY_DOWN)) == (KEY_UP | KEY_DOWN)) held &= ~(KEY_UP | KEY_DOWN);
    return (uint16_t)(~held & 0x03FF);
}
