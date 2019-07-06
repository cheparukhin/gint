//---
//	gint:display-fx - fx9860g drawing functions
//
//	This module is in charge of all monochrome rendering. The gray engine
//	has its own functions, but often relies on this module (because the
//	gray effect is created through two monochrome buffers).
//---

#ifndef GINT_DISPLAY_FX
#define GINT_DISPLAY_FX

#ifdef FX9860G

#include <gint/defs/types.h>

/* Expose the VRAM variable if GINT_NEED_VRAM is defined. It must always point
   to a 4-aligned buffer of size 1024. Any function can use it freely to:
   - Use another video ram area (triple buffering or more, gray engine);
   - Implement additional drawing functions;
   - Store data when not drawing. */
#ifdef GINT_NEED_VRAM
extern uint32_t *vram;
#endif

/* color_t - colors available for drawing
   The following colors are defined by the library:

     OPAQUE COLORS (override existing pixels)
       white, black	- the usual thing
       light, dark	- intermediate colors used with the gray engine

     OPERATORS (combine with existing pixels)
       none		- leaves unchanged
       invert		- inverts white <-> black, light <-> dark
       lighten		- shifts black -> dark -> light -> white -> white
       darken		- shifts white -> light -> dark -> black -> black

   Not all colors can be used with all functions. To avoid ambiguities, all
   functions explicitly indicate compatible colors. */
typedef enum
{
	/* Opaque colors */
	C_WHITE    = 0,
	C_LIGHT    = 1,
	C_DARK     = 2,
	C_BLACK    = 3,

	/* Monochrome operators */
	C_NONE     = 4,
	C_INVERT   = 5,

	/* Gray operators */
	C_LIGHTEN  = 6,
	C_DARKEN   = 7,

} color_t;

//---
//	Image rendering (bopti)
//---

/* image_t - image files encoded for bopti
   This format is the result of encoding images for bopti with the fxSDK's
   [fxconv] tool. The bopti routines can render it extremely fast, which makes
   it preferable over plain bitmaps if the images are never edited. */
typedef struct
{
	/* Image can only be rendered with the gray engine */
	uint gray	:1;
	/* Left for future use */
	uint		:3;
	/* Image profile (uniquely identifies a rendering function) */
	uint profile	:4;
	/* Full width, in pixels */
	uint width	:12;
	/* Full height, in pixels */
	uint height	:12;

	/* Raw layer data */
	uint8_t data[];

} GPACKED(4) image_t;

/* dimage() - render a full image
   This function blits an image on the VRAM using gint's special format. It is
   a special case of dsubimage() where the full image is drawn with clipping.

   @x @y   Coordinates of the top-left corner of the image
   @image  Pointer to image encoded with [fxconv] */
void dimage(int x, int y, image_t const *image);

/* Option values for dsubimage() */
enum {
	/* Disable clipping, ie. adjustments to the specified subrectangle and
	   screen location such that any part that overflows from the image or
	   the screen is ignored. Slightly faster. */
	DIMAGE_NOCLIP = 0x01,
};

/* dsubimage() - render a section of an image
   This function blits a subrectangle [left, top, width, height] of an image on
   the VRAM. It is more general than dimage() and also provides a few options.

   @x @y           Coordinates on screen of the rendered subrectangle
   @image          Pointer to image encoded with [fxconv]
   @left @top      Top-left coordinates of the subrectangle within [image]
   @width @height  Subrectangle dimensions */
void dsubimage(int x, int y, image_t const *image, int left, int top,
	int width, int height, int flags);

#endif /* FX9860G */

#endif /* GINT_DISPLAY_FX */
