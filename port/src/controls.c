/* controls.c — the in-game controls panel (pc_controls.h). */
#include <SDL.h>

#include "pc_controls.h"
#include "pc_settings.h"

static int open, row, column, waiting;

static const char *const action_names[PC_BIND_COUNT] = {
    "A", "B", "SELECT", "START", "UP", "DOWN", "LEFT", "RIGHT", "L", "R", "FAST-FORWARD",
};

int PcControlsActive(void) { return open; }

void PcControlsToggle(void)
{
    open = !open;
    waiting = 0;
    if (!open) PcSettingsSave();
}

int PcControlsCursor(int *col, int *wait)
{
    *col = column;
    *wait = waiting;
    return row;
}

const char *PcControlsActionName(int bind) { return action_names[bind]; }

const char *PcControlsKeyName(int bind)
{
    int sc = gPcSettings.key_bind[bind];
    const char *name = SDL_GetScancodeName((SDL_Scancode)sc);
    if (sc == SDL_SCANCODE_RETURN) return "Enter";
    return name && *name ? name : "-";
}

const char *PcControlsPadName(int bind)
{
    static const char *const names[] = {
        "A", "B", "X", "Y", "Back", "Guide", "Start", "L Stick", "R Stick", "LB", "RB",
        "D-pad Up", "D-pad Down", "D-pad Left", "D-pad Right", "Share", "Paddle 1", "Paddle 2",
        "Paddle 3", "Paddle 4", "Touchpad",
    };
    int b = gPcSettings.pad_bind[bind];
    if (b == PC_PAD_LT) return "LT";
    if (b == PC_PAD_RT) return "RT";
    return b >= 0 && b < (int)(sizeof names / sizeof names[0]) ? names[b] : "-";
}

static void move(int drow, int dcol)
{
    row = (row + drow + PC_BIND_COUNT) % PC_BIND_COUNT;
    column = (column + dcol + 2) % 2;
}

/* Whatever already had this key/button gets the old one (a swap). */
static void set_bind(int *binds, int value)
{
    for (int i = 0; i < PC_BIND_COUNT; i++)
        if (i != row && binds[i] == value)
            binds[i] = binds[row];
    binds[row] = value;
    PcSettingsSave();
}

static void reset_cell(void)
{
    if (column == 0) set_bind(gPcSettings.key_bind, gPcDefaultKeyBind[row]);
    else set_bind(gPcSettings.pad_bind, gPcDefaultPadBind[row]);
}

/* Keyboard: arrows move, Enter changes, Delete resets, Esc / F1 close. While
 * waiting for a keyboard binding, the next key is it (Esc cancels). */
void PcControlsKey(int sc)
{
    if (waiting) {
        if (sc == SDL_SCANCODE_ESCAPE) waiting = 0;
        else if (column == 0 && sc != SDL_SCANCODE_F1 && sc != SDL_SCANCODE_F2 && sc != SDL_SCANCODE_F11
                 && sc != SDL_SCANCODE_T) {   /* the panel, filter, fullscreen, chat */
            set_bind(gPcSettings.key_bind, sc);
            waiting = 0;
        }
        return;
    }
    switch (sc) {
    case SDL_SCANCODE_UP: move(-1, 0); break;
    case SDL_SCANCODE_DOWN: move(1, 0); break;
    case SDL_SCANCODE_LEFT: move(0, -1); break;
    case SDL_SCANCODE_RIGHT: move(0, 1); break;
    case SDL_SCANCODE_RETURN: case SDL_SCANCODE_KP_ENTER: waiting = 1; break;
    case SDL_SCANCODE_DELETE: reset_cell(); break;
    case SDL_SCANCODE_ESCAPE: case SDL_SCANCODE_F1: PcControlsToggle(); break;
    default: break;
    }
}

/* Controller: d-pad moves, A changes, Y resets, B closes. While waiting for a
 * controller binding, the next button is it (B cancels a keyboard wait). */
void PcControlsPad(int b)
{
    if (waiting) {
        if (column == 1) {
            set_bind(gPcSettings.pad_bind, b);
            waiting = 0;
        } else if (b == SDL_CONTROLLER_BUTTON_B) {
            waiting = 0;
        }
        return;
    }
    switch (b) {
    case SDL_CONTROLLER_BUTTON_DPAD_UP: move(-1, 0); break;
    case SDL_CONTROLLER_BUTTON_DPAD_DOWN: move(1, 0); break;
    case SDL_CONTROLLER_BUTTON_DPAD_LEFT: move(0, -1); break;
    case SDL_CONTROLLER_BUTTON_DPAD_RIGHT: move(0, 1); break;
    case SDL_CONTROLLER_BUTTON_A: waiting = 1; break;
    case SDL_CONTROLLER_BUTTON_Y: reset_cell(); break;
    case SDL_CONTROLLER_BUTTON_B: PcControlsToggle(); break;
    default: break;
    }
}
