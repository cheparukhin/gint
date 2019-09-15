#include <gint/dma.h>

/* dma_memset(): Fast 32-aligned memset */
void *dma_memset(void *dst, uint32_t l, size_t size)
{
	/* TODO: Use a proper IL memory allocation scheme */
	uint32_t *IL = (void *)0xe5200000;
	for(int i = 0; i < 8; i++) IL[i] = l;

	dma_transfer(1, DMA_32B, size >> 5, IL, DMA_FIXED, dst, DMA_INC);
	dma_transfer_wait(1);
	return dst;
}
