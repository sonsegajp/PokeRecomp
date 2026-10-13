#ifndef GUARD_ALLOC_H
#define GUARD_ALLOC_H


#define FREE_AND_SET_NULL(ptr)          \
{                                       \
    Free(ptr);                          \
    ptr = NULL;                         \
}

#define TRY_FREE_AND_SET_NULL(ptr) if (ptr != NULL) FREE_AND_SET_NULL(ptr)

#ifdef PLATFORM_PC
#define HEAP_SIZE 0x20000 // room for the double-width field tilemaps
#else
#define HEAP_SIZE 0x1C000
#endif
extern u8 gHeap[HEAP_SIZE];

void *Alloc(u32 size);
void *AllocZeroed(u32 size);
void Free(void *pointer);
void InitHeap(void *heapStart, u32 heapSize);

#endif // GUARD_ALLOC_H
