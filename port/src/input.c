/* input.c — keyboard and controllers to the GBA keypad (KEYINPUT, active-low).
 *
 * What each button is on the keyboard and on a controller comes from the
 * settings (gPcSettings.key_bind / pad_bind), remapped in the launcher or in
 * game on the controls panel (F1, or hold Back + Start). Any number of
 * controllers can be plugged in or out while playing; all of them drive the
 * game. Keys and buttons still held when the chat line or the controls panel
 * closes are ignored until they're let go, so the Enter that sends a chat
 * message doesn't also press START. */
#include <SDL.h>

#include "pc_port.h"
#include "pc_settings.h"
#include "pc_chat.h"
#include "pc_controls.h"
#include "pc_net.h"

enum {
    KEY_A = 0x001, KEY_B = 0x002, KEY_SELECT = 0x004, KEY_START = 0x008,
    KEY_RIGHT = 0x010, KEY_LEFT = 0x020, KEY_UP = 0x040, KEY_DOWN = 0x080,
    KEY_R = 0x100, KEY_L = 0x200,
};

static const uint16_t bind_bit[PC_BIND_COUNT] = {
    KEY_A, KEY_B, KEY_SELECT, KEY_START, KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT, KEY_L, KEY_R, 0,
};

#define MAX_PADS 4
#define AXIS_ON 16000

static SDL_GameController *pads[MAX_PADS];
static uint8_t key_swallow[SDL_NUM_SCANCODES];
static uint8_t pad_swallow[PC_PAD_MAX + 1];
static uint8_t trigger_was[2];

static void pad_add(int device)
{
    SDL_JoystickID id = SDL_JoystickGetDeviceInstanceID(device);
    int i;
    for (i = 0; i < MAX_PADS; i++)
        if (pads[i] && SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(pads[i])) == id) return;
    for (i = 0; i < MAX_PADS; i++)
        if (!pads[i]) { pads[i] = SDL_GameControllerOpen(device); return; }
}

static void pad_remove(SDL_JoystickID id)
{
    for (int i = 0; i < MAX_PADS; i++)
        if (pads[i] && SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(pads[i])) == id) {
            SDL_GameControllerClose(pads[i]);
            pads[i] = NULL;
        }
}

/* a controller binding on any pad: a button, or a trigger pulled past halfway */
static int pad_down(int b)
{
    for (int i = 0; i < MAX_PADS; i++) {
        if (!pads[i]) continue;
        if (b == PC_PAD_LT || b == PC_PAD_RT) {
            int axis = b == PC_PAD_LT ? SDL_CONTROLLER_AXIS_TRIGGERLEFT : SDL_CONTROLLER_AXIS_TRIGGERRIGHT;
            if (SDL_GameControllerGetAxis(pads[i], axis) > AXIS_ON) return 1;
        } else if (b >= 0 && b < SDL_CONTROLLER_BUTTON_MAX && SDL_GameControllerGetButton(pads[i], b)) {
            return 1;
        }
    }
    return 0;
}

/* Everything held right now is ignored until it's released. */
static void swallow_held(void)
{
    const Uint8 *keys = SDL_GetKeyboardState(NULL);
    for (int i = 0; i < SDL_NUM_SCANCODES; i++) key_swallow[i] = keys[i];
    for (int b = 0; b <= PC_PAD_MAX; b++) pad_swallow[b] = pad_down(b);
}

static int key_held(int sc)
{
    const Uint8 *keys = SDL_GetKeyboardState(NULL);
    if (sc <= 0 || sc >= SDL_NUM_SCANCODES) return 0;
    if (!keys[sc]) key_swallow[sc] = 0;
    return keys[sc] && !key_swallow[sc];
}

static int button_held(int b)
{
    int down;
    if (b < 0 || b > PC_PAD_MAX) return 0;
    down = pad_down(b);
    if (!down) pad_swallow[b] = 0;
    return down && !pad_swallow[b];
}

static void toggle_panel(void)
{
    if (PcChatActive()) return;
    PcControlsToggle();
    if (!PcControlsActive()) swallow_held();
}

static void chat_close(void)
{
    SDL_StopTextInput();
    swallow_held();
}

uint16_t PcInputPoll(int *quit, int *fast_forward)
{
    SDL_Event e;
    uint16_t held = 0;

    PcVideoKeepFocus();
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) *quit = 1;
        else if (e.type == SDL_CONTROLLERDEVICEADDED) pad_add(e.cdevice.which);
        else if (e.type == SDL_CONTROLLERDEVICEREMOVED) pad_remove(e.cdevice.which);
        else if (PcChatActive() && (e.type == SDL_TEXTINPUT || e.type == SDL_KEYDOWN)) {
            /* online chat input line: the keyboard types, the game sees nothing */
            if (e.type == SDL_TEXTINPUT) PcChatTypeUtf8(e.text.text);
            else if (e.key.keysym.scancode == SDL_SCANCODE_BACKSPACE) PcChatBackspace();
            else if (e.key.keysym.scancode == SDL_SCANCODE_RETURN || e.key.keysym.scancode == SDL_SCANCODE_KP_ENTER) {
                PcChatSubmit();
                chat_close();
            } else if (e.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
                PcChatCancel();
                chat_close();
            }
        }
        else if (PcControlsActive()) {
            int was_open = 1;
            if (e.type == SDL_KEYDOWN && !e.key.repeat) PcControlsKey(e.key.keysym.scancode);
            else if (e.type == SDL_CONTROLLERBUTTONDOWN) PcControlsPad(e.cbutton.button);
            else if (e.type == SDL_CONTROLLERAXISMOTION &&
                     (e.caxis.axis == SDL_CONTROLLER_AXIS_TRIGGERLEFT || e.caxis.axis == SDL_CONTROLLER_AXIS_TRIGGERRIGHT)) {
                int t = e.caxis.axis == SDL_CONTROLLER_AXIS_TRIGGERRIGHT, on = e.caxis.value > AXIS_ON;
                if (on && !trigger_was[t]) PcControlsPad(t ? PC_PAD_RT : PC_PAD_LT);
                trigger_was[t] = (uint8_t)on;
            } else {
                was_open = 0;
            }
            if (was_open && !PcControlsActive()) swallow_held();   /* closed it */
        }
        else if (e.type == SDL_KEYDOWN && !e.key.repeat && e.key.keysym.scancode == SDL_SCANCODE_T && PcNetConnected()) {
            PcChatOpen();
            SDL_StartTextInput();
        }
        else if (e.type == SDL_KEYDOWN && !e.key.repeat) {
            if (e.key.keysym.scancode == SDL_SCANCODE_ESCAPE && PcVideoIsEmbedded()) {
                PcVideoRequestLauncherExit();
            } else if (e.key.keysym.scancode == SDL_SCANCODE_F1) {
                toggle_panel();
            } else if (e.key.keysym.scancode == SDL_SCANCODE_F11) {
                gPcSettings.fullscreen ^= 1;
                PcSettingsApplyVideo();
                PcSettingsSave();
            } else if (e.key.keysym.scancode == SDL_SCANCODE_F2) {
                gPcSettings.filter = (gPcSettings.filter + 1) % PC_FILTER_COUNT;
                PcSettingsSave();
            }
        }
        else if (e.type == SDL_CONTROLLERBUTTONDOWN &&
                 (e.cbutton.button == SDL_CONTROLLER_BUTTON_BACK || e.cbutton.button == SDL_CONTROLLER_BUTTON_START)) {
            SDL_GameController *p = SDL_GameControllerFromInstanceID(e.cbutton.which);
            if (p && SDL_GameControllerGetButton(p, SDL_CONTROLLER_BUTTON_BACK) &&
                SDL_GameControllerGetButton(p, SDL_CONTROLLER_BUTTON_START))
                toggle_panel();
        }
    }

    if (PcChatActive() || PcControlsActive()) {   /* the game gets no input */
        *fast_forward = 0;
        return 0x03FF;
    }
    *fast_forward = key_held(gPcSettings.key_bind[PC_BIND_TURBO]) || button_held(gPcSettings.pad_bind[PC_BIND_TURBO]);
    for (int i = 0; i < PC_BIND_TURBO; i++)
        if (key_held(gPcSettings.key_bind[i]) || button_held(gPcSettings.pad_bind[i])) held |= bind_bit[i];
    for (int i = 0; i < MAX_PADS; i++) {   /* the left stick always walks too */
        if (!pads[i]) continue;
        int lx = SDL_GameControllerGetAxis(pads[i], SDL_CONTROLLER_AXIS_LEFTX);
        int ly = SDL_GameControllerGetAxis(pads[i], SDL_CONTROLLER_AXIS_LEFTY);
        if (lx > AXIS_ON) held |= KEY_RIGHT;
        if (lx < -AXIS_ON) held |= KEY_LEFT;
        if (ly > AXIS_ON) held |= KEY_DOWN;
        if (ly < -AXIS_ON) held |= KEY_UP;
    }
    /* A real d-pad can't press opposite directions at once. */
    if ((held & (KEY_LEFT | KEY_RIGHT)) == (KEY_LEFT | KEY_RIGHT)) held &= ~(KEY_LEFT | KEY_RIGHT);
    if ((held & (KEY_UP | KEY_DOWN)) == (KEY_UP | KEY_DOWN)) held &= ~(KEY_UP | KEY_DOWN);
    return (uint16_t)(~held & 0x03FF);
}
