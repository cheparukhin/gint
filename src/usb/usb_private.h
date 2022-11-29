//---
// gint:usb:usb-private - Private definitions for the USB driver
//---

#ifndef GINT_USB_USB_PRIVATE
#define GINT_USB_USB_PRIVATE

#include <gint/defs/attributes.h>
#include <gint/timer.h>
#include <gint/gint.h>
#include <gint/usb.h>

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
//
// When writing to a pipe, the general workflow is as follows:
//
// 1. The user performs a write of a block of memory of any size. Because the
//    FIFO for the pipe only has a limited size, the driver splits the write
//    into "rounds" of the size of the FIFO.
//
//    The rounds are written to the FIFO. If the FIFO is full, the write
//    continues until the FIFO can be accessed again (often after the contents
//    of the FIFO have been transmitted, except in double-buffer mode).
//
//    If the last round is smaller than the size of the FIFO, the data is not
//    transmitted; this allows the user to perform another write immediately.
//
// 2. The user performs more writes, each of which are split into rounds, with
//    each round possibly triggering a transfer (if the FIFO is full). Each
//    write only finishes after all the data is written and the pipe is
//    available for more writing.
//
// 3. After the last write, the user *commits* the pipe, causing any data
//    remaining in the FIFO to be transferred even if the FIFO is not full. The
//    commit operation finishes when the pipe is writable again.
//---

/* usb_pipe_configure(): Configure a pipe when opening the connection */
void usb_pipe_configure(int address, endpoint_t const *ep);

/* usb_pipe_clear(): Clear all data in the pipe */
void usb_pipe_clear(int pipe);

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
	if(__f) USB_LOG("%s: %d: (" #condition ") holds\n", \
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
