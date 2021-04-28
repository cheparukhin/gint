#include <gint/dma.h>

/* dma_memcpy(): Fast 32-aligned memcpy */
void *dma_memcpy(void * __restrict dst, const void * __restrict src,
	size_t size)
{
	dma_transfer_sync(1, DMA_32B, size >> 5, src, DMA_INC, dst, DMA_INC);
	return dst;
}
