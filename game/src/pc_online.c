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
#include "random.h"

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
static const u8 sText_GtsSearching[] = _("Searching the GTS…");

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

enum { MODE_BUSY, MODE_REGISTER, MODE_SERVERS, MODE_MESSAGE, MODE_GTS_PICK, MODE_GTS_WANT, MODE_GTS_LIST };
enum { BUSY_LOGIN, BUSY_REGISTER, BUSY_JOIN, BUSY_RENAME, BUSY_GTS_SEARCH, BUSY_GTS_MINE };
enum { GTS_NONE, GTS_DEPOSIT, GTS_SEARCH, GTS_MINE };   // the GTS screen: VAR_0x8006 + 1
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
    u8 gtsMode;          // GTS_* when this is the GTS screen
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

static const u8 *ScreenTitle(void);

static void DrawBusy(const u8 *text)
{
    PrintHeader(ScreenTitle());
    FillWindowPixelBuffer(WIN_LIST, PIXEL_FILL(1));
    PrintLine(2, text, sTextColor);
    FinishList();
    SetHighlight(-1);
    PrintHint(sText_HintWait);
}

static void DrawMessage(void)
{
    int i;
    PrintHeader(ScreenTitle());
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
           : op == BUSY_RENAME ? sText_Renaming : op == BUSY_GTS_SEARCH ? sText_GtsSearching : sText_Connecting);
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

static void GtsHandleBusy(void);

static void HandleBusy(void)
{
    int status = PcNetOpStatus();
    if (sOnline->busyOp >= BUSY_GTS_SEARCH)
    {
        GtsHandleBusy();
        return;
    }
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

// ---- the GTS screen (same look; opened by Online_OpenGts) ---------------------------
//
// DEPOSIT: say what you want for the Pokemon chosen in the party menu.
// SEARCH:  pick a species, browse its listings, choose one to offer for.
// MINE:    your own listings, choose one to take back.
// The trade itself happens back in the field script (Online_Gts* specials),
// after the save prompt, so a reload can never duplicate a Pokemon.

enum { MSG_GTS_DEPOSIT = 0x0A, MSG_GTS_SEARCH = 0x0B, MSG_GTS_OFFER = 0x0C, MSG_GTS_MINE = 0x0D, MSG_GTS_WITHDRAW = 0x0E };
enum { NET_GENDER_ANY, NET_GENDER_MALE, NET_GENDER_FEMALE, NET_GENDER_NONE };
enum { WANT_SPECIES, WANT_MIN, WANT_MAX, WANT_GENDER, WANT_OK, WANT_CANCEL, WANT_ROWS };

static const u8 sText_GtsDeposit[] = _("GTS  DEPOSIT");
static const u8 sText_GtsSearch[] = _("GTS  SEARCH");
static const u8 sText_GtsMine[] = _("GTS  YOUR POKéMON");
static const u8 sText_GtsWhich[] = _("WHICH POKéMON?");
static const u8 sText_HintPick[] = _("{DPAD_UPDOWN}PICK {DPAD_LEFTRIGHT}PAGE {A_BUTTON}OK {B_BUTTON}BACK");
static const u8 sText_HintChange[] = _("{DPAD_UPDOWN}PICK {DPAD_LEFTRIGHT}CHANGE {A_BUTTON}OK {B_BUTTON}BACK");
static const u8 sText_GtsTradeFor[] = _("Trade your POKéMON for:");
static const u8 sText_GtsWantSpecies[] = _("POKéMON");
static const u8 sText_GtsWantMin[] = _("LEVEL FROM");
static const u8 sText_GtsWantMax[] = _("LEVEL TO");
static const u8 sText_GtsWantGender[] = _("GENDER");
static const u8 sText_GtsDepositRow[] = _("DEPOSIT");
static const u8 sText_GtsCancel[] = _("CANCEL");
static const u8 sText_GtsAny[] = _("ANY");
static const u8 sText_GtsMale[] = _("♂");
static const u8 sText_GtsFemale[] = _("♀");
static const u8 sText_GtsNone[] = _("-----");
static const u8 sText_GtsEmpty[] = _("");
static const u8 sText_GtsNobody1[] = _("Nobody is offering that");
static const u8 sText_GtsNobody2[] = _("POKéMON right now.");
static const u8 sText_GtsNoneOfYours1[] = _("You have no POKéMON");
static const u8 sText_GtsNoneOfYours2[] = _("on the GTS.");
static const u8 sText_GtsFrom[] = _("FROM ");
static const u8 sText_GtsWants[] = _("WANTS ");
static const u8 sText_GtsNoSeen1[] = _("No POKéMON have been seen");
static const u8 sText_GtsNoSeen2[] = _("yet. Fill your POKéDEX first!");

#define GTS_MAX_LISTINGS 16
#define GTS_LIST_ROWS 5

struct GtsListing
{
    u32 id;
    u16 species;
    u8 level, gender;
    u16 want;
    u8 wantMin, wantMax, wantGender;
    u8 name[PLAYER_NAME_LENGTH + 1];
};

// What the screen hands back to the field script (lives past the screen).
static EWRAM_DATA struct
{
    u32 id;               // the chosen listing (SEARCH / MINE)
    u16 listedSpecies;    // ... and its Pokemon
    u16 wantSpecies;      // DEPOSIT: what to ask for; SEARCH: what the listing wants
    u8 wantMin, wantMax, wantGender;
} sGts = {0};

// Screen-only GTS data, allocated with the screen.
struct GtsScreen
{
    u16 pick[NATIONAL_DEX_COUNT];
    u16 pickCount;
    u16 pickCursor;
    u16 pickScroll;
    u8 count;
    struct GtsListing list[GTS_MAX_LISTINGS];
};
static EWRAM_DATA struct GtsScreen *sGtsScreen = NULL;

static const u8 *ScreenTitle(void)
{
    switch (sOnline->gtsMode)
    {
    case GTS_DEPOSIT: return sText_GtsDeposit;
    case GTS_SEARCH:  return sText_GtsSearch;
    case GTS_MINE:    return sText_GtsMine;
    }
    return sText_OnlineClub;
}

static const u8 *GenderText(u8 gender)
{
    return gender == NET_GENDER_MALE ? sText_GtsMale : gender == NET_GENDER_FEMALE ? sText_GtsFemale
         : gender == NET_GENDER_ANY ? sText_GtsAny : sText_GtsEmpty;
}

static u8 NetGender(struct Pokemon *mon)
{
    switch (GetMonGender(mon))
    {
    case MON_MALE:   return NET_GENDER_MALE;
    case MON_FEMALE: return NET_GENDER_FEMALE;
    }
    return NET_GENDER_NONE;
}

// Which "want" genders make sense for a species.
static bool8 GenderAllowed(u16 species, u8 gender)
{
    u8 ratio = gSpeciesInfo[species].genderRatio;
    if (gender == NET_GENDER_ANY)
        return TRUE;
    if (ratio == MON_GENDERLESS)
        return FALSE;
    if (gender == NET_GENDER_MALE)
        return ratio != MON_FEMALE;
    return ratio != MON_MALE;
}

// "Lv5-100" / "Lv12"
static u8 *LevelRange(u8 *dst, u8 lo, u8 hi)
{
    *dst++ = CHAR_LV;
    dst = ConvertIntToDecimalStringN(dst, lo, STR_CONV_MODE_LEFT_ALIGN, 3);
    if (hi != lo)
    {
        *dst++ = CHAR_HYPHEN;
        dst = ConvertIntToDecimalStringN(dst, hi, STR_CONV_MODE_LEFT_ALIGN, 3);
    }
    return dst;
}

// Species picker: every species the player has seen, national dex order.
static void BuildPickList(void)
{
    u16 n;
    sGtsScreen->pickCount = 0;
    for (n = 1; n <= NATIONAL_DEX_COUNT; n++)
        if (GetSetPokedexFlag(n, FLAG_GET_SEEN))
            sGtsScreen->pick[sGtsScreen->pickCount++] = n;
}

static void DrawPick(void)
{
    int i;
    u8 num[8];

    PrintHeader(sText_GtsWhich);
    FillWindowPixelBuffer(WIN_LIST, PIXEL_FILL(1));
    for (i = 0; i < VISIBLE_ROWS && sGtsScreen->pickScroll + i < sGtsScreen->pickCount; i++)
    {
        u16 dex = sGtsScreen->pick[sGtsScreen->pickScroll + i];
        num[0] = CHAR_N;
        num[1] = CHAR_o;
        num[2] = CHAR_PERIOD;
        ConvertIntToDecimalStringN(num + 3, dex, STR_CONV_MODE_LEADING_ZEROS, 3);
        AddTextPrinterParameterized3(WIN_LIST, FONT_NORMAL, 8, i * RowHeight() + 2, sValueColor, TEXT_SKIP_DRAW, num);
        AddTextPrinterParameterized3(WIN_LIST, FONT_NORMAL, 0x40, i * RowHeight() + 2, sTextColor, TEXT_SKIP_DRAW,
                                     gSpeciesNames[NationalPokedexNumToSpecies(dex)]);
    }
    FinishList();
    SetHighlight(sGtsScreen->pickCursor - sGtsScreen->pickScroll);
    PrintHint(sText_HintPick);
}

static void GtsShowPick(void)
{
    if (sGtsScreen->pickCount == 0)
    {
        ShowMessage(sText_GtsNoSeen1, sText_GtsNoSeen2, NULL, TRUE, MODE_GTS_PICK);
        return;
    }
    sOnline->mode = MODE_GTS_PICK;
    DrawPick();
}

static void DrawWant(void)
{
    u8 buf[16];
    int y;

    PrintHeader(ScreenTitle());
    FillWindowPixelBuffer(WIN_LIST, PIXEL_FILL(1));
    PrintLine(0, sText_GtsTradeFor, sTextColor);
    PrintLine(1 + WANT_SPECIES, sText_GtsWantSpecies, sTextColor);
    PrintLine(1 + WANT_MIN, sText_GtsWantMin, sTextColor);
    PrintLine(1 + WANT_MAX, sText_GtsWantMax, sTextColor);
    PrintLine(1 + WANT_GENDER, sText_GtsWantGender, sTextColor);
    PrintLine(1 + WANT_OK, sText_GtsDepositRow, sTextColor);
    PrintLine(1 + WANT_CANCEL, sText_GtsCancel, sTextColor);

    y = (1 + WANT_SPECIES) * RowHeight() + 2;
    AddTextPrinterParameterized3(WIN_LIST, FONT_NORMAL, 0x70, y, sValueColor, TEXT_SKIP_DRAW,
                                 sGts.wantSpecies ? gSpeciesNames[sGts.wantSpecies] : sText_GtsNone);
    ConvertIntToDecimalStringN(buf, sGts.wantMin, STR_CONV_MODE_LEFT_ALIGN, 3);
    AddTextPrinterParameterized3(WIN_LIST, FONT_NORMAL, 0x70, (1 + WANT_MIN) * RowHeight() + 2, sValueColor, TEXT_SKIP_DRAW, buf);
    ConvertIntToDecimalStringN(buf, sGts.wantMax, STR_CONV_MODE_LEFT_ALIGN, 3);
    AddTextPrinterParameterized3(WIN_LIST, FONT_NORMAL, 0x70, (1 + WANT_MAX) * RowHeight() + 2, sValueColor, TEXT_SKIP_DRAW, buf);
    AddTextPrinterParameterized3(WIN_LIST, FONT_NORMAL, 0x70, (1 + WANT_GENDER) * RowHeight() + 2, sValueColor, TEXT_SKIP_DRAW,
                                 GenderText(sGts.wantGender));
    FinishList();
    SetHighlight(1 + sOnline->cursor);
    PrintHint(sText_HintChange);
}

static void GtsShowWant(void)
{
    sOnline->mode = MODE_GTS_WANT;
    DrawWant();
}

static void DrawListing(int line, const struct GtsListing *l)
{
    u8 buf[16];
    int y = line * RowHeight() + 2;
    PrintLine(line, gSpeciesNames[l->species], sTextColor);
    *LevelRange(buf, l->level, l->level) = EOS;
    AddTextPrinterParameterized3(WIN_LIST, FONT_NORMAL, 0x60, y, sTextColor, TEXT_SKIP_DRAW, buf);
    if (l->gender == NET_GENDER_MALE || l->gender == NET_GENDER_FEMALE)
        AddTextPrinterParameterized3(WIN_LIST, FONT_NORMAL, 0x88, y, sValueColor, TEXT_SKIP_DRAW, GenderText(l->gender));
}

static void DrawList(void)
{
    int i;
    const struct GtsListing *cur = &sGtsScreen->list[sOnline->cursor];
    u8 *end;

    PrintHeader(ScreenTitle());
    FillWindowPixelBuffer(WIN_LIST, PIXEL_FILL(1));
    for (i = 0; i < GTS_LIST_ROWS && sOnline->scroll + i < sGtsScreen->count; i++)
        DrawListing(i, &sGtsScreen->list[sOnline->scroll + i]);

    // the highlighted listing: who offers it and what they want
    if (sOnline->gtsMode == GTS_SEARCH)
    {
        end = StringCopy(gStringVar3, sText_GtsFrom);
        StringCopy(end, cur->name);
        PrintLine(GTS_LIST_ROWS, gStringVar3, sValueColor);
    }
    end = StringCopy(gStringVar3, sText_GtsWants);
    end = StringCopy(end, gSpeciesNames[cur->want]);
    *end++ = CHAR_SPACE;
    end = LevelRange(end, cur->wantMin, cur->wantMax);
    if (cur->wantGender == NET_GENDER_MALE || cur->wantGender == NET_GENDER_FEMALE)
    {
        *end++ = CHAR_SPACE;
        end = StringCopy(end, GenderText(cur->wantGender));
    }
    *end = EOS;
    PrintLine(GTS_LIST_ROWS + 1, gStringVar3, sValueColor);
    FinishList();
    SetHighlight(sOnline->cursor - sOnline->scroll);
    PrintHint(sText_HintBrowse);
}

// STR_VAR_1 listed species, STR_VAR_2 wanted species, STR_VAR_3 "Lv5-100 ♂"
// (again after the party menu, which uses the string vars itself)
static void BufferGtsStrings(void)
{
    u8 *end;
    StringCopy(gStringVar1, gSpeciesNames[sGts.listedSpecies]);
    StringCopy(gStringVar2, gSpeciesNames[sGts.wantSpecies]);
    end = LevelRange(gStringVar3, sGts.wantMin, sGts.wantMax);
    if (sGts.wantGender == NET_GENDER_MALE || sGts.wantGender == NET_GENDER_FEMALE)
    {
        *end++ = CHAR_SPACE;
        end = StringCopy(end, GenderText(sGts.wantGender));
    }
    *end = EOS;
}

static void GtsShowList(void)
{
    sOnline->mode = MODE_GTS_LIST;
    sOnline->cursor = 0;
    sOnline->scroll = 0;
    DrawList();
}

static void GtsFinish(u8 taskId, u16 result)
{
    gSpecialVar_Result = result;
    LeaveScreen(taskId);
}

// M_GTS_LIST: u8 n, n x (u32 id, u16 species, u8 level, u8 gender, u16 want, u8 min, u8 max, u8 want_gender, name[8])
static void ParseListings(const u8 *p, int len)
{
    int i, n;
    sGtsScreen->count = 0;
    if (len < 1)
        return;
    n = p[0];
    p++;
    for (i = 0; i < n && i < GTS_MAX_LISTINGS && 1 + (i + 1) * 21 <= len; i++, p += 21)
    {
        struct GtsListing *l = &sGtsScreen->list[i];
        int c;
        l->id = p[0] | (p[1] << 8) | (p[2] << 16) | ((u32)p[3] << 24);
        l->species = p[4] | (p[5] << 8);
        l->level = p[6];
        l->gender = p[7];
        l->want = p[8] | (p[9] << 8);
        l->wantMin = p[10];
        l->wantMax = p[11];
        l->wantGender = p[12];
        for (c = 0; c < PLAYER_NAME_LENGTH && p[13 + c] != EOS; c++)
            l->name[c] = p[13 + c];
        l->name[c] = EOS;
        if (l->species == SPECIES_NONE || l->species >= NUM_SPECIES || l->want == SPECIES_NONE || l->want >= NUM_SPECIES)
            i--, n--;   // never show a listing this game doesn't know
        else
            sGtsScreen->count++;
    }
}

static void GtsHandleBusy(void)
{
    int status = PcNetOpStatus();
    u8 type;
    const u8 *reply;
    int len;

    if (status == PC_NET_OP_PENDING)
        return;
    len = status == PC_NET_OP_OK ? PcNetMasterReply(&type, &reply) : 0;
    if (status != PC_NET_OP_OK)
    {
        ShowMessage(sText_CantReach1, sText_CantReach2, NULL, TRUE, MODE_GTS_PICK);
        return;
    }
    ParseListings(reply, len);
    if (sGtsScreen->count)
        GtsShowList();
    else if (sOnline->busyOp == BUSY_GTS_SEARCH)
        ShowMessage(sText_GtsNobody1, sText_GtsNobody2, NULL, FALSE, MODE_GTS_PICK);
    else
        ShowMessage(sText_GtsNoneOfYours1, sText_GtsNoneOfYours2, NULL, TRUE, MODE_GTS_PICK);
}

static void GtsPickInput(u8 taskId)
{
    s16 c = sGtsScreen->pickCursor;
    s16 n = sGtsScreen->pickCount;

    if (JOY_REPT(DPAD_UP))
        c = c == 0 ? n - 1 : c - 1;
    else if (JOY_REPT(DPAD_DOWN))
        c = c + 1 >= n ? 0 : c + 1;
    else if (JOY_REPT(DPAD_LEFT))
        c = c >= VISIBLE_ROWS ? c - VISIBLE_ROWS : 0;
    else if (JOY_REPT(DPAD_RIGHT))
        c = c + VISIBLE_ROWS < n ? c + VISIBLE_ROWS : n - 1;
    else if (JOY_NEW(B_BUTTON))
    {
        PlaySE(SE_SELECT);
        if (sOnline->gtsMode == GTS_DEPOSIT)
            GtsShowWant();
        else
            GtsFinish(taskId, 0);
        return;
    }
    else if (JOY_NEW(A_BUTTON))
    {
        u16 species = NationalPokedexNumToSpecies(sGtsScreen->pick[c]);
        u8 payload[4];
        PlaySE(SE_SELECT);
        if (sOnline->gtsMode == GTS_DEPOSIT)
        {
            sGts.wantSpecies = species;
            if (!GenderAllowed(species, sGts.wantGender))
                sGts.wantGender = NET_GENDER_ANY;
            sOnline->cursor = WANT_OK;
            GtsShowWant();
            return;
        }
        payload[0] = species;
        payload[1] = species >> 8;
        payload[2] = 1;
        payload[3] = MAX_LEVEL;
        PcNetMasterRequest(MSG_GTS_SEARCH, payload, sizeof(payload));
        StartBusy(BUSY_GTS_SEARCH);
        return;
    }
    else
    {
        return;
    }
    if (c == sGtsScreen->pickCursor)
        return;
    sGtsScreen->pickCursor = c;
    if (c < sGtsScreen->pickScroll)
        sGtsScreen->pickScroll = c;
    else if (c >= sGtsScreen->pickScroll + VISIBLE_ROWS)
        sGtsScreen->pickScroll = c - VISIBLE_ROWS + 1;
    PlaySE(SE_SELECT);
    DrawPick();
}

static void GtsWantInput(u8 taskId)
{
    int delta = JOY_REPT(DPAD_LEFT) ? -1 : JOY_REPT(DPAD_RIGHT) ? 1 : 0;

    if (JOY_REPT(DPAD_UP) || JOY_REPT(DPAD_DOWN))
    {
        sOnline->cursor = JOY_REPT(DPAD_UP) ? (sOnline->cursor + WANT_ROWS - 1) % WANT_ROWS : (sOnline->cursor + 1) % WANT_ROWS;
        PlaySE(SE_SELECT);
        DrawWant();
    }
    else if (delta)
    {
        switch (sOnline->cursor)
        {
        case WANT_MIN:
            sGts.wantMin = sGts.wantMin + delta < 1 ? sGts.wantMax : sGts.wantMin + delta > sGts.wantMax ? 1 : sGts.wantMin + delta;
            break;
        case WANT_MAX:
            sGts.wantMax = sGts.wantMax + delta < sGts.wantMin ? MAX_LEVEL : sGts.wantMax + delta > MAX_LEVEL ? sGts.wantMin : sGts.wantMax + delta;
            break;
        case WANT_GENDER:
            do
                sGts.wantGender = (sGts.wantGender + 3 + delta) % 3;
            while (sGts.wantSpecies && !GenderAllowed(sGts.wantSpecies, sGts.wantGender));
            break;
        default:
            return;
        }
        PlaySE(SE_SELECT);
        DrawWant();
    }
    else if (JOY_NEW(B_BUTTON) || (JOY_NEW(A_BUTTON) && sOnline->cursor == WANT_CANCEL))
    {
        PlaySE(SE_SELECT);
        GtsFinish(taskId, 0);
    }
    else if (JOY_NEW(A_BUTTON) && (sOnline->cursor == WANT_SPECIES || (sOnline->cursor == WANT_OK && !sGts.wantSpecies)))
    {
        PlaySE(SE_SELECT);
        GtsShowPick();
    }
    else if (JOY_NEW(A_BUTTON) && sOnline->cursor == WANT_OK)
    {
        PlaySE(SE_SELECT);
        StringCopy(gStringVar2, gSpeciesNames[sGts.wantSpecies]);
        GtsFinish(taskId, 1);
    }
}

static void GtsListInput(u8 taskId)
{
    if (JOY_REPT(DPAD_UP) || JOY_REPT(DPAD_DOWN))
    {
        if (JOY_REPT(DPAD_UP))
            sOnline->cursor = sOnline->cursor == 0 ? sGtsScreen->count - 1 : sOnline->cursor - 1;
        else
            sOnline->cursor = sOnline->cursor + 1 >= sGtsScreen->count ? 0 : sOnline->cursor + 1;
        if (sOnline->cursor < sOnline->scroll)
            sOnline->scroll = sOnline->cursor;
        else if (sOnline->cursor >= sOnline->scroll + GTS_LIST_ROWS)
            sOnline->scroll = sOnline->cursor - GTS_LIST_ROWS + 1;
        PlaySE(SE_SELECT);
        DrawList();
    }
    else if (JOY_NEW(B_BUTTON))
    {
        PlaySE(SE_SELECT);
        if (sOnline->gtsMode == GTS_SEARCH)
            GtsShowPick();
        else
            GtsFinish(taskId, 0);
    }
    else if (JOY_NEW(A_BUTTON))
    {
        const struct GtsListing *l = &sGtsScreen->list[sOnline->cursor];
        PlaySE(SE_SELECT);
        sGts.id = l->id;
        sGts.listedSpecies = l->species;
        sGts.wantSpecies = l->want;
        sGts.wantMin = l->wantMin;
        sGts.wantMax = l->wantMax;
        sGts.wantGender = l->wantGender;
        BufferGtsStrings();
        GtsFinish(taskId, 1);
    }
}

static void GtsStart(void)
{
    sGtsScreen = AllocZeroed(sizeof(*sGtsScreen));
    BuildPickList();
    switch (sOnline->gtsMode)
    {
    case GTS_DEPOSIT:
        sGts.wantSpecies = SPECIES_NONE;
        sGts.wantMin = 1;
        sGts.wantMax = MAX_LEVEL;
        sGts.wantGender = NET_GENDER_ANY;
        sOnline->cursor = WANT_SPECIES;
        GtsShowWant();
        break;
    case GTS_SEARCH:
        GtsShowPick();
        break;
    default:
        PcNetMasterRequest(MSG_GTS_MINE, NULL, 0);
        StartBusy(BUSY_GTS_MINE);
        break;
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
            FREE_AND_SET_NULL(sGtsScreen);
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
            else if (sOnline->messageMode == MODE_GTS_PICK)
                GtsShowPick();
            else
                ShowServers();
        }
        break;
    case MODE_GTS_PICK:
        GtsPickInput(taskId);
        break;
    case MODE_GTS_WANT:
        GtsWantInput(taskId);
        break;
    case MODE_GTS_LIST:
        GtsListInput(taskId);
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
        if (sOnline->gtsMode)
        {
            GtsStart();
        }
        else if (sOnline->naming == NAMING_REGISTER)
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

// ---- world events (the game server's S_WORLD) -------------------------------------------------

// A mass outbreak on this map: some of the land encounters are its species.
bool8 Online_OutbreakMon(u16 *species, u8 *level)
{
    const struct PcNetWorld *w = PcNetWorldState();

    if (w == NULL || w->eventKind != PC_NET_EVENT_OUTBREAK
     || w->mapGroup != gSaveBlock1Ptr->location.mapGroup || w->mapNum != gSaveBlock1Ptr->location.mapNum
     || w->species == SPECIES_NONE || w->species >= NUM_SPECIES || w->species == SPECIES_UNOWN
     || w->minLevel == 0 || w->maxLevel > MAX_LEVEL || w->minLevel > w->maxLevel)
        return FALSE;
    if (Random() % 100 >= w->percent)
        return FALSE;
    *species = w->species;
    *level = w->minLevel + Random() % (w->maxLevel - w->minLevel + 1);
    return TRUE;
}

// The day's boosted type: extra rolls for a shiny wild Pokemon (each roll is
// the normal 1/8192), so the odds become about 1 in shinyOdds.
u8 Online_ShinyRerolls(u16 species)
{
    const struct PcNetWorld *w = PcNetWorldState();

    if (w == NULL || w->boostType >= NUMBER_OF_MON_TYPES || w->shinyOdds == 0 || species >= NUM_SPECIES)
        return 0;
    if (gSpeciesInfo[species].types[0] != w->boostType && gSpeciesInfo[species].types[1] != w->boostType)
        return 0;
    return min(8192 / w->shinyOdds, 64) - 1;
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
enum { ERR_GTS_GONE = 4, ERR_GTS_FULL = 5, ERR_ILLEGAL_MON = 6, ERR_GTS_NO_MATCH = 7 };
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

// For specials followed by waitstate that can answer immediately.
static void Task_EnableScript(u8 taskId)
{
    DestroyTask(taskId);
    ScriptContext_Enable();
}

static void ResumeScriptNextFrame(void)
{
    CreateTask(Task_EnableScript, 80);
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
        ResumeScriptNextFrame();
        return;
    }
    CreateTask(Task_CollectMail, 80);
    PcNetMasterRequest(MSG_MAIL_COLLECT, NULL, 0);
}

// special Online_OpenGts (VAR_0x8006 = GTS_DEPOSIT / SEARCH / MINE; after fadescreen;
// then waitstate). VAR_RESULT = 1 if something was chosen:
//   DEPOSIT: the wanted species etc. (STR_VAR_2 = its name)
//   SEARCH:  a listing (STR_VAR_1 listed, STR_VAR_2 wanted, STR_VAR_3 "Lv5-100 ♂")
//   MINE:    a listing to take back (STR_VAR_1)
void Online_OpenGts(void)
{
    sOnline = AllocZeroed(sizeof(*sOnline));
    sOnline->gtsMode = gSpecialVar_0x8006 + 1;
    gSpecialVar_Result = 0;
    gMain.savedCallback = CB2_ReturnToFieldContinueScriptPlayMapMusic;
    SetMainCallback2(CB2_OnlineInit);
}

// special Online_GtsCheckOffer (VAR_0x8004 = party slot): does it fit the chosen
// listing? VAR_RESULT 0 yes, 1 wrong species, 2 wrong level, 3 wrong gender.
// (The server checks again; this just gives a clear message.)
void Online_GtsCheckOffer(void)
{
    struct Pokemon *mon = &gPlayerParty[gSpecialVar_0x8004];
    u8 level = GetMonData(mon, MON_DATA_LEVEL, NULL);
    BufferGtsStrings();
    if (GetMonData(mon, MON_DATA_SPECIES_OR_EGG, NULL) != sGts.wantSpecies)
        gSpecialVar_Result = 1;
    else if (level < sGts.wantMin || level > sGts.wantMax)
        gSpecialVar_Result = 2;
    else if (sGts.wantGender != NET_GENDER_ANY && NetGender(mon) != sGts.wantGender)
        gSpecialVar_Result = 3;
    else
        gSpecialVar_Result = 0;
}

static u16 GtsErrorResult(void)
{
    switch (PcNetMasterError())
    {
    case ERR_ILLEGAL_MON: return 3;
    case ERR_GTS_FULL:    return 4;
    case ERR_GTS_GONE:    return 5;
    case ERR_GTS_NO_MATCH: return 6;
    }
    return 0;
}

// VAR_RESULT: 0 failed, 1 done, 3 not legal, 4 too many listings, 5 listing gone,
// 6 doesn't fit the listing. The game is saved right after any change.
static void Task_GtsReply(u8 taskId)
{
    s16 *data = gTasks[taskId].data;
    int status = PcNetOpStatus();
    u8 type;
    const u8 *reply;
    int len;

    if (status == PC_NET_OP_PENDING)
        return;
    len = status == PC_NET_OP_OK ? PcNetMasterReply(&type, &reply) : 0;
    gSpecialVar_0x8005 = 0;
    if (data[0] != MSG_GTS_DEPOSIT)
        BufferGtsStrings();   // for the messages (a received Pokemon replaces STR_VAR_1)
    if (status != PC_NET_OP_OK)
    {
        gSpecialVar_Result = GtsErrorResult();
    }
    else if (data[0] == MSG_GTS_DEPOSIT)
    {
        ZeroMonData(&gPlayerParty[data[1]]);
        CompactPartySlots();
        CalculatePlayerPartyCount();
        TrySavingData(SAVE_NORMAL);
        gSpecialVar_Result = 1;
    }
    else if (len >= 1 + (int)sizeof(struct Pokemon) && reply[0] == 1)
    {
        struct Pokemon mon;
        memcpy(&mon, reply + 1, sizeof(mon));
        if (data[0] == MSG_GTS_OFFER)
            gPlayerParty[data[1]] = mon;
        else
            gSpecialVar_0x8005 = PcGiveTradedMon(&mon) == MON_GIVEN_TO_PC;
        RegisterReceivedMon(&mon);
        TrySavingData(SAVE_NORMAL);
        gSpecialVar_Result = 1;
    }
    else
    {
        gSpecialVar_Result = 0;
    }
    DestroyTask(taskId);
    ScriptContext_Enable();
}

static void StartGtsRequest(u8 msg, const void *payload, int len)
{
    u8 taskId = CreateTask(Task_GtsReply, 80);
    gTasks[taskId].data[0] = msg;
    gTasks[taskId].data[1] = gSpecialVar_0x8004;
    PcNetMasterRequest(msg, payload, len);
}

// special Online_GtsDeposit (VAR_0x8004 = party slot; then waitstate)
void Online_GtsDeposit(void)
{
    u8 payload[sizeof(struct Pokemon) + 5];
    memcpy(payload, &gPlayerParty[gSpecialVar_0x8004], sizeof(struct Pokemon));
    payload[100] = sGts.wantSpecies;
    payload[101] = sGts.wantSpecies >> 8;
    payload[102] = sGts.wantMin;
    payload[103] = sGts.wantMax;
    payload[104] = sGts.wantGender;
    StartGtsRequest(MSG_GTS_DEPOSIT, payload, sizeof(payload));
}

// special Online_GtsOffer (VAR_0x8004 = party slot; then waitstate): STR_VAR_1 = received
void Online_GtsOffer(void)
{
    u8 payload[4 + sizeof(struct Pokemon)];
    payload[0] = sGts.id;
    payload[1] = sGts.id >> 8;
    payload[2] = sGts.id >> 16;
    payload[3] = sGts.id >> 24;
    memcpy(payload + 4, &gPlayerParty[gSpecialVar_0x8004], sizeof(struct Pokemon));
    StartGtsRequest(MSG_GTS_OFFER, payload, sizeof(payload));
}

// special Online_GtsWithdraw (then waitstate): STR_VAR_1 = the Pokemon,
// VAR_0x8005 = 1 if it went to the PC; VAR_RESULT 7 = no room.
void Online_GtsWithdraw(void)
{
    u8 payload[4];
    if (!HasRoomForMon())
    {
        gSpecialVar_Result = 7;
        ResumeScriptNextFrame();
        return;
    }
    payload[0] = sGts.id;
    payload[1] = sGts.id >> 8;
    payload[2] = sGts.id >> 16;
    payload[3] = sGts.id >> 24;
    StartGtsRequest(MSG_GTS_WITHDRAW, payload, sizeof(payload));
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
bool8 Online_OutbreakMon(u16 *species, u8 *level) { return FALSE; }
u8 Online_ShinyRerolls(u16 species) { return 0; }
void Online_GtsCheckOffer(void) {}
void Online_GtsDeposit(void) {}
void Online_GtsOffer(void) {}
void Online_GtsWithdraw(void) {}
#endif
