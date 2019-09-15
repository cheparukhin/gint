#include <gint/dma.h>

/* Allocate a 32-byte buffer in ILRAM */
GALIGNED(32) GILRAM static uint32_t ILbuf[8];

/* dma_memset(): Fast 32-aligned memset */
void *dma_memset(void *dst, uint32_t l, size_t size)
{
	/* Prepare the ILRAM buffer. We need to use ILRAM because the DMA will
	   have to read the operand once per block, as opposed to an assembler
	   routine that would hold it in a register. If we place it in RAM, the
	   DMA will perform twice as many RAM accesses as the handwritten
	   assembler, which would be very slow. By using ILRAM we use two
	   different memory regions, making the DMA faster than the CPU. */
	for(int i = 0; i < 8; i++) ILbuf[i] = l;

	dma_transfer(1, DMA_32B, size >> 5, ILbuf, DMA_FIXED, dst, DMA_INC);
	dma_transfer_wait(1);
	return dst;
}
