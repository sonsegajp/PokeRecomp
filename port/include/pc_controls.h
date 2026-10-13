/* pc_controls.h — the in-game controls panel (F1, or hold Back + Start on a
 * controller): every GBA button and fast-forward, with its keyboard key and
 * its controller button. Pick one, press the new key or button, and it's
 * saved to settings.ini (the same bindings the launcher's Options > Controls
 * edits). Drawn over the game by the chat overlay. Plain C, no SDL types. */
#ifndef PC_CONTROLS_H
#define PC_CONTROLS_H

int PcControlsActive(void);
void PcControlsToggle(void);
void PcControlsKey(int scancode);   /* a key pressed while the panel is open */
void PcControlsPad(int button);     /* a controller button (or PC_PAD_LT / PC_PAD_RT) */

/* for drawing it */
int PcControlsCursor(int *column, int *waiting);   /* row; column 0 keyboard, 1 controller */
const char *PcControlsActionName(int bind);
const char *PcControlsKeyName(int bind);
const char *PcControlsPadName(int bind);

#endif
