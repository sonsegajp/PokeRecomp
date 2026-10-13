/* stubs.c — hardware features a PC build doesn't have, and symbols the GBA
 * linker script / ROM header used to provide.
 *
 * Multiboot (sending programs to another GBA over the link cable), the
 * GameCube link and the wireless adapter's SIO32 interrupt are replaced by
 * the online layer later; here they report "nothing connected". */
#include <stdint.h>
#include <string.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef int8_t s8;

/* ROM header fields the game reads. */
const char RomHeaderGameCode[4] = {'B', 'P', 'R', 'E'};
const u8 RomHeaderSoftwareVersion = 0;

/* The heap the linker script placed at the end of EWRAM. */
u8 gHeap[0x1C000] __attribute__((aligned(4)));

/* IntrMain is ARM code copied to IWRAM on the console; interrupts are
 * dispatched by the host here, so it only needs to exist. */
u32 intr_main[1];

/* Multiboot: always fails ("no partner"). */
struct MultiBootParam;
void MultiBootInit(struct MultiBootParam *mp) { (void)mp; }
int MultiBootMain(struct MultiBootParam *mp) { (void)mp; return 0; }
void MultiBootStartMaster(struct MultiBootParam *mp, const u8 *src, int size, u8 palette, s8 speed)
{ (void)mp; (void)src; (void)size; (void)palette; (void)speed; }
int MultiBootCheckComplete(struct MultiBootParam *mp) { (void)mp; return 0; }
int MultiBoot(struct MultiBootParam *mp) { (void)mp; return 1; }

/* GameCube link (Colosseum/Box): never connected. */
struct GcmbStruct;
void GameCubeMultiBoot_Init(struct GcmbStruct *s) { (void)s; }
void GameCubeMultiBoot_Main(struct GcmbStruct *s) { (void)s; }
void GameCubeMultiBoot_ExecuteProgram(struct GcmbStruct *s) { (void)s; }
void GameCubeMultiBoot_HandleSerialInterrupt(struct GcmbStruct *s) { (void)s; }
void GameCubeMultiBoot_Quit(void) {}

/* Wireless adapter serial interrupt. */
void IntrSIO32(void) {}
