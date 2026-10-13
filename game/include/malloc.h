#ifndef GUARD_MALLOC_H
#define GUARD_MALLOC_H

#include "global.h"

#ifdef PLATFORM_PC
#define HEAP_SIZE 0x20000 // room for the double-width field tilemaps
#else
#define HEAP_SIZE 0x1C000
#endif
#define malloc Alloc
#define calloc(ct, sz) AllocZeroed((ct) * (sz))
#define free Free

#define FREE_AND_SET_NULL(ptr)          \
{                                       \
    free(ptr);                          \
    ptr = NULL;                         \
}

#define TRY_FREE_AND_SET_NULL(ptr) if (ptr != NULL) FREE_AND_SET_NULL(ptr)

extern u8 gHeap[];
void *Alloc(u32 size);
void *AllocZeroed(u32 size);
void Free(void *pointer);
void InitHeap(void *pointer, u32 size);

#endif // GUARD_MALLOC_H
