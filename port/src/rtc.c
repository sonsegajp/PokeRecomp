/* rtc.c — Emerald's real-time clock chip (the cartridge's Seiko RTC, which
 * pret's src/siirtc.c drives over the GPIO port), answered from the PC's
 * clock. The game keeps its own offset from this clock in the save (set on
 * the first day), so berries, tides and daily events follow real time.
 * FireRed/LeafGreen have no clock chip. */
#ifdef EMERALD
#include <time.h>
#include "global.h"
#include "siirtc.h"

static u8 sStatus = SIIRTCINFO_24HOUR;

static u8 Bcd(int v)
{
    return (u8)(((v / 10) << 4) | (v % 10));
}

static void Now(struct SiiRtcInfo *rtc, bool8 withDate)
{
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);

    if (withDate)
    {
        rtc->year = Bcd(tm->tm_year % 100);           /* 2000-2099 */
        rtc->month = Bcd(tm->tm_mon + 1);
        rtc->day = Bcd(tm->tm_mday);
        rtc->dayOfWeek = Bcd(tm->tm_wday);
    }
    rtc->hour = Bcd(tm->tm_hour);
    rtc->minute = Bcd(tm->tm_min);
    rtc->second = Bcd(tm->tm_sec > 59 ? 59 : tm->tm_sec);
}

void SiiRtcUnprotect(void) {}
void SiiRtcProtect(void) {}

u8 SiiRtcProbe(void)
{
    return 1;                                         /* present, in 24-hour mode, no errors */
}

bool8 SiiRtcReset(void)
{
    sStatus = SIIRTCINFO_24HOUR;
    return TRUE;
}

bool8 SiiRtcGetStatus(struct SiiRtcInfo *rtc)
{
    rtc->status = sStatus;
    return TRUE;
}

bool8 SiiRtcSetStatus(struct SiiRtcInfo *rtc)
{
    sStatus = (rtc->status & ~SIIRTCINFO_POWER) | SIIRTCINFO_24HOUR;
    return TRUE;
}

bool8 SiiRtcGetDateTime(struct SiiRtcInfo *rtc)
{
    Now(rtc, TRUE);
    return TRUE;
}

/* The PC's clock is the truth; the game's own offset (in the save) handles
 * the time the player sets. */
bool8 SiiRtcSetDateTime(struct SiiRtcInfo *rtc) { return TRUE; }

bool8 SiiRtcGetTime(struct SiiRtcInfo *rtc)
{
    Now(rtc, FALSE);
    return TRUE;
}

bool8 SiiRtcSetTime(struct SiiRtcInfo *rtc) { return TRUE; }
#endif
