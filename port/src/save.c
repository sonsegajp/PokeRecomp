/* save.c — the cartridge's 128 KB flash chip, backed by the selected cartridge save.
 *
 * Replaces pret's flash drivers (agb_flash*.c), which speak the chip's
 * command protocol over the cartridge bus. The game's own save logic
 * (sectors, checksums, signatures, rotation) runs unchanged on top of this
 * API. Writes mark the file dirty; the frame loop flushes it atomically. */
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <io.h>

#include "pc_hw.h"
#include "pc_port.h"
#include "pc_launcher.h"

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;

#define SECTOR_SIZE  0x1000
#define SECTOR_COUNT 32
/* Always resolve before locking/loading. Settings keep their own data directory. */
static char save_path[MAX_PATH] = "firered.sav";
static char save_temp[MAX_PATH] = "firered.sav.tmp";
static char save_backup[MAX_PATH] = "firered.sav.launcher.bak";

int PcSaveSetPath(const char *path)
{
    char resolved[MAX_PATH], parent[MAX_PATH], *file_part;
    DWORD count, attributes;
    if (!path) path = "firered.sav";
    if (!*path) return 0;
    count = GetFullPathNameA(path, sizeof resolved, resolved, &file_part);
    if (!count || count >= sizeof resolved || !file_part || !*file_part ||
        count + sizeof ".launcher.bak" > sizeof save_backup) return 0;
    strcpy(parent, resolved);
    parent[file_part - resolved] = 0;
    attributes = GetFileAttributesA(parent);
    if (attributes == INVALID_FILE_ATTRIBUTES || !(attributes & FILE_ATTRIBUTE_DIRECTORY)) return 0;
    attributes = GetFileAttributesA(resolved);
    if (attributes != INVALID_FILE_ATTRIBUTES && (attributes & FILE_ATTRIBUTE_DIRECTORY)) return 0;
    strcpy(save_path, resolved);
    memcpy(save_temp, resolved, count);
    memcpy(save_temp + count, ".tmp", sizeof ".tmp");
    memcpy(save_backup, resolved, count);
    memcpy(save_backup + count, ".launcher.bak", sizeof ".launcher.bak");
    return 1;
}

const char *PcSaveGetPath(void) { return save_path; }

static int dirty;

struct FlashSector { u32 size; u8 shift; u16 count; u16 top; };
struct FlashType {
    u32 romSize;
    struct FlashSector sector;
    u16 wait[2];
    union { struct { u8 makerId, deviceId; } separate; u16 joined; } ids;
};

/* Describes a Macronix MX29L010 (the chip FireRed shipped with). */
static const struct FlashType sFlashType = {
    0x20000, {SECTOR_SIZE, 12, SECTOR_COUNT, 0}, {3, 1}, {{0xC2, 0x09}},
};
static const u16 sMaxTime[] = {10, 65469, 0, 20, 65469, 0, 20, 65469, 0, 2000, 65469, 0};

const struct FlashType *gFlash = &sFlashType;
const u16 *gFlashMaxTime = sMaxTime;
u8 gFlashTimeoutFlag;
u16 gFlashNumRemainingBytes;

static u8 *sector_ptr(u16 sector) { return gPcFlash + (u32)(sector % SECTOR_COUNT) * SECTOR_SIZE; }

static u16 erase_chip(void)
{
    memset(gPcFlash, 0xFF, sizeof gPcFlash);
    dirty = 1;
    return 0;
}

static u16 erase_sector(u16 sector)
{
    memset(sector_ptr(sector), 0xFF, SECTOR_SIZE);
    dirty = 1;
    return 0;
}

static u16 program_byte(u16 sector, u32 offset, u8 data)
{
    if (offset >= SECTOR_SIZE) return 0x8000;
    sector_ptr(sector)[offset] = data;
    dirty = 1;
    return 0;
}

static u16 program_sector(u16 sector, void *src)
{
    memcpy(sector_ptr(sector), src, SECTOR_SIZE);
    dirty = 1;
    return 0;
}

static u16 wait_for_write(u8 phase, u8 *addr, u8 last)
{
    (void)phase; (void)addr; (void)last;
    return 0;
}

static u8 poll_status(u8 *status)
{
    (void)status;
    return 0;
}

u16 (*ProgramFlashByte)(u16, u32, u8) = program_byte;
u16 (*ProgramFlashSector)(u16, void *) = program_sector;
u16 (*EraseFlashChip)(void) = erase_chip;
u16 (*EraseFlashSector)(u16) = erase_sector;
u16 (*WaitForFlashWrite)(u8, u8 *, u8) = wait_for_write;
u8 (*PollFlashStatus)(u8 *) = poll_status;

u16 IdentifyFlash(void) { return 0; }  /* 0 = flash found */
u16 ReadFlashId(void) { return sFlashType.ids.joined; }
void SwitchFlashBank(u8 bank) { (void)bank; }
u16 SetFlashTimerIntr(u8 timer, void (**intr)(void)) { (void)timer; (void)intr; return 0; }
void StartFlashTimer(u8 phase) { (void)phase; }
void StopFlashTimer(void) {}

void ReadFlash(u16 sector, u32 offset, void *dest, u32 size)
{
    if (offset + size > SECTOR_SIZE) size = offset < SECTOR_SIZE ? SECTOR_SIZE - offset : 0;
    memcpy(dest, sector_ptr(sector) + offset, size);
}

u32 ProgramFlashSectorAndVerify(u16 sector, u8 *src)
{
    program_sector(sector, src);
    return 0;
}

u32 ProgramFlashSectorAndVerifyNBytes(u16 sector, void *src, u32 n)
{
    if (n > SECTOR_SIZE) n = SECTOR_SIZE;
    erase_sector(sector);
    memcpy(sector_ptr(sector), src, n);
    return 0;
}

void PcSaveLoad(void)
{
    FILE *f = fopen(save_path, "rb");

    memset(gPcFlash, 0xFF, sizeof gPcFlash);
    if (f) {
        fread(gPcFlash, 1, sizeof gPcFlash, f);
        fclose(f);
    }
    dirty = 0;
}

/* Write to a temp file and swap it in, so a crash never leaves a torn save. */
int PcSaveFlushChecked(void)
{
    FILE *f;

    if (!dirty) return 1;
    f = fopen(save_temp, "wb");
    if (!f) return 0;
    int ok = fwrite(gPcFlash, 1, sizeof gPcFlash, f) == sizeof gPcFlash;
    if (ok) ok = fflush(f) == 0;
    if (ok) ok = _commit(_fileno(f)) == 0;
    ok = (fclose(f) == 0) && ok;
    if (ok && MoveFileExA(save_temp, save_path, MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
        dirty = 0;
    return !dirty;
}

void PcSaveFlush(void)
{
    (void)PcSaveFlushChecked();
}

int PcLauncherBackupSave(void)
{
    return CopyFileA(save_path, save_backup, FALSE) != 0;
}
