#ifdef FX9860G

#include <gint/usb.h>
#include <gint/usb-ff-bulk.h>
#include <gint/display.h>
#include <gint/gray.h>

void usb_fxlink_screenshot_gray(GUNUSED bool onscreen)
{
	uint32_t *light, *dark;
	if(onscreen) dgray_getscreen(&light, &dark);
	else dgray_getvram(&light, &dark);

	usb_fxlink_header_t header;
	usb_fxlink_image_t subheader;

	usb_fxlink_fill_header(&header, "fxlink", "image",
		2048 + sizeof subheader);

	subheader.width = htole32(DWIDTH);
	subheader.height = htole32(DHEIGHT);
	subheader.pixel_format = htole32(USB_FXLINK_IMAGE_GRAY);

	int pipe = usb_ff_bulk_output();
	usb_write_sync(pipe, &header, sizeof header, 4, false);
	usb_write_sync(pipe, &subheader, sizeof subheader, 4, false);
	usb_write_sync(pipe, light, 1024, 4, false);
	usb_write_sync(pipe, dark, 1024, 4, false);
	usb_commit_sync(pipe);
}

#endif /* FX9860G */
