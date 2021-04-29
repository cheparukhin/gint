//---
// gint:usb - USB communication
//---

#ifndef GINT_USB
#define GINT_USB

#include <gint/defs/types.h>
#include <gint/std/endian.h>
#include <gint/gint.h>
#include <stdarg.h>

//---
// Interfaces to communicate with USB transfers
//
// These interfaces define how the calculator behaves on the USB connection,
// and can include stuff like:
//
// -> Communicate with a custom protocol and a custom program on the PC
//    (like Protocol 7 does with FA-124, or fxlink)
// -> Exchange text as a Communications and CDC Control (class 0x03) device
//    (like an Internet router)
// -> Share a video stream as a video input (class 0x0e) device (like a webcam)
//
// Normal add-ins that just want to use the USB connection don't need to worry
// about programming the interfaces; they can simply use interfaces that are
// already implemented. Start with usb_open().
//---

/* usb_interface_t: A USB interface that can be enabled in usb_open()

   This driver provides a device that only has one configuration (due to how
   rare it is for devices to have several configurations). However, a number of
   interfaces can be activated independently. This structure describes an
   interface with regards to this driver.

   The driver chooses endpoint numbers and slices of the FIFO buffer for the
   interface to use, therefore the supplied descriptors cannot specify them.
   Instead, the supplied descriptors should use arbitrary endpoint numbers; the
   driver will use them to communicate with the interface, and transparently
   use concrete endpoint numbers internally. */
typedef struct {
	/* NULL-terminated array of descriptors for the interface */
	void const **dc;
	/* Array of endpoint parameters, see below */
	struct usb_interface_endpoint *params;

	/* Answer class-specific SETUP requests */
	/* TODO */

	/* Receive data from an endpoint */
	/* TODO */

} usb_interface_t;

/* usb_interface_endpoint_t: Parameters for an interface endpoint

   This structure mainly specifies the settings for the pipe associated to the
   endpoint. There 10 pipes, but not all can be used with any transfer type,
   and not all have access to the same amount of memory. */
typedef struct usb_interface_endpoint {
	/* Endpoint number as specified in the interface's descriptors
	   (including the IN/OUT bit) */
	uint8_t endpoint;
	/* Requested buffer size, should be a multiple of 64 and not more than
	   2048. Valid only for bulk and isochronous endpoints. */
	uint16_t buffer_size;

} usb_interface_endpoint_t;

//---
// General functions
//---

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

	/* This pipe is busy (returned by usb_write_async()) */
	USB_WRITE_BUSY,

	/* This pipe is busy (returned by usb_commit_async()) */
	USB_COMMIT_BUSY,
};

/* usb_open(): Open the USB link

   This function opens the USB link and notifies the host that the device is
   ready to connect. Usually the host immediately queries the device, and after
   some exchanges the device can be used. The USB link might not be ready when
   this function returns, use the callback or usb_open_wait() for that.

   The first parameters is a NULL-terminated array of interfaces to open. To
   see available interfaces, please see header files in <gint/usb-*.h>. Each
   interface can be used independently, however if there are not enough USB
   resources (buffer memory, pipes or endpoints) for all of them, usb_open()
   will return an error.

   The second parameter is a callback to be (asynchronously) invoked when the
   USB link is ready. Use GINT_CALL() to create one, or pass GINT_CALL_NULL for
   no callback. You can also use usb_open_wait() to synchronously wait for the
   link to be ready.

   @interfaces  NULL-terminate list of interfaces to open
   @callback    Optional function to be called when the USB link opens */
int usb_open(usb_interface_t const **interfaces, gint_call_t callback);

/* usb_open_wait(): Wait until the USB link is ready
   When called after usb_open(), this function waits until the communication is
   established. You should only call this if usb_open() returns 0. */
void usb_open_wait(void);

/* usb_close(): Close the USB link

   This function closes the link opened by usb_open(), and notifies the host of
   the disconnection (if any was established). The USB link can be reopened
   later to perform more tasks.

   There are two reasons to close the USB link: to save battery power and to
   return to the calculator's main menu. You should thus close it if (1) the
   USB link might not be used for a while, or (2) you want to return to the
   main menu before using it again. */
void usb_close(void);

//---
// Pipe writing API
//---

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

   *WARNING*: Due to a current limitation in the DMA API, the same DMA channel
   is used for all DMA-based writes to USB pipes. Do not write to two USB pipes
   with DMA at the same time!

   If the pipe is busy due to an ongoing asynchronous write or commit, this
   function waits for the operation to complete and proceeds normally.

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
   transfers whatever data is left, and returns when the transfer completes. */
void usb_commit_sync(int pipe);

/* usb_commit_async(): Asynchronously commit a write

   This function commits the specified pipe, causing the pipe to transfer
   written data in the pipe.

   If the pipe is currently busy due to an ongoing write or commit, it returns
   USB_COMMIT_BUSY. You should call usb_commit_async() when the pipe is ready,
   which is either when the previous synchronous call returns, or when the
   callback of the previous asynchronous call is invoked.

   This function returns immediately and invokes (callback) when the transfer
   of the remaining data completes. */
int usb_commit_async(int pipe, gint_call_t callback);

//---
// USB debugging log
//---

/* usb_set_log(): Set the logging function for the USB driver

   The USB driver can produce logs, which are mostly useful to troubleshoot
   problems and add new protocols. The logging is disabled by default but can
   be enabled by specifying this function.

   It is up to you whether to store that in a buffer, rotate logs, send them to
   storage memory or a console on the PC. */
void usb_set_log(void (*logger)(char const *format, va_list args));

/* usb_log(): Send a message to the USB log */
void usb_log(char const *format, ...);

//---
// Standard descriptors
//---

/* Descriptor types */
enum {
	USB_DC_DEVICE                     = 1,
	USB_DC_CONFIGURATION              = 2,
	USB_DC_STRING                     = 3,
	USB_DC_INTERFACE                  = 4,
	USB_DC_ENDPOINT                   = 5,
	USB_DC_DEVICE_QUALIFIER           = 6,
	USB_DC_OTHER_SPEED_CONFIGURATION  = 7,
	USB_DC_INTERFACE_POWER            = 8,
};

/* Standard DEVICE descriptor */
typedef struct {
	uint8_t bLength; /* = 18 */
	uint8_t bDescriptorType; /* = USB_DC_DEVICE */
	uint16_t bcdUSB;

	uint8_t bDeviceClass;
	uint8_t bDeviceSubClass;
	uint8_t bDeviceProtocol;
	uint8_t bMaxPacketSize0;

	uint16_t idVendor;
	uint16_t idProduct;

	uint16_t bcdDevice;
	uint8_t iManufacturer;
	uint8_t iProduct;

	uint8_t iSerialNumber;
	uint8_t bNumConfigurations;

} GPACKED(2) usb_dc_device_t;

/* Standard CONFIGURATION descriptor */
typedef struct {
	uint8_t bLength; /* = 9 */
	uint8_t bDescriptorType; /* = USB_DC_CONFIG */
	uint16_t wTotalLength;

	uint8_t bNumInterfaces;
	uint8_t bConfigurationValue;
	uint8_t iConfiguration;

	uint8_t bmAttributes;
	uint8_t bMaxPower;

} GPACKED(1) usb_dc_configuration_t;

/* Standard INTERFACE descriptor */
typedef struct {
	uint8_t bLength; /* = 9 */
	uint8_t bDescriptorType; /* = USB_DC_INTERFACE */
	uint8_t bInterfaceNumber;
	uint8_t bAlternateSetting;

	uint8_t bNumEndpoints;
	uint8_t bInterfaceClass;
	uint8_t bInterfaceSubClass;
	uint8_t bInterfaceProtocol;

	uint8_t iInterface;

} GPACKED(1) usb_dc_interface_t;

/* Standard ENDPOINT descriptor */
typedef struct
{
	uint8_t bLength; /* = 7 */
	uint8_t bDescriptorType; /* = USB_DC_ENDPOINT */
	uint8_t bEndpointAddress;
	uint8_t bmAttributes;

	uint16_t wMaxPacketSize;
	uint8_t bInterval;

} GPACKED(1) usb_dc_endpoint_t;

/* Standard STRING descriptor */
typedef struct {
	uint8_t bLength;
	uint8_t bDescriptorType; /* = USB_DC_STRING */
	uint16_t data[];

} GPACKED(2) usb_dc_string_t;

/* usb_dc_string(): Create a STRING descriptor and return its ID

   This function registers the provided string in an array of STRING
   descriptors used to answer GET_DESCRIPTOR requests, and returns the string's
   ID. USB 2.0 only has provision for 256 strings in the device, so this
   function will return 0 when out of space.

   The string should be encoded as UTF-16 (big-endian), which can be achieved
   with a "u" prefix on the string literal, for instance: u"Hello,World!". If
   (len) is specified, it should be the number of UTF-16 code points to count
   in the string. If it is 0, it defaults to the length of the string. */
uint16_t usb_dc_string(uint16_t const *literal, size_t len);

/* usb_dc_string_get(): Get the descriptor for a STRING id
   This is mostly used by the driver to answer GET_DESCRIPTOR requests. */
usb_dc_string_t *usb_dc_string_get(uint16_t id);

#endif /* GINT_USB */
