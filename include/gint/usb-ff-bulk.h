//---
// gint:usb-ff-bulk - A trivial bulk-based transfer class
//---

#ifndef GINT_USB_FF_BULK
#define GINT_USB_FF_BULK

#ifdef __cplusplus
extern "C" {
#endif

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
// Direct bulk access
//
// The following functions can be used to access the bulk pipes directly. They
// provide the pipe numbers, which can be passed for instance to usb_write
// functions.
//---

/* usb_ff_bulk_output(): Pipe for calculator -> host communication */
int usb_ff_bulk_output(void);

//---
// fxlink protocol
//
// fxlink is a bulk-based communication tool in the fxSDK that can be used to
// conveniently transfer or manipulate information during the execution of an
// add-in. For instance, screenshots can be saved, files can be transferred,
// and text can be logged.
//
// Each communication with fxlink is a message that consists of:
// * A first write with a message header;
// * One or more writes with message data (of a size announced in the header);
// * A commit on the USB pipe (to ensure that everything is sent).
//
// There are two ways to use the fxlink protocol in this header; you can either
// use one of the convenience functions like usb_fxlink_screenshot() that do
// all the steps for you, or you can perform the writes and commits yourself to
// send custom messages.
//---

/* usb_fxlink_header_t: Message header for fxlink

   fxlink supports a minimalist protocol to receive data sent from the
   calculator and automatically process it (such as save it to file, convert
   to an image, etc). It is designed as a convenience feature, and it can be
   extended with custom types rather easily.

   A message is categorized with an (application, type) pair; both are UTF-8
   strings of up to 16 characters. The application name "fxlink" is reserved
   for built-in types supported by fxlink; any other custom application name
   can be set (in which case fxlink will call a user-provided program to handle
   the message).

   The size of the data to be transferred must be specified in order of the
   transfer to proceed correctly, as it cannot reliably be guessed on the other
   side (and guessing wrong means all further messages will be in trouble).

   As with the rest of the USB protocol, all the multi-byte integer fields in
   this header are encoded as *little-endian*. */
typedef struct
{
	/* Protocol version = 0x00000100 */
	uint32_t version;
	/* Size of the data to transfer (excluding this header) */
	uint32_t size;
	/* Size of individual transfers (related to the size of the FIFO) */
	uint32_t transfer_size;
	/* Application name, UTF-8 (might not be zero-terminated) */
	char application[16];
	/* Message type */
	char type[16];

} usb_fxlink_header_t;

/* usb_fxlink_fill_header(): Fill an fxlink message header

   This function will fill the specified fxlink header. You need to specify the
   exact amount of data that the fxlink message will contain; if you cannot
   determine it, consider splitting the message into several messages each with
   their own header, and use the application-specific script on the host to
   recombine them.

   Returns false if the parameters are invalid or don't fit, in this case the
   contents of the header are unchanged. */
bool usb_fxlink_fill_header(usb_fxlink_header_t *header,
	char const *application, char const *type, uint32_t data_size);

//---
// Short functions for fxlink built-in types
//---

/* Subheader for the fxlink built-in "image" type */
typedef struct
{
	uint32_t width;
	uint32_t height;
	/* Pixel format, see below */
	int pixel_format;

} usb_fxlink_image_t;

/* Pixel formats */
typedef enum
{
	/* Image is an array of *big-endian* uint16_t with RGB565 format */
	USB_FXLINK_IMAGE_RGB565 = 0,
	/* Image is an array of bits in mono format */
	USB_FXLINK_IMAGE_MONO,
	/* Image is two consecutive mono arrays, one for light, one for dark */
	USB_FXLINK_IMAGE_GRAY,

} usb_fxlink_image_format_t;

/* usb_fxlink_screenshot(): Take a screenshot

   This function takes a screenshot. If (onscreen = false), it sends the
   contents of the VRAM. This mode is best used just before dupdate(), since
   the VRAM contents are exactly as they will appear on screen a moment later.
   However, this is somewhat unintuitive for interactive GUI as the user has to
   press a button after the option is present on-screen, therefore the contents
   of the *next* frame are captured.

   If (onscreen = true), this function tries to send the pixels that are
   currently visible on-screen, with the following heuristic:

   * If there is only one VRAM, the VRAM contents are used in the hope they
     haven't changed since the last frame was presented with dupdate().
   * If there are two VRAMs (on fx-CG 50 or using the gray engine) the contents
     of the VRAM not currently being draw to are sent.

   This function does not read pixels directly from the display, as this is
   usually slow and currently not even implemented. */
void usb_fxlink_screenshot(bool onscreen);

#ifdef FX9860G
/* usb_fxlink_screenshot_gray(): Take a gray screenshot on fx-9860G

   This function is similar to usb_fxlink_screenshot(), but it takes a gray
   screenshot. It depends on the gray engine so if you use your add-in will
   automatically have the gray engine, that's why it's separate. */
void usb_fxlink_screenshot_gray(bool onscreen);
#endif

/* usb_fxlink_text(): Send raw text

   This function sends a string with the "text" type, which fxlink prints on
   the terminal. It will send a full fxlink message (with a commit), which is
   inefficient if there is a lot of text to send. For better speed, send the
   message manually by filling the header and doing the writes and commit
   manually.

   This function sends the text by blocks of 4 bytes or 2 bytes when alignment
   and size allow, and 1 byte otherwise. If size is 0, strlen(text) is used. */
void usb_fxlink_text(char const *text, int size);

/* usb_fxlink_videocapture(): Send a frame for a video recording

   This function is essentially the same as usb_fxlink_screenshot(). It sends a
   capture of the VRAM to fxlink but uses the "video" type, which fxlink
   displays in real-time or saves as a video file. The meaning of the onscreen
   setting is identical to usb_fxlink_screenshot().

   This function can be called with onscreen=false as a dupdate() hook to
   automatically send new frames to fxlink. */
void usb_fxlink_videocapture(bool onscreen);

#ifdef FX9860G
/* usb_fxlink_videocapture_gray(): Send a gray frame for a video recording
   Like usb_fxlink_videocapture(), but uses VRAM data from the gray engine. */
void usb_fxlink_videocapture_gray(bool onscreen);
#endif

#ifdef __cplusplus
}
#endif

#endif /* GINT_USB_FF_BULK */
