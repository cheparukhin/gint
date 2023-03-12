#include <gint/usb.h>
#include <gint/mpu/usb.h>
#include <gint/clock.h>
#include <gint/dma.h>
#include <gint/defs/util.h>

#include <string.h>

#include <gint/drivers/asyncio.h>
#include "usb_private.h"

#define USB SH7305_USB

//---
// Operations on pipes
//---

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
	USB.PIPECFG.BFRE    = 0;
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

	/* Keep receiving pipes open all the time */
	if(dir_receiving) {
		USB.PIPECTR[ep->pipe-1].PID = PID_BUF;
		USB.BRDYENB |= (1 << ep->pipe);
	}
}

void usb_pipe_clear(int pipe)
{
	if(pipe < 0 || pipe > 9) return;

	if(pipe == 0) {
		USB.DCPCTR.PID = PID_NAK;
		usb_while(USB.DCPCTR.PBUSY);

		USB.DCPCTR.SQCLR = 1;
		usb_while(USB.DCPCTR.SQMON != 0);
	}

	/* Set PID=NAK then use ACLRM to clear the pipe */
	USB.PIPECTR[pipe-1].PID = PID_NAK;
	usb_while(USB.PIPECTR[pipe-1].PBUSY);

	USB.PIPECTR[pipe-1].ACLRM = 1;
	USB.PIPECTR[pipe-1].ACLRM = 0;

	/* Clear the sequence bit (important after a world switch since we
	   restore hardware registers but the connection to the hosts restarts
	   from scratch!) */
	USB.PIPECTR[pipe-1].SQCLR = 1;
	usb_while(USB.PIPECTR[pipe-1].SQMON != 0);
}

void usb_pipe_reset(int pipe)
{
	if(pipe < 0 || pipe > 9) return;
	usb_pipe_clear(pipe);

	if(pipe == 0) {
		USB.DCPCFG.word = 0x0000;
		USB.DCPCTR.word = 0x0000;
		USB.DCPMAXP.word = 0x0000;
		return;
	}

	USB.PIPESEL.PIPESEL = pipe;
	USB.PIPECFG.word = 0x0000;
	USB.PIPECTR[pipe-1].word = 0x0000;
	USB.PIPEBUF.word = 0x0000;
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
static void fifo_bind(fifo_t ct, int pipe, int mode)
{
	int reading = (mode == FIFO_READ);
	int writing = (mode == FIFO_WRITE);

	/* RCNT=0 REW=0 MBW=2 BIGEND=1 ISEL=mode CURPIPE=0 */
	if(ct == CF) {
		USB.CFIFOSEL.word = 0x0900 | (mode << 5);
		usb_while(!USB.CFIFOCTR.FRDY || USB.CFIFOSEL.ISEL != mode);
	}
	/* RCNT=0 REW=0 DCLRM=reading DREQE=0 MBW=2 BIGEND=1 CURPIPE=pipe */
	if(ct == D0F) {
		USB.PIPESEL.PIPESEL = pipe;
		USB.D0FIFOSEL.word = 0x0900 | (reading << 13) | pipe;
		usb_while(!USB.D0FIFOCTR.FRDY || USB.PIPECFG.DIR != mode);
	}
	if(ct == D1F) {
		USB.PIPESEL.PIPESEL = pipe;
		USB.D1FIFOSEL.word = 0x0900 | (reading << 13) | pipe;
		usb_while(!USB.D1FIFOCTR.FRDY || USB.PIPECFG.DIR != mode);
	}

	/* Enable USB comunication when writing */
	if(writing && pipe == 0)
		USB.DCPCTR.PID = PID_BUF;
	if(writing && pipe != 0)
		USB.PIPECTR[pipe-1].PID = PID_BUF;
}

/* fifo_unbind(): Unbind a FIFO */
static void fifo_unbind(fifo_t ct)
{
	int pipe = -1;
	/* TODO: USB (DCP normalization): NAK when unbinding?
	if(ct == CF) {
		USB.DCPCTR.CCPL = 0;
		USB.DCPCTR.PID = PID_NAK;
		usb_while(!USB.DCPCTR.PBUSY);
	} */
	if(ct == D0F) pipe = USB.D0FIFOSEL.CURPIPE;
	if(ct == D1F) pipe = USB.D1FIFOSEL.CURPIPE;
	if(pipe <= 0)
		return;

	/* Disable communication on the pipe */
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

void usb_pipe_reset_fifos(void)
{
	fifo_unbind(CF);
	fifo_unbind(D0F);
	fifo_unbind(D1F);
}

//---
// Writing operations
//---

/* The writing logic is asynchronous, which makes it sometimes hard to track.
   The series of call for a write I/O is zero or more usb_write_async()
   followed by a usb_commit_async():

     write_io ::= usb_write_async* usb_commit_async

   A usb_write_async() will write to the hardware buffer as many times as it
   takes to exhaust the input, including 1 time if the hardware buffer can hold
   the entire input and 0 times if there is no input. Each _round_ consists of
   a call to write_round() to copy with the CPU or start the copy with the DMA,
   and a call to finish_round() when the copy is finished.

   If the round fills the buffer, finish_round() is triggered by the BEMP
   interrupt after the hardware finishes transferring. Otherwise finish_round()
   is triggered directly when writing finishes.

     complete_round ::= write_round
                        <Finish writing with CPU or DMA>
                        <USB module auto-commits pipe>
                        <BEMP interrupt after transmission>
                        finish_round

     partial_round ::= write_round
                       <Finish writing with CPU or DMA>
                       finish_round

   Note that the "<Finish writing>" event is asynchronous if the DMA is used. A
   full write will take zero or more complete rounds followed by zero or one
   partial round before finish_write_call() is called:

     usb_write_async ::= complete_round* partial_round? finish_write_call

   And a commit will trigger a transmission of whatever is left in the buffer
   (including nothing) and wait for the BEMP interrupt.

     usb_commit_async ::= <Manually commit pipe>
                          <BEMP interrut after transmission>
                          finish_write_call

   Most functions can execute either in the main thread or within an interrupt
   handler. */
GBSS static asyncio_op_t pipe_transfers[10];

void usb_pipe_init_transfers(void)
{
	for(int i = 0; i < 10; i++)
		asyncio_op_clear(&pipe_transfers[i]);
}

void usb_wait_all_transfers(bool await_long_writes)
{
	while(1) {
		bool all_done = true;
		for(int i = 0; i < 10; i++) {
			asyncio_op_t const *t = &pipe_transfers[i];
			all_done &= !asyncio_op_busy(t);
			if(await_long_writes)
				all_done &= (t->type != ASYNCIO_WRITE);
		}
		if(all_done)
			return;
		sleep();
	}
}

/* Size of a pipe's buffer area, in bytes */
static int pipe_bufsize(int pipe)
{
	if(pipe == 0)
		return USB.DCPMAXP.MXPS;

	USB.PIPESEL.PIPESEL = pipe;
	return (USB.PIPEBUF.BUFSIZE + 1) * 64;
}

/* Called when a write/fsync call and its hardware interactions all complete */
static void finish_write_call(asyncio_op_t *t, int pipe)
{
	/* Unbind the FIFO after a sync */
	if(t->type == ASYNCIO_SYNC) {
		fifo_unbind(t->controller);
		t->controller = NOF;
	}

	/* Disable interrupts */
	if(pipe != 0)
		USB.BEMPENB &= ~(1 << pipe);

	if(t->type == ASYNCIO_WRITE)
		asyncio_op_finish_write(t);
	else if(t->type == ASYNCIO_SYNC)
		asyncio_op_finish_sync(t);
	USB_TRACE("finish_write_call()");
}

/* This function is called when a round of writing has completed, including all
   hardware interactions. If the FIFO got filled by the writing, this is after
   the transmission and BEMP interrupt; otherwise this is when the CPU/DMA
   finished writing. */
static void finish_write_round(asyncio_op_t *t, int pipe)
{
//	USB_LOG("[PIPE%d] finish_write_round() for %d bytes\n",
//		pipe, t->round_size);
	asyncio_op_finish_write_round(t);

	/* Account for auto-transfers */
	if(t->buffer_used == pipe_bufsize(pipe))
		t->buffer_used = 0;

	USB_TRACE("finish_write_round()");

	if(t->size == 0)
		finish_write_call(t, pipe);
}

/* write_round(): Write up to a FIFO's worth of data to a pipe

   If this is a partial round (FIFO not going to be full), finish_write_round()
   is invoked after the write. Otherwise the FIFO is transmitted automatically
   and the BEMP handler will call finish_write_round() after the transfer. */
static void write_round(asyncio_op_t *t, int pipe)
{
	fifo_t ct = t->controller;

	void volatile *FIFO = NULL;
	if(ct == CF)  FIFO = &USB.CFIFO;
	if(ct == D0F) FIFO = &USB.D0FIFO;
	if(ct == D1F) FIFO = &USB.D1FIFO;

	/* Amount of data that can be transferred in a single run */
	int available = pipe_bufsize(pipe) - t->buffer_used;
	int size = min(t->size, available);

	/* If we write partially (size < available), call finish_write_round()
	   after the copy to notify the user that the pipe is ready. Otherwise,
	   a USB transfer will occur and the BEMP handler will do it. */
	bool partial = (size < available);

	asyncio_op_start_write_round(t, size);

	if(t->dma)
	{
		gint_call_t callback = partial ?
			GINT_CALL(finish_write_round, (void *)t, pipe) :
			GINT_CALL_NULL;

		/* Use DMA channel 3 for D0F and 4 for D1F */
		int channel = (ct == D0F) ? 3 : 4;

		/* TODO: USB: Can we use 32-byte DMA transfers? */
		bool ok = dma_transfer_async(channel, DMA_4B, size >> 2,
			t->data_w, DMA_INC, (void *)FIFO, DMA_FIXED, callback);
		if(!ok) USB_LOG("DMA async failed on channel %d!\n", channel);
	}
	else
	{
		usb_pipe_write4(t->data_w, size, &t->shbuf, &t->shbuf_size,
			FIFO);
		if(partial) finish_write_round(t, pipe);
	}

	USB_TRACE("write_round()");
}

int usb_write_async(int pipe, void const *data, int size, bool use_dma,
	gint_call_t callback)
{
	asyncio_op_t *t = &pipe_transfers[pipe];
	if(asyncio_op_busy(t))
		return USB_BUSY;

	/* If this if the first write of a series, find a controller. */
	/* TODO: usb_write_async(): TOC/TOU race on controller being free */
	if(t->controller == NOF) {
		fifo_t ct = fifo_find_available_controller(pipe);
		if(ct == NOF)
			return USB_WRITE_NOFIFO;
		fifo_bind(ct, pipe, FIFO_WRITE);
		t->controller = ct;
	}

	asyncio_op_start_write(t, data, size, use_dma, &callback);

	/* Set up the Buffer Empty interrupt to refill the buffer when it gets
	   empty, and be notified when the transfer completes. */
	USB.BEMPENB |= (1 << pipe);

	write_round(t, pipe);
	return 0;
}

int usb_write_sync_timeout(int pipe, void const *data, int size, bool use_dma,
	timeout_t const *timeout)
{
	volatile int flag = 0;

	while(1)
	{
		int rc = usb_write_async(pipe, data, size, use_dma,
			GINT_CALL_SET(&flag));
		if(rc == 0)
			break;
		if(rc == USB_WRITE_NOFIFO)
			USB_LOG("USB_WRITE_NOFIFO\n");
		if(rc != USB_BUSY)
			return rc;
		if(timeout_elapsed(timeout))
			return USB_TIMEOUT;
		sleep();
	}

	while(!flag)
	{
		if(timeout_elapsed(timeout))
			return USB_TIMEOUT;
		sleep();
	}
	return 0;
}

int usb_write_sync(int pipe, void const *data, int size, bool dma)
{
	return usb_write_sync_timeout(pipe, data, size, dma, NULL);
}

int usb_commit_async(int pipe, gint_call_t callback)
{
	asyncio_op_t *t = &pipe_transfers[pipe];
	if(asyncio_op_busy(t))
		return USB_BUSY;
	if(t->type != ASYNCIO_WRITE || t->controller == NOF)
		return USB_COMMIT_INACTIVE;

	/* Flush any remaining bytes in the short buffer. This cannot fill the
	   buffer and create an auto-transmission situation; instead the module
	   remains idle after this write. This is because we only use 32-bit
	   writes, therefore at worst the buffer is 4 bytes away from being
	   full, and will not be filled by an extra 0-3 bytes. */
	void volatile *FIFO = NULL;
	if(t->controller == CF)  FIFO = &USB.CFIFO;
	if(t->controller == D0F) FIFO = &USB.D0FIFO;
	if(t->controller == D1F) FIFO = &USB.D1FIFO;
	usb_pipe_flush4(t->shbuf, t->shbuf_size, FIFO);

	/* Switch from WRITE to SYNC type; this influences the BEMP handler and
	   the final finish_write_call() */
	asyncio_op_start_sync(t, &callback);

	/* TODO: Figure out why previous attempts to use BEMP to finish commit
	   TODO| calls on the DCP failed with a freeze */
	if(pipe == 0) {
		USB.CFIFOCTR.BVAL = 1;
		finish_write_call(t, pipe);
		return 0;
	}

	/* Set BVAL=1 and inform the BEMP handler of the commitment with the
	   SYNC type; the handler will invoke finish_write_call() */
	USB.BEMPENB |= (1 << pipe);
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
		if(rc != USB_BUSY)
			return rc;
		if(timeout_elapsed(timeout))
			return USB_TIMEOUT;
		sleep();
	}

	/* Wait until the commit completes */
	while(!flag)
	{
		if(timeout_elapsed(timeout))
			return USB_TIMEOUT;
		sleep();
	}
	return 0;
}

void usb_commit_sync(int pipe)
{
	usb_commit_sync_timeout(pipe, NULL);
}

void usb_pipe_write_bemp(int pipe)
{
	asyncio_op_t *t = &pipe_transfers[pipe];

	if(t->type == ASYNCIO_SYNC)
	{
		finish_write_call(t, pipe);
	}
	else
	{
		/* Finish a round; if there is more data, keep going */
		finish_write_round(t, pipe);
		if(t->data_w) write_round(t, pipe);
	}
}

//---
// Reading operations
//---

static int handle_incoming_hwseg(asyncio_op_t *t, int pipe);

#ifdef GINT_USB_DEBUG
static void USB_LOG_TR(char const *prefix, asyncio_op_t *t)
{
	USB_LOG("%s: %s buf=%d%s%s req=%d/%d",
		prefix,
		t->type == ASYNCIO_READ ? "READ" : "NONE",
		t->buffer_used, t->cont_r ? "+":"", t->interrupt_r ? "!":"",
		t->round_size, t->size);
}
#define USB_LOG_TR(PREFIX, OP, ...) do {	\
	int __E = USB.INTENB0.BRDYE;		\
	USB.INTENB0.BRDYE = 0;			\
	USB_LOG_TR(PREFIX, OP);			\
	__VA_OPT__(USB_LOG(" " __VA_ARGS__));	\
	USB_LOG("\n");				\
	USB.INTENB0.BRDYE = __E;		\
} while(0)
#else
#define USB_LOG_TR(PREFIX, OP, ...)
#endif

/* Unbind resources after a hardware segment is finished reading. */
static void finish_read_hwseg(asyncio_op_t *t, int pipe)
{
#ifdef GINT_USB_DEBUG
	/* Log DTLN to help identify bugs where we clear data we didn't read */
	int DTLN = -1;
	if(t->controller == CF)  DTLN = USB.CFIFOCTR.DTLN;
	if(t->controller == D0F) DTLN = USB.D0FIFOCTR.DTLN;
	if(t->controller == D1F) DTLN = USB.D1FIFOCTR.DTLN;
#endif

	if(t->controller == CF)  USB.CFIFOCTR.BCLR = 1;
	if(t->controller == D0F) USB.D0FIFOCTR.BCLR = 1;
	if(t->controller == D1F) USB.D1FIFOCTR.BCLR = 1;

	USB_LOG("frhwseg: DTLN=%d cont=%d interrupt=%d\n", DTLN, t->cont_r,
		t->interrupt_r);

	asyncio_op_finish_read_hwseg(t);
	USB_TRACE("finish_read_hwseg()");

	if(!t->cont_r) {
		fifo_unbind(t->controller);
		t->controller = NOF;
	}

	/* Re-enable communication for the next segment */
	USB_LOG_TR("frhwseg", t, "--> PID=BUF");
	USB.PIPECTR[pipe-1].PID = PID_BUF;
}

static void finish_read_round(asyncio_op_t *t, int pipe)
{
	USB_LOG("[PIPE%d] finished reading %d/%d bytes\n", pipe,
		t->round_size, t->buffer_used);

	bool hwseg_finished = asyncio_op_finish_read_round(t);
	USB_LOG_TR("frr1", t, "hwseg finished = %d", hwseg_finished);
	if(hwseg_finished) {
		finish_read_hwseg(t, pipe);
		handle_incoming_hwseg(t, pipe);
	}

	USB_LOG("req. progress: size=%d cont=%d interrupt=%d\n",
		t->size, t->cont_r, t->interrupt_r);

	/* End the call if we exhausted either the buffer or the transaction */
	if(t->size == 0 || !t->cont_r) {
		asyncio_op_finish_read(t);
		USB_LOG_TR("fc", t);
		USB_TRACE("finish_read_round() finish call");
	}
}

static void read_round(asyncio_op_t *t, int pipe)
{
	int round_size = (t->size < t->buffer_used ? t->size : t->buffer_used);
	asyncio_op_start_read_round(t, round_size);
	USB_LOG_TR("rr", t);

	/* No data to read: finish the round immediately */
	if(round_size == 0 || t->data_r == NULL) {
		finish_read_round(t, pipe);
		return;
	}

	/* Read stuff (TODO: Smart reads + DMA) */
	uint32_t volatile *FIFO = NULL;
	if(t->controller == CF)  FIFO = &USB.CFIFO;
	if(t->controller == D0F) FIFO = &USB.D0FIFO;
	if(t->controller == D1F) FIFO = &USB.D1FIFO;

	void *dataptr = t->data_r;
	for(int i = 0; i < round_size / 4; i++) {
		*(uint32_t *)dataptr = *FIFO;
		dataptr += 4;
	}
	if(round_size & 2) {
		*(uint16_t *)dataptr = *(uint16_t volatile *)FIFO;
		dataptr += 2;
	}
	if(round_size & 1) {
		*(uint8_t *)dataptr = *(uint8_t volatile *)FIFO;
		dataptr += 1;
	}

	finish_read_round(t, pipe);
}

static int handle_incoming_hwseg(asyncio_op_t *t, int pipe)
{
	/* Do nothing if no interrupt is waiting or if the pipe is already
	   processing a previous hardware segment */
	if(!t->interrupt_r || t->buffer_used > 0)
		return 0;

	/* PID will stay at NAK for the entire duration of the segment */
	USB.PIPECTR[pipe-1].PID = PID_NAK;

	if(t->controller == NOF) {
		fifo_t ct = fifo_find_available_controller(pipe);
		if(ct == NOF)
			return USB_READ_NOFIFO;
		fifo_bind(ct, pipe, FIFO_READ);
		t->controller = ct;
	}

	t->interrupt_r = false;

	/* Continue an ongoing transfer */
	if(asyncio_op_busy(t)) {
		USB_LOG("PIPE %d new hwseg while busy -> new round\n", pipe);
		read_round(t, pipe);
		return 0;
	}

	int data_available = 0;
	if(t->controller == CF)  data_available = USB.CFIFOCTR.DTLN;
	if(t->controller == D0F) data_available = USB.D0FIFOCTR.DTLN;
	if(t->controller == D1F) data_available = USB.D1FIFOCTR.DTLN;

	/* USB requires a zero-length or short packet to finish a transaction,
	   which equates to a partially-full buffer */
	bool cont = (data_available == pipe_bufsize(pipe));

	asyncio_op_start_read_hwseg(t, data_available, cont);
	USB_LOG_TR("nseg", t, "PIPECTR=%04x", USB.PIPECTR[pipe-1].word);
	return 0;
}

int usb_read_async(int pipe, void *data, int size, bool use_dma,
	int *read_size, gint_call_t callback)
{
	asyncio_op_t *t = &pipe_transfers[pipe];
	int rc;

	if((rc = handle_incoming_hwseg(t, pipe)))
		return rc;
	if(asyncio_op_busy(t))
		return USB_BUSY;
	if(t->type == ASYNCIO_NONE)
		return USB_READ_IDLE;

	asyncio_op_start_read(t, data, size, use_dma, read_size, &callback);
	read_round(t, pipe);
	return 0;
}

int usb_read_sync_timeout(int pipe, void *data, int size, bool dma,
	timeout_t const *timeout)
{
	int volatile flag = 0;
	int read_size = 0;

	/* Wait until there is stuff to read, then read it */
	while(1) {
		int rc = usb_read_async(pipe, data, size, dma, &read_size,
			GINT_CALL_SET(&flag));
		if(rc == 0)
			break;
		if(rc != USB_BUSY && rc != USB_READ_IDLE)
			return rc;
		if(timeout_elapsed(timeout))
			return USB_TIMEOUT;
		sleep();
	}

	/* Wait until the read completes */
	while(!flag) {
		if(timeout_elapsed(timeout))
			return USB_TIMEOUT;
		sleep();
	}

	/* Ignore 0-length reads */
	if(read_size == 0)
		return usb_read_sync_timeout(pipe, data, size, dma, timeout);

	return read_size;
}

int usb_read_sync(int pipe, void *data, int size, bool use_dma)
{
	return usb_read_sync_timeout(pipe, data, size, use_dma, NULL);
}

bool usb_poll(int pipe)
{
	if(pipe < 0)
		return false;

	asyncio_op_t *t = &pipe_transfers[pipe];
	if(handle_incoming_hwseg(t, pipe))
		return false;

	return (t->type == ASYNCIO_READ) && (t->buffer_used > 0 || t->cont_r);
}

void usb_pipe_read_brdy(int pipe)
{
	USB_LOG("[PIPE%d] BRDY with PIPECTR %04x\n", pipe,
		USB.PIPECTR[pipe-1].word);

	asyncio_op_t *t = &pipe_transfers[pipe];

	/* The BRDY interrupt also occurs after a pipe clear */
	if(!USB.PIPECTR[pipe-1].BSTS)
		return;

	/* Signal the arrival to the main thread but don't do anything yet.
	   This is both for proper error handling and because changing transfer
	   data asynchronously would require disabling the interrupt in other
	   sections of the driver, which seems to cause data loss. */
	t->interrupt_r = true;

	/* Notify the interface so it can read or schedule to read */
	endpoint_t *ep = usb_get_endpoint_by_pipe(pipe);
	if(ep->intf->notify_read)
		ep->intf->notify_read(ep->dc->bEndpointAddress);

	USB_LOG_TR("int", t);
}
