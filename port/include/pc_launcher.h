#ifndef PC_LAUNCHER_H
#define PC_LAUNCHER_H

/* Shared, layout-independent interface between the launcher entry points and
 * the original game. The storage screens and Pokemon operations remain in
 * pokemon_storage_system*.c. */
enum PcLauncherMode {
    PC_LAUNCHER_GAME,
    PC_LAUNCHER_BOXES,
    PC_LAUNCHER_MYSTERY_GIFT
};

extern int gPcLauncherMode;
extern int gPcLauncherBoxOption;

void PcLauncherBoot(void);
int PcLauncherExitBoxes(void);
int PcLauncherExitMysteryGift(void);
void PcLauncherFinish(int exitCode, const char *message);
int PcLauncherBackupSave(void);
int PcSaveFlushChecked(void);

#endif
