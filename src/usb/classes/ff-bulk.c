#include <gint/usb.h>

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

usb_interface_t usb_ff_bulk = {
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
