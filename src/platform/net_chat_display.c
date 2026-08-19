#include "global.h"
#include "gflib.h"
#include "menu.h"
#include "new_menu_helpers.h"
#include "text.h"
#include "window.h"
#include "string_util.h"

#define CHAT_WINDOW_WIDTH  28
#define CHAT_WINDOW_HEIGHT  4
#define CHAT_DISPLAY_TICKS 300

static const struct WindowTemplate sChatWindowTemplate = {
    .bg = 0,
    .tilemapLeft = 1,
    .tilemapTop = 15,
    .width = CHAT_WINDOW_WIDTH,
    .height = CHAT_WINDOW_HEIGHT,
    .paletteNum = 15,
    .baseBlock = 0x200,
};

static u8 sChatWindowId = 0xFF;
static int sChatTimer = 0;

static void AsciiToGameStr(u8 *dst, const char *src, int maxLen)
{
    int i = 0;
    while (*src && i < maxLen - 1)
    {
        unsigned char c = (unsigned char)*src++;
        if (c >= 'A' && c <= 'Z')
            dst[i] = 0xBB + (c - 'A');
        else if (c >= 'a' && c <= 'z')
            dst[i] = 0xD5 + (c - 'a');
        else if (c >= '0' && c <= '9')
            dst[i] = 0xA1 + (c - '0');
        else if (c == ' ')
            dst[i] = 0x00;
        else if (c == ':')
            dst[i] = 0xF0;
        else if (c == '!')
            dst[i] = 0xAB;
        else if (c == '?')
            dst[i] = 0xAC;
        else if (c == '.')
            dst[i] = 0xAD;
        else if (c == '-')
            dst[i] = 0xAE;
        else if (c == ',')
            dst[i] = 0xB8;
        else if (c == '\'')
            dst[i] = 0xB4;
        else if (c == '/')
            dst[i] = 0xBA;
        else
            dst[i] = 0x00;
        i++;
    }
    dst[i] = 0xFF;
}

void NetChatDisplay_ShowMessage(const char *username, const char *message)
{
    u8 gameStr[200];
    int pos = 0;

    AsciiToGameStr(gameStr, username, 20);
    while (gameStr[pos] != 0xFF) pos++;
    gameStr[pos++] = 0xF0; // ':'
    gameStr[pos++] = 0x00; // ' '
    AsciiToGameStr(&gameStr[pos], message, 170);

    if (sChatWindowId == 0xFF)
        sChatWindowId = AddWindow(&sChatWindowTemplate);

    FillWindowPixelBuffer(sChatWindowId, PIXEL_FILL(1));
    DrawDialogueFrame(sChatWindowId, FALSE);
    AddTextPrinterParameterized(sChatWindowId, FONT_NORMAL, gameStr, 0, 1, 0, NULL);
    PutWindowTilemap(sChatWindowId);
    CopyWindowToVram(sChatWindowId, COPYWIN_FULL);

    sChatTimer = CHAT_DISPLAY_TICKS;
}

void NetChatDisplay_Update(void)
{
    extern int NetClient_IsConnected(void);
    extern int NetClient_PopNewChat(char *userOut, int userMax, char *msgOut, int msgMax);
    extern struct PaletteFadeControl gPaletteFade;

    // Don't touch windows during screen fades or when not connected
    if (gPaletteFade.active)
        return;

    if (!NetClient_IsConnected())
    {
        if (sChatWindowId != 0xFF)
        {
            ClearStdWindowAndFrame(sChatWindowId, TRUE);
            RemoveWindow(sChatWindowId);
            sChatWindowId = 0xFF;
            sChatTimer = 0;
        }
        return;
    }

    char user[16], msg[128];
    if (NetClient_PopNewChat(user, sizeof(user), msg, sizeof(msg)))
        NetChatDisplay_ShowMessage(user, msg);

    if (sChatTimer > 0)
    {
        sChatTimer--;
        if (sChatTimer == 0 && sChatWindowId != 0xFF)
        {
            ClearStdWindowAndFrame(sChatWindowId, TRUE);
            RemoveWindow(sChatWindowId);
            sChatWindowId = 0xFF;
        }
    }
}

void NetChatDisplay_Hide(void)
{
    if (sChatWindowId != 0xFF)
    {
        ClearStdWindowAndFrame(sChatWindowId, TRUE);
        RemoveWindow(sChatWindowId);
        sChatWindowId = 0xFF;
        sChatTimer = 0;
    }
}
