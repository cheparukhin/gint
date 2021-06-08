#include <gint/usb.h>
#include <gint/usb-ff-bulk.h>
#include <gint/display.h>

#include <string.h>

static usb_dc_interface_t dc_interface = {
	.bLength              = sizeof(usb_dc_interface_t),
	.bDescriptorType      = USB_DC_INTERFACE,
	.bInterfaceNumber     = -1 /* Set by driver */,
	.bAlternateSetting    = 0,
	.bNumEndpoints        = 1,
	.bInterfaceClass      = 0xff, /* Vendor-Specific */
	.bInterfaceSubClass   = 0x77, /* (not recognized by Casio tools?) */
	.bInterfaceProtocol   = 0x00,
	.iInterface           = 0,
};

/* Endpoint for calculator -> PC communication */
static usb_dc_endpoint_t dc_endpoint1i = {
	.bLength             = sizeof(usb_dc_endpoint_t),
	.bDescriptorType     = USB_DC_ENDPOINT,
	.bEndpointAddress    = 0x81, /* 1 IN */
	.bmAttributes        = 0x02, /* Bulk transfer */
	/* TODO: Additional transactions per µframe ?! */
	.wMaxPacketSize      = htole16(512),
	.bInterval           = 1,
};

usb_interface_t const usb_ff_bulk = {
	/* List of descriptors */
	.dc = (void const *[]){
		&dc_interface,
		&dc_endpoint1i,
		NULL,
	},
	/* Parameters for each endpoint */
	.params = (usb_interface_endpoint_t []){
		{ .endpoint     = 0x81, /* 1 IN */
		  .buffer_size  = 2048, },
		{ 0 },
	},
};

GCONSTRUCTOR static void set_strings(void)
{
	dc_interface.iInterface = usb_dc_string(u"Bulk Input", 0);
}

//---
// Direct bulk access
//---

int usb_ff_bulk_output(void)
{
	return usb_interface_pipe(&usb_ff_bulk, 0x81);
}

//---
// fxlink protocol
//---

bool usb_fxlink_fill_header(usb_fxlink_header_t *header, char *application,
   char *type, uint32_t data_size)
{
	if(strlen(application) > 16 || strlen(type) > 16) return false;

	memset(header, 0, sizeof *header);
	header->version = htole32(0x00000100);
	header->size = htole32(data_size);
	/* TODO: usb_fxlink_header: avoid sync with interace definition */
	header->transfer_size = htole32(2048);

	strncpy(header->application, application, 16);
	strncpy(header->type, type, 16);

	return true;
}

void usb_fxlink_screenshot(GUNUSED bool onscreen)
{
	void *source = gint_vram;
	int size, format;

	#ifdef FX9860G
	size = 1024;
	format = USB_FXLINK_IMAGE_MONO;
	#endif

	#ifdef FXCG50
	if(onscreen) {
		uint16_t *main, *secondary;
		dgetvram(&main, &secondary);
		source = (gint_vram == main) ? secondary : main;
	}
	size = DWIDTH * DHEIGHT * 2;
	format = USB_FXLINK_IMAGE_RGB565;
	#endif

	usb_fxlink_header_t header;
	usb_fxlink_image_t subheader;

	usb_fxlink_fill_header(&header, "fxlink", "image",
		size + sizeof subheader);

	subheader.width = htole32(DWIDTH);
	subheader.height = htole32(DHEIGHT);
	subheader.pixel_format = htole32(format);

	int pipe = usb_ff_bulk_output();
	usb_write_sync(pipe, &header, sizeof header, 4, false);
	usb_write_sync(pipe, &subheader, sizeof subheader, 4, false);
	usb_write_sync(pipe, source, size, 4, false);
	usb_commit_sync(pipe);
}

void usb_fxlink_text(char const *text, int size)
{
	if(size == 0) size = strlen(text);

	int unit_size = 4;
	if((uint32_t)text & 3 || size & 3) unit_size = 2;
	if((uint32_t)text & 1 || size & 1) unit_size = 1;

	usb_fxlink_header_t header;
	usb_fxlink_fill_header(&header, "fxlink", "text", size);

	int pipe = usb_ff_bulk_output();
	usb_write_sync(pipe, &header, sizeof header, unit_size, false);
	usb_write_sync(pipe, text, size, unit_size, false);
	usb_commit_sync(pipe);
}
