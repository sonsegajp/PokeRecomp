#include "global.h"
#include "gflib.h"
#include "scanline_effect.h"
#include "text_window_graphics.h"
#include "menu.h"
#include "task.h"
#include "overworld.h"
#include "help_system.h"
#include "text_window.h"
#include "strings.h"
#include "field_fadetransition.h"
#include "gba/m4a_internal.h"
#ifdef PLATFORM_PC
#include "pc_settings.h"
#include "sound.h"
#include "constants/songs.h"
#endif

// can't include the one in menu_helpers.h since Task_OptionMenu needs bool32 for matching
bool32 IsActiveOverworldLinkBusy(void);

// Menu items
enum
{
    MENUITEM_TEXTSPEED = 0,
    MENUITEM_BATTLESCENE,
    MENUITEM_BATTLESTYLE,
    MENUITEM_SOUND,
    MENUITEM_BUTTONMODE,
    MENUITEM_FRAMETYPE,
    MENUITEM_CANCEL,
    MENUITEM_COUNT
};

// Window Ids
enum
{
    WIN_TEXT_OPTION,
    WIN_OPTIONS
};

// RAM symbols
struct OptionMenu
{
    /*0x00*/ u16 option[MENUITEM_COUNT];
    /*0x0E*/ u16 cursorPos;
    /*0x10*/ u8 loadState;
    /*0x11*/ u8 state;
    /*0x12*/ u8 loadPaletteState;
#ifdef PLATFORM_PC
    u8 page; // 0 = the game's options, 1.. = PC SETTINGS pages
#endif
};

static EWRAM_DATA struct OptionMenu *sOptionMenuPtr = NULL;

//Function Declarataions
static void CB2_InitOptionMenu(void);
static void VBlankCB_OptionMenu(void);
static void OptionMenu_InitCallbacks(void);
static void OptionMenu_SetVBlankCallback(void);
static void CB2_OptionMenu(void);
static void SetOptionMenuTask(void);
static void InitOptionMenuBg(void);
static void OptionMenu_PickSwitchCancel(void);
static void OptionMenu_ResetSpriteData(void);
static bool8 LoadOptionMenuPalette(void);
static void Task_OptionMenu(u8 taskId);
static u8 OptionMenu_ProcessInput(void);
static void BufferOptionMenuString(u8 selection);
static void CloseAndSaveOptionMenu(u8 taskId);
static void PrintOptionMenuHeader(void);
static void DrawOptionMenuBg(void);
static void LoadOptionMenuItemNames(void);
static void UpdateSettingSelectionDisplay(u16 selection);

// Data Definitions
static const struct WindowTemplate sOptionMenuWinTemplates[] =
{
    {
        .bg = 1,
        .tilemapLeft = 2,
        .tilemapTop = 3,
        .width = 26,
        .height = 2,
        .paletteNum = 1,
        .baseBlock = 2
    },
    {
        .bg = 0,
        .tilemapLeft = 2,
        .tilemapTop = 7,
        .width = 26,
        .height = 12,
        .paletteNum = 1,
        .baseBlock = 0x36
    },
    {
        .bg = 2,
        .tilemapLeft = 0,
        .tilemapTop = 0,
        .width = 30,
        .height = 2,
        .paletteNum = 15,
        .baseBlock = 0x16e
    },
    DUMMY_WIN_TEMPLATE
};

static const struct BgTemplate sOptionMenuBgTemplates[] =
{
   {
       .bg = 1,
       .charBaseIndex = 1,
       .mapBaseIndex = 30,
       .screenSize = 0,
       .paletteMode = 0,
       .priority = 0,
       .baseTile = 0
   },
   {
       .bg = 0,
       .charBaseIndex = 1,
       .mapBaseIndex = 31,
       .screenSize = 0,
       .paletteMode = 0,
       .priority = 1,
       .baseTile = 0
   },
   {
       .bg = 2,
       .charBaseIndex = 1,
       .mapBaseIndex = 29,
       .screenSize = 0,
       .paletteMode = 0,
       .priority = 2,
       .baseTile = 0
   },
};

static const u16 sOptionMenuPalette[] = INCBIN_U16("graphics/misc/option_menu.gbapal");
static const u16 sOptionMenuItemCounts[MENUITEM_COUNT] = {3, 2, 2, 2, 3, 10, 0};

static const u8 *const sOptionMenuItemsNames[MENUITEM_COUNT] =
{
    [MENUITEM_TEXTSPEED]   = gText_TextSpeed,
    [MENUITEM_BATTLESCENE] = gText_BattleScene,
    [MENUITEM_BATTLESTYLE] = gText_BattleStyle,
    [MENUITEM_SOUND]       = gText_Sound,
    [MENUITEM_BUTTONMODE]  = gText_ButtonMode,
    [MENUITEM_FRAMETYPE]   = gText_Frame,
    [MENUITEM_CANCEL]      = gText_OptionMenuCancel,
};

static const u8 *const sTextSpeedOptions[] =
{
    gText_TextSpeedSlow, 
    gText_TextSpeedMid, 
    gText_TextSpeedFast
};

static const u8 *const sBattleSceneOptions[] =
{
    gText_BattleSceneOn, 
    gText_BattleSceneOff
};

static const u8 *const sBattleStyleOptions[] =
{
    gText_BattleStyleShift,
    gText_BattleStyleSet
};

static const u8 *const sSoundOptions[] =
{
    gText_SoundMono, 
    gText_SoundStereo
};

static const u8 *const sButtonTypeOptions[] =
{
    gText_ButtonTypeHelp,
	gText_ButtonTypeLR,
	gText_ButtonTypeLEqualsA
};

#ifdef PLATFORM_PC
// PC SETTINGS: extra pages after the game's own options. Each row edits a
// field of gPcSettings; changes apply immediately and are saved on exit.
#define PC_PAGE_COUNT 3   // page 0 is the game's OPTION list

enum { PCROW_VALUE, PCROW_BACK, PCROW_NEXT };

struct PcOptionRow
{
    const u8 *name;
    u8 kind;
    int *value;
    int min, max;
    const u8 *const *labels;   // NULL: print the number (with prefix/suffix)
    const u8 *prefix;
    const u8 *suffix;
};

static const u8 sText_PcSettings[] = _("PC SETTINGS");
static const u8 sText_PcDisplay[] = _("PC SETTINGS - DISPLAY");
static const u8 sText_PcAudio[] = _("PC SETTINGS - SOUND & SPEED");
static const u8 sText_PcHint[] = _("{DPAD_UPDOWN}PICK {DPAD_LEFTRIGHT}SWITCH {B_BUTTON}BACK");
static const u8 sText_NextPage[] = _("SOUND & SPEED");
static const u8 sText_Back[] = _("BACK");
static const u8 sText_More[] = _("{RIGHT_ARROW}");

static const u8 sText_Screen[] = _("SCREEN");
static const u8 sText_Filter[] = _("FILTER");
static const u8 sText_Scaling[] = _("SCALING");
static const u8 sText_Colors[] = _("COLORS");
static const u8 sText_Widescreen[] = _("WIDESCREEN");
static const u8 sText_Vsync[] = _("VSYNC");
static const u8 sText_Volume[] = _("VOLUME");
static const u8 sText_Mixer[] = _("SOUND CHIP");
static const u8 sText_FastForward[] = _("FAST FORWARD");

static const u8 sText_X[] = _("x");
static const u8 sText_Empty[] = _("");
static const u8 sText_Percent[] = _("0%");

static const u8 sText_FilterNearest[] = _("PIXELS");
static const u8 sText_FilterSharp[] = _("SHARP");
static const u8 sText_FilterSmooth[] = _("SMOOTH");
static const u8 sText_FilterLcd[] = _("GBA LCD");
static const u8 sText_FilterCrt[] = _("CRT");
static const u8 *const sFilterLabels[] = {sText_FilterNearest, sText_FilterSharp, sText_FilterSmooth, sText_FilterLcd, sText_FilterCrt};

static const u8 sText_ScaleFit[] = _("FIT");
static const u8 sText_ScaleInteger[] = _("INTEGER");
static const u8 sText_ScaleStretch[] = _("STRETCH");
static const u8 *const sScalingLabels[] = {sText_ScaleFit, sText_ScaleInteger, sText_ScaleStretch};

static const u8 sText_ColorsPc[] = _("VIVID");
static const u8 sText_ColorsGba[] = _("GBA LCD");
static const u8 *const sColorLabels[] = {sText_ColorsPc, sText_ColorsGba};

static const u8 sText_WideOff[] = _("OFF");
static const u8 sText_Wide1610[] = _("16:10");
static const u8 sText_Wide169[] = _("16:9");
static const u8 *const sWideLabels[] = {sText_WideOff, sText_Wide1610, sText_Wide169};

static const u8 sText_On[] = _("ON");
static const u8 *const sOffOnLabels[] = {sText_WideOff, sText_On};

static const u8 sText_MixerGba[] = _("GBA");
static const u8 sText_MixerHq[] = _("HI-FI");
static const u8 *const sMixerLabels[] = {sText_MixerGba, sText_MixerHq};

// SCREEN: x2..x8 window sizes, then FULL (stored as fullscreen=1)
static int sScreenChoice;
static const u8 sText_Full[] = _("FULL");

static const struct PcOptionRow sPcPages[PC_PAGE_COUNT][7] =
{
    [1] = {
        {sText_Screen, PCROW_VALUE, &sScreenChoice, 2, 9, NULL, sText_X, sText_Empty},
        {sText_Filter, PCROW_VALUE, &gPcSettings.filter, 0, PC_FILTER_COUNT - 1, sFilterLabels},
        {sText_Scaling, PCROW_VALUE, &gPcSettings.scaling, 0, PC_SCALE_COUNT - 1, sScalingLabels},
        {sText_Colors, PCROW_VALUE, &gPcSettings.color_correct, 0, 1, sColorLabels},
        {sText_Widescreen, PCROW_VALUE, &gPcSettings.widescreen, 0, PC_WIDE_COUNT - 1, sWideLabels},
        {sText_Vsync, PCROW_VALUE, &gPcSettings.vsync, 0, 1, sOffOnLabels},
        {sText_NextPage, PCROW_NEXT},
    },
    [2] = {
        {sText_Volume, PCROW_VALUE, &gPcSettings.volume, 0, 10, NULL, sText_Empty, sText_Percent},
        {sText_Mixer, PCROW_VALUE, &gPcSettings.hq_audio, 0, 1, sMixerLabels},
        {sText_FastForward, PCROW_VALUE, &gPcSettings.fast_forward, 2, 8, NULL, sText_X, sText_Empty},
        {sText_Back, PCROW_BACK},
    },
};

static u8 PcPageRowCount(u8 page)
{
    u8 n = 0;
    while (n < 7 && sPcPages[page][n].name != NULL)
        n++;
    return n;
}

static void PcScreenChoiceLoad(void)
{
    sScreenChoice = gPcSettings.fullscreen ? 9 : gPcSettings.window_scale;
    if (sScreenChoice < 2)
        sScreenChoice = 2;
}

static void PcScreenChoiceStore(void)
{
    if (sScreenChoice == 9)
    {
        gPcSettings.fullscreen = TRUE;
    }
    else
    {
        gPcSettings.fullscreen = FALSE;
        gPcSettings.window_scale = sScreenChoice;
    }
}

// Left/right on a PC row. Returns TRUE if the value changed.
static bool8 PcRowStep(const struct PcOptionRow *row, int dir)
{
    int v;

    if (row->kind != PCROW_VALUE)
        return FALSE;
    v = *row->value + dir;
    if (v > row->max)
        v = row->min;
    if (v < row->min)
        v = row->max;
    *row->value = v;
    if (row->value == &sScreenChoice)
        PcScreenChoiceStore();
    PcSettingsApplyVideo();
    return TRUE;
}

static void PcRowValueString(const struct PcOptionRow *row, u8 *dst)
{
    u8 buf[8];

    *dst = EOS;
    if (row->kind == PCROW_NEXT)
        StringCopy(dst, sText_More);
    if (row->kind != PCROW_VALUE)
        return;
    if (row->value == &sScreenChoice && sScreenChoice == 9)
    {
        StringCopy(dst, sText_Full);
    }
    else if (row->labels)
    {
        StringCopy(dst, row->labels[*row->value - row->min]);
    }
    else
    {
        StringCopy(dst, row->prefix);
        ConvertIntToDecimalStringN(buf, *row->value, STR_CONV_MODE_LEFT_ALIGN, 2);
        StringAppend(dst, buf);
        StringAppend(dst, row->suffix);
    }
}
#endif

static const u8 sOptionMenuPickSwitchCancelTextColor[]= {TEXT_DYNAMIC_COLOR_6, TEXT_COLOR_WHITE, TEXT_COLOR_DARK_GRAY};
static const u8 sOptionMenuTextColor[] = {TEXT_COLOR_TRANSPARENT, TEXT_COLOR_LIGHT_RED, TEXT_COLOR_RED};

// Functions
static void CB2_InitOptionMenu(void)
{
    RunTasks();
    AnimateSprites();
    BuildOamBuffer();
    UpdatePaletteFade();
}

static void VBlankCB_OptionMenu(void)
{
    LoadOam();
    ProcessSpriteCopyRequests();
    TransferPlttBuffer();
}

void CB2_OptionsMenuFromStartMenu(void)
{
    u8 i;
    
    if (gMain.savedCallback == NULL)
        gMain.savedCallback = CB2_ReturnToFieldWithOpenMenu;
    sOptionMenuPtr = AllocZeroed(sizeof(struct OptionMenu));
    sOptionMenuPtr->loadState = 0;
    sOptionMenuPtr->loadPaletteState = 0;
    sOptionMenuPtr->state = 0;
    sOptionMenuPtr->cursorPos = 0;
    sOptionMenuPtr->option[MENUITEM_TEXTSPEED] = gSaveBlock2Ptr->optionsTextSpeed;
    sOptionMenuPtr->option[MENUITEM_BATTLESCENE] = gSaveBlock2Ptr->optionsBattleSceneOff;
    sOptionMenuPtr->option[MENUITEM_BATTLESTYLE] = gSaveBlock2Ptr->optionsBattleStyle;
    sOptionMenuPtr->option[MENUITEM_SOUND] = gSaveBlock2Ptr->optionsSound;
    sOptionMenuPtr->option[MENUITEM_BUTTONMODE] = gSaveBlock2Ptr->optionsButtonMode;
    sOptionMenuPtr->option[MENUITEM_FRAMETYPE] = gSaveBlock2Ptr->optionsWindowFrameType;
    
    for (i = 0; i < MENUITEM_COUNT - 1; i++)
    {
        if (sOptionMenuPtr->option[i] > (sOptionMenuItemCounts[i]) - 1)
            sOptionMenuPtr->option[i] = 0;
    }
#ifdef PLATFORM_PC
    sOptionMenuPtr->page = 0;
    PcScreenChoiceLoad();
#endif
    SetHelpContext(HELPCONTEXT_OPTIONS);
    SetMainCallback2(CB2_OptionMenu);
}

static void OptionMenu_InitCallbacks(void)
{
    SetVBlankCallback(NULL);
    SetHBlankCallback(NULL);
}

static void OptionMenu_SetVBlankCallback(void)
{
    SetVBlankCallback(VBlankCB_OptionMenu);
}

static void CB2_OptionMenu(void)
{
    u8 i, state;
    state = sOptionMenuPtr->state;
    switch (state)
    {
    case 0:
        OptionMenu_InitCallbacks();
        break;
    case 1:
        InitOptionMenuBg();
        break;
    case 2:
        OptionMenu_ResetSpriteData();
        break;
    case 3:
        if (LoadOptionMenuPalette() != TRUE)
            return;
        break;
    case 4:
        PrintOptionMenuHeader();
        break;
    case 5:
        DrawOptionMenuBg();
        break;
    case 6:
        LoadOptionMenuItemNames();
        break;
    case 7:
        for (i = 0; i < MENUITEM_COUNT; i++)
            BufferOptionMenuString(i);
        break;
    case 8:
        UpdateSettingSelectionDisplay(sOptionMenuPtr->cursorPos);
        break;
    case 9:
        OptionMenu_PickSwitchCancel();
        break;
    default:
        SetOptionMenuTask();
		break;
    }
    sOptionMenuPtr->state++;
}

static void SetOptionMenuTask(void)
{
    CreateTask(Task_OptionMenu, 0);
    SetMainCallback2(CB2_InitOptionMenu);
}

static void InitOptionMenuBg(void)
{
    void *dest = (void *)VRAM;
    DmaClearLarge16(3, dest, VRAM_SIZE, 0x1000);    
    DmaClear32(3, (void *)OAM, OAM_SIZE);
    DmaClear16(3, (void *)PLTT, PLTT_SIZE);    
    SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_MODE_0);
    ResetBgsAndClearDma3BusyFlags(0);
    InitBgsFromTemplates(0, sOptionMenuBgTemplates, NELEMS(sOptionMenuBgTemplates));
    ChangeBgX(0, 0, 0);
    ChangeBgY(0, 0, 0);
    ChangeBgX(1, 0, 0);
    ChangeBgY(1, 0, 0);
    ChangeBgX(2, 0, 0);
    ChangeBgY(2, 0, 0);
    ChangeBgX(3, 0, 0);
    ChangeBgY(3, 0, 0);
    InitWindows(sOptionMenuWinTemplates);
    DeactivateAllTextPrinters();
    SetGpuReg(REG_OFFSET_BLDCNT, BLDCNT_TGT1_BG0 | BLDCNT_EFFECT_BLEND | BLDCNT_EFFECT_LIGHTEN);
    SetGpuReg(REG_OFFSET_BLDY, BLDCNT_TGT1_BG1);
    SetGpuReg(REG_OFFSET_WININ, WININ_WIN0_BG0);
    SetGpuReg(REG_OFFSET_WINOUT, WINOUT_WIN01_BG0 | WINOUT_WIN01_BG1 | WINOUT_WIN01_BG2 | WINOUT_WIN01_CLR);
    SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_OBJ_1D_MAP | DISPCNT_OBJ_ON | DISPCNT_WIN0_ON);
    ShowBg(0);
    ShowBg(1);
    ShowBg(2);
};

static void OptionMenu_PickSwitchCancel(void)
{
    s32 x;
#ifdef PLATFORM_PC
    if (sOptionMenuPtr->page != 0)
    {
        x = 0xE4 - GetStringWidth(FONT_SMALL, sText_PcHint, 0);
        FillWindowPixelBuffer(2, PIXEL_FILL(15));
        AddTextPrinterParameterized3(2, FONT_SMALL, x, 0, sOptionMenuPickSwitchCancelTextColor, 0, sText_PcHint);
        PutWindowTilemap(2);
        CopyWindowToVram(2, COPYWIN_FULL);
        return;
    }
#endif
    x = 0xE4 - GetStringWidth(FONT_SMALL, gText_PickSwitchCancel, 0);
    FillWindowPixelBuffer(2, PIXEL_FILL(15));
    AddTextPrinterParameterized3(2, FONT_SMALL, x, 0, sOptionMenuPickSwitchCancelTextColor, 0, gText_PickSwitchCancel);
    PutWindowTilemap(2);
    CopyWindowToVram(2, COPYWIN_FULL);
}

static void OptionMenu_ResetSpriteData(void)
{
    ResetSpriteData();
    ResetPaletteFade();
    FreeAllSpritePalettes();
    ResetTasks();
    ScanlineEffect_Stop();
}

static bool8 LoadOptionMenuPalette(void)
{
    switch (sOptionMenuPtr->loadPaletteState)
    {
    case 0:
        LoadBgTiles(1, GetUserWindowGraphics(sOptionMenuPtr->option[MENUITEM_FRAMETYPE])->tiles, 0x120, 0x1AA);
        break;
    case 1:
        LoadPalette(GetUserWindowGraphics(sOptionMenuPtr->option[MENUITEM_FRAMETYPE])->palette, BG_PLTT_ID(2), PLTT_SIZE_4BPP);
        break;
    case 2:
        LoadPalette(sOptionMenuPalette, BG_PLTT_ID(1), sizeof(sOptionMenuPalette));
        LoadPalette(GetTextWindowPalette(2), BG_PLTT_ID(15), PLTT_SIZE_4BPP);
        break;
    case 3:
        LoadStdWindowGfxOnBg(1, 0x1B3, BG_PLTT_ID(3));
        break;
    default:
        return TRUE;
    }
    sOptionMenuPtr->loadPaletteState++;
    return FALSE;
}

static void Task_OptionMenu(u8 taskId)
{
#ifdef PLATFORM_PC
    u8 i;
#endif
    switch (sOptionMenuPtr->loadState)
    {
    case 0:
        BeginNormalPaletteFade(PALETTES_ALL, 0, 0x10, 0, RGB_BLACK);
        OptionMenu_SetVBlankCallback();
        sOptionMenuPtr->loadState++;
        break;
    case 1:
        if (gPaletteFade.active)
            return;
        sOptionMenuPtr->loadState++;
        break;
    case 2:
        if (((bool32)IsActiveOverworldLinkBusy()) == TRUE)
            break;
        switch (OptionMenu_ProcessInput())
        {
        case 0:
            break;
        case 1:
            sOptionMenuPtr->loadState++;
            break;
        case 2:
            LoadBgTiles(1, GetUserWindowGraphics(sOptionMenuPtr->option[MENUITEM_FRAMETYPE])->tiles, 0x120, 0x1AA);
            LoadPalette(GetUserWindowGraphics(sOptionMenuPtr->option[MENUITEM_FRAMETYPE])->palette, BG_PLTT_ID(2), PLTT_SIZE_4BPP);
            BufferOptionMenuString(sOptionMenuPtr->cursorPos);
            break;
        case 3:
            UpdateSettingSelectionDisplay(sOptionMenuPtr->cursorPos);
            break;
        case 4:
            BufferOptionMenuString(sOptionMenuPtr->cursorPos);
            break;
#ifdef PLATFORM_PC
        case 5: // page changed
            PrintOptionMenuHeader();
            LoadOptionMenuItemNames();
            for (i = 0; i < MENUITEM_COUNT; i++)
                BufferOptionMenuString(i);
            UpdateSettingSelectionDisplay(sOptionMenuPtr->cursorPos);
            OptionMenu_PickSwitchCancel();
            break;
#endif
        }
        break;
    case 3:
        BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 0x10, RGB_BLACK);
        sOptionMenuPtr->loadState++;
        break;
    case 4:
        if (gPaletteFade.active)
            return;
        sOptionMenuPtr->loadState++;
        break;
    case 5:
        CloseAndSaveOptionMenu(taskId);
        break;
    }
}

#ifdef PLATFORM_PC
static u8 PcSwitchPage(u8 page)
{
    sOptionMenuPtr->page = page;
    sOptionMenuPtr->cursorPos = 0;
    PlaySE(SE_SELECT);
    return 5;
}

// Input on the PC pages. Same conventions as the game's page.
static u8 PcOptionMenu_ProcessInput(void)
{
    u8 page = sOptionMenuPtr->page;
    u8 count = PcPageRowCount(page);
    const struct PcOptionRow *row = &sPcPages[page][sOptionMenuPtr->cursorPos];

    if (JOY_NEW(A_BUTTON) && row->kind == PCROW_NEXT)
        return PcSwitchPage(page + 1 < PC_PAGE_COUNT ? page + 1 : 0);
    if (JOY_REPT(DPAD_RIGHT) || JOY_REPT(DPAD_LEFT))
        return PcRowStep(row, JOY_REPT(DPAD_RIGHT) ? 1 : -1) ? 4 : 0;
    if (JOY_REPT(DPAD_UP))
    {
        sOptionMenuPtr->cursorPos = sOptionMenuPtr->cursorPos == 0 ? count - 1 : sOptionMenuPtr->cursorPos - 1;
        return 3;
    }
    if (JOY_REPT(DPAD_DOWN))
    {
        sOptionMenuPtr->cursorPos = sOptionMenuPtr->cursorPos == count - 1 ? 0 : sOptionMenuPtr->cursorPos + 1;
        return 3;
    }
    if (JOY_NEW(B_BUTTON) || (JOY_NEW(A_BUTTON) && row->kind == PCROW_BACK))
    {
        PcSettingsSave();
        return PcSwitchPage(0);
    }
    return 0;
}
#endif

static u8 OptionMenu_ProcessInput(void)
{
    u16 current;
    u16 *curr;
#ifdef PLATFORM_PC
    if (sOptionMenuPtr->page != 0)
        return PcOptionMenu_ProcessInput();
    // The last row opens PC SETTINGS (it was CANCEL, which A/B already do).
    if (JOY_NEW(A_BUTTON) && sOptionMenuPtr->cursorPos == MENUITEM_CANCEL)
        return PcSwitchPage(1);
#endif
    if (JOY_REPT(DPAD_RIGHT))
    {
        current = sOptionMenuPtr->option[(sOptionMenuPtr->cursorPos)];
        if (current == (sOptionMenuItemCounts[sOptionMenuPtr->cursorPos] - 1))
            sOptionMenuPtr->option[sOptionMenuPtr->cursorPos] = 0;
        else
            sOptionMenuPtr->option[sOptionMenuPtr->cursorPos] = current + 1;
        if (sOptionMenuPtr->cursorPos == MENUITEM_FRAMETYPE)
            return 2;
        else
            return 4;
    }
    else if (JOY_REPT(DPAD_LEFT))
    {
        curr = &sOptionMenuPtr->option[sOptionMenuPtr->cursorPos];
        if (*curr == 0)
            *curr = sOptionMenuItemCounts[sOptionMenuPtr->cursorPos] - 1;
        else
            --*curr;
        
        if (sOptionMenuPtr->cursorPos == MENUITEM_FRAMETYPE)
            return 2;
        else
            return 4;
    }
    else if (JOY_REPT(DPAD_UP))
    {
        if (sOptionMenuPtr->cursorPos == MENUITEM_TEXTSPEED)
            sOptionMenuPtr->cursorPos = MENUITEM_CANCEL;
        else
            sOptionMenuPtr->cursorPos = sOptionMenuPtr->cursorPos - 1;
        return 3;        
    }
    else if (JOY_REPT(DPAD_DOWN))
    {
        if (sOptionMenuPtr->cursorPos == MENUITEM_CANCEL)
            sOptionMenuPtr->cursorPos = MENUITEM_TEXTSPEED;
        else
            sOptionMenuPtr->cursorPos = sOptionMenuPtr->cursorPos + 1;
        return 3;
    }
    else if (JOY_NEW(B_BUTTON) || JOY_NEW(A_BUTTON))
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

static void BufferOptionMenuString(u8 selection)
{
    u8 str[20];
    u8 buf[12];
    u8 dst[3];
    u8 x, y;
    
    memcpy(dst, sOptionMenuTextColor, 3);
    x = 0x82;
    y = ((GetFontAttribute(FONT_NORMAL, FONTATTR_MAX_LETTER_HEIGHT) - 1) * selection) + 2;
    FillWindowPixelRect(1, 1, x, y, 0x46, GetFontAttribute(FONT_NORMAL, FONTATTR_MAX_LETTER_HEIGHT));
#ifdef PLATFORM_PC
    if (sOptionMenuPtr->page != 0)
    {
        if (selection < PcPageRowCount(sOptionMenuPtr->page))
        {
            PcRowValueString(&sPcPages[sOptionMenuPtr->page][selection], str);
            AddTextPrinterParameterized3(1, FONT_NORMAL, x, y, dst, -1, str);
        }
        PutWindowTilemap(1);
        CopyWindowToVram(1, COPYWIN_FULL);
        return;
    }
    if (selection == MENUITEM_CANCEL)
        AddTextPrinterParameterized3(1, FONT_NORMAL, x, y, dst, -1, sText_More);
#endif

    switch (selection)
    {
    case MENUITEM_TEXTSPEED:
        AddTextPrinterParameterized3(1, FONT_NORMAL, x, y, dst, -1, sTextSpeedOptions[sOptionMenuPtr->option[selection]]);
        break;
    case MENUITEM_BATTLESCENE:
        AddTextPrinterParameterized3(1, FONT_NORMAL, x, y, dst, -1, sBattleSceneOptions[sOptionMenuPtr->option[selection]]);
        break;
    case MENUITEM_BATTLESTYLE:
        AddTextPrinterParameterized3(1, FONT_NORMAL, x, y, dst, -1, sBattleStyleOptions[sOptionMenuPtr->option[selection]]);
        break;
    case MENUITEM_SOUND:
        AddTextPrinterParameterized3(1, FONT_NORMAL, x, y, dst, -1, sSoundOptions[sOptionMenuPtr->option[selection]]);
        break;
    case MENUITEM_BUTTONMODE:
        AddTextPrinterParameterized3(1, FONT_NORMAL, x, y, dst, -1, sButtonTypeOptions[sOptionMenuPtr->option[selection]]);
        break;
    case MENUITEM_FRAMETYPE:
        StringCopy(str, gText_FrameType);
        ConvertIntToDecimalStringN(buf, sOptionMenuPtr->option[selection] + 1, 1, 2);
        StringAppendN(str, buf, 3);
        AddTextPrinterParameterized3(1, FONT_NORMAL, x, y, dst, -1, str);
        break;
    default:
        break;
    }
    PutWindowTilemap(1);
    CopyWindowToVram(1, COPYWIN_FULL);
}

static void CloseAndSaveOptionMenu(u8 taskId)
{
    gFieldCallback = FieldCB_DefaultWarpExit;
    SetMainCallback2(gMain.savedCallback);
    FreeAllWindowBuffers();
    gSaveBlock2Ptr->optionsTextSpeed = sOptionMenuPtr->option[MENUITEM_TEXTSPEED];
    gSaveBlock2Ptr->optionsBattleSceneOff = sOptionMenuPtr->option[MENUITEM_BATTLESCENE];
    gSaveBlock2Ptr->optionsBattleStyle = sOptionMenuPtr->option[MENUITEM_BATTLESTYLE];
    gSaveBlock2Ptr->optionsSound = sOptionMenuPtr->option[MENUITEM_SOUND];
    gSaveBlock2Ptr->optionsButtonMode = sOptionMenuPtr->option[MENUITEM_BUTTONMODE];
    gSaveBlock2Ptr->optionsWindowFrameType = sOptionMenuPtr->option[MENUITEM_FRAMETYPE];
    SetPokemonCryStereo(gSaveBlock2Ptr->optionsSound);
#ifdef PLATFORM_PC
    PcSettingsSave();
#endif
    FREE_AND_SET_NULL(sOptionMenuPtr);
    DestroyTask(taskId);
}

static void PrintOptionMenuHeader(void)
{
    FillWindowPixelBuffer(0, PIXEL_FILL(1));
#ifdef PLATFORM_PC
    if (sOptionMenuPtr->page != 0)
    {
        AddTextPrinterParameterized(WIN_TEXT_OPTION, FONT_NORMAL, sOptionMenuPtr->page == 1 ? sText_PcDisplay : sText_PcAudio, 8, 1, TEXT_SKIP_DRAW, NULL);
        PutWindowTilemap(0);
        CopyWindowToVram(0, COPYWIN_FULL);
        return;
    }
#endif
    AddTextPrinterParameterized(WIN_TEXT_OPTION, FONT_NORMAL, gText_Option, 8, 1, TEXT_SKIP_DRAW, NULL);
    PutWindowTilemap(0);
    CopyWindowToVram(0, COPYWIN_FULL);
}

static void DrawOptionMenuBg(void)
{
    u8 h;
    h = 2;
    
    FillBgTilemapBufferRect(1, 0x1B3, 1, 2, 1, 1, 3);
    FillBgTilemapBufferRect(1, 0x1B4, 2, 2, 0x1B, 1, 3);
    FillBgTilemapBufferRect(1, 0x1B5, 0x1C, 2, 1, 1, 3);
    FillBgTilemapBufferRect(1, 0x1B6, 1, 3, 1, h, 3);
    FillBgTilemapBufferRect(1, 0x1B8, 0x1C, 3, 1, h, 3);
    FillBgTilemapBufferRect(1, 0x1B9, 1, 5, 1, 1, 3);
    FillBgTilemapBufferRect(1, 0x1BA, 2, 5, 0x1B, 1, 3);
    FillBgTilemapBufferRect(1, 0x1BB, 0x1C, 5, 1, 1, 3);
    FillBgTilemapBufferRect(1, 0x1AA, 1, 6, 1, 1, h);
    FillBgTilemapBufferRect(1, 0x1AB, 2, 6, 0x1A, 1, h);
    FillBgTilemapBufferRect(1, 0x1AC, 0x1C, 6, 1, 1, h);
    FillBgTilemapBufferRect(1, 0x1AD, 1, 7, 1, 0x10, h);
    FillBgTilemapBufferRect(1, 0x1AF, 0x1C, 7, 1, 0x10, h);
    FillBgTilemapBufferRect(1, 0x1B0, 1, 0x13, 1, 1, h);
    FillBgTilemapBufferRect(1, 0x1B1, 2, 0x13, 0x1A, 1, h);
    FillBgTilemapBufferRect(1, 0x1B2, 0x1C, 0x13, 1, 1, h);
    CopyBgTilemapBufferToVram(1);
}

static void LoadOptionMenuItemNames(void)
{
    u8 i;
    
    FillWindowPixelBuffer(1, PIXEL_FILL(1));
#ifdef PLATFORM_PC
    if (sOptionMenuPtr->page != 0)
    {
        u8 count = PcPageRowCount(sOptionMenuPtr->page);
        for (i = 0; i < count; i++)
            AddTextPrinterParameterized(WIN_OPTIONS, FONT_NORMAL, sPcPages[sOptionMenuPtr->page][i].name, 8,
                                        (u8)((i * (GetFontAttribute(FONT_NORMAL, FONTATTR_MAX_LETTER_HEIGHT))) + 2) - i, TEXT_SKIP_DRAW, NULL);
        return;
    }
    for (i = 0; i < MENUITEM_COUNT; i++)
    {
        const u8 *name = i == MENUITEM_CANCEL ? sText_PcSettings : sOptionMenuItemsNames[i];
        AddTextPrinterParameterized(WIN_OPTIONS, FONT_NORMAL, name, 8, (u8)((i * (GetFontAttribute(FONT_NORMAL, FONTATTR_MAX_LETTER_HEIGHT))) + 2) - i, TEXT_SKIP_DRAW, NULL);
    }
    return;
#endif
    for (i = 0; i < MENUITEM_COUNT; i++)
    {
        AddTextPrinterParameterized(WIN_OPTIONS, FONT_NORMAL, sOptionMenuItemsNames[i], 8, (u8)((i * (GetFontAttribute(FONT_NORMAL, FONTATTR_MAX_LETTER_HEIGHT))) + 2) - i, TEXT_SKIP_DRAW, NULL);    
    }
}

static void UpdateSettingSelectionDisplay(u16 selection)
{
    u16 maxLetterHeight, y;
    
    maxLetterHeight = GetFontAttribute(FONT_NORMAL, FONTATTR_MAX_LETTER_HEIGHT);
    y = selection * (maxLetterHeight - 1) + 0x3A;
    SetGpuReg(REG_OFFSET_WIN0V, WIN_RANGE(y, y + maxLetterHeight));
    SetGpuReg(REG_OFFSET_WIN0H, WIN_RANGE(0x10, 0xE0));
}
