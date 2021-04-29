#include <gint/usb.h>
#include <gint/mpu/usb.h>
#include <gint/clock.h>
#include <gint/dma.h>
#include <gint/defs/util.h>
#include <gint/std/string.h>
#include "usb_private.h"

#define USB SH7305_USB

//---
// Operations on pipe controllers
//---

/* pipect_t: Pipe controllers used to access pipe configuration and contents */
typedef enum {
	CF,   /* Used for the Default Control Pipe */
	D0F,  /* Used for main-thread access to other pipes */
	D1F,  /* Used for interrupt-thread access to other pipes */
} pipect_t;

/* pipect(): Determine which controller to use to access a pipe */
pipect_t pipect(int pipe)
{
	if(pipe == 0) return CF;

	/* In normal program flow, use D0FIF0; in interrupt mode, use D1FIF0.
	   This avoids access races */
	uint32_t sr, IMASK;
	__asm__("stc sr, %0": "=r"(sr));
	IMASK = (sr >> 4) & 0xf;

	return (IMASK ? D1F : D0F);
}

/* usb_pipe_configure(): Configure a pipe when opening the connection */
void usb_pipe_configure(int address, endpoint_t const *ep)
{
	/* Maps USB 2.0 transfer types to SH7305 USB module transfer types */
	static uint8_t type_map[4] = { 0, 3, 1, 2 };
	USB.PIPESEL.PIPESEL = ep->pipe;

	USB.PIPECFG.TYPE    = type_map[ep->dc->bmAttributes & 0x03];
	USB.PIPECFG.BFRE    = 0;
	USB.PIPECFG.DBLB    = 0;
	USB.PIPECFG.CNTMD   = 1;
	USB.PIPECFG.SHTNAK  = 0;
	USB.PIPECFG.DIR     = (address & 0x80) != 0;
	USB.PIPECFG.EPNUM   = (address & 0x0f);

	USB.PIPEBUF.BUFSIZE = ep->bufsize - 1;
	USB.PIPEBUF.BUFNMB  = ep->bufnmb;

	USB.PIPEMAXP.MXPS   = le16toh(ep->dc->wMaxPacketSize);
}

/* usb_pipe_clear(): Clear all data in the pipe */
void usb_pipe_clear(int pipe)
{
	if(pipe <= 0 || pipe > 9) return;

	/* Set PID=NAK then use ACLRM to clear the pipe */
	USB.PIPECTR[pipe-1].PID = 0;
	while(USB.PIPECTR[pipe-1].PBUSY) {}

	USB.PIPECTR[pipe-1].ACLRM = 1;
	USB.PIPECTR[pipe-1].ACLRM = 0;
	while(!USB.PIPECTR[pipe-1].BSTS) {}

	USB.PIPECTR[pipe-1].PID = 0;
	USB.PIPECTR[pipe-1].SQCLR = 1;
}

/* pipe_mode(): Set the pipe in reading or writing mode */
static void pipe_mode(pipect_t ct, int pipe, int mode, int size)
{
	size = (size - (size == 4) - 1) & 3;

	if(ct == CF)
	{
		if(mode == 1) USB.DCPCTR.PID = 1;
		/* RCNT=0 REW=0 MBW=size BIGEND=1 ISEL=mode CURPIPE=0 */
		USB.CFIFOSEL.word = 0x0100 | (mode << 5) | (size << 10);
		usb_while(!USB.CFIFOCTR.FRDY || USB.CFIFOSEL.ISEL != mode);
		return;
	}

	/* Set PID to BUF */
	USB.PIPECTR[pipe-1].PID = 1;

	/* RCNT=0 REW=0 DCLRM=0 DREQE=0 MBW=size BIGEND=1 */
	if(ct == D0F) USB.D0FIFOSEL.word = 0x0100 | (size << 10) | pipe;
	if(ct == D1F) USB.D1FIFOSEL.word = 0x0100 | (size << 10) | pipe;

	if(ct == D0F) usb_while(!USB.D0FIFOCTR.FRDY || USB.PIPECFG.DIR!=mode);
	if(ct == D1F) usb_while(!USB.D1FIFOCTR.FRDY || USB.PIPECFG.DIR!=mode);
}
void usb_pipe_mode_read(int pipe, int read_size)
{
	return pipe_mode(pipect(pipe), pipe, 0, read_size);
}
void usb_pipe_mode_write(int pipe, int write_size)
{
	return pipe_mode(pipect(pipe), pipe, 1, write_size);
}

//---
// Writing operations
//---

/* Current operation waiting to be performed on each pipe */
struct transfer {
	/* Address of data to transfer next; NULL if no transfer */
	void const *data;
	/* Size of data left to transfer */
	int size;
	/* Size of data currently in the FIFO (less than the FIFO capacity) */
	uint16_t used;
	/* Data sent in the last transfer not yet finished by finish_round() */
	uint16_t flying;
	/* Write size */
	uint8_t unit_size;
	/* Whether the data has been committed to a transfer */
	bool committed;
	/* Whether to use the DMA */
	bool dma;
	/* Callback to be invoked at the end of the current write or commit
	   (both cannot exist at the same time) */
	gint_call_t callback;
};
/* Multi-round operations to be continued whenever buffers are ready */
GBSS static struct transfer volatile pipe_transfers[10];

void usb_pipe_init_transfers(void)
{
	memset((void *)pipe_transfers, 0, sizeof pipe_transfers);
}

static void write_8(uint8_t const *data, int size, uint8_t volatile *FIFO)
{
	for(int i = 0; i < size; i++) *FIFO = data[i];
}
static void write_16(uint16_t const *data, int size, uint16_t volatile *FIFO)
{
	for(int i = 0; i < size; i++) *FIFO = data[i];
}
static void write_32(uint32_t const *data, int size, uint32_t volatile *FIFO)
{
	for(int i = 0; i < size; i++) *FIFO = data[i];
}

/* Check whether a pipe is busy with a multi-round write or a transfer */
GINLINE static bool pipe_busy(int pipe)
{
	/* Multi-round write still not finished */
	if(pipe_transfers[pipe].data) return true;
	/* Transfer in progress */
	if(pipe && !USB.PIPECTR[pipe-1].BSTS) return true;
	/* Callback for a just-finished transfer not yet called */
	if(pipe_transfers[pipe].flying) return true;
	/* All good */
	return false;
}

/* Size of a pipe's buffer area, in bytes */
static int pipe_bufsize(int pipe)
{
	if(pipe == 0) return USB.DCPMAXP.MXPS;

	USB.PIPESEL.PIPESEL = pipe;
	return (USB.PIPEBUF.BUFSIZE + 1) * 64;
}

/* finish_round(): Update transfer logic after a write round completes

   This function is called when a write round completes, either by the handler
   of the BEMP interrupt if the round filled the FIFO, or by the handler of the
   DMA transfer the or write_round() function itself if it didn't.

   It the current write operation has finished with this round, this function
   invokes the write_async callback. */
static void finish_round(struct transfer volatile *t, int pipe)
{
	/* Update the pointer as a result of the newly-finished write */
	t->used += t->flying;
	t->data += t->flying;
	t->size -= t->flying;
	t->flying = 0;

	/* Account for auto-transfers */
	if(t->used == pipe_bufsize(pipe)) t->used = 0;

	/* Invoke the callback at the end */
	if(t->size == 0)
	{
		t->data = NULL;
		if(t->callback.function) gint_call(t->callback);
	}
}

/* write_round(): Write up to a FIFO's worth of data to a pipe
   If this is a partial round (FIFO not going to be full), finish_round() is
   invoked after the write. Otherwise the FIFO is transmitted automatically and
   the BEMP handler will call finish_round() after the transfer. */
static void write_round(struct transfer volatile *t, int pipe)
{
	pipect_t ct = pipect(pipe);
	void volatile *FIFO = NULL;

	if(ct == CF)  FIFO = &USB.CFIFO;
	if(ct == D0F) FIFO = &USB.D0FIFO;
	if(ct == D1F) FIFO = &USB.D1FIFO;

	if(pipe) pipe_mode(ct, pipe, 1, t->unit_size);

	/* Amount of data that can be transferred in a single run */
	int available = pipe_bufsize(pipe) - (pipe == 0 ? 0 : t->used);
	int size = min(t->size, available);
	t->flying = size;

	/* If this is a partial write (size < available), call finish_round()
	   after the copy to notify the user that the pipe is ready. Otherwise,
	   a USB transfer will occur and the BEMP handler will do it. */
	bool partial = (size < available);

	if(t->dma)
	{
		/* TODO: USB: Can we use 32-byte DMA transfers? */
		int block_size = DMA_1B;
		if(t->unit_size == 2) block_size = DMA_2B, size >>= 1;
		if(t->unit_size == 4) block_size = DMA_4B, size >>= 2;

		gint_call_t callback = !partial ? GINT_CALL_NULL :
			GINT_CALL(finish_round, (void *)t, pipe);

		/* TODO: DMA support in usb_write_async()/write_round() */
		/* TODO: USB: Don't use a fixed DMA channel */
		dma_transfer_async(3, block_size, size,
			t->data, DMA_INC, (void *)FIFO, DMA_FIXED, callback);
	}
	else
	{
		if(t->unit_size == 1) write_8(t->data, size, FIFO);
		if(t->unit_size == 2) write_16(t->data, size >> 1, FIFO);
		if(t->unit_size == 4) write_32(t->data, size >> 2, FIFO);
		if(partial) finish_round(t, pipe);
	}
}

int usb_write_async(int pipe, void const *data, int size, int unit_size,
	bool use_dma, gint_call_t callback)
{
	if(pipe_busy(pipe)) return USB_WRITE_BUSY;

	struct transfer volatile *t = &pipe_transfers[pipe];
	if(!data || !size) return 0;

	t->data = data;
	t->size = size;
	t->unit_size = unit_size;
	t->dma = use_dma;
	t->committed = false;
	t->callback = callback;

	/* Set up the Buffer Empty interrupt to refill the buffer when it gets
	   empty, and be notified when the transfer completes. */
	if(pipe) USB.BEMPENB.word |= (1 << pipe);

	write_round(t, pipe);
	return 0;
}

int usb_write_sync(int pipe, void const *data, int size, int unit_size,
	bool use_dma)
{
	/* Wait for a previous write and/or transfer to finish */
	while(pipe_busy(pipe)) sleep();

	volatile int flag = 0;
	usb_write_async(pipe, data, size, unit_size, use_dma,
		GINT_CALL_SET(&flag));

	while(!flag) sleep();

	return 0;
}


int usb_commit_async(int pipe, gint_call_t callback)
{
	struct transfer volatile *t = &pipe_transfers[pipe];
	if(pipe_busy(pipe)) return USB_COMMIT_BUSY;

	/* TODO: USB: Commit on the DCP? */
	if(pipe == 0) return 0;

	/* Commiting an empty pipe is a no-op */
	if(t->used == 0)
	{
		if(callback.function) gint_call(callback);
		return 0;
	}

	/* Set BVAL=1 and inform the BMEP handler of the commitment with the
	   committed flag; the handler will invoke the commit callback */
	t->committed = true;
	t->callback = callback;

	pipect_t ct = pipect(pipe);
	if(ct == D0F) USB.D0FIFOCTR.BVAL = 1;
	if(ct == D1F) USB.D1FIFOCTR.BVAL = 1;
	usb_log("[PIPE%d] Committed transfer\n", pipe);

	return 0;
}

void usb_commit_sync(int pipe)
{
	volatile int flag = 0;
	int rc = 0;

	/* Wait until the pipe is free, then commit */
	do rc = usb_commit_async(pipe, GINT_CALL_SET(&flag));
	while(rc == USB_COMMIT_BUSY);

	/* Wait until the commit completes */
	while(!flag) sleep();
}

/* usb_pipe_write_bemp(): Callback for the BEMP interrupt on a pipe */
void usb_pipe_write_bemp(int pipe)
{
	struct transfer volatile *t = &pipe_transfers[pipe];

	if(t->committed)
	{
		/* Finish transfer, disable interrupt, reset logic */
		t->committed = false;
		t->used = 0;
		USB.BEMPENB.word &= ~(1 << pipe);

		if(t->callback.function) gint_call(t->callback);
	}
	else
	{
		/* Finish a round; if there is more data, keep going */
		finish_round(t, pipe);
		if(t->data) write_round(t, pipe);
	}
}
