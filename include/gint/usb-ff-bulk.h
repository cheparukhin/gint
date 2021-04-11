//---
// gint:usb-ff-bulk - A trivial bulk-based transfer class
//---

#ifndef GINT_USB_FF_BULK
#define GINT_USB_FF_BULK

#include <gint/usb.h>

/* The bulk transfer interface with class code 0xff provides a very simple
   communication channel between the calculator and a host. There is
   (currently) a single IN pipe that sends data from the calculator to the
   host, at high-speed (USB 2.0).

   The class code of this interface is 0xff, which means that the protocol is
   custom and requires a custom program on the host to receive the data (unlike
   for instance LINK on a Graph 90+E which behaves as a USB stick and can be
   used with the file browser). fxlink can be used to the effect. */

extern usb_interface_t const usb_ff_bulk;

//---
// fxlink protocol
//---

/* usb_fxlink_header_t: Packet header for fxlink

   fxlink supports a minimalistic protocol to receive data sent from the
   calculator and automatically process it (such as save it to file, convert
   to an image, etc). It is designed as a convenience feature, and it can be
   extended with custom types rather easily.

   A packet is categorized with an (application, type) pair; both are UTF-8
   strings of up to 16 characters. The application name "fxlink" is reserved
   for built-in types supported by fxlink; any other custom application name
   can be set (in which case fxlink will call a user-provided program to handle
   the packet).

   The size of the data to be transferred must be specified in order of the
   transfer to proceed correctly, as it cannot reliably be guessed on the other
   side (and guessing wrong means all further packets will be in trouble).

   Most of the time you don't need to create custom packets yourself since the
   convenience functions below will create them for you. */
typedef struct
{
	/* Protocol version = 0x00000100 */
	uint32_t version;
	/* Size of the data to transfer, including this header */
	uint32_t size;
	/* Size of individual transfers (related to the size of the FIFO) */
	uint32_t transfer_size;
	/* Application name, UTF-8 (might not be zero-terminated) */
	char application[16];
	/* Packet type */
	char type[16];

} usb_fxlink_header_t;

#endif /* GINT_USB_FF_BULK */
