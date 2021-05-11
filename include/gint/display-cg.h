//---
//	gint:display-cg - fxcg50 rendering functions
//
//	This module covers all 16-bit opaque rendering functions. For
//	gamma-related functions, color composition, check out a color library.
//
//	All the functions in this module work on a 396x224 resolution - gint
//	lets you use the full surface!
//---

#ifndef GINT_DISPLAY_CG
#define GINT_DISPLAY_CG

#ifdef FXCG50

#include <gint/defs/types.h>

/* Dimensions of the VRAM */
#define DWIDTH 396
#define DHEIGHT 224

/* gint VRAM address. This value must always point to a 32-aligned buffer of
   size 177408. Any function can use it freely to perform rendering or store
   data when not drawing. Triple buffering is already implemented in gint, see
   the dvram() function below.

   In this module, colors are in the 16-bit big-endian R5G6B5 format, as it is
   the format used by the display controller. */
extern uint16_t *gint_vram;

/* Provide a platform-agnostic definition of color_t.
   Some functions also support transparency, in which case they take an [int]
   as argument and recognize negative values as transparent. */
typedef uint16_t color_t;

enum {
	/* Compatibility with fx9860g color names */
	C_WHITE = 0xffff,
	C_LIGHT = 0xaaaa,
	C_DARK  = 0x5555,
	C_BLACK = 0x0000,

	/* Other colors */
	C_RED   = 0xf800,
	C_GREEN = 0x07e0,
	C_BLUE  = 0x001f,

	C_NONE = -1,
   C_INVERT = -2,
};

/* RGB color maker. Takes three channels of value 0..31 each (the extra bit of
   green is not used). */
#define C_RGB(r,g,b) (((r) << 11) | ((g) << 6) | (b))

//---
//	Image rendering (bopti)
//---

/* bopti_image_t: Image files encoded for bopti
   This format is created by the fxSDK's [fxconv] tool from standard images. */
typedef struct
{
	/* Color profile (type of palette), could be extended into a bit field
	   later on */
	uint16_t profile;

	/* Color code assigned to transparent pixels (unused in 16-bit) */
	uint16_t alpha;

	/* Full width and height, in pixels */
	uint16_t width;
	uint16_t height;

	/* Color palette:
	   * 16-bit and 16-bit alpha: none
	   * 8-bit: 256 colors (total 512 bytes)
	   * 4-bit: 16 colors (total 32 bytes)
	   Then raw pixel data in row-major order. */
	uint16_t data[];

} GPACKED(4) bopti_image_t;

/* Old alias to image_t, now deprecated because of libimg */
typedef bopti_image_t image_t __attribute__((deprecated(
	"image_t has been renamed to bopti_image_t")));

//---
//	Video RAM management
//---

/* dsetvram() - Control video RAM address and triple buffering

   Normal rendering under gint uses double-buffering: there is one image
   displayed on the screen and one in memory, in a region called the video RAM
   (VRAM). The application draws frames in the VRAM then sends them to the
   screen only when they are finished, using dupdate().

   On fxcg50, the performance bottleneck is almost always the graphical
   rendering (especially in games) because the high amount of data, 173 kB per
   frame in full-resolution, makes graphics manipulation computationally
   expensive. The transfer also takes about 10 ms in itself.

   Since gint transfers data to the screen using the DMA, it is possible to run
   the application while the finished frame is being transferred. However,
   writing to the VRAM during this period will cause display artifacts since
   the VRAM it is still being read by the DMA.

   The solution to this is to use triple-buffering with the display and two
   VRAMs that are alternately being written to while the other is being
   transferred. The VRAM switching is handled by dupdate() and is activated
   whenever two VRAMs are configured.

   By default gint uses triple buffering with two VRAMs in the system stack.

   VRAMs must be contiguous, 32-aligned, (2*396*224)-byte buffers.

   @main       Main VRAM area, used alone if [secondary] is NULL
   @secondary  Additional VRAM area, enables triple buffering if non-NULL */
void dsetvram(uint16_t *main, uint16_t *secondary);

/* dgetvram() - Get VRAM addresses
   Returns the VRAM buffer addresses used to render on fx-CG 50. */
void dgetvram(uint16_t **main, uint16_t **secondary);

#endif /* FXCG50 */

#endif /* GINT_DISPLAY_CG */
