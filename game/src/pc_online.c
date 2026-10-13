// PC port: online play — the ONLINE CLUB screen, the field side of being on
// a server (other trainers walking around, battle/trade requests), and the
// specials the Pokémon Center 2F scripts use. The network itself is the
// platform layer (port/src/net.c, pc_net.h).
//
// Battles and trades between trainers on a server run the game's own Cable
// Club code over a virtual link cable (see link.c NetLinkFrame).
#include "global.h"
#ifdef PLATFORM_PC
#include "gflib.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "event_scripts.h"
#include "fieldmap.h"
#include "field_fadetransition.h"
#include "field_player_avatar.h"
#include "menu.h"
#include "new_menu_helpers.h"
#include "overworld.h"
#include "scanline_effect.h"
#include "script.h"
#include "sound.h"
#include "strings.h"
#include "task.h"
#include "text_window.h"
#include "text_window_graphics.h"
#include "constants/event_objects.h"
#include "constants/event_object_movement.h"
#include "constants/songs.h"
#include "constants/maps.h"
#include "link.h"
#include "field_weather.h"
#include "m4a.h"
#include "battle.h"
#include "battle_setup.h"
#include "cable_club.h"
#include "trade.h"
#include "constants/battle.h"
#include "constants/cable_club.h"
#include "constants/trainers.h"
#include "naming_screen.h"
#include "pokedex.h"
#include "data.h"
#include "pokemon_storage_system.h"
#include "save.h"
#include "pc_net.h"
#include "pc_chat.h"
#include "pc_online.h"

bool32 PcFieldIsOnScreen(void); // overworld.c

// ---- text ------------------------------------------------------------------

static const u8 sText_OnlineClub[] = _("ONLINE CLUB");
static const u8 sText_ServerBrowser[] = _("SERVER BROWSER");
static const u8 sText_Registration[] = _("REGISTRATION");
static const u8 sText_HintBrowse[] = _("{DPAD_UPDOWN}PICK {A_BUTTON}OK {B_BUTTON}BACK");
static const u8 sText_HintOk[] = _("{A_BUTTON}OK");
static const u8 sText_HintWait[] = _("PLEASE WAIT…");
static const u8 sText_Connecting[] = _("Contacting the online service…");
static const u8 sText_Registering[] = _("Registering your TRAINER ID…");
static const u8 sText_Joining[] = _("Connecting to the server…");
static const u8 sText_RegisterAsk1[] = _("To play online, register this");
static const u8 sText_RegisterAsk2[] = _("trainer with the online service.");
static const u8 sText_RegisterName[] = _("ONLINE NAME");
static const u8 sText_ChangeName[] = _("CHANGE NAME");
static const u8 sText_Renaming[] = _("Changing your online name…");
static const u8 sText_Renamed[] = _("Done! Your online name is now");
static const u8 sText_RegisterYes[] = _("REGISTER");
static const u8 sText_RegisterNo[] = _("NOT NOW");
static const u8 sText_Refresh[] = _("REFRESH LIST");
static const u8 sText_Disconnect[] = _("DISCONNECT");
static const u8 sText_Back[] = _("BACK");
static const u8 sText_NoServers[] = _("No servers are open right now.");
static const u8 sText_Joined[] = _("{STR_VAR_1}");
static const u8 sText_YouAreOn[] = _("Connected!");
static const u8 sText_YouAreOn2[] = _("Other trainers on this server");
static const u8 sText_YouAreOn3[] = _("now appear around KANTO.");
static const u8 sText_LeftServer[] = _("You left the server.");
static const u8 sText_CantReach1[] = _("Couldn't reach the online");
static const u8 sText_CantReach2[] = _("service. Check your connection.");
static const u8 sText_JoinFailed1[] = _("Couldn't join that server.");
static const u8 sText_JoinFailed2[] = _("It may be full or offline.");
static const u8 sText_Registered[] = _("Registered! Your online name is");
static const u8 sText_Here[] = _("{RIGHT_ARROW}");

static const u8 sTextColor[] = {TEXT_COLOR_TRANSPARENT, TEXT_COLOR_DARK_GRAY, TEXT_COLOR_LIGHT_GRAY};
static const u8 sValueColor[] = {TEXT_COLOR_TRANSPARENT, TEXT_COLOR_LIGHT_RED, TEXT_COLOR_RED};
static const u8 sHintColor[] = {TEXT_DYNAMIC_COLOR_6, TEXT_COLOR_WHITE, TEXT_COLOR_DARK_GRAY};

// ---- charset helpers --------------------------------------------------------------

// ASCII (server names) -> the game's charset.
static void AsciiToGame(u8 *dst, const char *src, int max)
{
    int i;
    for (i = 0; i < max - 1 && src[i]; i++)
    {
        char c = src[i];
        if (c >= 'A' && c <= 'Z') dst[i] = CHAR_A + (c - 'A');
        else if (c >= 'a' && c <= 'z') dst[i] = CHAR_a + (c - 'a');
        else if (c >= '0' && c <= '9') dst[i] = CHAR_0 + (c - '0');
        else if (c == '!') dst[i] = CHAR_EXCL_MARK;
        else if (c == '?') dst[i] = CHAR_QUESTION_MARK;
        else if (c == '.') dst[i] = CHAR_PERIOD;
        else if (c == '-') dst[i] = CHAR_HYPHEN;
        else if (c == '/') dst[i] = CHAR_SLASH;
        else if (c == ',') dst[i] = CHAR_COMMA;
        else dst[i] = CHAR_SPACE;
    }
    dst[i] = EOS;
}

// "RED (4821)" (the font has no '#')
static u8 *CopyNameWithTag(u8 *dst, const u8 *name, u16 tag)
{
    u8 buf[8];
    int i;
    for (i = 0; i < PLAYER_NAME_LENGTH && name[i] != EOS; i++)
        *dst++ = name[i];
    *dst++ = CHAR_SPACE;
    *dst++ = CHAR_LEFT_PAREN;
    ConvertIntToDecimalStringN(buf, tag, STR_CONV_MODE_LEADING_ZEROS, 4);
    dst = StringCopy(dst, buf);
    *dst++ = CHAR_RIGHT_PAREN;
    *dst = EOS;
    return dst;
}

// ---- the ONLINE CLUB screen ---------------------------------------------------------------

enum { MODE_BUSY, MODE_REGISTER, MODE_SERVERS, MODE_MESSAGE };
enum { BUSY_LOGIN, BUSY_REGISTER, BUSY_JOIN, BUSY_RENAME };
enum { ROW_SERVER, ROW_REFRESH, ROW_RENAME, ROW_DISCONNECT, ROW_BACK };
enum { NAMING_NONE, NAMING_REGISTER, NAMING_RENAME };

#define VISIBLE_ROWS 7
#define WIN_HEADER 0
#define WIN_LIST   1
#define WIN_HINT   2

struct OnlineScreen
{
    u8 loadState;
    u8 mode;
    u8 busyOp;
    u8 cursor;
    u8 scroll;
    u8 rowCount;
    u8 rows[PC_NET_MAX_SERVERS + 3];
    u8 serverOf[PC_NET_MAX_SERVERS + 3];
    u8 messageExit;      // A on the message leaves the screen
    u8 messageMode;      // otherwise, the mode to go back to
    const u8 *message[3];
    u16 timer;
    u8 naming;           // coming back from the naming screen: what for
};

static EWRAM_DATA struct OnlineScreen *sOnline = NULL;
static EWRAM_DATA u8 sOnlineName[PLAYER_NAME_LENGTH + 1] = {0};   // edited on the naming screen
static EWRAM_DATA bool8 sJoinedWarp = FALSE;     // joined: leave the screen into Pallet Town
static EWRAM_DATA bool8 sShowJoinedMessage = FALSE;

static const u16 sOnlinePalette[] = INCBIN_U16("graphics/misc/option_menu.gbapal");

static const struct WindowTemplate sOnlineWindows[] =
{
    [WIN_HEADER] = { .bg = 1, .tilemapLeft = 2, .tilemapTop = 3, .width = 26, .height = 2, .paletteNum = 1, .baseBlock = 2 },
    [WIN_LIST]   = { .bg = 0, .tilemapLeft = 2, .tilemapTop = 7, .width = 26, .height = 12, .paletteNum = 1, .baseBlock = 0x36 },
    [WIN_HINT]   = { .bg = 2, .tilemapLeft = 0, .tilemapTop = 0, .width = 30, .height = 2, .paletteNum = 15, .baseBlock = 0x16e },
    DUMMY_WIN_TEMPLATE
};

static const struct BgTemplate sOnlineBgs[] =
{
    { .bg = 1, .charBaseIndex = 1, .mapBaseIndex = 30, .screenSize = 0, .paletteMode = 0, .priority = 0, .baseTile = 0 },
    { .bg = 0, .charBaseIndex = 1, .mapBaseIndex = 31, .screenSize = 0, .paletteMode = 0, .priority = 1, .baseTile = 0 },
    { .bg = 2, .charBaseIndex = 1, .mapBaseIndex = 29, .screenSize = 0, .paletteMode = 0, .priority = 2, .baseTile = 0 },
};

static void Task_OnlineScreen(u8 taskId);
static void CB2_OnlineInit(void);

static void VBlankCB_Online(void)
{
    LoadOam();
    ProcessSpriteCopyRequests();
    TransferPlttBuffer();
}

static void CB2_OnlineRun(void)
{
    RunTasks();
    AnimateSprites();
    BuildOamBuffer();
    UpdatePaletteFade();
}

// The same framed look as the OPTION screen (and its frame-type setting).
static void DrawFrames(void)
{
    FillBgTilemapBufferRect(1, 0x1B3, 1, 2, 1, 1, 3);
    FillBgTilemapBufferRect(1, 0x1B4, 2, 2, 0x1B, 1, 3);
    FillBgTilemapBufferRect(1, 0x1B5, 0x1C, 2, 1, 1, 3);
    FillBgTilemapBufferRect(1, 0x1B6, 1, 3, 1, 2, 3);
    FillBgTilemapBufferRect(1, 0x1B8, 0x1C, 3, 1, 2, 3);
    FillBgTilemapBufferRect(1, 0x1B9, 1, 5, 1, 1, 3);
    FillBgTilemapBufferRect(1, 0x1BA, 2, 5, 0x1B, 1, 3);
    FillBgTilemapBufferRect(1, 0x1BB, 0x1C, 5, 1, 1, 3);
    FillBgTilemapBufferRect(1, 0x1AA, 1, 6, 1, 1, 2);
    FillBgTilemapBufferRect(1, 0x1AB, 2, 6, 0x1A, 1, 2);
    FillBgTilemapBufferRect(1, 0x1AC, 0x1C, 6, 1, 1, 2);
    FillBgTilemapBufferRect(1, 0x1AD, 1, 7, 1, 0x10, 2);
    FillBgTilemapBufferRect(1, 0x1AF, 0x1C, 7, 1, 0x10, 2);
    FillBgTilemapBufferRect(1, 0x1B0, 1, 0x13, 1, 1, 2);
    FillBgTilemapBufferRect(1, 0x1B1, 2, 0x13, 0x1A, 1, 2);
    FillBgTilemapBufferRect(1, 0x1B2, 0x1C, 0x13, 1, 1, 2);
    CopyBgTilemapBufferToVram(1);
}

static u8 RowHeight(void)
{
    return GetFontAttribute(FONT_NORMAL, FONTATTR_MAX_LETTER_HEIGHT) - 1;
}

static void PrintHint(const u8 *hint)
{
    FillWindowPixelBuffer(WIN_HINT, PIXEL_FILL(15));
    AddTextPrinterParameterized3(WIN_HINT, FONT_SMALL, 0xE4 - GetStringWidth(FONT_SMALL, hint, 0), 0, sHintColor, 0, hint);
    PutWindowTilemap(WIN_HINT);
    CopyWindowToVram(WIN_HINT, COPYWIN_FULL);
}

static void PrintHeader(const u8 *title)
{
    FillWindowPixelBuffer(WIN_HEADER, PIXEL_FILL(1));
    AddTextPrinterParameterized(WIN_HEADER, FONT_NORMAL, title, 8, 1, TEXT_SKIP_DRAW, NULL);
    if (PcNetHasAccount() && sOnline->mode == MODE_SERVERS)
    {
        CopyNameWithTag(gStringVar1, PcNetMyName(), PcNetMyTag());
        AddTextPrinterParameterized3(WIN_HEADER, FONT_SMALL, 0xCC - GetStringWidth(FONT_SMALL, gStringVar1, 0), 3,
                                     sValueColor, TEXT_SKIP_DRAW, gStringVar1);
    }
    PutWindowTilemap(WIN_HEADER);
    CopyWindowToVram(WIN_HEADER, COPYWIN_FULL);
}

static void SetHighlight(int row)
{
    if (row < 0)
    {
        SetGpuReg(REG_OFFSET_WIN0V, 0);
        SetGpuReg(REG_OFFSET_WIN0H, 0);
        return;
    }
    u16 y = row * RowHeight() + 0x3A;
    SetGpuReg(REG_OFFSET_WIN0V, WIN_RANGE(y, y + RowHeight() + 1));
    SetGpuReg(REG_OFFSET_WIN0H, WIN_RANGE(0x10, 0xE0));
}

static void PrintLine(int line, const u8 *text, const u8 *color)
{
    AddTextPrinterParameterized3(WIN_LIST, FONT_NORMAL, 8, line * RowHeight() + 2, color, TEXT_SKIP_DRAW, text);
}

static void FinishList(void)
{
    PutWindowTilemap(WIN_LIST);
    CopyWindowToVram(WIN_LIST, COPYWIN_FULL);
}

static void BuildServerRows(void)
{
    int i, n = 0;
    for (i = 0; i < PcNetServerCount() && n < PC_NET_MAX_SERVERS; i++, n++)
    {
        sOnline->rows[n] = ROW_SERVER;
        sOnline->serverOf[n] = i;
    }
    sOnline->rows[n++] = ROW_REFRESH;
    sOnline->rows[n++] = ROW_RENAME;
    if (PcNetConnected())
        sOnline->rows[n++] = ROW_DISCONNECT;
    sOnline->rows[n++] = ROW_BACK;
    sOnline->rowCount = n;
    if (sOnline->cursor >= n)
        sOnline->cursor = n - 1;
}

static void DrawServers(void)
{
    int i;
    u8 buf[32], count[12];

    PrintHeader(sText_ServerBrowser);
    FillWindowPixelBuffer(WIN_LIST, PIXEL_FILL(1));
    if (PcNetServerCount() == 0)
        PrintLine(0, sText_NoServers, sTextColor);
    for (i = 0; i < VISIBLE_ROWS && sOnline->scroll + i < sOnline->rowCount; i++)
    {
        int row = sOnline->scroll + i;
        int line = i + (PcNetServerCount() == 0 ? 1 : 0);
        switch (sOnline->rows[row])
        {
        case ROW_SERVER:
        {
            int s = sOnline->serverOf[row];
            u8 *end;
            AsciiToGame(buf, PcNetServerName(s), 20);
            PrintLine(line, buf, sTextColor);
            end = ConvertIntToDecimalStringN(count, PcNetServerPlayers(s), STR_CONV_MODE_LEFT_ALIGN, 3);
            (void)end;
            if (PcNetConnected())
            {
                AsciiToGame(buf, PcNetServerName(s), 20);
                AsciiToGame(gStringVar2, PcNetConnectedServerName(), 20);
                if (StringCompare(buf, gStringVar2) == 0)
                    AddTextPrinterParameterized3(WIN_LIST, FONT_NORMAL, 0, line * RowHeight() + 2, sValueColor, TEXT_SKIP_DRAW, sText_Here);
            }
            AddTextPrinterParameterized3(WIN_LIST, FONT_NORMAL, 0xC0 - GetStringWidth(FONT_NORMAL, count, 0), line * RowHeight() + 2,
                                         sValueColor, TEXT_SKIP_DRAW, count);
            break;
        }
        case ROW_REFRESH:    PrintLine(line, sText_Refresh, sTextColor); break;
        case ROW_RENAME:     PrintLine(line, sText_ChangeName, sTextColor); break;
        case ROW_DISCONNECT: PrintLine(line, sText_Disconnect, sTextColor); break;
        case ROW_BACK:       PrintLine(line, sText_Back, sTextColor); break;
        }
    }
    FinishList();
    SetHighlight(sOnline->cursor - sOnline->scroll + (PcNetServerCount() == 0 ? 1 : 0));
    PrintHint(sText_HintBrowse);
}

static void DrawRegister(void)
{
    PrintHeader(sText_Registration);
    FillWindowPixelBuffer(WIN_LIST, PIXEL_FILL(1));
    PrintLine(0, sText_RegisterAsk1, sTextColor);
    PrintLine(1, sText_RegisterAsk2, sTextColor);
    PrintLine(3, sText_RegisterName, sTextColor);
    AddTextPrinterParameterized3(WIN_LIST, FONT_NORMAL, 0x80, 3 * RowHeight() + 2, sValueColor, TEXT_SKIP_DRAW, sOnlineName);
    PrintLine(5, sText_RegisterYes, sTextColor);
    PrintLine(6, sText_RegisterNo, sTextColor);
    FinishList();
    SetHighlight(sOnline->cursor == 0 ? 3 : 4 + sOnline->cursor);
    PrintHint(sText_HintBrowse);
}

static void DrawBusy(const u8 *text)
{
    PrintHeader(sText_OnlineClub);
    FillWindowPixelBuffer(WIN_LIST, PIXEL_FILL(1));
    PrintLine(2, text, sTextColor);
    FinishList();
    SetHighlight(-1);
    PrintHint(sText_HintWait);
}

static void DrawMessage(void)
{
    int i;
    PrintHeader(sText_OnlineClub);
    FillWindowPixelBuffer(WIN_LIST, PIXEL_FILL(1));
    for (i = 0; i < 3; i++)
        if (sOnline->message[i])
            PrintLine(1 + i, sOnline->message[i],
                      i == 1 && (sOnline->message[0] == sText_Registered || sOnline->message[0] == sText_Renamed) ? sValueColor : sTextColor);
    FinishList();
    SetHighlight(-1);
    PrintHint(sText_HintOk);
}

static void ShowMessage(const u8 *a, const u8 *b, const u8 *c, bool8 exit, u8 backTo)
{
    sOnline->mode = MODE_MESSAGE;
    sOnline->message[0] = a;
    sOnline->message[1] = b;
    sOnline->message[2] = c;
    sOnline->messageExit = exit;
    sOnline->messageMode = backTo;
    DrawMessage();
}

static void StartBusy(u8 op)
{
    sOnline->mode = MODE_BUSY;
    sOnline->busyOp = op;
    sOnline->timer = 0;
    DrawBusy(op == BUSY_REGISTER ? sText_Registering : op == BUSY_JOIN ? sText_Joining
           : op == BUSY_RENAME ? sText_Renaming : sText_Connecting);
}

static void ShowServers(void)
{
    sOnline->mode = MODE_SERVERS;
    BuildServerRows();
    DrawServers();
}

static void StartLogin(void)
{
    PcNetBeginLogin();
    StartBusy(BUSY_LOGIN);
}

// The online name is entered on the game's own naming screen; the ONLINE
// CLUB screen is rebuilt when it returns (see CB2_OnlineInit case 5).
static void OpenNaming(u8 taskId, u8 purpose)
{
    sOnline->naming = purpose;
    BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
    gTasks[taskId].data[0] = 2;
}

static void CB2_OnlineAfterNaming(void)
{
    int i;
    for (i = 0; i <= PLAYER_NAME_LENGTH && sOnlineName[i] != EOS; i++)
        ;
    for (; i <= PLAYER_NAME_LENGTH; i++)   // a fixed 8-byte name for the server
        sOnlineName[i] = EOS;
    sOnline->loadState = 0;
    SetMainCallback2(CB2_OnlineInit);
}

static void LeaveScreen(u8 taskId)
{
    BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
    gTasks[taskId].data[0] = 1;
}

static void HandleBusy(void)
{
    int status = PcNetOpStatus();
    if (status == PC_NET_OP_PENDING)
        return;
    switch (sOnline->busyOp)
    {
    case BUSY_LOGIN:
        if (status == PC_NET_OP_OK)
            ShowServers();
        else
            ShowMessage(sText_CantReach1, sText_CantReach2, NULL, TRUE, MODE_SERVERS);
        break;
    case BUSY_REGISTER:
        if (status == PC_NET_OP_OK)
        {
            static u8 sNameTag[20];
            CopyNameWithTag(sNameTag, PcNetMyName(), PcNetMyTag());
            PlaySE(SE_SUCCESS);
            ShowMessage(sText_Registered, sNameTag, NULL, FALSE, MODE_SERVERS);
            BuildServerRows();
        }
        else
        {
            ShowMessage(sText_CantReach1, sText_CantReach2, NULL, TRUE, MODE_SERVERS);
        }
        break;
    case BUSY_RENAME:
        if (status == PC_NET_OP_OK)
        {
            static u8 sNewNameTag[20];
            CopyNameWithTag(sNewNameTag, PcNetMyName(), PcNetMyTag());
            PlaySE(SE_SUCCESS);
            ShowMessage(sText_Renamed, sNewNameTag, NULL, FALSE, MODE_SERVERS);
        }
        else
        {
            ShowMessage(sText_CantReach1, sText_CantReach2, NULL, FALSE, MODE_SERVERS);
        }
        break;
    case BUSY_JOIN:
        if (status == PC_NET_OP_OK)
        {
            PlaySE(SE_SUCCESS);
            gSpecialVar_Result = TRUE;
            sJoinedWarp = TRUE;
            ShowMessage(sText_YouAreOn, sText_YouAreOn2, sText_YouAreOn3, TRUE, MODE_SERVERS);
        }
        else
        {
            PlaySE(SE_FAILURE);
            ShowMessage(sText_JoinFailed1, sText_JoinFailed2, NULL, FALSE, MODE_SERVERS);
        }
        break;
    }
}

static void HandleServerInput(u8 taskId)
{
    if (JOY_REPT(DPAD_UP) || JOY_REPT(DPAD_DOWN))
    {
        if (JOY_REPT(DPAD_UP))
            sOnline->cursor = sOnline->cursor == 0 ? sOnline->rowCount - 1 : sOnline->cursor - 1;
        else
            sOnline->cursor = sOnline->cursor + 1 >= sOnline->rowCount ? 0 : sOnline->cursor + 1;
        if (sOnline->cursor < sOnline->scroll)
            sOnline->scroll = sOnline->cursor;
        else if (sOnline->cursor >= sOnline->scroll + VISIBLE_ROWS)
            sOnline->scroll = sOnline->cursor - VISIBLE_ROWS + 1;
        PlaySE(SE_SELECT);
        DrawServers();
    }
    else if (JOY_NEW(B_BUTTON))
    {
        PlaySE(SE_SELECT);
        LeaveScreen(taskId);
    }
    else if (JOY_NEW(A_BUTTON))
    {
        PlaySE(SE_SELECT);
        switch (sOnline->rows[sOnline->cursor])
        {
        case ROW_SERVER:
            PcNetBeginJoin(sOnline->serverOf[sOnline->cursor], gSaveBlock2Ptr->playerGender);
            StartBusy(BUSY_JOIN);
            break;
        case ROW_REFRESH:
            StartLogin();
            break;
        case ROW_RENAME:
            memcpy(sOnlineName, PcNetMyName(), sizeof(sOnlineName));
            OpenNaming(taskId, NAMING_RENAME);
            break;
        case ROW_DISCONNECT:
            PcNetDisconnect();
            BuildServerRows();
            ShowMessage(sText_LeftServer, NULL, NULL, FALSE, MODE_SERVERS);
            break;
        case ROW_BACK:
            LeaveScreen(taskId);
            break;
        }
    }
}

static void Task_OnlineScreen(u8 taskId)
{
    if (gTasks[taskId].data[0] == 2)   // off to the naming screen
    {
        if (!gPaletteFade.active)
        {
            FreeAllWindowBuffers();
            DestroyTask(taskId);
            DoNamingScreen(NAMING_SCREEN_PLAYER, sOnlineName, gSaveBlock2Ptr->playerGender, 0, 0, CB2_OnlineAfterNaming);
        }
        return;
    }
    if (gTasks[taskId].data[0])   // leaving: wait for the fade, then back to the field
    {
        if (!gPaletteFade.active)
        {
            FreeAllWindowBuffers();
            FREE_AND_SET_NULL(sOnline);
            DestroyTask(taskId);
            if (sJoinedWarp)
            {
                // Everyone on a server starts out in Pallet Town, in front of
                // the player's house; the welcome message plays on arrival.
                sJoinedWarp = FALSE;
                sShowJoinedMessage = TRUE;
                SetWarpDestination(MAP_GROUP(MAP_PALLET_TOWN), MAP_NUM(MAP_PALLET_TOWN), -1, 6, 8);
                WarpIntoMap();
                gFieldCallback = FieldCB_DefaultWarpExit;
                SetMainCallback2(CB2_LoadMap);
            }
            else
            {
                SetMainCallback2(gMain.savedCallback);
            }
        }
        return;
    }
    if (gPaletteFade.active)
        return;
    switch (sOnline->mode)
    {
    case MODE_BUSY:
        HandleBusy();
        break;
    case MODE_REGISTER:   // rows: 0 ONLINE NAME, 1 REGISTER, 2 NOT NOW
        if (JOY_NEW(DPAD_UP) || JOY_NEW(DPAD_DOWN))
        {
            sOnline->cursor = JOY_NEW(DPAD_UP) ? (sOnline->cursor + 2) % 3 : (sOnline->cursor + 1) % 3;
            PlaySE(SE_SELECT);
            DrawRegister();
        }
        else if (JOY_NEW(B_BUTTON) || (JOY_NEW(A_BUTTON) && sOnline->cursor == 2))
        {
            PlaySE(SE_SELECT);
            LeaveScreen(taskId);
        }
        else if (JOY_NEW(A_BUTTON) && sOnline->cursor == 0)
        {
            PlaySE(SE_SELECT);
            OpenNaming(taskId, NAMING_REGISTER);
        }
        else if (JOY_NEW(A_BUTTON))
        {
            PlaySE(SE_SELECT);
            PcNetBeginRegister(sOnlineName);
            StartBusy(BUSY_REGISTER);
        }
        break;
    case MODE_SERVERS:
        HandleServerInput(taskId);
        break;
    case MODE_MESSAGE:
        if (JOY_NEW(A_BUTTON) || JOY_NEW(B_BUTTON))
        {
            PlaySE(SE_SELECT);
            if (sOnline->messageExit)
                LeaveScreen(taskId);
            else
                ShowServers();
        }
        break;
    }
}

static void CB2_OnlineInit(void)
{
    switch (sOnline->loadState)
    {
    case 0:
        SetVBlankCallback(NULL);
        SetHBlankCallback(NULL);
        break;
    case 1:
        DmaClearLarge16(3, (void *)VRAM, VRAM_SIZE, 0x1000);
        DmaClear32(3, (void *)OAM, OAM_SIZE);
        DmaClear16(3, (void *)PLTT, PLTT_SIZE);
        SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_MODE_0);
        ResetBgsAndClearDma3BusyFlags(0);
        InitBgsFromTemplates(0, sOnlineBgs, NELEMS(sOnlineBgs));
        ChangeBgX(0, 0, 0); ChangeBgY(0, 0, 0);
        ChangeBgX(1, 0, 0); ChangeBgY(1, 0, 0);
        ChangeBgX(2, 0, 0); ChangeBgY(2, 0, 0);
        InitWindows(sOnlineWindows);
        DeactivateAllTextPrinters();
        SetGpuReg(REG_OFFSET_BLDCNT, BLDCNT_TGT1_BG0 | BLDCNT_EFFECT_BLEND | BLDCNT_EFFECT_LIGHTEN);
        SetGpuReg(REG_OFFSET_BLDY, BLDCNT_TGT1_BG1);
        SetGpuReg(REG_OFFSET_WININ, WININ_WIN0_BG0);
        SetGpuReg(REG_OFFSET_WINOUT, WINOUT_WIN01_BG0 | WINOUT_WIN01_BG1 | WINOUT_WIN01_BG2 | WINOUT_WIN01_CLR);
        SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_OBJ_1D_MAP | DISPCNT_OBJ_ON | DISPCNT_WIN0_ON);
        ShowBg(0);
        ShowBg(1);
        ShowBg(2);
        break;
    case 2:
        ResetSpriteData();
        ResetPaletteFade();
        FreeAllSpritePalettes();
        ResetTasks();
        ScanlineEffect_Stop();
        break;
    case 3:
        LoadBgTiles(1, GetUserWindowGraphics(gSaveBlock2Ptr->optionsWindowFrameType)->tiles, 0x120, 0x1AA);
        LoadPalette(GetUserWindowGraphics(gSaveBlock2Ptr->optionsWindowFrameType)->palette, BG_PLTT_ID(2), PLTT_SIZE_4BPP);
        LoadPalette(sOnlinePalette, BG_PLTT_ID(1), sizeof(sOnlinePalette));
        LoadPalette(GetTextWindowPalette(2), BG_PLTT_ID(15), PLTT_SIZE_4BPP);
        LoadStdWindowGfxOnBg(1, 0x1B3, BG_PLTT_ID(3));
        break;
    case 4:
        DrawFrames();
        break;
    case 5:
        if (sOnline->naming == NAMING_REGISTER)
        {
            sOnline->naming = NAMING_NONE;
            if (sOnlineName[0] == EOS)
                StringCopy(sOnlineName, gSaveBlock2Ptr->playerName);
            sOnline->mode = MODE_REGISTER;
            sOnline->cursor = 1;   // straight on to REGISTER
            DrawRegister();
        }
        else if (sOnline->naming == NAMING_RENAME)
        {
            sOnline->naming = NAMING_NONE;
            if (sOnlineName[0] != EOS && memcmp(sOnlineName, PcNetMyName(), sizeof(sOnlineName)) != 0)
            {
                PcNetBeginRename(sOnlineName);
                StartBusy(BUSY_RENAME);
            }
            else
            {
                ShowServers();
            }
        }
        else if (!PcNetHasAccount())
        {
            StringCopy(sOnlineName, gSaveBlock2Ptr->playerName);   // the trainer name to start with
            sOnline->mode = MODE_REGISTER;
            sOnline->cursor = 0;
            DrawRegister();
        }
        else
        {
            StartLogin();
        }
        break;
    default:
        BeginNormalPaletteFade(PALETTES_ALL, 0, 16, 0, RGB_BLACK);
        SetVBlankCallback(VBlankCB_Online);
        CreateTask(Task_OnlineScreen, 0);
        SetMainCallback2(CB2_OnlineRun);
        return;
    }
    sOnline->loadState++;
}

// special Online_OpenScreen (after fadescreen FADE_TO_BLACK; then waitstate).
// VAR_RESULT = TRUE if the player joined a server.
void Online_OpenScreen(void)
{
    sOnline = AllocZeroed(sizeof(*sOnline));
    gSpecialVar_Result = FALSE;
    gMain.savedCallback = CB2_ReturnToFieldContinueScriptPlayMapMusic;
    SetMainCallback2(CB2_OnlineInit);
}

// ---- specials for the 2F and for other trainers -----------------------------------------------

void Online_IsConnected(void)
{
    gSpecialVar_Result = PcNetConnected();
}

void Online_BufferServerName(void)
{
    AsciiToGame(gStringVar1, PcNetConnectedServerName(), 20);
}

static bool8 InCableClubRoom(void)
{
    u16 map = (gSaveBlock1Ptr->location.mapGroup << 8) | gSaveBlock1Ptr->location.mapNum;
    return map == MAP_TRADE_CENTER || map == MAP_BATTLE_COLOSSEUM_2P || map == MAP_BATTLE_COLOSSEUM_4P
        || map == MAP_RECORD_CORNER || map == MAP_UNION_ROOM;
}

// ---- other trainers in the field --------------------------------------------------------------

#define AVATAR_LOCALID_BASE 0xE0
#define MAX_AVATARS 10

static EWRAM_DATA s8 sAvatarSlot[MAX_AVATARS] = {0};    // net player slot + 1, 0 = free
static EWRAM_DATA u8 sAvatarMapGroup = 0, sAvatarMapNum = 0;
static EWRAM_DATA u16 sTargetPlayerId = 0;
static EWRAM_DATA u8 sTargetKind = 0;
static EWRAM_DATA u16 sPosTimer = 0;
static EWRAM_DATA struct { u8 group, num; s16 x, y; u8 facing, flags; } sLastPos = {0};
static EWRAM_DATA u16 sIncomingFrom = 0;
static EWRAM_DATA u8 sIncomingKind = 0;
static EWRAM_DATA u16 sIncomingWait = 0;

static const struct PcNetPlayer *AvatarPlayer(int a)
{
    return sAvatarSlot[a] ? PcNetGetPlayer(sAvatarSlot[a] - 1) : NULL;
}

static void RemoveAvatar(int a)
{
    RemoveObjectEventByLocalIdAndMap(AVATAR_LOCALID_BASE + a, sAvatarMapNum, sAvatarMapGroup);
    sAvatarSlot[a] = 0;
}

static void StepAvatar(struct ObjectEvent *obj, const struct PcNetPlayer *p)
{
    s16 x = obj->currentCoords.x - MAP_OFFSET, y = obj->currentCoords.y - MAP_OFFSET;
    s16 dx = p->x - x, dy = p->y - y;
    u8 dir;

    if (!ObjectEventClearHeldMovementIfFinished(obj) && obj->heldMovementActive)
        return;
    if (dx == 0 && dy == 0)
    {
        if (obj->facingDirection != p->facing && p->facing >= DIR_SOUTH && p->facing <= DIR_EAST)
            ObjectEventSetHeldMovement(obj, GetFaceDirectionMovementAction(p->facing));
        return;
    }
    if (dx > 3 || dx < -3 || dy > 3 || dy < -3)
    {
        MoveObjectEventToMapCoords(obj, p->x + MAP_OFFSET, p->y + MAP_OFFSET);
        return;
    }
    if (dx != 0)
        dir = dx > 0 ? DIR_EAST : DIR_WEST;
    else
        dir = dy > 0 ? DIR_SOUTH : DIR_NORTH;
    ObjectEventSetHeldMovement(obj, (p->flags & 1) ? GetWalkFastMovementAction(dir) : GetWalkNormalMovementAction(dir));
}

static void UpdateAvatars(void)
{
    int a, s;
    u8 group = gSaveBlock1Ptr->location.mapGroup, num = gSaveBlock1Ptr->location.mapNum;

    if (group != sAvatarMapGroup || num != sAvatarMapNum)
    {
        // a new map: its object events were rebuilt without ours
        memset(sAvatarSlot, 0, sizeof(sAvatarSlot));
        sAvatarMapGroup = group;
        sAvatarMapNum = num;
    }
    for (a = 0; a < MAX_AVATARS; a++)
    {
        const struct PcNetPlayer *p = AvatarPlayer(a);
        u8 objId;
        if (sAvatarSlot[a] && (!p || p->mapGroup != group || p->mapNum != num))
        {
            RemoveAvatar(a);
            continue;
        }
        if (p && TryGetObjectEventIdByLocalIdAndMap(AVATAR_LOCALID_BASE + a, num, group, &objId) == 0)
            StepAvatar(&gObjectEvents[objId], p);
        else if (p)
            sAvatarSlot[a] = 0;   // despawned by the game (off screen): respawn below
    }
    for (s = 0; s < PC_NET_MAX_PLAYERS; s++)
    {
        const struct PcNetPlayer *p = PcNetGetPlayer(s);
        int free = -1;
        if (!p || p->mapGroup != group || p->mapNum != num)
            continue;
        for (a = 0; a < MAX_AVATARS; a++)
        {
            if (sAvatarSlot[a] == s + 1)
                break;
            if (!sAvatarSlot[a] && free < 0)
                free = a;
        }
        if (a < MAX_AVATARS || free < 0)
            continue;
        if (SpawnSpecialObjectEventParameterized(p->gender ? OBJ_EVENT_GFX_GREEN_NORMAL : OBJ_EVENT_GFX_RED_NORMAL,
                                                 MOVEMENT_TYPE_NONE, AVATAR_LOCALID_BASE + free,
                                                 p->x + MAP_OFFSET, p->y + MAP_OFFSET,
                                                 gObjectEvents[gPlayerAvatar.objectEventId].currentElevation) < OBJECT_EVENTS_COUNT)
            sAvatarSlot[free] = s + 1;
    }
}

static void SendPresence(void)
{
    struct ObjectEvent *player = &gObjectEvents[gPlayerAvatar.objectEventId];
    u8 flags = (gPlayerAvatar.flags & PLAYER_AVATAR_FLAG_DASH) ? 1 : 0;
    s16 x = player->currentCoords.x - MAP_OFFSET, y = player->currentCoords.y - MAP_OFFSET;

    if (ArePlayerFieldControlsLocked())
        flags |= 2;   // busy (talking, in a menu)
    if (++sPosTimer < 60
     && sLastPos.group == gSaveBlock1Ptr->location.mapGroup && sLastPos.num == gSaveBlock1Ptr->location.mapNum
     && sLastPos.x == x && sLastPos.y == y && sLastPos.facing == player->facingDirection && sLastPos.flags == flags)
        return;
    sPosTimer = 0;
    sLastPos.group = gSaveBlock1Ptr->location.mapGroup;
    sLastPos.num = gSaveBlock1Ptr->location.mapNum;
    sLastPos.x = x;
    sLastPos.y = y;
    sLastPos.facing = player->facingDirection;
    sLastPos.flags = flags;
    PcNetSendPos(sLastPos.group, sLastPos.num, x, y, sLastPos.facing, flags);
}

static void BufferPlayerName(u8 *dst, u16 id)
{
    int s;
    for (s = 0; s < PC_NET_MAX_PLAYERS; s++)
    {
        const struct PcNetPlayer *p = PcNetGetPlayer(s);
        if (p && p->id == id)
        {
            CopyNameWithTag(dst, p->name, p->tag);
            return;
        }
    }
    dst[0] = EOS;
}

// Test hooks (--net-autojoin / --net-request / --net-accept): drive the
// same calls the screens and scripts make, without the menus.
static void DebugNetHooks(void)
{
    static u8 stage;
    int s;

    if (gPcDebugNet.autojoin && !PcNetConnected())
    {
        switch (stage)
        {
        case 0:
            if (PcNetHasAccount())
                PcNetBeginLogin();
            else
                PcNetBeginRegister(gSaveBlock2Ptr->playerName);
            stage = 1;
            break;
        case 1:
            if (PcNetOpStatus() == PC_NET_OP_OK)
            {
                PcNetBeginJoin(0, gSaveBlock2Ptr->playerGender);
                stage = 2;
            }
            else if (PcNetOpStatus() == PC_NET_OP_FAILED)
                stage = 0;
            break;
        case 2:
            if (PcNetOpStatus() == PC_NET_OP_FAILED)
                stage = 0;
            break;
        }
        return;
    }
    if (ArePlayerFieldControlsLocked() || ScriptContext_IsEnabled())
        return;
    if (gPcDebugNet.chat[0])
    {
        PcChatOpen();
        PcChatTypeUtf8(gPcDebugNet.chat);
        PcChatSubmit();
        gPcDebugNet.chat[0] = 0;
    }
    if (gPcDebugNet.request_kind)
    {
        for (s = 0; s < PC_NET_MAX_PLAYERS; s++)
        {
            const struct PcNetPlayer *p = PcNetGetPlayer(s);
            if (p && p->mapGroup == gSaveBlock1Ptr->location.mapGroup && p->mapNum == gSaveBlock1Ptr->location.mapNum)
            {
                sTargetPlayerId = p->id;
                gSpecialVar_0x8004 = gPcDebugNet.request_kind;
                gPcDebugNet.request_kind = 0;
                ScriptContext_SetupScript(Online_EventScript_DebugRequest);
                return;
            }
        }
    }
}

// Called every overworld frame (CB1_Overworld).
void Online_OverworldUpdate(void)
{
    u16 from;
    u8 kind;

    DebugNetHooks();

    if (!PcNetConnected())
    {
        int a;
        for (a = 0; a < MAX_AVATARS; a++)
            if (sAvatarSlot[a])
                RemoveAvatar(a);
        return;
    }
    if (!PcFieldIsOnScreen())
        return;
    {
        // A finished (or failed) battle/trade: the link is closed and the player
        // is free again outside the Cable Club rooms -> end the session.
        static u16 sIdleClosed;
        if (PcNetLinkActive() && !gReceivedRemoteLinkPlayers && !ArePlayerFieldControlsLocked()
         && !ScriptContext_IsEnabled() && !InCableClubRoom())
        {
            if (++sIdleClosed > 120)
                PcNetLinkEnd();
        }
        else
        {
            sIdleClosed = 0;
        }
    }
    SendPresence();
    UpdateAvatars();

    if (sShowJoinedMessage && !ArePlayerFieldControlsLocked() && !ScriptContext_IsEnabled())
    {
        sShowJoinedMessage = FALSE;
        ScriptContext_SetupScript(Online_EventScript_JoinedMessage);
    }
    if (PcNetPollIncoming(&from, &kind))
    {
        sIncomingFrom = from;
        sIncomingKind = kind;
        sIncomingWait = 1;
    }
    if (sIncomingWait)
    {
        if (!ArePlayerFieldControlsLocked() && !ScriptContext_IsEnabled())
        {
            sIncomingWait = 0;
            sTargetPlayerId = sIncomingFrom;
            sTargetKind = sIncomingKind;
            BufferPlayerName(gStringVar1, sIncomingFrom);
            gSpecialVar_0x8006 = sIncomingKind;
            ScriptContext_SetupScript(gPcDebugNet.accept ? Online_EventScript_DebugAccept : Online_EventScript_IncomingRequest);
        }
        else if (++sIncomingWait > 600)   // busy for 10 s: say no
        {
            sIncomingWait = 0;
            PcNetRespond(sIncomingFrom, FALSE);
        }
    }
}

// field_control_avatar: the script for talking to an object, or NULL.
const u8 *Online_GetAvatarScript(u8 localId)
{
    int a = localId - AVATAR_LOCALID_BASE;
    const struct PcNetPlayer *p;
    if (localId < AVATAR_LOCALID_BASE || a >= MAX_AVATARS || !(p = AvatarPlayer(a)))
        return NULL;
    sTargetPlayerId = p->id;
    return Online_EventScript_TalkToPlayer;
}

// special Online_BufferTarget: STR_VAR_1 = the trainer's name.
void Online_BufferTarget(void)
{
    BufferPlayerName(gStringVar1, sTargetPlayerId);
}

static void Task_WaitRequest(u8 taskId)
{
    int result = PcNetRequestResult();
    if (result == 0 && ++gTasks[taskId].data[0] < 60 * 30)
        return;
    gSpecialVar_Result = result == 1;
    DestroyTask(taskId);
    ScriptContext_Enable();
}

// special Online_SendRequest (VAR_0x8004 = kind; then waitstate).
// VAR_RESULT = TRUE if they accepted and the link session is up.
void Online_SendRequest(void)
{
    sTargetKind = gSpecialVar_0x8004;
    PcNetRequest(sTargetPlayerId, sTargetKind);
    CreateTask(Task_WaitRequest, 80);
}

// special Online_AcceptRequest (then waitstate). VAR_RESULT as above.
void Online_AcceptRequest(void)
{
    PcNetRespond(sTargetPlayerId, TRUE);
    CreateTask(Task_WaitRequest, 80);
}

void Online_DeclineRequest(void)
{
    PcNetRespond(sTargetPlayerId, FALSE);
}

// After the link-up, online battles and trades start right where the two
// trainers met (no walk through the Cable Club rooms) -- the same steps as
// cable_club.c's wired start tasks, which are static there.
static void Task_OnlineStartBattle(u8 taskId)
{
    s16 *data = gTasks[taskId].data;

    switch (data[0])
    {
    case 0:
        LockPlayerFieldControls();
        FadeScreen(FADE_TO_BLACK, 0);
        gLinkType = LINKTYPE_BATTLE;
        // The Colosseum spot normally sets this (left = 0, right = 1); the
        // battle matches trainers to sides by it.
        SetLocalLinkPlayerId(GetMultiplayerId());
        ClearLinkCallback_2();
        data[0]++;
        break;
    case 1:
        if (!gPaletteFade.active && ++data[1] > 20)
            data[0]++;
        break;
    case 2:
        SetCloseLinkCallback();
        data[0]++;
        break;
    case 3:
        if (gReceivedRemoteLinkPlayers)
            break;
        if (gLinkPlayers[0].trainerId & 1)
            PlayMapChosenOrBattleBGM(MUS_RS_VS_GYM_LEADER);
        else
            PlayMapChosenOrBattleBGM(MUS_RS_VS_TRAINER);
        gBattleTypeFlags = BATTLE_TYPE_TRAINER | BATTLE_TYPE_LINK;
        if (gSpecialVar_0x8004 == USING_DOUBLE_BATTLE)
            gBattleTypeFlags |= BATTLE_TYPE_DOUBLE;
        CleanupOverworldWindowsAndTilemaps();
        gTrainerBattleOpponent_A = TRAINER_LINK_OPPONENT;
        SetMainCallback2(CB2_InitBattle);
        gMain.savedCallback = CB2_ReturnFromCableClubBattle;
        DestroyTask(taskId);
        break;
    }
}

static void Task_OnlineStartTrade(u8 taskId)
{
    s16 *data = gTasks[taskId].data;

    switch (data[0])
    {
    case 0:
        LockPlayerFieldControls();
        FadeScreen(FADE_TO_BLACK, 0);
        SetLocalLinkPlayerId(GetMultiplayerId());   // the Trade Center seat would set this
        ClearLinkCallback_2();
        data[0]++;
        break;
    case 1:
        if (!gPaletteFade.active)
            data[0]++;
        break;
    case 2:
        gSelectedTradeMonPositions[TRADE_PLAYER] = 0;
        gSelectedTradeMonPositions[TRADE_PARTNER] = 0;
        m4aMPlayAllStop();
        SetCloseLinkCallback();
        data[0]++;
        break;
    case 3:
        if (!gReceivedRemoteLinkPlayers)
        {
            SetMainCallback2(CB2_StartCreateTradeMenu);
            DestroyTask(taskId);
        }
        break;
    }
}

// special Online_StartBattle / Online_StartTrade (after a successful link-up;
// then waitstate -- the activity takes over and returns to the field).
void Online_StartBattle(void)
{
    CreateTask(Task_OnlineStartBattle, 80);
}

void Online_StartTrade(void)
{
    CreateTask(Task_OnlineStartTrade, 80);
}

// special Online_SessionKind: VAR_RESULT = PC_NET_KIND_* of the session.
void Online_SessionKind(void)
{
    gSpecialVar_Result = PcNetLinkKind();
}

// ---- global trade: Wonder Trade + mailbox (GTS screen: Online_OpenGts) ----------

enum { MSG_WONDER = 0x08, MSG_MAIL_COLLECT = 0x09 };
enum { ERR_ILLEGAL_MON = 6 };
u8 PcGiveTradedMon(struct Pokemon *mon);   // pokemon.c

STATIC_ASSERT(sizeof(struct Pokemon) == 100, PokemonIs100BytesOnTheWire);

static void RegisterReceivedMon(struct Pokemon *mon)
{
    u16 species = GetMonData(mon, MON_DATA_SPECIES, NULL);
    u16 dexNum = SpeciesToNationalPokedexNum(species);
    GetSetPokedexFlag(dexNum, FLAG_SET_SEEN);
    GetSetPokedexFlag(dexNum, FLAG_SET_CAUGHT);
    StringCopy(gStringVar1, gSpeciesNames[species]);
}

static bool8 HasRoomForMon(void)
{
    int box, pos;
    if (CalculatePlayerPartyCount() < PARTY_SIZE)
        return TRUE;
    for (box = 0; box < TOTAL_BOXES_COUNT; box++)
        for (pos = 0; pos < IN_BOX_COUNT; pos++)
            if (GetBoxMonDataAt(box, pos, MON_DATA_SPECIES) == SPECIES_NONE)
                return TRUE;
    return FALSE;
}

void Online_HasAccount(void)
{
    gSpecialVar_Result = PcNetHasAccount();
}

// VAR_RESULT: 0 failed, 1 traded (STR_VAR_1 = received), 2 deposit waiting, 3 not legal.
// The game is saved right after any change, so a reload can't duplicate a Pokémon.
static void Task_WonderTrade(u8 taskId)
{
    u8 slot = gTasks[taskId].data[0];
    int status = PcNetOpStatus();
    u8 type;
    const u8 *reply;

    if (status == PC_NET_OP_PENDING)
        return;
    if (status == PC_NET_OP_OK && PcNetMasterReply(&type, &reply) >= 1 + (int)sizeof(struct Pokemon))
    {
        if (reply[0] == 1)
        {
            memcpy(&gPlayerParty[slot], reply + 1, sizeof(struct Pokemon));
            RegisterReceivedMon(&gPlayerParty[slot]);
            gSpecialVar_Result = 1;
        }
        else
        {
            ZeroMonData(&gPlayerParty[slot]);
            CompactPartySlots();
            CalculatePlayerPartyCount();
            gSpecialVar_Result = 2;
        }
        TrySavingData(SAVE_NORMAL);
    }
    else
    {
        gSpecialVar_Result = PcNetMasterError() == ERR_ILLEGAL_MON ? 3 : 0;
    }
    DestroyTask(taskId);
    ScriptContext_Enable();
}

// special Online_WonderTrade (VAR_0x8004 = party slot; then waitstate)
void Online_WonderTrade(void)
{
    u8 taskId = CreateTask(Task_WonderTrade, 80);
    gTasks[taskId].data[0] = gSpecialVar_0x8004;
    PcNetMasterRequest(MSG_WONDER, &gPlayerParty[gSpecialVar_0x8004], sizeof(struct Pokemon));
}

// VAR_RESULT: 0 failed, 1 received (STR_VAR_1; VAR_0x8005 = 1 if it went to the PC),
// 2 nothing waiting, 4 no room.
static void Task_CollectMail(u8 taskId)
{
    int status = PcNetOpStatus();
    u8 type;
    const u8 *reply;

    if (status == PC_NET_OP_PENDING)
        return;
    gSpecialVar_0x8005 = 0;
    if (status == PC_NET_OP_OK && PcNetMasterReply(&type, &reply) >= 3 + (int)sizeof(struct Pokemon))
    {
        if (reply[1])
        {
            struct Pokemon mon;
            memcpy(&mon, reply + 3, sizeof(mon));
            gSpecialVar_0x8005 = PcGiveTradedMon(&mon) == MON_GIVEN_TO_PC;
            RegisterReceivedMon(&mon);
            TrySavingData(SAVE_NORMAL);
            gSpecialVar_Result = 1;
        }
        else
        {
            gSpecialVar_Result = 2;
        }
    }
    else
    {
        gSpecialVar_Result = 0;
    }
    DestroyTask(taskId);
    ScriptContext_Enable();
}

// special Online_CollectMail (then waitstate): one Pokémon per call.
void Online_CollectMail(void)
{
    if (!HasRoomForMon())
    {
        gSpecialVar_Result = 4;   // checked before asking, so nothing is taken from the server
        return;
    }
    CreateTask(Task_CollectMail, 80);
    PcNetMasterRequest(MSG_MAIL_COLLECT, NULL, 0);
}

// special Online_OpenGts (after fadescreen; then waitstate) -- the GTS screen.
void Online_OpenGts(void)
{
    SetMainCallback2(CB2_ReturnToFieldContinueScriptPlayMapMusic);
}

#else
// The ARM (GBA) build has no network; the scripts are never reached there.
void Online_OpenScreen(void) {}
void Online_IsConnected(void) {}
void Online_BufferServerName(void) {}
void Online_BufferTarget(void) {}
void Online_SendRequest(void) {}
void Online_AcceptRequest(void) {}
void Online_DeclineRequest(void) {}
void Online_SessionKind(void) {}
void Online_StartBattle(void) {}
void Online_StartTrade(void) {}
void Online_HasAccount(void) {}
void Online_WonderTrade(void) {}
void Online_CollectMail(void) {}
void Online_OpenGts(void) {}
#endif
