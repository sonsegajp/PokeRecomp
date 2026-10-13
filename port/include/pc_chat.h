/* pc_chat.h — online text chat: an overlay drawn over the game in the game's
 * own dialogue font at half size (upper and lower case). T opens the input
 * line, Enter sends, Esc cancels. Plain C, no SDL types. */
#ifndef PC_CHAT_H
#define PC_CHAT_H

#include <stdint.h>

int PcChatActive(void);                       /* the input line is open (keys go to chat) */
void PcChatOpen(void);
void PcChatTypeUtf8(const char *text);        /* typed characters (SDL text input) */
void PcChatBackspace(void);
void PcChatSubmit(void);
void PcChatCancel(void);

/* A message arrived (name/text in the game's charset, 0xFF-terminated). */
void PcChatReceive(const uint8_t *name, uint16_t tag, const uint8_t *text);   /* name NULL = the server */

/* The overlay for this frame at (w, h) = 2x the game's logical size, RGBA;
 * NULL when there is nothing to show. */
const uint32_t *PcChatOverlay(int w, int h);

/* Test: if chat is showing, write the frame at 2x with the overlay on top as
 * a BMP (headless runs have no window to look at). */
void PcChatDebugShot(const uint32_t *frame, int w, int h, const char *path);

#endif
