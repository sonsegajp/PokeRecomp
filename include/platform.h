#ifndef GUARD_PLATFORM_H
#define GUARD_PLATFORM_H

#include <stdio.h>
#include "global.h"
#include "siirtc.h"

// Debug print used by the platform layer; emerald's pc_port relies on this.
#ifdef DEBUG
#define DBGPRINTF(...) printf(__VA_ARGS__)
#else
#define DBGPRINTF(...) ((void)0)
#endif

void Platform_StoreSaveFile(void);
void Platform_ReadFlash(u16 sectorNum, u32 offset, u8 *dest, u32 size);
void Platform_QueueAudio(float *audioBuffer, s32 samplesPerFrame);
u16 Platform_GetKeyInput(void);
void Platform_GetStatus(struct SiiRtcInfo *rtc);
void Platform_SetStatus(struct SiiRtcInfo *rtc);
static void UpdateInternalClock(void);
void Platform_GetDateTime(struct SiiRtcInfo *rtc);
void Platform_SetDateTime(struct SiiRtcInfo *rtc);
void Platform_GetTime(struct SiiRtcInfo *rtc);
void Platform_SetTime(struct SiiRtcInfo *rtc);
void Platform_SetAlarm(u8 *alarmData);

#endif