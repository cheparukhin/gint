#include <gint/dma.h>

/* dma_memcpy(): Fast 32-aligned memcpy */
void *dma_memcpy(void * restrict dst, const void * restrict src, size_t size)
{
	dma_transfer(1, DMA_32B, size >> 5, src, DMA_INC, dst, DMA_INC);
	dma_transfer_wait(1);

	return dst;
}
