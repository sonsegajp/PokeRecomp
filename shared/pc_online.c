// PC port: online play — the ONLINE CLUB screen, the field side of being on
// a server (other trainers walking around, battle/trade requests), and the
// specials the Pokémon Center 2F scripts use. The network itself is the
// platform layer (port/src/net.c, pc_net.h).
//
// Battles and trades between trainers on a server run the game's own Cable
// Club code over a virtual link cable (see link.c NetLinkFrame).
#include "global.h"
#ifdef PLATFORM_PC
#ifdef EMERALD
#include "bg.h"
#include "window.h"
#include "text.h"
#include "sprite.h"
#include "palette.h"
#include "gpu_regs.h"
#include "malloc.h"
#include "dma3.h"
#include "string_util.h"
#include "decompress.h"
#include "main.h"
#include "field_screen_effect.h"
#include "mail.h"
#include "constants/rgb.h"
#else
#include "gflib.h"
#include "field_fadetransition.h"
#include "new_menu_helpers.h"
#include "text_window_graphics.h"
#include "mail_data.h"
#include "quest_log.h"
#endif
#include "event_data.h"
#include "event_object_movement.h"
#include "event_scripts.h"
#include "fieldmap.h"
#include "field_player_avatar.h"
#include "menu.h"
#include "overworld.h"
#include "scanline_effect.h"
#include "script.h"
#include "sound.h"
#include "strings.h"
#include "task.h"
#include "text_window.h"
#include "constants/event_objects.h"
#include "constants/event_object_movement.h"
#include "constants/songs.h"
#include "constants/maps.h"
#include "constants/layouts.h"
#include "script_movement.h"
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
#include "pc_game.h"
#include "pc_travel.h"
#include "pc_travel_game.h"
#include "item.h"
#include "money.h"
#include "constants/items.h"
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
#ifdef EMERALD
static const u8 sText_YouAreOn3[] = _("now appear around HOENN.");
#else
static const u8 sText_YouAreOn3[] = _("now appear around KANTO.");
#endif
static const u8 sText_LeftServer[] = _("You left the server.");
static const u8 sText_CantReach1[] = _("Couldn't reach the online");
static const u8 sText_CantReach2[] = _("service. Check your connection.");
static const u8 sText_JoinFailed1[] = _("Couldn't join that server.");
static const u8 sText_JoinFailed2[] = _("It may be full or offline.");
static const u8 sText_Registered[] = _("Registered! Your online name is");
static const u8 sText_Here[] = _("{RIGHT_ARROW}");
static const u8 sText_Friends[] = _("FRIENDS");
static const u8 sText_RegionKanto[] = _("KANTO");
static const u8 sText_RegionHoenn[] = _("HOENN");
static const u8 sText_Guild[] = _("GUILD");
static const u8 sText_Market[] = _("POKéMON MARKET");
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

enum { MODE_BUSY, MODE_REGISTER, MODE_SERVERS, MODE_MESSAGE, MODE_GTS_PICK, MODE_GTS_WANT, MODE_GTS_LIST,
       MODE_FRIENDS, MODE_FRIEND_MENU, MODE_GUILD, MODE_CONFIRM,
       MODE_MKT_MENU, MODE_MKT_ITEMS, MODE_MKT_PRICE, MODE_MKT_BROWSE, MODE_MKT_BUY, MODE_MKT_MINE };
enum { BUSY_LOGIN, BUSY_REGISTER, BUSY_JOIN, BUSY_RENAME, BUSY_GTS_SEARCH, BUSY_GTS_MINE, BUSY_FRIENDS, BUSY_GUILD,
       BUSY_MKT_SELL, BUSY_MKT_BROWSE, BUSY_MKT_BUY, BUSY_MKT_MINE, BUSY_MKT_CANCEL, BUSY_MKT_COLLECT };
enum { GTS_NONE, GTS_DEPOSIT, GTS_SEARCH, GTS_MINE };   // the GTS screen: VAR_0x8006 + 1
enum { ROW_SERVER, ROW_REFRESH, ROW_RENAME, ROW_DISCONNECT, ROW_BACK, ROW_FRIENDS, ROW_GUILD };
enum { NAMING_NONE, NAMING_REGISTER, NAMING_RENAME, NAMING_GUILD_CREATE, NAMING_GUILD_JOIN };

#define GUILD_NAME_LEN 10   // 8 letters + EOS (server/protocol.py)
#define MAX_FRIENDS 32
#define MAX_MEMBERS 32

#define VISIBLE_ROWS 7
#define WIN_HEADER 0
#define WIN_LIST   1
#define WIN_HINT   2

struct OnlineFriend
{
    u32 id;
    u8 name[PLAYER_NAME_LENGTH + 1];
    u16 tag;
    u8 online;
    char server[25];
};

struct OnlineMember
{
    u8 name[PLAYER_NAME_LENGTH + 1];
    u16 tag;
    u8 online, owner;
};

struct OnlineScreen
{
    u8 loadState;
    u8 mode;
    u8 busyOp;
    u8 cursor;
    u8 scroll;
    u8 rowCount;
    u8 rows[PC_NET_MAX_SERVERS + 6];
    u8 serverOf[PC_NET_MAX_SERVERS + 6];
    u8 messageExit;      // A on the message leaves the screen
    u8 messageMode;      // otherwise, the mode to go back to
    const u8 *message[3];
    u16 timer;
    u8 naming;           // coming back from the naming screen: what for
    u8 gtsMode;          // GTS_* when this is the GTS screen
    bool8 market;        // this is the POKéMON MARKET screen
    u8 friendCount, friendSel;
    struct OnlineFriend friends[MAX_FRIENDS];
    u8 inGuild, memberCount, guildOp, confirmAction;
    u8 travelServer;     // CONFIRM_TRAVEL: the other region's world
    u8 guildName[GUILD_NAME_LEN];
    struct OnlineMember members[MAX_MEMBERS];
};

static EWRAM_DATA struct OnlineScreen *sOnline = NULL;
static EWRAM_DATA u8 sOnlineName[PLAYER_NAME_LENGTH + 1] = {0};   // edited on the naming screen
static EWRAM_DATA bool8 sJoinedWarp = FALSE;     // joined: leave the screen into Pallet Town
static EWRAM_DATA bool8 sShowJoinedMessage = FALSE;

#ifdef EMERALD
// Emerald's OPTION screen look: its text palette, background and window frame
static const u16 sOnlinePalette[] = INCGFX_U16("graphics/interface/option_menu_text.pal", ".gbapal");
static const u16 sOnlineBgPalette[] = {RGB(17, 18, 31)};
#else
static const u16 sOnlinePalette[] = INCBIN_U16("graphics/misc/option_menu.gbapal");
#endif

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
static void MarketReturn(u8 mode);
static void MarketTakeBack(void);

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
#ifdef EMERALD
{
    // both windows in the player's frame (tiles at 0x1AA, palette 2)
    FillBgTilemapBufferRect(1, 0x1AA, 1, 2, 1, 1, 2);
    FillBgTilemapBufferRect(1, 0x1AB, 2, 2, 0x1A, 1, 2);
    FillBgTilemapBufferRect(1, 0x1AC, 0x1C, 2, 1, 1, 2);
    FillBgTilemapBufferRect(1, 0x1AD, 1, 3, 1, 2, 2);
    FillBgTilemapBufferRect(1, 0x1AF, 0x1C, 3, 1, 2, 2);
    FillBgTilemapBufferRect(1, 0x1B0, 1, 5, 1, 1, 2);
    FillBgTilemapBufferRect(1, 0x1B1, 2, 5, 0x1A, 1, 2);
    FillBgTilemapBufferRect(1, 0x1B2, 0x1C, 5, 1, 1, 2);
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
#else
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
#endif

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
    if (sOnline->market)
    {
        u8 *end = gStringVar1;
        *end++ = CHAR_CURRENCY;
        ConvertIntToDecimalStringN(end, GetMoney(&gSaveBlock1Ptr->money), STR_CONV_MODE_LEFT_ALIGN, 7);
        AddTextPrinterParameterized3(WIN_HEADER, FONT_SMALL, 0xCC - GetStringWidth(FONT_SMALL, gStringVar1, 0), 3,
                                     sValueColor, TEXT_SKIP_DRAW, gStringVar1);
    }
    else if (PcNetHasAccount() && sOnline->mode == MODE_SERVERS)
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

// A long list shows arrows at its right edge while more of it is above / below.
static void DrawScrollArrows(int scroll, int count, int rows)
{
    static const u8 sUpArrow[] = {CHAR_UP_ARROW, EOS};
    static const u8 sDownArrow[] = {CHAR_DOWN_ARROW, EOS};

    if (scroll > 0)
        AddTextPrinterParameterized3(WIN_LIST, FONT_NORMAL, 0xC6, 2, sTextColor, TEXT_SKIP_DRAW, sUpArrow);
    if (scroll + rows < count)
        AddTextPrinterParameterized3(WIN_LIST, FONT_NORMAL, 0xC6, (rows - 1) * RowHeight() + 2, sTextColor,
                                     TEXT_SKIP_DRAW, sDownArrow);
}

static void FinishList(void)
{
    PutWindowTilemap(WIN_LIST);
    CopyWindowToVram(WIN_LIST, COPYWIN_FULL);
}

static void BuildServerRows(void)
{
    int i, n = 0;
    // this region's worlds, then the other region's if the player owns that game
    for (i = 0; i < PcNetServerCount() && n < PC_NET_MAX_SERVERS; i++)
    {
        if (PcNetServerRegion(i) != PC_NET_REGION)
            continue;
        sOnline->rows[n] = ROW_SERVER;
        sOnline->serverOf[n++] = i;
    }
    for (i = 0; i < PcNetServerCount() && n < PC_NET_MAX_SERVERS; i++)
    {
        if (PcNetServerRegion(i) == PC_NET_REGION || !PcNetOwnsRegion(PcNetServerRegion(i)))
            continue;
        sOnline->rows[n] = ROW_SERVER;
        sOnline->serverOf[n++] = i;
    }
    sOnline->rows[n++] = ROW_FRIENDS;
    sOnline->rows[n++] = ROW_GUILD;
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
            if (PcNetServerRegion(s) != PC_NET_REGION)   // another region's world: travel there
            {
                const u8 *region = PcNetServerRegion(s) == PC_NET_REGION_HOENN ? sText_RegionHoenn : sText_RegionKanto;
                AddTextPrinterParameterized3(WIN_LIST, FONT_SMALL, 0x98 - GetStringWidth(FONT_SMALL, region, 0), line * RowHeight() + 3,
                                             sTextColor, TEXT_SKIP_DRAW, region);
            }
            break;
        }
        case ROW_FRIENDS:    PrintLine(line, sText_Friends, sTextColor); break;
        case ROW_GUILD:      PrintLine(line, sText_Guild, sTextColor); break;
        case ROW_REFRESH:    PrintLine(line, sText_Refresh, sTextColor); break;
        case ROW_RENAME:     PrintLine(line, sText_ChangeName, sTextColor); break;
        case ROW_DISCONNECT: PrintLine(line, sText_Disconnect, sTextColor); break;
        case ROW_BACK:       PrintLine(line, sText_Back, sTextColor); break;
        }
    }
    DrawScrollArrows(sOnline->scroll, sOnline->rowCount, VISIBLE_ROWS);
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

// ---- FRIENDS and GUILD pages (lists from the master server) ---------------------------------

enum { MSG_FRIEND_ADD = 0x05, MSG_FRIEND_LIST = 0x06, MSG_FRIEND_REMOVE = 0x0F,
       MSG_GUILD_INFO = 0x10, MSG_GUILD_CREATE = 0x11, MSG_GUILD_JOIN = 0x12, MSG_GUILD_LEAVE = 0x13 };
enum { NET_ERR_NOT_FOUND = 4, NET_ERR_FULL = 5, NET_ERR_NO_MATCH = 7 };
enum { CONFIRM_REMOVE_FRIEND, CONFIRM_LEAVE_GUILD, CONFIRM_TAKE_BACK, CONFIRM_TRAVEL };
enum { FMENU_JOIN, FMENU_REMOVE, FMENU_BACK };

static const u8 sText_Offline[] = _("OFFLINE");
static const u8 sText_Online[] = _("ONLINE");
static const u8 sText_Leader[] = _("LEADER");
static const u8 sText_NoFriends1[] = _("No friends yet. Press {A_BUTTON} on a");
static const u8 sText_NoFriends2[] = _("trainer on a server to add one.");
static const u8 sText_JoinTheirServer[] = _("JOIN THEIR SERVER");
static const u8 sText_RemoveFriend[] = _("REMOVE FRIEND");
static const u8 sText_NoGuild1[] = _("You are not in a GUILD.");
static const u8 sText_NoGuild2[] = _("Make one, or join one by name.");
static const u8 sText_CreateGuild[] = _("CREATE GUILD");
static const u8 sText_JoinGuild[] = _("JOIN GUILD");
static const u8 sText_LeaveGuild[] = _("LEAVE GUILD");
static const u8 sText_HintGuild[] = _("/G: GUILD CHAT {A_BUTTON}OK {B_BUTTON}BACK");
static const u8 sText_Yes[] = _("YES");
static const u8 sText_TravelKanto[] = _("Travel to KANTO as yourself?");
static const u8 sText_TravelHoenn[] = _("Travel to HOENN as yourself?");
static const u8 sText_TravelNote[] = _("Your party comes too. (It saves.)");
static const u8 sText_CantTravel1[] = _("That region's game isn't set up.");
static const u8 sText_CantTravel2[] = _("Import its ROM in the launcher.");
static const u8 sText_No[] = _("NO");
static const u8 sText_RemoveAsk[] = _("Remove this trainer from FRIENDS?");
static const u8 sText_LeaveAsk[] = _("Really leave the GUILD?");
static const u8 sText_GuildTaken1[] = _("A GUILD with that name");
static const u8 sText_GuildTaken2[] = _("already exists.");
static const u8 sText_GuildFull[] = _("That GUILD is full.");
static const u8 sText_GuildNone1[] = _("There is no GUILD");
static const u8 sText_GuildNone2[] = _("with that name.");

static EWRAM_DATA u8 sGuildNameBuf[GUILD_NAME_LEN] = {0};   // edited on the naming screen

static void CopyNetName(u8 *dst, const u8 *src)
{
    int i;
    for (i = 0; i < PLAYER_NAME_LENGTH && src[i] != EOS; i++)
        dst[i] = src[i];
    dst[i] = EOS;
}

static void StartSocial(u8 msg, const void *payload, int len, u8 busyOp)
{
    PcNetMasterRequest(msg, payload, len);
    StartBusy(busyOp);
}

// M_FRIENDS: u8 n, n x (u32 id, name[8], u16 tag, u8 online, char server[24])
static void ParseFriends(const u8 *p, int len)
{
    int i, n = len >= 1 ? p[0] : 0;
    sOnline->friendCount = 0;
    for (i = 0; i < n && i < MAX_FRIENDS && 1 + (i + 1) * 39 <= len; i++)
    {
        const u8 *e = p + 1 + i * 39;
        struct OnlineFriend *f = &sOnline->friends[i];
        f->id = e[0] | (e[1] << 8) | (e[2] << 16) | ((u32)e[3] << 24);
        CopyNetName(f->name, e + 4);
        f->tag = e[12] | (e[13] << 8);
        f->online = e[14];
        memcpy(f->server, e + 15, 24);
        f->server[24] = 0;
        sOnline->friendCount++;
    }
}

// M_GUILD: u8 in_guild, name[10], u8 n, n x (name[8], u16 tag, u8 online, u8 owner)
static void ParseGuild(const u8 *p, int len)
{
    int i, n;
    sOnline->inGuild = len >= 1 + GUILD_NAME_LEN + 1 && p[0];
    sOnline->memberCount = 0;
    if (!sOnline->inGuild)
        return;
    for (i = 0; i < GUILD_NAME_LEN - 1 && p[1 + i] != EOS; i++)
        sOnline->guildName[i] = p[1 + i];
    sOnline->guildName[i] = EOS;
    n = p[1 + GUILD_NAME_LEN];
    p += 2 + GUILD_NAME_LEN;
    len -= 2 + GUILD_NAME_LEN;
    for (i = 0; i < n && i < MAX_MEMBERS && (i + 1) * 12 <= len; i++)
    {
        struct OnlineMember *m = &sOnline->members[i];
        CopyNetName(m->name, p + i * 12);
        m->tag = p[i * 12 + 8] | (p[i * 12 + 9] << 8);
        m->online = p[i * 12 + 10];
        m->owner = p[i * 12 + 11];
        sOnline->memberCount++;
    }
}

static void ClampList(u8 count)
{
    if (sOnline->cursor >= count)
        sOnline->cursor = count - 1;
    if (sOnline->scroll > sOnline->cursor)
        sOnline->scroll = sOnline->cursor;
    if (sOnline->cursor >= sOnline->scroll + VISIBLE_ROWS)
        sOnline->scroll = sOnline->cursor - VISIBLE_ROWS + 1;
}

// L / R: a page up or down a long list.
static bool8 ListPage(u8 *cursor, u8 count)
{
    if (JOY_NEW(L_BUTTON) && *cursor > 0)
        *cursor = *cursor > VISIBLE_ROWS ? *cursor - VISIBLE_ROWS : 0;
    else if (JOY_NEW(R_BUTTON) && *cursor + 1 < count)
        *cursor = *cursor + VISIBLE_ROWS < count ? *cursor + VISIBLE_ROWS : count - 1;
    else
        return FALSE;
    return TRUE;
}

static bool8 ListMove(u8 count)
{
    if (ListPage(&sOnline->cursor, count))
    {
        ClampList(count);
        PlaySE(SE_SELECT);
        return TRUE;
    }
    if (!JOY_REPT(DPAD_UP) && !JOY_REPT(DPAD_DOWN))
        return FALSE;
    if (JOY_REPT(DPAD_UP))
        sOnline->cursor = sOnline->cursor == 0 ? count - 1 : sOnline->cursor - 1;
    else
        sOnline->cursor = sOnline->cursor + 1 >= count ? 0 : sOnline->cursor + 1;
    ClampList(count);
    PlaySE(SE_SELECT);
    return TRUE;
}

static void PrintRight(int line, const u8 *text, const u8 *color)
{
    AddTextPrinterParameterized3(WIN_LIST, FONT_NORMAL, 0xC0 - GetStringWidth(FONT_NORMAL, text, 0),
                                 line * RowHeight() + 2, color, TEXT_SKIP_DRAW, text);
}

// -- FRIENDS: one row per friend, then BACK

static void DrawFriends(void)
{
    int i, count = sOnline->friendCount + 1;
    u8 buf[32];

    PrintHeader(sText_Friends);
    FillWindowPixelBuffer(WIN_LIST, PIXEL_FILL(1));
    if (sOnline->friendCount == 0)
    {
        PrintLine(0, sText_NoFriends1, sTextColor);
        PrintLine(1, sText_NoFriends2, sTextColor);
        PrintLine(3, sText_Back, sTextColor);
        FinishList();
        SetHighlight(3);
        PrintHint(sText_HintBrowse);
        return;
    }
    for (i = 0; i < VISIBLE_ROWS && sOnline->scroll + i < count; i++)
    {
        int row = sOnline->scroll + i;
        if (row == sOnline->friendCount)
        {
            PrintLine(i, sText_Back, sTextColor);
            continue;
        }
        CopyNameWithTag(buf, sOnline->friends[row].name, sOnline->friends[row].tag);
        PrintLine(i, buf, sTextColor);
        if (sOnline->friends[row].online)
        {
            AsciiToGame(buf, sOnline->friends[row].server, 14);
            PrintRight(i, buf, sValueColor);
        }
        else
        {
            PrintRight(i, sText_Offline, sTextColor);
        }
    }
    DrawScrollArrows(sOnline->scroll, count, VISIBLE_ROWS);
    FinishList();
    SetHighlight(sOnline->cursor - sOnline->scroll);
    PrintHint(sText_HintBrowse);
}

static void ShowFriends(void)
{
    sOnline->mode = MODE_FRIENDS;
    ClampList(sOnline->friendCount + 1);
    DrawFriends();
}

// The server a friend is on, as an index in our server list (or -1).
static int FriendServerIndex(const struct OnlineFriend *f)
{
    int i;
    if (!f->online)
        return -1;
    if (PcNetConnected() && strcmp(PcNetConnectedServerName(), f->server) == 0)
        return -1;   // already there
    for (i = 0; i < PcNetServerCount(); i++)
        if (strcmp(PcNetServerName(i), f->server) == 0)
            return i;
    return -1;
}

static void BuildFriendMenu(void)
{
    int n = 0;
    if (FriendServerIndex(&sOnline->friends[sOnline->friendSel]) >= 0)
        sOnline->rows[n++] = FMENU_JOIN;
    sOnline->rows[n++] = FMENU_REMOVE;
    sOnline->rows[n++] = FMENU_BACK;
    sOnline->rowCount = n;
}

static void DrawFriendMenu(void)
{
    const struct OnlineFriend *f = &sOnline->friends[sOnline->friendSel];
    u8 buf[32];
    int i;

    PrintHeader(sText_Friends);
    FillWindowPixelBuffer(WIN_LIST, PIXEL_FILL(1));
    CopyNameWithTag(buf, f->name, f->tag);
    PrintLine(0, buf, sValueColor);
    if (f->online)
        AsciiToGame(buf, f->server, 20);
    PrintLine(1, f->online ? buf : sText_Offline, sTextColor);
    for (i = 0; i < sOnline->rowCount; i++)
        PrintLine(3 + i, sOnline->rows[i] == FMENU_JOIN ? sText_JoinTheirServer
                       : sOnline->rows[i] == FMENU_REMOVE ? sText_RemoveFriend : sText_Back, sTextColor);
    FinishList();
    SetHighlight(3 + sOnline->cursor);
    PrintHint(sText_HintBrowse);
}

static void ShowFriendMenu(void)
{
    sOnline->mode = MODE_FRIEND_MENU;
    BuildFriendMenu();
    sOnline->cursor = 0;
    DrawFriendMenu();
}

// -- GUILD: not in one -> CREATE / JOIN / BACK; in one -> members, LEAVE, BACK

static u8 GuildRowCount(void)
{
    return sOnline->inGuild ? sOnline->memberCount + 2 : 3;
}

static void DrawGuild(void)
{
    int i;
    u8 buf[32];

    FillWindowPixelBuffer(WIN_LIST, PIXEL_FILL(1));
    if (!sOnline->inGuild)
    {
        PrintHeader(sText_Guild);
        PrintLine(0, sText_NoGuild1, sTextColor);
        PrintLine(1, sText_NoGuild2, sTextColor);
        PrintLine(3, sText_CreateGuild, sTextColor);
        PrintLine(4, sText_JoinGuild, sTextColor);
        PrintLine(5, sText_Back, sTextColor);
        FinishList();
        SetHighlight(3 + sOnline->cursor);
        PrintHint(sText_HintBrowse);
        return;
    }
    {
        u8 *end = StringCopy(buf, sText_Guild);
        *end++ = CHAR_COLON;
        *end++ = CHAR_SPACE;
        StringCopy(end, sOnline->guildName);
        PrintHeader(buf);
    }
    for (i = 0; i < VISIBLE_ROWS && sOnline->scroll + i < GuildRowCount(); i++)
    {
        int row = sOnline->scroll + i;
        if (row == sOnline->memberCount)
            PrintLine(i, sText_LeaveGuild, sTextColor);
        else if (row == sOnline->memberCount + 1)
            PrintLine(i, sText_Back, sTextColor);
        else
        {
            const struct OnlineMember *m = &sOnline->members[row];
            CopyNameWithTag(buf, m->name, m->tag);
            PrintLine(i, buf, m->owner ? sValueColor : sTextColor);
            PrintRight(i, m->online ? sText_Online : sText_Offline, m->online ? sValueColor : sTextColor);
        }
    }
    DrawScrollArrows(sOnline->scroll, GuildRowCount(), VISIBLE_ROWS);
    FinishList();
    SetHighlight(sOnline->cursor - sOnline->scroll);
    PrintHint(sText_HintGuild);
}

static void ShowGuild(void)
{
    sOnline->mode = MODE_GUILD;
    ClampList(GuildRowCount());
    DrawGuild();
}

// -- YES / NO

static const u8 sText_TakeBackAsk[] = _("Take these items back?");

static void DrawConfirm(void)
{
    PrintHeader(sOnline->confirmAction == CONFIRM_LEAVE_GUILD ? sText_Guild
              : sOnline->confirmAction == CONFIRM_TAKE_BACK ? ScreenTitle()
              : sOnline->confirmAction == CONFIRM_TRAVEL ? sText_ServerBrowser : sText_Friends);
    FillWindowPixelBuffer(WIN_LIST, PIXEL_FILL(1));
    if (sOnline->confirmAction == CONFIRM_TRAVEL)
    {
        PrintLine(0, PcNetServerRegion(sOnline->travelServer) == PC_NET_REGION_HOENN ? sText_TravelHoenn : sText_TravelKanto,
                  sTextColor);
        PrintLine(1, sText_TravelNote, sTextColor);
    }
    else
    PrintLine(1, sOnline->confirmAction == CONFIRM_LEAVE_GUILD ? sText_LeaveAsk
               : sOnline->confirmAction == CONFIRM_TAKE_BACK ? sText_TakeBackAsk : sText_RemoveAsk, sTextColor);
    PrintLine(3, sText_Yes, sTextColor);
    PrintLine(4, sText_No, sTextColor);
    FinishList();
    SetHighlight(3 + sOnline->cursor);
    PrintHint(sText_HintBrowse);
}

static void ShowConfirm(u8 action)
{
    sOnline->mode = MODE_CONFIRM;
    sOnline->confirmAction = action;
    sOnline->cursor = 1;   // NO first
    DrawConfirm();
}

static void SocialHandleBusy(void)
{
    int status = PcNetOpStatus();
    u8 type;
    const u8 *reply;
    int len;

    if (status == PC_NET_OP_PENDING)
        return;
    len = status == PC_NET_OP_OK ? PcNetMasterReply(&type, &reply) : 0;
    if (sOnline->busyOp == BUSY_FRIENDS)
    {
        if (status != PC_NET_OP_OK)
        {
            ShowMessage(sText_CantReach1, sText_CantReach2, NULL, FALSE, MODE_SERVERS);
            return;
        }
        ParseFriends(reply, len);
        ShowFriends();
        return;
    }
    // BUSY_GUILD
    if (status == PC_NET_OP_OK)
    {
        ParseGuild(reply, len);
        ShowGuild();
        return;
    }
    switch (PcNetMasterError())
    {
    case NET_ERR_FULL:
        if (sOnline->guildOp == MSG_GUILD_CREATE)
            ShowMessage(sText_GuildTaken1, sText_GuildTaken2, NULL, FALSE, MODE_GUILD);
        else
            ShowMessage(sText_GuildFull, NULL, NULL, FALSE, MODE_GUILD);
        break;
    case NET_ERR_NOT_FOUND:
        ShowMessage(sText_GuildNone1, sText_GuildNone2, NULL, FALSE, MODE_GUILD);
        break;
    default:
        ShowMessage(sText_CantReach1, sText_CantReach2, NULL, FALSE, MODE_SERVERS);
        break;
    }
}

static void GuildRequest(u8 msg, const u8 *name)
{
    sOnline->guildOp = msg;
    StartSocial(msg, name, name ? GUILD_NAME_LEN : 0, BUSY_GUILD);
}

static void FriendsInput(u8 taskId)
{
    u8 count = sOnline->friendCount + 1;
    if (sOnline->friendCount == 0)
    {
        if (JOY_NEW(A_BUTTON) || JOY_NEW(B_BUTTON))
        {
            PlaySE(SE_SELECT);
            ShowServers();
        }
        return;
    }
    if (ListMove(count))
        DrawFriends();
    else if (JOY_NEW(B_BUTTON) || (JOY_NEW(A_BUTTON) && sOnline->cursor == sOnline->friendCount))
    {
        PlaySE(SE_SELECT);
        sOnline->cursor = 0;
        sOnline->scroll = 0;
        ShowServers();
    }
    else if (JOY_NEW(A_BUTTON))
    {
        PlaySE(SE_SELECT);
        sOnline->friendSel = sOnline->cursor;
        ShowFriendMenu();
    }
}

static void FriendMenuInput(u8 taskId)
{
    if (JOY_NEW(DPAD_UP) || JOY_NEW(DPAD_DOWN))
    {
        sOnline->cursor = (sOnline->cursor + (JOY_NEW(DPAD_UP) ? sOnline->rowCount - 1 : 1)) % sOnline->rowCount;
        PlaySE(SE_SELECT);
        DrawFriendMenu();
    }
    else if (JOY_NEW(B_BUTTON) || (JOY_NEW(A_BUTTON) && sOnline->rows[sOnline->cursor] == FMENU_BACK))
    {
        PlaySE(SE_SELECT);
        sOnline->cursor = sOnline->friendSel;
        ShowFriends();
    }
    else if (JOY_NEW(A_BUTTON) && sOnline->rows[sOnline->cursor] == FMENU_REMOVE)
    {
        PlaySE(SE_SELECT);
        ShowConfirm(CONFIRM_REMOVE_FRIEND);
    }
    else if (JOY_NEW(A_BUTTON))   // JOIN THEIR SERVER
    {
        int s = FriendServerIndex(&sOnline->friends[sOnline->friendSel]);
        PlaySE(SE_SELECT);
        if (s >= 0)
        {
            PcNetBeginJoin(s, gSaveBlock2Ptr->playerGender);
            StartBusy(BUSY_JOIN);
        }
    }
}

static void GuildInput(u8 taskId)
{
    if (ListMove(GuildRowCount()))
    {
        DrawGuild();
        return;
    }
    if (JOY_NEW(B_BUTTON))
    {
        PlaySE(SE_SELECT);
        sOnline->cursor = 0;
        sOnline->scroll = 0;
        ShowServers();
        return;
    }
    if (!JOY_NEW(A_BUTTON))
        return;
    if (!sOnline->inGuild)
    {
        PlaySE(SE_SELECT);
        if (sOnline->cursor == 2)
        {
            sOnline->cursor = 0;
            ShowServers();
        }
        else
        {
            memset(sGuildNameBuf, EOS, sizeof(sGuildNameBuf));
            OpenNaming(taskId, sOnline->cursor == 0 ? NAMING_GUILD_CREATE : NAMING_GUILD_JOIN);
        }
    }
    else if (sOnline->cursor == sOnline->memberCount)
    {
        PlaySE(SE_SELECT);
        ShowConfirm(CONFIRM_LEAVE_GUILD);
    }
    else if (sOnline->cursor == sOnline->memberCount + 1)
    {
        PlaySE(SE_SELECT);
        sOnline->cursor = 0;
        sOnline->scroll = 0;
        ShowServers();
    }
}

static void ConfirmInput(u8 taskId)
{
    if (JOY_NEW(DPAD_UP) || JOY_NEW(DPAD_DOWN))
    {
        sOnline->cursor ^= 1;
        PlaySE(SE_SELECT);
        DrawConfirm();
    }
    else if (JOY_NEW(B_BUTTON) || (JOY_NEW(A_BUTTON) && sOnline->cursor == 1))
    {
        PlaySE(SE_SELECT);
        sOnline->cursor = 0;
        if (sOnline->confirmAction == CONFIRM_LEAVE_GUILD)
            ShowGuild();
        else if (sOnline->confirmAction == CONFIRM_TAKE_BACK)
            MarketReturn(MODE_MKT_MINE);
        else if (sOnline->confirmAction == CONFIRM_TRAVEL)
            ShowServers();
        else
            ShowFriendMenu();
    }
    else if (JOY_NEW(A_BUTTON))
    {
        PlaySE(SE_SELECT);
        sOnline->cursor = 0;
        sOnline->scroll = 0;
        if (sOnline->confirmAction == CONFIRM_LEAVE_GUILD)
        {
            GuildRequest(MSG_GUILD_LEAVE, NULL);
        }
        else if (sOnline->confirmAction == CONFIRM_TAKE_BACK)
        {
            MarketTakeBack();
        }
        else if (sOnline->confirmAction == CONFIRM_TRAVEL)
        {
            // saves and hands over to the other region's game
            if (!PcTravelDepart(sOnline->travelServer))
                ShowMessage(sText_CantTravel1, sText_CantTravel2, NULL, FALSE, MODE_SERVERS);
        }
        else
        {
            u32 id = sOnline->friends[sOnline->friendSel].id;
            u8 payload[4] = {id, id >> 8, id >> 16, id >> 24};
            StartSocial(MSG_FRIEND_REMOVE, payload, sizeof(payload), BUSY_FRIENDS);
        }
    }
}

static void GtsHandleBusy(void);
static void MarketHandleBusy(void);

static void HandleBusy(void)
{
    int status = PcNetOpStatus();
    if (sOnline->busyOp == BUSY_GTS_SEARCH || sOnline->busyOp == BUSY_GTS_MINE)
    {
        GtsHandleBusy();
        return;
    }
    if (sOnline->busyOp == BUSY_FRIENDS || sOnline->busyOp == BUSY_GUILD)
    {
        SocialHandleBusy();
        return;
    }
    if (sOnline->busyOp >= BUSY_MKT_SELL)
    {
        MarketHandleBusy();
        return;
    }
    if (status == PC_NET_OP_PENDING)
        return;
    switch (sOnline->busyOp)
    {
    case BUSY_LOGIN:
        if (status == PC_NET_OP_OK)
            ShowServers();
        else if (!PcNetHasAccount())   // the service didn't know this login: register
        {
            StringCopy(sOnlineName, gSaveBlock2Ptr->playerName);
            sOnline->mode = MODE_REGISTER;
            sOnline->cursor = 0;
            DrawRegister();
        }
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
            sOnline->cursor = sOnline->scroll = 0;   // the browser starts on the first world
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
    if (ListPage(&sOnline->cursor, sOnline->rowCount))
    {
        ClampList(sOnline->rowCount);
        PlaySE(SE_SELECT);
        DrawServers();
    }
    else if (JOY_REPT(DPAD_UP) || JOY_REPT(DPAD_DOWN))
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
            if (PcNetServerRegion(sOnline->serverOf[sOnline->cursor]) != PC_NET_REGION)
            {
                sOnline->travelServer = sOnline->serverOf[sOnline->cursor];
                ShowConfirm(CONFIRM_TRAVEL);
                break;
            }
            PcNetBeginJoin(sOnline->serverOf[sOnline->cursor], gSaveBlock2Ptr->playerGender);
            StartBusy(BUSY_JOIN);
            break;
        case ROW_FRIENDS:
            sOnline->cursor = sOnline->scroll = 0;
            StartSocial(MSG_FRIEND_LIST, NULL, 0, BUSY_FRIENDS);
            break;
        case ROW_GUILD:
            sOnline->cursor = sOnline->scroll = 0;
            GuildRequest(MSG_GUILD_INFO, NULL);
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
    if (sOnline->market)
        return sText_Market;
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
    DrawScrollArrows(sGtsScreen->pickScroll, sGtsScreen->pickCount, VISIBLE_ROWS);
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
    DrawScrollArrows(sOnline->scroll, sGtsScreen->count, GTS_LIST_ROWS);
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

// ---- the POKé MARKET screen (same look; opened by Online_OpenMarket) -----------------------
//
// Trainers sell items to each other through the master server (server/market.py).
// The bag and money change only after the server has done its part, and items
// being listed leave the bag (and the game is saved) before the server sees
// them, so a crash can lose a sale but never duplicate items.

enum { MSG_MARKET_SELL = 0x14, MSG_MARKET_BROWSE = 0x15, MSG_MARKET_BUY = 0x16, MSG_MARKET_MINE = 0x17,
       MSG_MARKET_CANCEL = 0x18, MSG_MARKET_COLLECT = 0x19 };
enum { MKT_SELL, MKT_BUY, MKT_MINE, MKT_EXIT, MKT_MENU_ROWS };
enum { PRICE_QTY, PRICE_EACH, PRICE_LIST, PRICE_CANCEL, PRICE_ROWS };
enum { BUY_QTY, BUY_OK, BUY_CANCEL, BUY_ROWS };

#define MKT_MAX_BAG 160
#define MKT_MAX_LIST 48
#define MKT_LIST_ROWS 6
#define MKT_MAX_MONEY 999999
#define MKT_MAX_PRICE 999999
#define MKT_FEE_PERCENT 5

static const u8 sText_MktSell[] = _("SELL");
static const u8 sText_MktBuy[] = _("BUY");
static const u8 sText_MktMine[] = _("MY SHOP");
static const u8 sText_MktExit[] = _("EXIT");
static const u8 sText_MktWhatSell[] = _("WHAT WILL YOU SELL?");
static const u8 sText_MktQuantity[] = _("QUANTITY");
static const u8 sText_MktPriceEach[] = _("PRICE EACH");
static const u8 sText_MktYouGet[] = _("YOU GET (-5% FEE)");
static const u8 sText_MktListIt[] = _("PUT ON SALE");
static const u8 sText_MktTotal[] = _("TOTAL");
static const u8 sText_MktYouHave[] = _("YOU HAVE");
static const u8 sText_MktBuyIt[] = _("BUY");
static const u8 sText_MktCollect[] = _("COLLECT");
static const u8 sText_MktFrom[] = _("FROM ");
static const u8 sText_MktNothingToSell1[] = _("You have no items that");
static const u8 sText_MktNothingToSell2[] = _("can be sold here.");
static const u8 sText_MktNothingForSale1[] = _("Nothing is for sale");
static const u8 sText_MktNothingForSale2[] = _("right now.");
static const u8 sText_MktListed[] = _("Your items are on sale!");
static const u8 sText_MktListed2[] = _("Collect the money at MY SHOP.");
static const u8 sText_MktTooMany1[] = _("You already have 10 things");
static const u8 sText_MktTooMany2[] = _("on sale.");
static const u8 sText_MktBought[] = _("Thank you! You bought them.");
static const u8 sText_MktSoldOut[] = _("Sorry, that sold out.");
static const u8 sText_MktChanged[] = _("That offer just changed.");
static const u8 sText_MktNoMoney[] = _("You don't have enough money.");
static const u8 sText_MktNoRoom[] = _("There's no room in the BAG.");
static const u8 sText_MktCollected[] = _("You collected your money!");
static const u8 sText_MktTookBack[] = _("You took your items back.");
static const u8 sText_MktNoSales[] = _("Nothing on sale yet.");

struct MarketListing
{
    u32 id;
    u16 item, quantity;
    u32 price;
    u8 name[PLAYER_NAME_LENGTH + 1];
};

struct MarketScreen
{
    u16 bagItem[MKT_MAX_BAG];
    u16 bagQty[MKT_MAX_BAG];
    u8 bagCount;
    u16 item, quantity, maxQuantity;
    u32 price;
    u8 count;
    u8 sel;
    u8 rowsBeforeListings;   // MY SHOP: the COLLECT row
    u32 wallet;
    u16 accel;
    struct MarketListing list[MKT_MAX_LIST];
};
static EWRAM_DATA struct MarketScreen *sMkt = NULL;

static u32 MyMoney(void)
{
    return GetMoney(&gSaveBlock1Ptr->money);
}

// "₽12345"
static u8 *MoneyText(u8 *dst, u32 amount)
{
    *dst++ = CHAR_CURRENCY;
    return ConvertIntToDecimalStringN(dst, amount, STR_CONV_MODE_LEFT_ALIGN, 7);
}

// "x12"
static u8 *QtyText(u8 *dst, u16 qty)
{
    *dst++ = CHAR_x;
    return ConvertIntToDecimalStringN(dst, qty, STR_CONV_MODE_LEFT_ALIGN, 3);
}

static bool8 MarketTradable(u16 item)
{
    u8 pocket = ItemId_GetPocket(item);
    return item != ITEM_NONE && item < ITEMS_COUNT && pocket != POCKET_KEY_ITEMS
        && ItemId_GetImportance(item) == 0 && !ItemIsMail(item);
}

static void BuildBagList(void)
{
    static const u8 sPockets[] = {POCKET_ITEMS, POCKET_POKE_BALLS, POCKET_TM_CASE, POCKET_BERRY_POUCH};
    int p, i;
    sMkt->bagCount = 0;
    for (p = 0; p < NELEMS(sPockets); p++)
    {
        const struct BagPocket *pocket = &gBagPockets[sPockets[p] - 1];
        for (i = 0; i < pocket->capacity && sMkt->bagCount < MKT_MAX_BAG; i++)
        {
            u16 item = pocket->itemSlots[i].itemId;
            u16 qty = BagGetQuantityByPocketPosition(sPockets[p], i);
            if (qty && MarketTradable(item))
            {
                sMkt->bagItem[sMkt->bagCount] = item;
                sMkt->bagQty[sMkt->bagCount] = qty;
                sMkt->bagCount++;
            }
        }
    }
}

// Hold LEFT/RIGHT to change faster.
static s32 Accelerated(s32 slow)
{
    if (!JOY_HELD(DPAD_LEFT) && !JOY_HELD(DPAD_RIGHT))
        sMkt->accel = 0;
    else if (sMkt->accel < 0xFFFF)
        sMkt->accel++;
    if (sMkt->accel > 40 && slow < 1000) return slow * 1000;
    if (sMkt->accel > 25) return slow * 100;
    if (sMkt->accel > 12) return slow * 10;
    return slow;
}

static void MarketPrintRight(int line, const u8 *text, const u8 *color)
{
    AddTextPrinterParameterized3(WIN_LIST, FONT_NORMAL, 0xC0 - GetStringWidth(FONT_NORMAL, text, 0),
                                 line * RowHeight() + 2, color, TEXT_SKIP_DRAW, text);
}

// -- top menu

static void DrawMarketMenu(void)
{
    PrintHeader(sText_Market);
    FillWindowPixelBuffer(WIN_LIST, PIXEL_FILL(1));
    PrintLine(MKT_SELL, sText_MktSell, sTextColor);
    PrintLine(MKT_BUY, sText_MktBuy, sTextColor);
    PrintLine(MKT_MINE, sText_MktMine, sTextColor);
    PrintLine(MKT_EXIT, sText_MktExit, sTextColor);
    FinishList();
    SetHighlight(sOnline->cursor);
    PrintHint(sText_HintBrowse);
}

static void MarketShowMenu(void)
{
    sOnline->mode = MODE_MKT_MENU;
    if (sOnline->cursor >= MKT_MENU_ROWS)
        sOnline->cursor = 0;
    DrawMarketMenu();
}

// -- SELL: pick an item from the bag, then quantity and price

static void DrawSellItems(void)
{
    int i;
    u8 buf[16];
    PrintHeader(sText_MktWhatSell);
    FillWindowPixelBuffer(WIN_LIST, PIXEL_FILL(1));
    for (i = 0; i < VISIBLE_ROWS && sOnline->scroll + i < sMkt->bagCount; i++)
    {
        int row = sOnline->scroll + i;
        PrintLine(i, ItemId_GetName(sMkt->bagItem[row]), sTextColor);
        *QtyText(buf, sMkt->bagQty[row]) = EOS;
        MarketPrintRight(i, buf, sValueColor);
    }
    DrawScrollArrows(sOnline->scroll, sMkt->bagCount, VISIBLE_ROWS);
    FinishList();
    SetHighlight(sOnline->cursor - sOnline->scroll);
    PrintHint(sText_HintBrowse);
}

static void MarketShowSellItems(void)
{
    BuildBagList();
    if (sMkt->bagCount == 0)
    {
        ShowMessage(sText_MktNothingToSell1, sText_MktNothingToSell2, NULL, FALSE, MODE_MKT_MENU);
        return;
    }
    sOnline->mode = MODE_MKT_ITEMS;
    sOnline->cursor = sOnline->scroll = 0;
    DrawSellItems();
}

static const u8 sPriceLines[PRICE_ROWS] = {1, 2, 5, 6};

static void DrawPrice(void)
{
    u8 buf[24];
    PrintHeader(sText_Market);
    FillWindowPixelBuffer(WIN_LIST, PIXEL_FILL(1));
    PrintLine(0, ItemId_GetName(sMkt->item), sValueColor);
    PrintLine(1, sText_MktQuantity, sTextColor);
    *QtyText(buf, sMkt->quantity) = EOS;
    MarketPrintRight(1, buf, sValueColor);
    PrintLine(2, sText_MktPriceEach, sTextColor);
    *MoneyText(buf, sMkt->price) = EOS;
    MarketPrintRight(2, buf, sValueColor);
    PrintLine(3, sText_MktYouGet, sTextColor);
    *MoneyText(buf, (u32)((u64)sMkt->price * sMkt->quantity * (100 - MKT_FEE_PERCENT) / 100)) = EOS;
    MarketPrintRight(3, buf, sTextColor);
    PrintLine(5, sText_MktListIt, sTextColor);
    PrintLine(6, sText_GtsCancel, sTextColor);
    FinishList();
    SetHighlight(sPriceLines[sOnline->cursor]);
    PrintHint(sText_HintChange);
}

static void MarketShowPrice(void)
{
    u16 base;
    sOnline->mode = MODE_MKT_PRICE;
    sMkt->item = sMkt->bagItem[sMkt->sel];
    sMkt->maxQuantity = min(sMkt->bagQty[sMkt->sel], 999);
    sMkt->quantity = 1;
    base = ItemId_GetPrice(sMkt->item);
    sMkt->price = base ? base : 100;   // start at the POKé MART price
    sMkt->accel = 0;
    sOnline->cursor = PRICE_QTY;
    DrawPrice();
}

// -- BUY: everyone's offers, cheapest of each item first

static void DrawBrowse(void)
{
    int i;
    u8 buf[24];
    u8 *end;
    PrintHeader(sText_Market);
    FillWindowPixelBuffer(WIN_LIST, PIXEL_FILL(1));
    for (i = 0; i < MKT_LIST_ROWS && sOnline->scroll + i < sMkt->count; i++)
    {
        const struct MarketListing *l = &sMkt->list[sOnline->scroll + i];
        PrintLine(i, ItemId_GetName(l->item), sTextColor);
        *QtyText(buf, l->quantity) = EOS;
        AddTextPrinterParameterized3(WIN_LIST, FONT_NORMAL, 0x70, i * RowHeight() + 2, sTextColor, TEXT_SKIP_DRAW, buf);
        *MoneyText(buf, l->price) = EOS;
        MarketPrintRight(i, buf, sValueColor);
    }
    end = StringCopy(buf, sText_MktFrom);
    StringCopy(end, sMkt->list[sOnline->cursor].name);
    PrintLine(MKT_LIST_ROWS, buf, sValueColor);
    DrawScrollArrows(sOnline->scroll, sMkt->count, MKT_LIST_ROWS);
    FinishList();
    SetHighlight(sOnline->cursor - sOnline->scroll);
    PrintHint(sText_HintBrowse);
}

static void MarketFetchBrowse(void)
{
    PcNetMasterRequest(MSG_MARKET_BROWSE, NULL, 0);
    StartBusy(BUSY_MKT_BROWSE);
}

static u16 MaxBuyable(const struct MarketListing *l)
{
    u16 n = l->quantity;
    u32 money = MyMoney();
    if (l->price && money / l->price < n)
        n = money / l->price;
    while (n > 0 && !CheckBagHasSpace(l->item, n))
        n--;
    return n;
}

static const u8 sBuyLines[BUY_ROWS] = {1, 5, 6};

static void DrawBuy(void)
{
    const struct MarketListing *l = &sMkt->list[sMkt->sel];
    u8 buf[24];
    PrintHeader(sText_Market);
    FillWindowPixelBuffer(WIN_LIST, PIXEL_FILL(1));
    PrintLine(0, ItemId_GetName(l->item), sValueColor);
    *MoneyText(buf, l->price) = EOS;
    MarketPrintRight(0, buf, sValueColor);
    PrintLine(1, sText_MktQuantity, sTextColor);
    *QtyText(buf, sMkt->quantity) = EOS;
    MarketPrintRight(1, buf, sValueColor);
    PrintLine(2, sText_MktTotal, sTextColor);
    *MoneyText(buf, l->price * sMkt->quantity) = EOS;
    MarketPrintRight(2, buf, sTextColor);
    PrintLine(3, sText_MktYouHave, sTextColor);
    *MoneyText(buf, MyMoney()) = EOS;
    MarketPrintRight(3, buf, sTextColor);
    PrintLine(5, sText_MktBuyIt, sTextColor);
    PrintLine(6, sText_GtsCancel, sTextColor);
    FinishList();
    SetHighlight(sBuyLines[sOnline->cursor]);
    PrintHint(sText_HintChange);
}

static void MarketShowBuy(void)
{
    const struct MarketListing *l = &sMkt->list[sMkt->sel];
    sOnline->mode = MODE_MKT_BUY;
    sMkt->maxQuantity = MaxBuyable(l);
    sMkt->quantity = sMkt->maxQuantity ? 1 : 0;
    sMkt->accel = 0;
    sOnline->cursor = BUY_QTY;
    DrawBuy();
}

// -- MY SHOP: money waiting to be collected, your offers (A takes one back)

static u8 MineRowCount(void)
{
    return sMkt->rowsBeforeListings + sMkt->count + 1;   // + BACK
}

static void DrawMine(void)
{
    int i;
    u8 buf[24];
    PrintHeader(sText_MktMine);
    FillWindowPixelBuffer(WIN_LIST, PIXEL_FILL(1));
    if (sMkt->count == 0 && !sMkt->rowsBeforeListings)
        PrintLine(1, sText_MktNoSales, sTextColor);
    for (i = 0; i < VISIBLE_ROWS && sOnline->scroll + i < MineRowCount(); i++)
    {
        int row = sOnline->scroll + i;
        int line = i + (sMkt->count == 0 && !sMkt->rowsBeforeListings ? 3 : 0);
        if (row < sMkt->rowsBeforeListings)
        {
            PrintLine(line, sText_MktCollect, sTextColor);
            *MoneyText(buf, sMkt->wallet) = EOS;
            MarketPrintRight(line, buf, sValueColor);
        }
        else if (row == MineRowCount() - 1)
        {
            PrintLine(line, sText_Back, sTextColor);
        }
        else
        {
            const struct MarketListing *l = &sMkt->list[row - sMkt->rowsBeforeListings];
            PrintLine(line, ItemId_GetName(l->item), sTextColor);
            *QtyText(buf, l->quantity) = EOS;
            AddTextPrinterParameterized3(WIN_LIST, FONT_NORMAL, 0x70, line * RowHeight() + 2, sTextColor, TEXT_SKIP_DRAW, buf);
            *MoneyText(buf, l->price) = EOS;
            MarketPrintRight(line, buf, sValueColor);
        }
    }
    DrawScrollArrows(sOnline->scroll, MineRowCount(), VISIBLE_ROWS);
    FinishList();
    SetHighlight(sOnline->cursor - sOnline->scroll + (sMkt->count == 0 && !sMkt->rowsBeforeListings ? 3 : 0));
    PrintHint(sText_HintBrowse);
}

static void MarketFetchMine(void)
{
    PcNetMasterRequest(MSG_MARKET_MINE, NULL, 0);
    StartBusy(BUSY_MKT_MINE);
}

static void MarketSave(void)
{
    TrySavingData(SAVE_NORMAL);
}

// M_MARKET_LIST: u8 n, n x (u32 id, u16 item, u16 qty, u32 price, name[8]);
// M_MARKET_MINE_LIST: u32 wallet, u8 n, n x (u32 id, u16 item, u16 qty, u32 price)
static void MarketParse(const u8 *p, int len, bool8 mine)
{
    int i, n, size = mine ? 12 : 12 + PC_NET_NAME_LEN;
    sMkt->count = 0;
    if (mine)
    {
        if (len < 5)
            return;
        sMkt->wallet = p[0] | (p[1] << 8) | (p[2] << 16) | ((u32)p[3] << 24);
        p += 4;
        len -= 4;
    }
    n = len >= 1 ? p[0] : 0;
    for (i = 0; i < n && sMkt->count < MKT_MAX_LIST && 1 + (i + 1) * size <= len; i++)
    {
        const u8 *e = p + 1 + i * size;
        struct MarketListing *l = &sMkt->list[sMkt->count];
        l->id = e[0] | (e[1] << 8) | (e[2] << 16) | ((u32)e[3] << 24);
        l->item = e[4] | (e[5] << 8);
        l->quantity = e[6] | (e[7] << 8);
        l->price = e[8] | (e[9] << 8) | (e[10] << 16) | ((u32)e[11] << 24);
        l->name[0] = EOS;
        if (!mine)
            CopyNetName(l->name, e + 12);
        if (MarketTradable(l->item) && l->quantity && l->price)
            sMkt->count++;
    }
}

static void MarketHandleBusy(void)
{
    int status = PcNetOpStatus();
    u8 type;
    const u8 *reply;
    int len;

    if (status == PC_NET_OP_PENDING)
        return;
    len = status == PC_NET_OP_OK ? PcNetMasterReply(&type, &reply) : 0;
    switch (sOnline->busyOp)
    {
    case BUSY_MKT_SELL:
        if (status == PC_NET_OP_OK)
        {
            PlaySE(SE_SHOP);
            ShowMessage(sText_MktListed, sText_MktListed2, NULL, FALSE, MODE_MKT_MENU);
        }
        else
        {
            AddBagItem(sMkt->item, sMkt->quantity);   // the server didn't take them: give them back
            MarketSave();
            if (PcNetMasterError() == NET_ERR_FULL)
                ShowMessage(sText_MktTooMany1, sText_MktTooMany2, NULL, FALSE, MODE_MKT_MENU);
            else
                ShowMessage(sText_CantReach1, sText_CantReach2, NULL, FALSE, MODE_MKT_MENU);
        }
        break;
    case BUSY_MKT_BROWSE:
        if (status != PC_NET_OP_OK)
            ShowMessage(sText_CantReach1, sText_CantReach2, NULL, FALSE, MODE_MKT_MENU);
        else
        {
            MarketParse(reply, len, FALSE);
            if (sMkt->count == 0)
                ShowMessage(sText_MktNothingForSale1, sText_MktNothingForSale2, NULL, FALSE, MODE_MKT_MENU);
            else
            {
                sOnline->mode = MODE_MKT_BROWSE;
                if (sOnline->cursor >= sMkt->count)
                    sOnline->cursor = sMkt->count - 1;
                if (sOnline->scroll > sOnline->cursor)
                    sOnline->scroll = sOnline->cursor;
                DrawBrowse();
            }
        }
        break;
    case BUSY_MKT_BUY:
        if (status == PC_NET_OP_OK && len >= 8)
        {
            u16 item = reply[0] | (reply[1] << 8), qty = reply[2] | (reply[3] << 8);
            u32 cost = reply[4] | (reply[5] << 8) | (reply[6] << 16) | ((u32)reply[7] << 24);
            RemoveMoney(&gSaveBlock1Ptr->money, cost);
            AddBagItem(item, qty);
            MarketSave();
            PlaySE(SE_SHOP);
            ShowMessage(sText_MktBought, NULL, NULL, FALSE, MODE_MKT_BROWSE);
        }
        else if (PcNetMasterError() == NET_ERR_NOT_FOUND)
            ShowMessage(sText_MktSoldOut, NULL, NULL, FALSE, MODE_MKT_BROWSE);
        else if (PcNetMasterError() == NET_ERR_NO_MATCH)
            ShowMessage(sText_MktChanged, NULL, NULL, FALSE, MODE_MKT_BROWSE);
        else
            ShowMessage(sText_CantReach1, sText_CantReach2, NULL, FALSE, MODE_MKT_MENU);
        break;
    case BUSY_MKT_MINE:
        if (status != PC_NET_OP_OK)
            ShowMessage(sText_CantReach1, sText_CantReach2, NULL, FALSE, MODE_MKT_MENU);
        else
        {
            MarketParse(reply, len, TRUE);
            sMkt->rowsBeforeListings = sMkt->wallet ? 1 : 0;
            sOnline->mode = MODE_MKT_MINE;
            if (sOnline->cursor >= MineRowCount())
                sOnline->cursor = MineRowCount() - 1;
            if (sOnline->scroll > sOnline->cursor)
                sOnline->scroll = sOnline->cursor;
            DrawMine();
        }
        break;
    case BUSY_MKT_CANCEL:
        if (status == PC_NET_OP_OK && len >= 4)
        {
            AddBagItem(reply[0] | (reply[1] << 8), reply[2] | (reply[3] << 8));
            MarketSave();
            ShowMessage(sText_MktTookBack, NULL, NULL, FALSE, MODE_MKT_MINE);
        }
        else if (PcNetMasterError() == NET_ERR_NOT_FOUND)
            ShowMessage(sText_MktSoldOut, NULL, NULL, FALSE, MODE_MKT_MINE);
        else
            ShowMessage(sText_CantReach1, sText_CantReach2, NULL, FALSE, MODE_MKT_MENU);
        break;
    case BUSY_MKT_COLLECT:
        if (status == PC_NET_OP_OK && len >= 4)
        {
            AddMoney(&gSaveBlock1Ptr->money, reply[0] | (reply[1] << 8) | (reply[2] << 16) | ((u32)reply[3] << 24));
            MarketSave();
            PlaySE(SE_SHOP);
            ShowMessage(sText_MktCollected, NULL, NULL, FALSE, MODE_MKT_MINE);
        }
        else
            ShowMessage(sText_CantReach1, sText_CantReach2, NULL, FALSE, MODE_MKT_MENU);
        break;
    }
}

// After a message: back to where it came from (fresh lists from the server).
static void MarketReturn(u8 mode)
{
    if (mode == MODE_MKT_BROWSE)
        MarketFetchBrowse();
    else if (mode == MODE_MKT_MINE)
        MarketFetchMine();
    else
        MarketShowMenu();
}

static void MarketMenuInput(u8 taskId)
{
    if (JOY_NEW(DPAD_UP) || JOY_NEW(DPAD_DOWN))
    {
        sOnline->cursor = (sOnline->cursor + (JOY_NEW(DPAD_UP) ? MKT_MENU_ROWS - 1 : 1)) % MKT_MENU_ROWS;
        PlaySE(SE_SELECT);
        DrawMarketMenu();
    }
    else if (JOY_NEW(B_BUTTON) || (JOY_NEW(A_BUTTON) && sOnline->cursor == MKT_EXIT))
    {
        PlaySE(SE_SELECT);
        LeaveScreen(taskId);
    }
    else if (JOY_NEW(A_BUTTON))
    {
        u8 pick = sOnline->cursor;
        PlaySE(SE_SELECT);
        sOnline->cursor = sOnline->scroll = 0;
        if (pick == MKT_SELL)
            MarketShowSellItems();
        else if (pick == MKT_BUY)
            MarketFetchBrowse();
        else
            MarketFetchMine();
    }
}

static void SellItemsInput(u8 taskId)
{
    if (ListMove(sMkt->bagCount))
        DrawSellItems();
    else if (JOY_NEW(B_BUTTON))
    {
        PlaySE(SE_SELECT);
        sOnline->cursor = MKT_SELL;
        MarketShowMenu();
    }
    else if (JOY_NEW(A_BUTTON))
    {
        PlaySE(SE_SELECT);
        sMkt->sel = sOnline->cursor;
        MarketShowPrice();
    }
}

static void PriceInput(u8 taskId)
{
    if (JOY_NEW(DPAD_UP) || JOY_NEW(DPAD_DOWN))
    {
        sOnline->cursor = (sOnline->cursor + (JOY_NEW(DPAD_UP) ? PRICE_ROWS - 1 : 1)) % PRICE_ROWS;
        PlaySE(SE_SELECT);
        DrawPrice();
    }
    else if ((JOY_REPT(DPAD_LEFT) || JOY_REPT(DPAD_RIGHT)) && sOnline->cursor <= PRICE_EACH)
    {
        s32 dir = JOY_REPT(DPAD_LEFT) ? -1 : 1;
        if (sOnline->cursor == PRICE_QTY)
        {
            s32 q = sMkt->quantity + dir * (sMkt->accel > 12 ? 10 : 1);
            sMkt->accel++;
            sMkt->quantity = q < 1 ? 1 : q > sMkt->maxQuantity ? sMkt->maxQuantity : q;
        }
        else
        {
            s32 p = (s32)sMkt->price + dir * Accelerated(1);
            sMkt->price = p < 1 ? 1 : p > MKT_MAX_PRICE ? MKT_MAX_PRICE : p;
        }
        PlaySE(SE_SELECT);
        DrawPrice();
    }
    else if (!JOY_HELD(DPAD_LEFT) && !JOY_HELD(DPAD_RIGHT))
    {
        sMkt->accel = 0;
        if (JOY_NEW(B_BUTTON) || (JOY_NEW(A_BUTTON) && sOnline->cursor == PRICE_CANCEL))
        {
            PlaySE(SE_SELECT);
            sOnline->cursor = sMkt->sel;
            sOnline->mode = MODE_MKT_ITEMS;
            DrawSellItems();
        }
        else if (JOY_NEW(A_BUTTON) && sOnline->cursor == PRICE_LIST)
        {
            u8 payload[8];
            PlaySE(SE_SELECT);
            // off the bag (and saved) before the server has them: never duplicated
            if (!RemoveBagItem(sMkt->item, sMkt->quantity))
                return;
            MarketSave();
            payload[0] = sMkt->item;
            payload[1] = sMkt->item >> 8;
            payload[2] = sMkt->quantity;
            payload[3] = sMkt->quantity >> 8;
            payload[4] = sMkt->price;
            payload[5] = sMkt->price >> 8;
            payload[6] = sMkt->price >> 16;
            payload[7] = sMkt->price >> 24;
            sOnline->cursor = MKT_SELL;
            PcNetMasterRequest(MSG_MARKET_SELL, payload, sizeof(payload));
            StartBusy(BUSY_MKT_SELL);
        }
    }
}

static void BrowseInput(u8 taskId)
{
    if (JOY_REPT(DPAD_UP) || JOY_REPT(DPAD_DOWN))
    {
        if (JOY_REPT(DPAD_UP))
            sOnline->cursor = sOnline->cursor == 0 ? sMkt->count - 1 : sOnline->cursor - 1;
        else
            sOnline->cursor = sOnline->cursor + 1 >= sMkt->count ? 0 : sOnline->cursor + 1;
        if (sOnline->cursor < sOnline->scroll)
            sOnline->scroll = sOnline->cursor;
        else if (sOnline->cursor >= sOnline->scroll + MKT_LIST_ROWS)
            sOnline->scroll = sOnline->cursor - MKT_LIST_ROWS + 1;
        PlaySE(SE_SELECT);
        DrawBrowse();
    }
    else if (JOY_NEW(B_BUTTON))
    {
        PlaySE(SE_SELECT);
        sOnline->cursor = MKT_BUY;
        MarketShowMenu();
    }
    else if (JOY_NEW(A_BUTTON))
    {
        PlaySE(SE_SELECT);
        sMkt->sel = sOnline->cursor;
        MarketShowBuy();
    }
}

static void BuyInput(u8 taskId)
{
    const struct MarketListing *l = &sMkt->list[sMkt->sel];
    if (JOY_NEW(DPAD_UP) || JOY_NEW(DPAD_DOWN))
    {
        sOnline->cursor = (sOnline->cursor + (JOY_NEW(DPAD_UP) ? BUY_ROWS - 1 : 1)) % BUY_ROWS;
        PlaySE(SE_SELECT);
        DrawBuy();
    }
    else if ((JOY_REPT(DPAD_LEFT) || JOY_REPT(DPAD_RIGHT)) && sOnline->cursor == BUY_QTY && sMkt->maxQuantity)
    {
        s32 q = sMkt->quantity + (JOY_REPT(DPAD_LEFT) ? -1 : 1) * (sMkt->accel > 12 ? 10 : 1);
        sMkt->accel++;
        sMkt->quantity = q < 1 ? 1 : q > sMkt->maxQuantity ? sMkt->maxQuantity : q;
        PlaySE(SE_SELECT);
        DrawBuy();
    }
    else if (!JOY_HELD(DPAD_LEFT) && !JOY_HELD(DPAD_RIGHT))
    {
        sMkt->accel = 0;
        if (JOY_NEW(B_BUTTON) || (JOY_NEW(A_BUTTON) && sOnline->cursor == BUY_CANCEL))
        {
            PlaySE(SE_SELECT);
            sOnline->cursor = sMkt->sel;
            sOnline->mode = MODE_MKT_BROWSE;
            DrawBrowse();
        }
        else if (JOY_NEW(A_BUTTON) && sOnline->cursor == BUY_OK)
        {
            u8 payload[10];
            PlaySE(SE_SELECT);
            if (sMkt->quantity == 0 || !IsEnoughMoney(&gSaveBlock1Ptr->money, l->price * sMkt->quantity))
            {
                ShowMessage(sText_MktNoMoney, NULL, NULL, FALSE, MODE_MKT_BROWSE);
                return;
            }
            if (!CheckBagHasSpace(l->item, sMkt->quantity))
            {
                ShowMessage(sText_MktNoRoom, NULL, NULL, FALSE, MODE_MKT_BROWSE);
                return;
            }
            payload[0] = l->id;
            payload[1] = l->id >> 8;
            payload[2] = l->id >> 16;
            payload[3] = l->id >> 24;
            payload[4] = sMkt->quantity;
            payload[5] = sMkt->quantity >> 8;
            payload[6] = l->price;
            payload[7] = l->price >> 8;
            payload[8] = l->price >> 16;
            payload[9] = l->price >> 24;
            sOnline->cursor = sMkt->sel;
            PcNetMasterRequest(MSG_MARKET_BUY, payload, sizeof(payload));
            StartBusy(BUSY_MKT_BUY);
        }
    }
}

static void MineInput(u8 taskId)
{
    u8 count = MineRowCount();
    if (ListMove(count))
        DrawMine();
    else if (JOY_NEW(B_BUTTON) || (JOY_NEW(A_BUTTON) && sOnline->cursor == count - 1))
    {
        PlaySE(SE_SELECT);
        sOnline->cursor = MKT_MINE;
        MarketShowMenu();
    }
    else if (JOY_NEW(A_BUTTON) && sOnline->cursor < sMkt->rowsBeforeListings)
    {
        u32 room = MKT_MAX_MONEY - min(MyMoney(), MKT_MAX_MONEY);
        u8 payload[4] = {room, room >> 8, room >> 16, room >> 24};
        PlaySE(SE_SELECT);
        if (room == 0)
            return;
        PcNetMasterRequest(MSG_MARKET_COLLECT, payload, sizeof(payload));
        StartBusy(BUSY_MKT_COLLECT);
    }
    else if (JOY_NEW(A_BUTTON))
    {
        PlaySE(SE_SELECT);
        sMkt->sel = sOnline->cursor - sMkt->rowsBeforeListings;
        ShowConfirm(CONFIRM_TAKE_BACK);
    }
}

static void MarketTakeBack(void)
{
    const struct MarketListing *l = &sMkt->list[sMkt->sel];
    u8 payload[4] = {l->id, l->id >> 8, l->id >> 16, l->id >> 24};
    if (!CheckBagHasSpace(l->item, l->quantity))
    {
        ShowMessage(sText_MktNoRoom, NULL, NULL, FALSE, MODE_MKT_MINE);
        return;
    }
    PcNetMasterRequest(MSG_MARKET_CANCEL, payload, sizeof(payload));
    StartBusy(BUSY_MKT_CANCEL);
}

static void MarketStart(void)
{
    sMkt = AllocZeroed(sizeof(*sMkt));
    sOnline->cursor = 0;
    MarketShowMenu();
}

static void Task_OnlineScreen(u8 taskId)
{
    if (gTasks[taskId].data[0] == 2)   // off to the naming screen
    {
        if (!gPaletteFade.active)
        {
            FreeAllWindowBuffers();
            DestroyTask(taskId);
            if (sOnline->naming == NAMING_GUILD_CREATE || sOnline->naming == NAMING_GUILD_JOIN)
                DoNamingScreen(NAMING_SCREEN_GUILD, sGuildNameBuf, gSaveBlock2Ptr->playerGender, 0, 0, CB2_OnlineAfterNaming);
            else
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
            FREE_AND_SET_NULL(sMkt);
            DestroyTask(taskId);
            if (sJoinedWarp)
            {
                // Everyone on a server starts out in their region's first
                // town (Pallet / Littleroot), between the houses; the welcome
                // message plays on arrival.
                u8 group, num, facing;
                s16 x, y;

                sJoinedWarp = FALSE;
                sShowJoinedMessage = TRUE;
                if (PcNetLastPosition(&group, &num, &x, &y, &facing))   // back where they left off
                    SetWarpDestination(group, num, -1, x, y);
                else
#ifdef EMERALD
                SetWarpDestination(MAP_GROUP(MAP_LITTLEROOT_TOWN), MAP_NUM(MAP_LITTLEROOT_TOWN), -1, 10, 10);
#else
                SetWarpDestination(MAP_GROUP(MAP_PALLET_TOWN), MAP_NUM(MAP_PALLET_TOWN), -1, 6, 8);
#endif
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
            else if (sOnline->messageMode == MODE_GUILD)
                ShowGuild();
            else if (sOnline->messageMode >= MODE_MKT_MENU)
                MarketReturn(sOnline->messageMode);
            else
                ShowServers();
        }
        break;
    case MODE_FRIENDS:
        FriendsInput(taskId);
        break;
    case MODE_FRIEND_MENU:
        FriendMenuInput(taskId);
        break;
    case MODE_GUILD:
        GuildInput(taskId);
        break;
    case MODE_CONFIRM:
        ConfirmInput(taskId);
        break;
    case MODE_MKT_MENU:
        MarketMenuInput(taskId);
        break;
    case MODE_MKT_ITEMS:
        SellItemsInput(taskId);
        break;
    case MODE_MKT_PRICE:
        PriceInput(taskId);
        break;
    case MODE_MKT_BROWSE:
        BrowseInput(taskId);
        break;
    case MODE_MKT_BUY:
        BuyInput(taskId);
        break;
    case MODE_MKT_MINE:
        MineInput(taskId);
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
#ifdef EMERALD
        LoadBgTiles(1, GetWindowFrameTilesPal(gSaveBlock2Ptr->optionsWindowFrameType)->tiles, 0x120, 0x1AA);
        LoadPalette(GetWindowFrameTilesPal(gSaveBlock2Ptr->optionsWindowFrameType)->pal, BG_PLTT_ID(2), PLTT_SIZE_4BPP);
        LoadPalette(sOnlineBgPalette, BG_PLTT_ID(0), sizeof(sOnlineBgPalette));
        LoadPalette(sOnlinePalette, BG_PLTT_ID(1), sizeof(sOnlinePalette));
        LoadPalette(GetTextWindowPalette(2), BG_PLTT_ID(15), PLTT_SIZE_4BPP);
#else
        LoadBgTiles(1, GetUserWindowGraphics(gSaveBlock2Ptr->optionsWindowFrameType)->tiles, 0x120, 0x1AA);
        LoadPalette(GetUserWindowGraphics(gSaveBlock2Ptr->optionsWindowFrameType)->palette, BG_PLTT_ID(2), PLTT_SIZE_4BPP);
        LoadPalette(sOnlinePalette, BG_PLTT_ID(1), sizeof(sOnlinePalette));
        LoadPalette(GetTextWindowPalette(2), BG_PLTT_ID(15), PLTT_SIZE_4BPP);
        LoadStdWindowGfxOnBg(1, 0x1B3, BG_PLTT_ID(3));
#endif
        break;
    case 4:
        DrawFrames();
        break;
    case 5:
        if (sOnline->gtsMode)
        {
            GtsStart();
        }
        else if (sOnline->market)
        {
            MarketStart();
        }
        else if (sOnline->naming == NAMING_GUILD_CREATE || sOnline->naming == NAMING_GUILD_JOIN)
        {
            u8 op = sOnline->naming == NAMING_GUILD_CREATE ? MSG_GUILD_CREATE : MSG_GUILD_JOIN;
            int i;
            sOnline->naming = NAMING_NONE;
            for (i = 0; i < GUILD_NAME_LEN && sGuildNameBuf[i] != EOS; i++)
                ;
            for (; i < GUILD_NAME_LEN; i++)
                sGuildNameBuf[i] = EOS;
            if (sGuildNameBuf[0] != EOS)
                GuildRequest(op, sGuildNameBuf);
            else
                ShowGuild();
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
#define GUIDE_LOCALID 0xDF   // the online guide on every Pokemon Center 2F (pc_online.inc)
// a professor's lab aide (they have walking sprites): Oak's in Kanto, Birch's in Hoenn
#ifdef EMERALD
#define GUIDE_GFX OBJ_EVENT_GFX_SCIENTIST_1
#else
#define GUIDE_GFX OBJ_EVENT_GFX_SCIENTIST
#endif
// The 2F floor plan (same in every Pokemon Center of both regions): the desks
// are at x = 2 / 6 / 10 behind the counter; y = 4 is the row in front of it,
// rows 5 and 6 are open floor and the stairs down are at (1, 6).
#define GUIDE_DESK_ROW 4
#define GUIDE_WALK_ROW 5     // he walks along here between desks
#define GUIDE_LEAVE_ROW 6    // and leaves along here
#define GUIDE_STAIRS_X 1
#define GUIDE_STAIRS_Y 6
#define GUIDE_REST_X 4       // where he waits if he can't take the stairs
static const u8 sGuideDeskX[] = {2, 6, 10};   // ONLINE CLUB, GLOBAL TRADE, POKEMON MARKET
#define GUIDE_WALK_LEAVE 3
#define GUIDE_WALK_APPROACH 4
static EWRAM_DATA u8 sGuideMoves[2][48] = {0};
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

// Every trainer looks like their own game's hero: Kanto players as Red/Leaf,
// Hoenn players as Brendan/May, with that game's own sprites (ported between
// the games, xgfx/; see pc_travel.c PcTravelGraphicsInfo).
static u16 AvatarGfx(const struct PcNetPlayer *p)
{
    bool8 hoenn = p->game == PC_NET_GAME_EMERALD || p->game == PC_NET_GAME_RUBY || p->game == PC_NET_GAME_SAPPHIRE;
#ifdef EMERALD
    if (!hoenn)
        return p->gender ? OBJ_EVENT_GFX_LEAF : OBJ_EVENT_GFX_RED;
    return p->gender ? OBJ_EVENT_GFX_LINK_MAY : OBJ_EVENT_GFX_LINK_BRENDAN;   // Emerald's own, not the player's ids
#else
    if (hoenn)
        return p->gender ? OBJ_EVENT_GFX_PC_HOENN_MAY : OBJ_EVENT_GFX_PC_HOENN_BRENDAN;
    return p->gender ? OBJ_EVENT_GFX_PC_KANTO_GREEN : OBJ_EVENT_GFX_PC_KANTO_RED;   // not the player's ids
#endif
}

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
        if (SpawnSpecialObjectEventParameterized(AvatarGfx(p),
                                                 MOVEMENT_TYPE_NONE, AVATAR_LOCALID_BASE + free,
                                                 p->x + MAP_OFFSET, p->y + MAP_OFFSET,
                                                 gObjectEvents[gPlayerAvatar.objectEventId].currentElevation) < OBJECT_EVENTS_COUNT)
            sAvatarSlot[free] = s + 1;
    }
}

// One name tag over a field object, at the top-center of its sprite.
static int AddNameTag(int n, int16_t *x, int16_t *y, u8 (*names)[8], u8 *self,
                      u8 objId, const u8 *name, u8 isSelf)
{
    struct Sprite *sprite;
    int i;

    if (n >= PC_CHAT_MAX_TAGS || objId >= OBJECT_EVENTS_COUNT || !gObjectEvents[objId].active)
        return n;
    sprite = &gSprites[gObjectEvents[objId].spriteId];
    if (sprite->invisible || gObjectEvents[objId].invisible)
        return n;
    // where it was last drawn: the OAM entry (9-bit x, 8-bit y, both wrap)
    x[n] = (sprite->oam.x >= 256 ? sprite->oam.x - 512 : sprite->oam.x) - sprite->centerToCornerVecX;
    y[n] = (sprite->oam.y >= 192 ? sprite->oam.y - 256 : sprite->oam.y) + 9;   // trainer frames are blank above the head
    for (i = 0; i < 7 && name[i] != EOS; i++)
        names[n][i] = name[i];
    names[n][i] = EOS;
    self[n] = isSelf;
    return n + 1;
}

// Everyone's online name over their head while walking around.
static void UpdateNameTags(void)
{
    int16_t x[PC_CHAT_MAX_TAGS], y[PC_CHAT_MAX_TAGS];
    u8 names[PC_CHAT_MAX_TAGS][8], self[PC_CHAT_MAX_TAGS];
    int a, n = 0;
    u8 objId;

    if (gPaletteFade.active || ArePlayerFieldControlsLocked() || ScriptContext_IsEnabled())
        return;   // a menu, a message or a warp is up: the tags would sit on top of it
    n = AddNameTag(n, x, y, names, self, gPlayerAvatar.objectEventId, PcNetMyName(), TRUE);
    for (a = 0; a < MAX_AVATARS; a++)
    {
        const struct PcNetPlayer *p = AvatarPlayer(a);
        if (p && TryGetObjectEventIdByLocalIdAndMap(AVATAR_LOCALID_BASE + a, sAvatarMapNum, sAvatarMapGroup, &objId) == 0)
            n = AddNameTag(n, x, y, names, self, objId, p->name, FALSE);
    }
    PcChatSetNameTags(n, x, y, (const uint8_t (*)[8])names, self);
}

static void SendPresence(void)
{
    struct ObjectEvent *player = &gObjectEvents[gPlayerAvatar.objectEventId];
    u8 flags = (gPlayerAvatar.flags & PLAYER_AVATAR_FLAG_DASH) ? PC_NET_POS_DASH : 0;
    s16 x = player->currentCoords.x - MAP_OFFSET, y = player->currentCoords.y - MAP_OFFSET;

    if (ArePlayerFieldControlsLocked())
        flags |= PC_NET_POS_BUSY;   // busy (talking, in a menu)
    else if ((gPlayerAvatar.flags & PLAYER_AVATAR_FLAG_ON_FOOT) && !InCableClubRoom())
        flags |= PC_NET_POS_SAFE;   // somewhere to come back to after leaving
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

    if (gPcDebugNet.travel && !ArePlayerFieldControlsLocked() && !ScriptContext_IsEnabled())
    {
        // log in, then leave for the other region's first world (the ONLINE CLUB's YES)
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
            if (PcNetOpStatus() == PC_NET_OP_FAILED)
                stage = 0;
            if (PcNetOpStatus() != PC_NET_OP_OK)
                break;
            for (s = 0; s < PcNetServerCount(); s++)
                if (PcNetServerRegion(s) != PC_NET_REGION)
                    break;
            gPcDebugNet.travel = 0;
            if (s < PcNetServerCount())
                PcTravelDepart(s);
            break;
        }
        return;
    }
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
                int i, pick = 0;
                for (i = PcNetServerCount() - 1; i >= 0; i--)   // the first world of our own region
                    if (PcNetServerRegion(i) == PC_NET_REGION)
                        pick = i;
                PcNetBeginJoin(pick, gSaveBlock2Ptr->playerGender);
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

static bool8 OnPokemonCenter2F(void)
{
#ifdef EMERALD
    return gMapHeader.mapLayoutId == LAYOUT_POKEMON_CENTER_2F;
#else
    return gMapHeader.mapLayoutId == LAYOUT_POKEMON_CENTER_2F
        || gMapHeader.mapLayoutId == LAYOUT_ONE_ISLAND_POKEMON_CENTER_2F;
#endif
}

// The first time a trainer comes up to a Pokemon Center 2F, the professor's
// AIDE is standing there. He spots them (!), comes over, walks them through
// each online desk with the player following him, then leaves downstairs.
// Once per save (PC_FLAG_ONLINE_GUIDE).
static void UpdateGuide(void)
{
    static u8 sSettled;   // frames the field has been free: the arrival (fade, map name, ...) is over
    u8 objId;

    if (!OnPokemonCenter2F() || FlagGet(PC_FLAG_ONLINE_GUIDE))
    {
        sSettled = 0;
        return;
    }
    if (TryGetObjectEventIdByLocalIdAndMap(GUIDE_LOCALID, gSaveBlock1Ptr->location.mapNum,
                                           gSaveBlock1Ptr->location.mapGroup, &objId) != 0)
        SpawnSpecialObjectEventParameterized(GUIDE_GFX, MOVEMENT_TYPE_FACE_DOWN, GUIDE_LOCALID,
                                             GUIDE_REST_X + MAP_OFFSET, GUIDE_WALK_ROW + MAP_OFFSET,
                                             gObjectEvents[gPlayerAvatar.objectEventId].currentElevation);
    if (ArePlayerFieldControlsLocked() || ScriptContext_IsEnabled() || gPaletteFade.active
#ifndef EMERALD
     || gQuestLogState != 0
#endif
     )
    {
        sSettled = 0;
        return;
    }
    if (++sSettled < 30)
        return;
    sSettled = 0;
    FlagSet(PC_FLAG_ONLINE_GUIDE);
    gSpecialVar_LastTalked = GUIDE_LOCALID;
    ScriptContext_SetupScript(Online_EventScript_Guide);
}

static u8 *GuideSteps(u8 *m, int n, u8 action)
{
    while (n-- > 0)
        *m++ = action;
    return m;
}

// A walk across the 2F floor: up/down to `row`, along it to x = tx, then
// up/down to ty, ending facing `face`. (Script movement doesn't collide; the
// rows used are open floor in every Pokemon Center.)
static void GuidePath(u8 *m, s16 x, s16 y, s16 row, s16 tx, s16 ty, u8 face)
{
    m = GuideSteps(m, row > y ? row - y : y - row, row > y ? MOVEMENT_ACTION_WALK_NORMAL_DOWN : MOVEMENT_ACTION_WALK_NORMAL_UP);
    m = GuideSteps(m, tx > x ? tx - x : x - tx, tx > x ? MOVEMENT_ACTION_WALK_NORMAL_RIGHT : MOVEMENT_ACTION_WALK_NORMAL_LEFT);
    m = GuideSteps(m, ty > row ? ty - row : row - ty, ty > row ? MOVEMENT_ACTION_WALK_NORMAL_DOWN : MOVEMENT_ACTION_WALK_NORMAL_UP);
    *m++ = face;
    *m = MOVEMENT_ACTION_STEP_END;
}

static struct ObjectEvent *GuideObject(void)
{
    u8 objId;
    if (TryGetObjectEventIdByLocalIdAndMap(GUIDE_LOCALID, gSaveBlock1Ptr->location.mapNum,
                                           gSaveBlock1Ptr->location.mapGroup, &objId))
        return NULL;
    return &gObjectEvents[objId];
}

// special Online_GuideWalk (then waitmovement on both), VAR_0x8004:
//   GUIDE_WALK_APPROACH: the AIDE comes over to stand beside the player;
//   0 / 1 / 2: to the ONLINE CLUB / GLOBAL TRADE / MARKET desk. He stops in
//     front of it and turns around; the player walks in his footsteps one
//     step behind him (they start out next to each other);
//   GUIDE_WALK_LEAVE: he goes down the stairs (or aside, if the player is
//     standing on them).
void Online_GuideWalk(void)
{
    struct ObjectEvent *guide = GuideObject();
    struct ObjectEvent *player = &gObjectEvents[gPlayerAvatar.objectEventId];
    s16 gx, gy, px = player->currentCoords.x - MAP_OFFSET, py = player->currentCoords.y - MAP_OFFSET;
    u8 mapNum = gSaveBlock1Ptr->location.mapNum, mapGroup = gSaveBlock1Ptr->location.mapGroup;

    if (guide == NULL)
        return;
    gx = guide->currentCoords.x - MAP_OFFSET;
    gy = guide->currentCoords.y - MAP_OFFSET;
    if (gSpecialVar_0x8004 == GUIDE_WALK_APPROACH)
    {
        s16 tx = px < 12 ? px + 1 : px - 1;
        u8 *m = sGuideMoves[1];
        GuidePath(sGuideMoves[0], gx, gy, gy, tx, py, tx > px ? MOVEMENT_ACTION_FACE_LEFT : MOVEMENT_ACTION_FACE_RIGHT);
        *m++ = tx > px ? MOVEMENT_ACTION_FACE_RIGHT : MOVEMENT_ACTION_FACE_LEFT;   // the player turns to her
        *m = MOVEMENT_ACTION_STEP_END;
        ScriptMovement_StartObjectMovementScript(LOCALID_PLAYER, mapNum, mapGroup, sGuideMoves[1]);
    }
    else if (gSpecialVar_0x8004 < ARRAY_COUNT(sGuideDeskX))
    {
        s16 desk = sGuideDeskX[gSpecialVar_0x8004], x = gx, y = gy;
        u8 *m = sGuideMoves[0], *f = sGuideMoves[1];
        int steps;

        // him: off the desk row sideways (the player is right behind him),
        // along the open floor, up to the next desk; then he turns around
        if (y == GUIDE_DESK_ROW)
        {
            s16 side = desk > x ? 1 : -1;
            if (x + side == px && y == py)   // the player is on that side: step down instead
            {
                m = GuideSteps(m, 1, MOVEMENT_ACTION_WALK_NORMAL_DOWN);
                y++;
            }
            else
            {
                m = GuideSteps(m, 1, side > 0 ? MOVEMENT_ACTION_WALK_NORMAL_RIGHT : MOVEMENT_ACTION_WALK_NORMAL_LEFT);
                x += side;
            }
        }
        GuidePath(m, x, y, GUIDE_WALK_ROW, desk, GUIDE_DESK_ROW, MOVEMENT_ACTION_FACE_DOWN);
        for (steps = 0; sGuideMoves[0][steps] != MOVEMENT_ACTION_FACE_DOWN; steps++)
            ;
        // the player follows in his footsteps, one step behind: into the
        // tile he just left, then every step but his last
        if (steps > 0)
        {
            *f++ = gx > px ? MOVEMENT_ACTION_WALK_NORMAL_RIGHT : gx < px ? MOVEMENT_ACTION_WALK_NORMAL_LEFT
                 : gy > py ? MOVEMENT_ACTION_WALK_NORMAL_DOWN : MOVEMENT_ACTION_WALK_NORMAL_UP;
            memcpy(f, sGuideMoves[0], steps - 1);
            f += steps - 1;
        }
        *f++ = MOVEMENT_ACTION_FACE_UP;
        *f = MOVEMENT_ACTION_STEP_END;
        ScriptMovement_StartObjectMovementScript(LOCALID_PLAYER, mapNum, mapGroup, sGuideMoves[1]);
    }
    else if (px == GUIDE_STAIRS_X && py == GUIDE_STAIRS_Y)
    {
        GuidePath(sGuideMoves[0], gx, gy, GUIDE_WALK_ROW, GUIDE_REST_X, GUIDE_WALK_ROW, MOVEMENT_ACTION_FACE_DOWN);
    }
    else
    {
        // step aside from the player behind him, then along the open floor to the stairs
        u8 *m = GuideSteps(sGuideMoves[0], 1, gx < 12 ? MOVEMENT_ACTION_WALK_NORMAL_RIGHT : MOVEMENT_ACTION_WALK_NORMAL_LEFT);
        GuidePath(m, gx < 12 ? gx + 1 : gx - 1, gy, GUIDE_LEAVE_ROW, GUIDE_STAIRS_X, GUIDE_STAIRS_Y, MOVEMENT_ACTION_FACE_LEFT);
    }
    ScriptMovement_StartObjectMovementScript(GUIDE_LOCALID, mapNum, mapGroup, sGuideMoves[0]);
}

// special Online_GuideRemove: he went down the stairs.
void Online_GuideRemove(void)
{
    struct ObjectEvent *guide = GuideObject();
    if (guide && guide->currentCoords.x - MAP_OFFSET == GUIDE_STAIRS_X && guide->currentCoords.y - MAP_OFFSET == GUIDE_STAIRS_Y)
        RemoveObjectEventByLocalIdAndMap(GUIDE_LOCALID, gSaveBlock1Ptr->location.mapNum, gSaveBlock1Ptr->location.mapGroup);
}

// Just arrived from another region: log in and join the world picked there
// (no warp; the trainer is where their trip put them).
static void TravelJoinHook(void)
{
    static const char *sWorld;
    static u8 sStage;
    int i;

    switch (sStage)
    {
    case 0:
        if ((sWorld = PcTravelTakeJoin()) == NULL)
            return;
        if (PcNetHasAccount())
            PcNetBeginLogin();
        else
            PcNetBeginRegister(gSaveBlock2Ptr->playerName);
        sStage = 1;
        break;
    case 1:
        if (PcNetOpStatus() == PC_NET_OP_FAILED)
            sStage = 3;
        if (PcNetOpStatus() != PC_NET_OP_OK)
            break;
        for (i = 0; i < PcNetServerCount(); i++)
            if (!strcmp(PcNetServerName(i), sWorld))
                break;
        if (i == PcNetServerCount())
        {
            sStage = 3;   // that world went away meanwhile
            break;
        }
        PcNetBeginJoin(i, gSaveBlock2Ptr->playerGender);
        sStage = 2;
        break;
    case 2:
        if (PcNetOpStatus() == PC_NET_OP_OK)
        {
            sShowJoinedMessage = TRUE;
            sStage = 3;
        }
        else if (PcNetOpStatus() == PC_NET_OP_FAILED)
        {
            sStage = 3;
        }
        break;
    }
}

// Called every overworld frame (CB1_Overworld).
void Online_OverworldUpdate(void)
{
    u16 from;
    u8 kind;

    PcTravelFieldUpdate();
    TravelJoinHook();
    DebugNetHooks();
    if (PcFieldIsOnScreen())
        UpdateGuide();

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
    UpdateNameTags();

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
    if (localId == GUIDE_LOCALID && OnPokemonCenter2F())
        return Online_EventScript_Guide;
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

static void ResumeScriptNextFrame(void);

// special Online_AddFriend (the trainer from Online_BufferTarget; then waitstate):
// VAR_RESULT = TRUE if they are now on the FRIENDS list.
static void Task_AddFriend(u8 taskId)
{
    int status = PcNetOpStatus();
    if (status == PC_NET_OP_PENDING)
        return;
    gSpecialVar_Result = status == PC_NET_OP_OK;
    DestroyTask(taskId);
    ScriptContext_Enable();
}

void Online_AddFriend(void)
{
    int s;
    for (s = 0; s < PC_NET_MAX_PLAYERS; s++)
    {
        const struct PcNetPlayer *p = PcNetGetPlayer(s);
        if (p && p->id == sTargetPlayerId)
        {
            u8 payload[PC_NET_NAME_LEN + 2];
            memcpy(payload, p->name, PC_NET_NAME_LEN);
            payload[PC_NET_NAME_LEN] = p->tag;
            payload[PC_NET_NAME_LEN + 1] = p->tag >> 8;
            PcNetMasterRequest(MSG_FRIEND_ADD, payload, sizeof(payload));
            CreateTask(Task_AddFriend, 80);
            return;
        }
    }
    gSpecialVar_Result = FALSE;   // they left
    ResumeScriptNextFrame();
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
#ifdef EMERALD
        if (gLinkPlayers[0].trainerId & 1)
            PlayMapChosenOrBattleBGM(MUS_VS_GYM_LEADER);
        else
            PlayMapChosenOrBattleBGM(MUS_VS_TRAINER);
#else
        if (gLinkPlayers[0].trainerId & 1)
            PlayMapChosenOrBattleBGM(MUS_RS_VS_GYM_LEADER);
        else
            PlayMapChosenOrBattleBGM(MUS_RS_VS_TRAINER);
#endif
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

// special Online_OpenMarket (after fadescreen; then waitstate): the POKéMON MARKET.
void Online_OpenMarket(void)
{
    sOnline = AllocZeroed(sizeof(*sOnline));
    sOnline->market = TRUE;
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
void Online_AddFriend(void) {}
void Online_OpenMarket(void) {}
#endif
