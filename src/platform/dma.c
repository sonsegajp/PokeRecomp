#include "global.h"
#include "platform.h"
#include "platform/dma.h"

#ifndef DMA_DEST_MASK
#define DMA_DEST_MASK  (3 << 5)
#define DMA_DEST_INC   (0 << 5)
#define DMA_DEST_DEC   (1 << 5)
#define DMA_DEST_RELOAD (3 << 5)
#define DMA_SRC_MASK   (3 << 7)
#define DMA_SRC_INC    (0 << 7)
#define DMA_SRC_DEC    (1 << 7)
#endif

struct DMATransfer {
    union {
        const void *src;
        const u16 *src16;
        const u32 *src32;
    };
    union {
        void *dst;
        vu16 *dst16;
        vu32 *dst32;
    };
    u32 size;
    u16 control;
} DMAList[DMA_COUNT];

static int IsGbaHardwareAddr(const void *addr) {
    (void)addr;
    return 0;
}

void ClearDMAList(void)
{
    for (int i = 0; i < DMA_COUNT; i++)
    {
        DMAList[i].src = NULL;
        DMAList[i].dst = NULL;
        DMAList[i].size = 0;
        DMAList[i].control = 0;
    }
}

void RunDMAs(u32 type)
{
    for (int dmaNum = 0; dmaNum < DMA_COUNT; dmaNum++)
    {
        struct DMATransfer *dma = &DMAList[dmaNum];
        u32 dmaCntReg = (&REG_DMA0CNT)[dmaNum * 3];
        if (!((dmaCntReg >> 16) & DMA_ENABLE))
        {
            dma->control &= ~DMA_ENABLE;
        }

        if ( (dma->control & DMA_ENABLE) &&
           (((dma->control & DMA_START_MASK) >> 12) == type))
        {
            if (!dma->src || !dma->dst || IsGbaHardwareAddr(dma->src) || IsGbaHardwareAddr(dma->dst))
            {
                dma->control &= ~DMA_ENABLE;
                continue;
            }
            for (int i = 0; i < (dma->size); i++)
            {
                if ((dma->control) & DMA_32BIT)
                     *dma->dst32 = *dma->src32;
                else *dma->dst16 = *dma->src16;

                // process destination pointer changes
                if (((dma->control) & DMA_DEST_MASK) == DMA_DEST_INC)
                {
                    if ((dma->control) & DMA_32BIT)
                            dma->dst32++;
                    else    dma->dst16++;
                }
                else if (((dma->control) & DMA_DEST_MASK) == DMA_DEST_DEC)
                {
                    if ((dma->control) & DMA_32BIT)
                            dma->dst32--;
                    else    dma->dst16--;
                }
                else if (((dma->control) & DMA_DEST_MASK) == DMA_DEST_RELOAD) // TODO
                {
                    if ((dma->control) & DMA_32BIT)
                            dma->dst32++;
                    else    dma->dst16++;
                }

                // process source pointer changes
                if (((dma->control) & DMA_SRC_MASK) == DMA_SRC_INC)
                {
                    if ((dma->control) & DMA_32BIT)
                            dma->src32++;
                    else    dma->src16++;
                }
                else if (((dma->control) & DMA_SRC_MASK) == DMA_SRC_DEC)
                {
                    if ((dma->control) & DMA_32BIT)
                            dma->src32--;
                    else    dma->src16--;
                }
            }

            if (dma->control & DMA_REPEAT)
            {
                dma->size = ((&REG_DMA0CNT)[dmaNum * 3] & 0x1FFFF);
                if (((dma->control) & DMA_DEST_MASK) == DMA_DEST_RELOAD)
                {
                    dma->dst = (void *)((&REG_DMA0DAD)[dmaNum * 3]);
                }
            }
            else
            {
                dma->control &= ~DMA_ENABLE;
            }
        }
    }
}

void DmaSet(int dmaNum, const void *src, void *dest, u32 control)
{
    if (dmaNum >= DMA_COUNT)
    {
        DBGPRINTF("DmaSet with invalid DMA number: dmaNum=%d, src=%p, dest=%p, control=%d\n", dmaNum, src, dest, control);
        return;
    }

    if (IsGbaHardwareAddr(dest) || IsGbaHardwareAddr(src))
        return;

    (&REG_DMA0SAD)[dmaNum * 3] = src;
    (&REG_DMA0DAD)[dmaNum * 3] = dest;
    (&REG_DMA0CNT)[dmaNum * 3] = control;

    struct DMATransfer *dma = &DMAList[dmaNum];
    dma->src = src;
    dma->dst = dest;
    dma->size = control & 0x1ffff;
    dma->control = control >> 16;

    RunDMAs(DMA_NOW);
}