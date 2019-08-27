//---
//	gint:dma - Direct Memory Access for efficient data transfer
//
//	Currently this module is used only to transfer data to the display on
//	fxcg50, but fast memcpy() is apparently possible as well.
//---

#ifndef GINT_DMA
#define GINT_DMA

#include <gint/defs/types.h>

/* dma_size_t - Transfer block size */
typedef enum
{
	/* Normal transfers of 1, 2, 4, 8, 16 or 32 bytes at a time */
	DMA_1B = 0,
	DMA_2B = 1,
	DMA_4B = 2,
	DMA_8B = 7,
	DMA_16B = 3,
	DMA_32B = 4,

	/* Transfers of 16 or 32 bytes divided in two operations */
	DMA_16B_DIV = 11,
	DMA_32B_DIV = 12,

} dma_size_t;

/* dma_address_t - Addressing mode for source and destination regions */
typedef enum
{
	/* Fixed address mode: the same address is read/written at each step */
	DMA_FIXED = 0,
	/* Increment: address is incremented by transfer size at each step */
	DMA_INC = 1,
	/* Decrement: only allowed for 1/2/4-byte transfers */
	DMA_DEC = 2,
	/* Fixed division mode: address is fixed even in 16/32-divide mode */
	DMA_FIXEDDIV = 3,

} dma_address_t;

/* dma_transfer() - Start a data transfer on channel 0
   This function returns just when the transfer starts. The transfer will end
   later on and the DMA will be stopped by an interrupt. Call
   dma_transfer_wait() if you need to wait for the transfer to finish. Don't
   start a new transfer until the current one is finished!

   @channel   DMA channel (0..5)
   @size      Transfer size
   @blocks    Number of blocks (transferred memory = size * blocks)
   @src       Source pointer, must be aligned with transfer size
   @src_mode  Source address mode
   @dst       Destination address, must be aligned with transfer size
   @dst_mode  Destination address mode */
void dma_transfer(int channel, dma_size_t size, uint length,
	void const *src, dma_address_t src_mode,
	void *dst, dma_address_t dst_mode);

/* dma_transfer_wait() - Wait for a transfer on channel 0 to finish

   You should call this function when you need to transfer to be complete
   before continuing execution. If you are sure that the transfer is finished,
   this is not necessary (the only way to know is to look at the DMA registers
   or record interrupts).

   @channel   DMA channel (0..5) */
void dma_transfer_wait(int channel);

/* dma_transfer_noint() - Perform a data transfer without interrupts
   This function performs a transfer much like dma_transfer(), but doesn't use
   interrupts and *actively waits* for the transfer to finish, returning when
   it's finished. Don't call dma_transfer_wait() after using this function.

   Not using interrupts is a bad design idea for a majority of programs, and is
   only ever needed to display panic messages inside exception handlers. */
void dma_transfer_noint(int channel, dma_size_t size, uint blocks,
	void const *src, dma_address_t src_mode,
	void *dst, dma_address_t dst_mode);

#endif /* GINT_DMA */
