//---
// gint:usb:usb-private - Private definitions for the USB driver
//---

#ifndef GINT_USB_USB_PRIVATE
#define GINT_USB_USB_PRIVATE

#include <gint/defs/attributes.h>
#include <gint/timer.h>
#include <gint/gint.h>

//---
// Configuration of the communication surface between module and host
//---

/* usb_configure_solve(): Find a configuration that can open these interfaces

   This function determines a way to share USB resources (endpoint numbers,
   pipes, and FIFO memory) between the provided interfaces, in order to open
   the connection with all of these interfaces enabled.

   This function must only be called when the USB connection is closed. It
   returns 0 on success and one of the USB_* error codes otherwise.

   @interfaces  NULL-terminated list of interfaces to open
   Returns an USB_* error code. */
int usb_configure_solve(usb_interface_t const **interfaces);

/* usb_configure_log(): Print configuration results in the usb_log()
   This function can be called even if usb_configure_solve() fails. */
void usb_configure_log(void);

/* usb_configure(): Load the generated configuration to the USB module

   This function configures the USB module's pipes and FIFO memory to prepare
   handling requests to the interfaces activated in usb_configure_solve(). This
   configuration step is un-done by either another configuration through a
   successful usb_open(), or a context restore in the USB driver. */
void usb_configure(void);

/* Error codes for USB functions */
enum {
	/* There are no interfaces */
	USB_OPEN_NO_INTERFACE = 1,
	/* There are more interfaces than supported (16) */
	USB_OPEN_TOO_MANY_INTERFACES,
	/* There are not enough endpoint numbers for every interface, or there
	   are not enough pipes to set them up */
	USB_OPEN_TOO_MANY_ENDPOINTS,
	/* There is not enough FIFO memory to use the requested buffer sizes */
	USB_OPEN_NOT_ENOUGH_MEMORY,
	/* Information is missing, such as buffer size for some endpoints */
	USB_OPEN_MISSING_DATA,
	/* Invalid parameters: bad endpoint numbers, bad buffer sizes... */
	USB_OPEN_INVALID_PARAMS,

	/* A write is already pending on this pipe */
	USB_WRITE_BUSY,
};

/* endpoint_t: Driver information for each open endpoint in the device

   There is one such structure for all 16 configurable endpoints, for each
   direction (totaling 32). endpoint_get() is used to query the structure by
   endpoint number (including the IN/OUT bit). */
typedef struct {
	/* Which interface this endpoint belongs to */
	usb_interface_t const *intf;
	/* Associated endpoint descriptor */
	usb_dc_endpoint_t const *dc;
	/* Associated pipe, must be a number from 1..9 */
	uint8_t pipe;
	/* Allocated pipe buffer area; this is valid for pipes 1..5. The
	   bufsize here is in range 1..32, as opposed to the field in PIPEBUF
	   which is in range 0..31. */
	uint8_t bufnmb;
	uint8_t bufsize;

} endpoint_t;

/* usb_configure_interfaces(): List configured interfaces */
usb_interface_t const * const *usb_configure_interfaces(void);

/* usb_configure_address(): Get the concrete endpoint address

   This function returns the endpoint address associated with the provided
   interface-numbered endpoint. The value is defined if an interface-provided
   endpoint descriptor with bEndpointAddress equal to either (address) or
   (address ^ 0x80) has been processed.

   This function is used both to access endpoint data for an interface-provided
   endpoint number, and to ensure that two interface-provided enpoint numbers
   with same base and opposing directions are assigned the same concrete
   endpoint number with its two opposing directions.

   @intf     Interface that provided the address
   @address  Endpoint address (as numbered by the interaface)
   -> Returns the assigned endpoint address, or -1 if unassigned. */
int usb_configure_address(usb_interface_t const *intf, int address);

/* usb_configure_endpoint(): Get endpoint data for a concrete address */
endpoint_t *usb_configure_endpoint(int endpoint);

//---
// Pipe operations
//---

/* usb_pipe_configure(): Configure a pipe when opening the connection */
void usb_pipe_configure(int address, endpoint_t const *ep);

/* usb_pipe_clear(): Clear all data in the pipe */
void usb_pipe_clear(int pipe);

/* usb_pipe_mode_read(): Set a pipe in read mode */
void usb_pipe_mode_read(int pipe, int read_size);

/* usb_pipe_mode_write(): Set a pipe in write mode */
void usb_pipe_mode_write(int pipe, int write_size);

/* usb_write_sync(): Synchronously write to a USB pipe

   This functions writes (size) bytes of (data) into the specified pipe, by
   units of (unit_size) bytes. The unit size must be 1, 2 or 4, and both (data)
   and (size) must be multiples of the unit size. In general, you should try to
   use the largest possible unit size, as it will be much faster. In a sequence
   of writes that concludes with a commit, all the writes must use the same
   unit size.

   If the data fits into the pipe, this function returns right away, and the
   data is *not* transmitted. Otherwise, data is written until the pipe is
   full, at which point it is automatically transmitted. After the transfer,
   this function resumes writing, returning only once everything is written.
   Even then the last bytes will still not have been transmitted, to allow for
   other writes to follow. After the last write in a sequence, use
   usb_commit_sync() or usb_commit_async() to transmit the last bytes.

   If (use_dma=true), the write is performed wita the DMA instead of the CPU,
   which is generally faster.

   If the pipe is busy due to a previous asynchronous write, this function
   waits for the previous write to finish before proceeding normally.

   @pipe       Pipe to write into
   @data       Source data (unit_size-aligned)
   @size       Size of source (multiple of unit_size)
   @unit_size  FIFO access size (must be 1, 2, or 4)
   @dma        Whether to use the DMA to perform the write
   -> Returns an error code (0 on success). */
int usb_write_sync(int pipe, void const *data, int size, int unit_size,
   bool use_dma);

/* usb_write_async(): Asynchronously write to a USB pipe

   This function is similar to usb_write_sync(), but it only starts the writing
   and returns immediately without ever waiting. The writing then occurs in the
   background of the calling code, and the caller is notified through a
   callback when it completes. Use GINT_CALL() to create a callback or pass
   GINT_CALL_NULL.

   If the pipe is busy due to a previous asynchronous write, this function
   returns USB_PIPE_BUSY. When called with (use_dma=true), it returns as soon
   as the DMA starts, without even a guarantee that the first few bytes have
   been written.

   There is no guarantee that the write is complete until the callback is
   called, however calling again with data=NULL and size=0 can be used to
   determine whether the write has finished, since it will return 0 if the pipe
   is idle and USB_PIPE_BUSY otherwise.

   @pipe       Pipe to write into
   @data       Source data (unit_size-aligned)
   @size       Size of source (multiple of unit_size)
   @unit_size  FIFO access size (must be 1, 2, or 4)
   @dma        Whether to use the DMA to perform the write
   @callback   Optional callback to invoke when the write completes
   -> Returns an error code (0 on success). */
int usb_write_async(int pipe, void const *data, int size, int unit_size,
	bool use_dma, gint_call_t callback);

/* usb_commit_sync(): Synchronously commit a write

   This function waits for any pending write on the pipe to finish, then
   transfers whatever data is left, and returns when the transfer completes.

   @pipe  Pipe that has been used in previous usb_write_*() calls */
void usb_commit_sync(int pipe);

/* usb_commit_async(): Asynchronously commit a write

   This function commits the specified pipe, causing the pipe to transfer
   written data as soon as all the writes complete. It returns immediately and
   instead the specified callback is invoked when the transfer completes. */
void usb_commit_async(int pipe, gint_call_t callback);

/* usb_pipe_write_bemp(): Callback for the BEMP interrupt on a pipe */
void usb_pipe_write_bemp(int pipe);

/* usb_pipe_init_transfers(): Initialize transfer information */
void usb_pipe_init_transfers(void);

//---
// Timout waits
//---

/* usb_while(): A while loop with a timeout */
#define usb_while(condition) ({ \
	volatile int __f = 0; \
	int __t = timer_configure(TIMER_ANY, 100000 /*µs*/, \
		GINT_CALL_SET_STOP(&__f)); \
	if(__t >= 0) timer_start(__t); \
	while((condition) && __f == 0) {} \
	if(__f) usb_log("%s: %d: (" #condition ") holds\n", \
		__FUNCTION__, __LINE__); \
	if(__t >= 0) timer_stop(__t); \
	__f != 0; \
})

//---
// SETUP requests
//---

/* Standard SETUP requests */
enum {
	GET_STATUS         =  0,
	CLEAR_FEATURE      =  1,
	SET_FEATURE        =  3,
	SET_ADDRESS        =  5,
	GET_DESCRIPTOR     =  6,
	SET_DESCRIPTOR     =  7,
	GET_CONFIGURATION  =  8,
	SET_CONFIGURATION  =  9,
	GET_INTERFACE      = 10,
	SET_INTERFACE      = 11,
	SYNCH_FRAME        = 12,
};

/* usb_req_setup(): Answer a SETUP request from the userspace handler

   THis function handles a SETUP request from the host, detected with the VALID
   bit in the INTSTS0 register. The inputs are the USBREQ, USBVAL, USBINDX and
   USBLENG registers, along with the DCP FIFO. */
void usb_req_setup(void);

#endif /* GINT_USB_USB_PRIVATE */
