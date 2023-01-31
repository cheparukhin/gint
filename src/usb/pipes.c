#include <gint/usb.h>
#include <gint/mpu/usb.h>
#include <gint/clock.h>
#include <gint/dma.h>
#include <gint/defs/util.h>

#include <string.h>

#include "asyncio.h"
#include "usb_private.h"

#define USB SH7305_USB

//---
// Operations on pipes
//---

/* usb_pipe_configure(): Configure a pipe when opening the connection */
void usb_pipe_configure(int address, endpoint_t const *ep)
{
	/* Maps USB 2.0 transfer types to SH7305 USB module transfer types */
	static uint8_t type_map[4] = {
		0, TYPE_ISOCHRONOUS, TYPE_BULK, TYPE_INTERRUPT };

	if(ep->pipe > 0) {
		/* Set PID to NAK so we can modify the pipe's configuration */
		USB.PIPECTR[ep->pipe-1].PID = PID_NAK;
		usb_while(USB.PIPECTR[ep->pipe-1].PBUSY);
	}

	int type = type_map[ep->dc->bmAttributes & 0x03];
	bool dir_transmitting = (address & 0x80) != 0;
	bool dir_receiving = !dir_transmitting;

	USB.PIPESEL.PIPESEL = ep->pipe;
	USB.PIPECFG.TYPE    = type;
	USB.PIPECFG.BFRE    = dir_receiving;
	/* Enable continuous mode on all bulk transfer pipes
	   TODO: Also make it double mode*/
	USB.PIPECFG.DBLB    = 0;
	USB.PIPECFG.CNTMD   = (type == TYPE_BULK);
	USB.PIPECFG.SHTNAK  = 1;
	USB.PIPECFG.DIR     = dir_transmitting;
	USB.PIPECFG.EPNUM   = (address & 0x0f);

	USB.PIPEBUF.BUFSIZE = ep->bufsize - 1;
	USB.PIPEBUF.BUFNMB  = ep->bufnmb;

	USB.PIPEMAXP.MXPS   = le16toh(ep->dc->wMaxPacketSize);

	USB.PIPEPERI.IFIS = 0;
	USB.PIPEPERI.IITV = 0;

	if(ep->pipe >= 1 && ep->pipe <= 5) {
		USB.PIPETR[ep->pipe-1].TRE.TRCLR = 1;
		USB.PIPETR[ep->pipe-1].TRE.TRENB = 0;
	}
}

/* usb_pipe_clear(): Clear all data in the pipe */
void usb_pipe_clear(int pipe)
{
	if(pipe <= 0 || pipe > 9) return;

	/* Set PID=NAK then use ACLRM to clear the pipe */
	USB.PIPECTR[pipe-1].PID = PID_NAK;
	usb_while(USB.PIPECTR[pipe-1].PBUSY);

	USB.PIPECTR[pipe-1].ACLRM = 1;
	USB.PIPECTR[pipe-1].ACLRM = 0;

	/* Clear the sequence bit (important after a world switch since we restore
	   hardware registers but the host connection is starting from scratch!) */
	USB.PIPECTR[pipe-1].SQCLR = 1;
	usb_while(USB.PIPECTR[pipe-1].SQMON != 0);
}

//---
// Operation on FIFO controllers
//---

/* fifoct_t: FIFO controllers to access pipe queues */
typedef enum {
	NOF = 0,  /* No FIFO controller */
	CF,       /* Used for the Default Control Pipe */
	D0F,      /* FIFO Controller 0 */
	D1F,      /* FIFO Controller 1 */
} GPACKEDENUM fifo_t;

enum {
	FIFO_READ   = 0, /* Read mode */
	FIFO_WRITE  = 1, /* Write mode */
};

/* fifo_find_available_controller(): Get a FIFO controller for a pipe */
static fifo_t fifo_find_available_controller(int pipe)
{
	if(pipe == 0) return CF;

	if(USB.D0FIFOSEL.CURPIPE == 0) return D0F;
	USB_LOG("D0 is unavailable!\n");
	if(USB.D1FIFOSEL.CURPIPE == 0) return D1F;
	USB_LOG("D1 is unavailable!\n");

	return NOF;
}

/* fifo_bind(): Bind a FIFO to a pipe in reading or writing mode */
static void fifo_bind(fifo_t ct, int pipe, int mode, int size)
{
	size = (size - (size == 4) - 1) & 3;

	if(pipe == 0) {
		if(USB.CFIFOSEL.ISEL == 1 && USB.DCPCTR.PID == 1)
			return;

		if(mode == FIFO_WRITE)
			USB.DCPCTR.PID = PID_BUF;
		/* RCNT=0 REW=0 MBW=size BIGEND=1 ISEL=mode CURPIPE=0 */
		USB.CFIFOSEL.word = 0x0100 | (mode << 5) | (size << 10);
		usb_while(!USB.CFIFOCTR.FRDY || USB.CFIFOSEL.ISEL != mode);
		return;
	}

	__typeof__(USB.D0FIFOSEL) sel;
	sel.RCNT = 0;
	sel.REW = 0;
	sel.DCLRM = (mode == FIFO_READ);
	sel.DREQE = 0;
	sel.MBW = size;
	sel.BIGEND = 1;
	sel.CURPIPE = pipe;

	if(ct == D0F) {
		USB.D0FIFOSEL.word = sel.word;
		usb_while(!USB.D0FIFOCTR.FRDY || USB.PIPECFG.DIR != mode);
	}
	if(ct == D1F) {
		USB.D1FIFOSEL.word = sel.word;
		usb_while(!USB.D1FIFOCTR.FRDY || USB.PIPECFG.DIR != mode);
	}

	/* Enable USB comunication! */
	USB.PIPECTR[pipe-1].PID = PID_BUF;
}

/* fifo_unbind(): Unbind a FIFO */
static void fifo_unbind(fifo_t ct)
{
	int pipe = -1;
	if(ct == D0F) pipe = USB.D0FIFOSEL.CURPIPE;
	if(ct == D1F) pipe = USB.D1FIFOSEL.CURPIPE;
	if(pipe <= 0)
		return;

	/* Disbable communication on the pipe */
	USB.PIPECTR[pipe-1].PID = PID_NAK;
	usb_while(USB.PIPECTR[pipe-1].PBUSY);

	if(ct == D0F) {
		USB.D0FIFOSEL.word = 0x0000;
		usb_while(USB.D0FIFOSEL.CURPIPE != 0);
	}
	if(ct == D1F) {
		USB.D1FIFOSEL.word = 0x0000;
		usb_while(USB.D1FIFOSEL.CURPIPE != 0);
	}
}

//---
// Writing operations
//---

/* Current operation waiting to be performed on each pipe. There are two
   possible states for a pipe's transfer data:
   -> Either there is a transfer going on, in which case (data != NULL),
      (size != 0), and (buffer_used) has no meaning.
   -> Either there is no transfer going on, and (data = NULL), (size = 0).

   A controller is assigned to t->controller when a write first occurs until
   the pipe is fully committed. (ct = NOF) indicates an unused pipe, while
   (ct != NOF) indicates that stuff has been written and is waiting a commit.

   Additionally, between a call to write_round() and the corresponding
   finish_write(), the (round_size) attribute is set to a non-zero value
   indicating how many bytes are waiting for write completion. */

/* Multi-round operations to be continued whenever buffers are ready */
GBSS static asyncio_op_t volatile pipe_transfers[10];

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
	if(pipe_transfers[pipe].data_w) return true;
	/* Transfer in progress */
	if(pipe && !USB.PIPECTR[pipe-1].BSTS) return true;
	/* Callback for a just-finished transfer not yet called */
	if(pipe_transfers[pipe].round_size) return true;
	/* All good */
	return false;
}

/* Size of a pipe's buffer area, in bytes */
static int pipe_bufsize(int pipe)
{
	if(pipe == 0)
		return USB.DCPMAXP.MXPS;

	USB.PIPESEL.PIPESEL = pipe;
	return (USB.PIPEBUF.BUFSIZE + 1) * 64;
}

/* finish_transfer(): Finish a multi-round write transfer

   This function is called when the final round of a transfer has completed,
   either by the handler of the BEMP interrupt or by the usb_commit_async()
   function if the pipe is being committed when empty. */
static void finish_transfer(asyncio_op_t volatile *t, int pipe)
{
	/* Free the FIFO controller */
	fifo_unbind(t->controller);
	t->controller = NOF;

	/* Mark the transfer as unused */
	t->committed_w = false;
	t->buffer_used = 0;

	/* Disable the interrupt */
	if(pipe != 0)
		USB.BEMPENB &= ~(1 << pipe);

	gint_call(t->callback);
	USB_TRACE("finish_transfer()");
}

/* finish_round(): Update transfer logic after a write round completes

   This function is called when a write round completes, either by the handler
   of the BEMP interrupt if the round filled the FIFO, or by the handler of the
   DMA transfer or the write_round() function itself if it didn't.

   It the current write operation has finished with this round, this function
   invokes the write_async callback. */
static void finish_round(asyncio_op_t volatile *t, int pipe)
{
	/* Update the pointer as a result of the newly-finished write */
	t->buffer_used += t->round_size;
	t->data_w += t->round_size;
	t->size -= t->round_size;
	t->round_size = 0;

	/* Account for auto-transfers */
	if(t->buffer_used == pipe_bufsize(pipe))
		t->buffer_used = 0;

	/* At the end, free the FIFO and invoke the callback. Hold the
	   controller until the pipe is committed */
	if(t->size == 0) {
		t->data_w = NULL;
		gint_call(t->callback);
	}

	USB_TRACE("finish_round()");
}

/* write_round(): Write up to a FIFO's worth of data to a pipe

   If this is a partial round (FIFO not going to be full), finish_round() is
   invoked after the write. Otherwise the FIFO is transmitted automatically and
   the BEMP handler will call finish_round() after the transfer. */
static void write_round(asyncio_op_t volatile *t, int pipe)
{
	fifo_t ct = t->controller;
	void volatile *FIFO = NULL;

	if(ct == CF)  FIFO = &USB.CFIFO;
	if(ct == D0F) FIFO = &USB.D0FIFO;
	if(ct == D1F) FIFO = &USB.D1FIFO;
	fifo_bind(ct, pipe, FIFO_WRITE, t->unit_size);

	/* Amount of data that can be transferred in a single run */
	int available = pipe_bufsize(pipe) - (pipe == 0 ? 0 : t->buffer_used);
	int size = min(t->size, available);
	t->round_size = size;

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

		gint_call_t callback = partial ?
			GINT_CALL(finish_round, (void *)t, pipe) :
			GINT_CALL_NULL;

		/* Use DMA channel 3 for D0F and 4 for D1F */
		int channel = (ct == D0F) ? 3 : 4;

		bool ok = dma_transfer_async(channel, block_size, size,
			t->data_w, DMA_INC, (void *)FIFO, DMA_FIXED, callback);
		if(!ok) USB_LOG("DMA async failed on channel %d!\n", channel);
	}
	else
	{
		if(t->unit_size == 1) write_8(t->data_w, size, FIFO);
		if(t->unit_size == 2) write_16(t->data_w, size >> 1, FIFO);
		if(t->unit_size == 4) write_32(t->data_w, size >> 2, FIFO);
		if(partial) finish_round(t, pipe);
	}

	USB_TRACE("write_round()");
}

int usb_write_async(int pipe, void const *data, int size, int unit_size,
	bool use_dma, gint_call_t callback)
{
	if(pipe_busy(pipe)) return USB_WRITE_BUSY;

	asyncio_op_t volatile *t = &pipe_transfers[pipe];
	if(!data || !size) return 0;

	/* Re-use the controller from a previous write if there is one,
	   otherwise try to get a new free one */
	/* TODO: usb_write_async(): TOC/TOU race on controller being free */
	fifo_t ct = t->controller;
	if(ct == NOF) ct = fifo_find_available_controller(pipe);
	if(ct == NOF) return USB_WRITE_NOFIFO;

	t->data_w = data;
	t->size = size;
	t->unit_size = (pipe == 0) ? 1 : unit_size;
	t->dma = use_dma;
	t->committed_w = false;
	t->controller = ct;
	t->callback = callback;

	/* Set up the Buffer Empty interrupt to refill the buffer when it gets
	   empty, and be notified when the transfer completes. */
	if(pipe) USB.BEMPENB |= (1 << pipe);

	write_round(t, pipe);
	return 0;
}

int usb_write_sync_timeout(int pipe, void const *data, int size, int unit_size,
	bool use_dma, timeout_t const *timeout)
{
	volatile int flag = 0;

	while(1)
	{
		int rc = usb_write_async(pipe, data, size, unit_size, use_dma,
			GINT_CALL_SET(&flag));
		if(rc == 0)
			break;
		if(rc == USB_WRITE_NOFIFO)
			USB_LOG("USB_WRITE_NOFIFO\n");
		if(rc != USB_WRITE_BUSY)
			return rc;
		if(timeout_elapsed(timeout))
			return USB_WRITE_TIMEOUT;
		sleep();
	}

	while(!flag)
	{
		if(timeout_elapsed(timeout))
			return USB_WRITE_TIMEOUT;
		sleep();
	}
	return 0;
}

int usb_write_sync(int pipe, void const *data, int size, int unit, bool dma)
{
	return usb_write_sync_timeout(pipe, data, size, unit, dma, NULL);
}

int usb_commit_async(int pipe, gint_call_t callback)
{
	asyncio_op_t volatile *t = &pipe_transfers[pipe];
	if(pipe_busy(pipe)) return USB_COMMIT_BUSY;

	if(t->controller == NOF) return USB_COMMIT_INACTIVE;

	t->committed_w = true;
	t->callback = callback;

	/* TODO: Handle complex commits on the DCP */
	if(pipe == 0)
	{
		finish_transfer(t, pipe);
		USB.CFIFOCTR.BVAL = 1;
		return 0;
	}

	/* Committing an empty pipe ends the transfer on the spot */
	if(t->buffer_used == 0)
	{
		finish_transfer(t, pipe);
		return 0;
	}

	/* Set BVAL=1 and inform the BEMP handler of the commitment with the
	   committed_w flag; the handler will invoke finish_transfer() */
	fifo_bind(t->controller, pipe, FIFO_WRITE, t->unit_size);
	if(t->controller == D0F) USB.D0FIFOCTR.BVAL = 1;
	if(t->controller == D1F) USB.D1FIFOCTR.BVAL = 1;
	USB_LOG("[PIPE%d] Committed transfer\n", pipe);

	return 0;
}

int usb_commit_sync_timeout(int pipe, timeout_t const *timeout)
{
	int volatile flag = 0;

	/* Wait until the pipe is free, then commit */
	while(1)
	{
		int rc = usb_commit_async(pipe, GINT_CALL_SET(&flag));
		if(rc == 0)
			break;
		if(rc != USB_COMMIT_BUSY)
			return rc;
		if(timeout_elapsed(timeout))
			return USB_COMMIT_TIMEOUT;
		sleep();
	}

	/* Wait until the commit completes */
	while(!flag)
	{
		if(timeout_elapsed(timeout))
			return USB_COMMIT_TIMEOUT;
		sleep();
	}
	return 0;
}

void usb_commit_sync(int pipe)
{
	usb_commit_sync_timeout(pipe, NULL);
}

/* usb_pipe_write_bemp(): Callback for the BEMP interrupt on a pipe */
void usb_pipe_write_bemp(int pipe)
{
	asyncio_op_t volatile *t = &pipe_transfers[pipe];

	if(t->committed_w)
	{
		finish_transfer(t, pipe);
	}
	else
	{
		/* Finish a round; if there is more data, keep going */
		finish_round(t, pipe);
		if(t->data_w) write_round(t, pipe);
	}
}
