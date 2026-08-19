#ifdef _WIN32
#include <windows.h>
#include <stdio.h>
#include <dbghelp.h>

static char sDummyBuffer[4096] = {0};
static DWORD sLastCrashEIP = 0;
static int sRepeatCount = 0;

static LONG CALLBACK BattleAnimCrashGuard(EXCEPTION_POINTERS *ep)
{
    if (ep->ExceptionRecord->ExceptionCode != 0xC0000005)
        return EXCEPTION_CONTINUE_SEARCH;

    DWORD addr = (DWORD)ep->ExceptionRecord->ExceptionInformation[1];
    DWORD eip = (DWORD)ep->ContextRecord->Eip;

    // Rate-limit logging: only log new crash locations
    if (eip == sLastCrashEIP)
    {
        sRepeatCount++;
    }
    else
    {
        if (sRepeatCount > 1)
        {
            FILE *f = fopen("crash_debug.log", "a");
            if (f) { fprintf(f, "  (repeated %d times)\n", sRepeatCount); fclose(f); }
        }
        sLastCrashEIP = eip;
        sRepeatCount = 1;

        FILE *f = fopen("crash_debug.log", "a");
        if (f) {
            unsigned char *ip = (unsigned char *)eip;
            fprintf(f, "AV at EIP=0x%08lX addr=0x%08lX EAX=0x%08lX EDX=0x%08lX\n",
                (unsigned long)eip, (unsigned long)addr,
                (unsigned long)ep->ContextRecord->Eax,
                (unsigned long)ep->ContextRecord->Edx);
            fprintf(f, "  Bytes: %02X %02X %02X %02X %02X %02X %02X %02X\n",
                ip[0], ip[1], ip[2], ip[3], ip[4], ip[5], ip[6], ip[7]);
            fflush(f);
            fclose(f);
        }
    }

    // Force return from crashing function
    DWORD ebp = ep->ContextRecord->Ebp;
    if (ebp > 0x10000 && ebp < 0x7FFFFFFF)
    {
        DWORD savedEbp = *(DWORD *)ebp;
        DWORD returnAddr = *(DWORD *)(ebp + 4);
        if (returnAddr > 0x10000 && returnAddr < 0x7FFFFFFF)
        {
            ep->ContextRecord->Eip = returnAddr;
            ep->ContextRecord->Esp = ebp + 8;
            ep->ContextRecord->Ebp = savedEbp;
            ep->ContextRecord->Eax = 0;
            return EXCEPTION_CONTINUE_EXECUTION;
        }
    }

    ep->ContextRecord->Eax = (DWORD)sDummyBuffer;
    ep->ContextRecord->Eip += 1;
    return EXCEPTION_CONTINUE_EXECUTION;
}

void InstallCrashGuard(void)
{
    memset(sDummyBuffer, 0, sizeof(sDummyBuffer));
    AddVectoredExceptionHandler(1, BattleAnimCrashGuard);
}

__attribute__((constructor)) static void AutoInstallCrashGuard(void)
{
    InstallCrashGuard();
}
#else
void InstallCrashGuard(void) {}
#endif
