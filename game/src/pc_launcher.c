/* Direct launcher entry points into the original FRLG menus. No save layout,
 * Pokemon data, storage interactions, or storage artwork is reimplemented. */
#ifdef PLATFORM_PC
#include "global.h"
#include "gflib.h"
#include "help_system.h"
#include "item.h"
#include "load_save.h"
#include "mystery_gift_menu.h"
#include "new_game.h"
#include "pokemon_storage_system_internal.h"
#include "save.h"
#include "sound.h"
#include "gba/m4a_internal.h"
#include "new_menu_helpers.h"
#include "task.h"
#include "pc_launcher.h"

void PcLauncherBoot(void)
{
    if (gPcLauncherMode == PC_LAUNCHER_GAME)
        return;

    ResetMenuAndMonGlobals();
    Save_ResetSaveCounters();
    if (LoadGameSave(SAVE_NORMAL) != SAVE_STATUS_OK)
    {
        SetMainCallback2(NULL);
        PcLauncherFinish(2, "PC Boxes and Mystery Gift require a valid existing firered.sav. Start or import a game and save first.");
        return;
    }

    /* LoadGameSave also restores the party and overworld object state. Keep
     * those objects intact so the normal serializer preserves the saved map. */
    SetBagPocketsPointers();
    SetPokemonCryStereo(gSaveBlock2Ptr->optionsSound);
    SetVBlankCallback(NULL);
    SetHBlankCallback(NULL);
    ResetTasks();
    ResetSpriteData();
    ResetPaletteFade();
    ResetTempTileDataBuffers();
    gSoftResetDisabled = TRUE;

    if (gPcLauncherMode == PC_LAUNCHER_BOXES)
    {
        EnterPokeStorage(gPcLauncherBoxOption);
    }
    else
    {
        HelpSystem_Disable();
        SetMainCallback2(CB2_InitMysteryGift);
    }
}

int PcLauncherExitBoxes(void)
{
    if (gPcLauncherMode != PC_LAUNCHER_BOXES)
        return FALSE;

    /* Reached only after the original storage screen has confirmed that no
     * Pokemon or item is still held by the cursor and completed its exit. */
    SetVBlankCallback(NULL);
    SetMainCallback2(NULL);
    if (!PcLauncherBackupSave())
    {
        PcLauncherFinish(3, "Could not back up firered.sav. Box changes were not saved.");
        return TRUE;
    }
    if (TrySavingData(SAVE_NORMAL) != SAVE_STATUS_OK || !PcSaveFlushChecked())
    {
        PcLauncherFinish(3, "Could not save PC Box changes. The previous save is in firered.sav.launcher.bak.");
        return TRUE;
    }
    PcLauncherFinish(0, "PC Box changes saved.");
    return TRUE;
}

int PcLauncherExitMysteryGift(void)
{
    if (gPcLauncherMode != PC_LAUNCHER_MYSTERY_GIFT)
        return FALSE;
    SetVBlankCallback(NULL);
    SetMainCallback2(NULL);
    PcLauncherFinish(PcSaveFlushChecked() ? 0 : 3, "Returned from Mystery Gift.");
    return TRUE;
}
#endif
