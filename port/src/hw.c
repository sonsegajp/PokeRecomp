/* hw.c — storage for the GBA hardware the game addresses directly. */
#include "pc_hw.h"

unsigned char gPcIo[PC_IO_SIZE] __attribute__((aligned(4)));
unsigned char gPcPltt[PC_PLTT_SIZE] __attribute__((aligned(4)));
unsigned char gPcVram[PC_VRAM_SIZE] __attribute__((aligned(4)));
unsigned char gPcOam[PC_OAM_SIZE] __attribute__((aligned(4)));
unsigned char gPcFlash[PC_FLASH_SIZE] __attribute__((aligned(4)));

void *gPcSoundInfoPtr;
unsigned short gPcIntrCheck;
void *gPcIntrVector;
