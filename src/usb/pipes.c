#include <gint/usb.h>
#include <gint/mpu/usb.h>
#include <gint/clock.h>
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

	/* Set PID to NAK to clear the toggle bit, then BUF */
	USB.PIPECTR[pipe-1].PID = 0;
	USB.PIPECTR[pipe-1].SQCLR = 1;
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
	int used;
	/* Write size */
	uint8_t unit_size;
	/* Whether the data has been committed to a transfer */
	bool committed;
	/* Whether to use the DMA */
	bool dma;
	/* Callback at the end of the transfer */
	gint_call_t callback;
};
/* Operations to be continued whenever buffers get empty */
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

/* Commit the pipe if there is no data left and the commit flag is set */
static void maybe_commit(struct transfer volatile *t, int pipe)
{
	/* The DCP is always committed immediately and with CCPL */
	if(pipe == 0) return;
	pipect_t ct = pipect(pipe);

	/* The buffer is committed automatically if full because continuous
	   mode is enabled. Manually commit if no data is left. */
	if(t->committed && !t->data)
	{
		if(ct == D0F) USB.D0FIFOCTR.BVAL = 1;
		if(ct == D1F) USB.D1FIFOCTR.BVAL = 1;

		t->committed = false;
		usb_log("[PIPE%d] Committed transfer\n", pipe);
	}
}

/* write_round(): Write up to a FIFO's worth of data to a pipe

   Returns true if this last write will empty the queue, false if further
   writes are required. When writing with the DMA, returning true does not
   imply that the pipe can be accessed. */
static bool write_round(struct transfer volatile *t, int pipe)
{
	pipect_t ct = pipect(pipe);
	void volatile *FIFO = NULL;

	if(ct == CF)  FIFO = &USB.CFIFO;
	if(ct == D0F) FIFO = &USB.D0FIFO;
	if(ct == D1F) FIFO = &USB.D1FIFO;

	if(pipe) pipe_mode(ct, pipe, 1, t->unit_size);

	/* Amount of data that can be transferred in a single run */
	int bufsize=64, available=64;
	if(pipe != 0)
	{
		USB.PIPESEL.PIPESEL = pipe;
		bufsize = (USB.PIPEBUF.BUFSIZE + 1) * 64;
		available = bufsize - t->used;
	}
	int size = min(t->size, available);

	if(t->dma)
	{
		/* TODO: DMA support in usb_pipe_write(), write_round() */
		/* After the DMA starts the code below will update pointers for
		   the next iteration */
		// dma_start(X, Y, Z,
		//	GINT_CALL(maybe_commit, (void *)t, pipe));
	}
	else
	{
		if(t->unit_size == 1) write_8(t->data, size, FIFO);
		if(t->unit_size == 2) write_16(t->data, size >> 1, FIFO);
		if(t->unit_size == 4) write_32(t->data, size >> 2, FIFO);
	}

	t->used += size;
	t->data += size;
	t->size -= size;
	if(t->used == bufsize || t->committed) t->used = 0;
	if(t->size == 0) t->data = NULL;

	/* After a CPU write, commit if needed */
	if(!t->dma) maybe_commit(t, pipe);
	return (t->data == NULL);
}

/* usb_write_async(): Asynchronously write to a USB pipe */
int usb_write_async(int pipe, void const *data, int size, int unit_size,
	bool use_dma, gint_call_t callback)
{
	struct transfer volatile *t = &pipe_transfers[pipe];

	/* Do not initiate a write if a previous write is unfinished or an
	   ongoing transfer is awaiting completion */
	if(t->data || (pipe && !USB.PIPECTR[pipe-1].BSTS))
		return USB_WRITE_BUSY;

	if(!data || !size) return 0;

	t->data = data;
	t->size = size;
	t->unit_size = unit_size;
	t->dma = use_dma;
	t->committed = false;
	t->callback = callback;

	// TODO: Support callback in usb_write_async()

	write_round(t, pipe);

	/* Set up the Buffer Empty interrupt to refill the buffer when it gets
	   empty, and be notified when the transfer completes. */
	if(pipe) USB.BEMPENB.word |= (1 << pipe);
	return 0;
}

/* usb_write_sync(): Synchronously write to a USB pipe */
int usb_write_sync(int pipe, void const *data, int size, int unit_size,
   bool use_dma)
{
	struct transfer volatile *t = &pipe_transfers[pipe];

	/* Wait for a previous write and/or transfer to finish */
	while(t->data || (pipe && !USB.PIPECTR[pipe-1].BSTS)) sleep();

	usb_write_async(pipe, data, size, unit_size, use_dma, GINT_CALL_NULL);

	/* Wait for the write to finish (but not the transfer) */
	while(t->data) sleep();
	return 0;
}

void usb_commit_async(int pipe, gint_call_t callback)
{
	struct transfer volatile *t = &pipe_transfers[pipe];
	t->committed = true;
	t->callback = callback;

	/* Commit the pipe if writes have been completed already */
	maybe_commit(t, pipe);
}

/* usb_pipe_write_bemp(): Callback for the BEMP interrupt on a pipe */
void usb_pipe_write_bemp(int pipe)
{
	/* Eliminate interrupts that occur when the pipe is set up but no
	   transfer is occurring */
	struct transfer volatile *t = &pipe_transfers[pipe];
	if(!t->data) return;

	bool complete = write_round(t, pipe);
	if(!complete) return;

	USB.BEMPENB.word &= ~(1 << pipe);

	if(t->callback.function) gint_call(t->callback);
}
