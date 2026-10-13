/* nullguard.c — reads through NULL pointers, as the GBA tolerates them.
 *
 * On the console, address 0 is the BIOS: a stray read through a NULL
 * struct pointer (e.g. a sprite callback that runs one frame after its
 * screen freed its data) returns harmless junk and the game moves on. On
 * Windows the low 64 KB can't be mapped, so the same read crashes.
 *
 * When an access violation hits the low 64 KB, we decode the faulting
 * instruction's memory operand, point its base register at a zero-filled
 * page, single-step the instruction, then put the register back (unless the
 * instruction overwrote it). Reads see zeros; writes land in the scratch page
 * and are wiped (the BIOS ignores writes too). Each faulting site is logged
 * once to nullguard.log so it can be fixed properly in the game code. */
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>

#define LOW_LIMIT 0x10000

static uint8_t zero_page[LOW_LIMIT * 2] __attribute__((aligned(4096)));
static int pending_reg = -1;   /* register redirected for the single step */
static DWORD pending_value;     /* the redirected value */
static DWORD pending_original;  /* what it held before */
static uintptr_t logged[256];
static int logged_count;

/* Pointer to the context slot of 32-bit register `n` (x86 encoding order). */
static DWORD *reg_slot(CONTEXT *c, int n)
{
    switch (n & 7) {
    case 0: return &c->Eax;
    case 1: return &c->Ecx;
    case 2: return &c->Edx;
    case 3: return &c->Ebx;
    case 4: return &c->Esp;
    case 5: return &c->Ebp;
    case 6: return &c->Esi;
    default: return &c->Edi;
    }
}

/* Find the base register of the instruction's ModRM memory operand. */
static int base_register(const uint8_t *ip)
{
    const uint8_t *p = ip;
    uint8_t modrm, mod, rm;

    /* prefixes */
    while (*p == 0x66 || *p == 0x67 || *p == 0xF2 || *p == 0xF3 || *p == 0x2E ||
           *p == 0x3E || *p == 0x26 || *p == 0x36 || *p == 0x64 || *p == 0x65 || *p == 0xF0)
        p++;
    if (*p == 0x0F) {
        p++;
        if (*p == 0x38 || *p == 0x3A) p++;
    }
    p++; /* opcode */
    modrm = *p++;
    mod = modrm >> 6;
    rm = modrm & 7;
    if (mod == 3)
        return -1;           /* register operand: not a memory access */
    if (rm == 4) {           /* SIB */
        uint8_t sib = *p;
        int base = sib & 7;
        if (base == 5 && mod == 0)
            return -1;       /* no base, absolute + index */
        return base;
    }
    if (rm == 5 && mod == 0)
        return -1;           /* absolute address */
    return rm;
}

static void log_site(uintptr_t ip, uintptr_t addr)
{
    for (int i = 0; i < logged_count; i++)
        if (logged[i] == ip) return;
    if (logged_count < 256) logged[logged_count++] = ip;
    FILE *f = fopen("nullguard.log", "a");
    if (f) {
        /* rva = offset from the exe's load address, stable across runs (ASLR moves eip) */
        fprintf(f, "NULL access at rva=%08lx addr=%08lx\n",
                (unsigned long)(ip - (uintptr_t)GetModuleHandleA(NULL)), (unsigned long)addr);
        fclose(f);
    }
}

static LONG CALLBACK handler(EXCEPTION_POINTERS *e)
{
    EXCEPTION_RECORD *r = e->ExceptionRecord;
    CONTEXT *c = e->ContextRecord;

    if (r->ExceptionCode == EXCEPTION_SINGLE_STEP && pending_reg >= 0) {
        DWORD *slot = reg_slot(c, pending_reg);
        if (*slot == pending_value)
            *slot = pending_original;
        pending_reg = -1;
        memset(zero_page, 0, sizeof zero_page); /* drop anything written */
        c->EFlags &= ~0x100;
        return EXCEPTION_CONTINUE_EXECUTION;
    }

    if (r->ExceptionCode == EXCEPTION_ACCESS_VIOLATION && r->NumberParameters >= 2) {
        uintptr_t addr = r->ExceptionInformation[1];
        int base;
        DWORD *slot;

        if (addr >= LOW_LIMIT)
            return EXCEPTION_CONTINUE_SEARCH;
        base = base_register((const uint8_t *)c->Eip);
        if (base < 0)
            return EXCEPTION_CONTINUE_SEARCH;
        slot = reg_slot(c, base);
        if (*slot >= LOW_LIMIT)
            return EXCEPTION_CONTINUE_SEARCH;

        log_site(c->Eip, addr);
        pending_reg = base;
        pending_original = *slot;
        pending_value = (DWORD)(uintptr_t)zero_page + *slot;
        *slot = pending_value;
        c->EFlags |= 0x100; /* trap after this one instruction */
        return EXCEPTION_CONTINUE_EXECUTION;
    }
    return EXCEPTION_CONTINUE_SEARCH;
}

void PcNullGuardInstall(void)
{
    AddVectoredExceptionHandler(1, handler);
}
