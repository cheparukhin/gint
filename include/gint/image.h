//---
// gint:image - Image manipulation and rendering
//
// Note: this module is currently only available on fx-CG.
//
// This header provides image manipulation functions. This mainly consists of a
// reference-based image format, various access and modification functions, and
// a number of high-performance transformations and rendering effects. If you
// find yourself limited by rendering time, note that RAM writing speed is
// often the bottleneck, and image rendering is much faster in Azur (which is
// what the renderer was initially designed for).
//
// We support 3 bit depths: full-color 16-bit (RGB565), indexed 8-bit (P8) and
// indexed 4-bit (P4). All three have an "alpha" variation where one color is
// treated as transparent, leading to 6 total formats.
//
// The image renderers support so-called *dynamic effects*, which are image
// transformations performed on-the-fly while rendering, without generating an
// intermediate image. They comprise straightforward transformations that
// achieve similar performance to straight rendering and can be combined to
// some extent, which makes them reliable whenever applicable.
//
// TODO: Switch to libimg-style image refs.
//---

#ifndef GINT_IMAGE
#define GINT_IMAGE

#ifdef __cplusplus
extern "C" {
#endif

#ifndef FXCG50
#error <gint/image.h> is only supported on FXCG50
#else

#include <gint/defs/attributes.h>
#include <gint/defs/types.h>

//---
// Image structures
//---

/* Image formats. Note that transparency really only indicates the default
   rendering method, as a transparent background can always be added or removed
   by a dynamic effect on any image. */
enum {
	IMAGE_RGB565      = 0,  /* RGB565 without alpha */
	IMAGE_RGB565A     = 1,  /* RGB565 with one transparent color */
	IMAGE_P8_RGB565   = 4,  /* 8-bit palette, all opaque colors */
	IMAGE_P8_RGB565A  = 5,  /* 8-bit with one transparent color */
	IMAGE_P4_RGB565   = 6,  /* 4-bit palette, all opaque colors */
	IMAGE_P4_RGB565A  = 3,  /* 4-bit with one transparent color */

	IMAGE_DEPRECATED_P8 = 2,
};

/* image_t: gint's native bitmap image format
   Images of this format can be created through this header's API but also by
   using the fxSDK's built-in image converters with fxconv. */
typedef struct
{
	/* Color format, one of the IMAGE_* values defined above. */
	uint16_t profile;
	/* For formats with alpha, value or index used for transparency. */
	uint16_t alpha;
	/* Full width and height, in pixels */
	uint16_t width;
	uint16_t height;

	/* Here we lose structure because of the flexible array.

	   RGB565, RGB565A:
	     * Pixels in row-major order, 16 bits per pixel
	   P8:
	     * Palette with 256 entries (512 bytes total)
	     * Pixels in row-major order, 8 bits per pixel
	   P8_RGB565A, P8_RGB565:
	     * Number of entries in palette, N (2 bytes)
	     * Palette with N entries (2N bytes)
	     * Pixels in row-major order, 8 bits per pixel (signed indices in
	       an uint16_t array starting at <palette>+<256 bytes>)
	   P4/P4_RGB565A, P4_RGB565:
	     * Palette with 16 entries (32 bytes total)
	     * Pixels in row-major order, 4 bits per pixel, each row
	       byte-padded */
	uint16_t data[];

} GPACKED(4) image_t;

/* Dynamic effects: these transformations can be applied on images while
   rendering. Not all effects can be combined; unless specified otherwise:
   - HFLIP and VFLIP can both be added regardless of any other effect
   - At most one color effect can be applied */
enum {
	/* Value 0x01 is reserved, because it is DIMAGE_NOCLIP, which although
	   part of the old API still needs to be supported. */

	/* [Any]: Skip clipping the command against the source image */
	IMAGE_NOCLIP_INPUT   = 0x04,
	/* [Any]: Skip clipping the command against the output VRAM */
	IMAGE_NOCLIP_OUTPUT  = 0x08,
	/* [Any]: Skip clipping both */
	IMAGE_NOCLIP         = IMAGE_NOCLIP_INPUT | IMAGE_NOCLIP_OUTPUT,

	// Geometric effects. These values should remain at exactly bit 8 and
	// following, or change gint_image_mkcmd() along with it.

	/* [Any]: Flip image vertically */
	IMAGE_VFLIP          = 0x0100,
	/* [Any]: Flip image horizontally */
	IMAGE_HFLIP          = 0x0200,

	// Color effects

	/* [RGB565, P8_RGB565, P4_RGB565]: Make a color transparent
	   Adds one argument:
	   * Color to clear (RGB16: 16-bit value; P8/P4: palette index) */
	IMAGE_CLEARBG        = 0x10,
	/* [RGB565, P8_RGB565, P4_RGB565]: Turn a color into another
	   Adds two arguments:
	   * Color to replace (RGB16: 16-bit value; P8/P4: palette index)
	   * Replacement color (16-bit value) */
	IMAGE_SWAPCOLOR      = 0x20,
	/* [RGB565A, P8_RGB565A, P4_RGB565A]: Add a background
	    Adds one argument:
	    * Background color (16-bit value) */
	IMAGE_ADDBG          = 0x40,
	/* [RGB565A, P8_RGB565A, P4_RGB565A]: Dye all non-transparent pixels
	   Adds one argument:
	   * Dye color (16-bit value) */
	IMAGE_DYE            = 0x80,
};

//---
// Image access and information
//---

/* TODO: Expand */

int image_get_pixel(image_t const *img, int x, int y);

int image_decode_pixel(image_t const *img, int pixel);

//---
// Image rendering functions
//
// The following functions extend dimage() and dsubimage(). The [effects]
// parameter takes a combination of IMAGE_* flags and effects, limited to the
// combinations previously described, with additional arguments depending on
// the color effect being applied.
//
//   dimage_effect(x, y, img, effects, ...)
//   dsubimage_effect(x, y, img, left, top, w, h, effects, ...)
//
// However if you use these super-generic functions you will link the code for
// all effects and all formats into your add-in, which takes a fair amount of
// space. If that's a problem, you can use the more specific functions below:
//
// * dimage_<FORMAT>_<EFFECT>() for one particular format (rgb16, p8, p4) along
//   with one particular color effect (clearbg, swapcolor, addbg, dye).
// * dimage_<FORMAT>() is like the above when no color effect is applied.
//
// All of them support the HFLIP and VFLIP flags. For effect-specific functions
// the corresponding effect flag can be omitted (fi. IMAGE_CLEARBG is implicit
// when using dimage_p8_clearbg()).
//---

/* dimage_effect(): Generalized dimage() supporting dynamic effects */
#define dimage_effect(x, y, img, eff, ...) \
	dsubimage_effect(x, y, img, 0, 0, (img)->width, (img)->height, eff, \
		##__VA_ARGS__)
/* dsubimage_effect(): Generalized dsubimage() supporting dynamic effects */
void dsubimage_effect(int x, int y, image_t const *img,
	int left, int top, int w, int h, int effects, ...);

/* Specific versions for each format */
#define DIMAGE_SIG1(NAME, ...) \
	void dimage_ ## NAME(int x, int y, image_t const *img,##__VA_ARGS__); \
	void dsubimage_ ## NAME(int x, int y, image_t const *img, \
		int left, int top, int w, int h, ##__VA_ARGS__);
#define DIMAGE_SIG(NAME, ...) \
	DIMAGE_SIG1(rgb16 ## NAME, ##__VA_ARGS__) \
	DIMAGE_SIG1(p8 ## NAME, ##__VA_ARGS__) \
	DIMAGE_SIG1(p4 ## NAME, ##__VA_ARGS__)

/* d[sub]image_{rgb16,p8,p4}_effect(..., effects, <extra arguments>) */
DIMAGE_SIG(_effect, int effects, ...)
/* d[sub]image_{rgb16,p8,p4}(..., effects) (no color effect, like dimage()) */
DIMAGE_SIG(, int effects)
/* d[sub]image_{rgb16,p8,p4}_clearbg(..., effects, bg_color_or_index) */
DIMAGE_SIG(_clearbg, int effects, int bg_color_or_index)
/* d[sub]image_{rgb16,p8,p4}_swapcolor(..., effects, source, replacement) */
DIMAGE_SIG(_swapcolor, int effects, int source, int replacement)
/* d[sub]image_{rgb16,p8,p4}_addbg(..., effects, bg_color) */
DIMAGE_SIG(_addbg, int effects, int bg_color)
/* d[sub]image_{rgb16,p8,p4}_dye(..., effects, dye_color) */
DIMAGE_SIG(_dye, int effects, int dye_color)

#define dimage_rgb16_effect(x, y, img, eff, ...) \
	dsubimage_rgb16_effect(x, y, img, 0, 0, (img)->width, (img)->height, \
		eff, ##__VA_ARGS__)
#define dimage_p8_effect(x, y, img, eff, ...) \
	dsubimage_p8_effect(x, y, img, 0, 0, (img)->width, (img)->height, \
		eff, ##__VA_ARGS__)
#define dimage_p4_effect(x, y, img, eff, ...) \
	dsubimage_p4_effect(x, y, img, 0, 0, (img)->width, (img)->height, \
		eff, ##__VA_ARGS__)

#undef DIMAGE_SIG
#undef DIMAGE_SIG1

//---
// Clipping utilities
//---

/* Double box specifying both a source and target area */
struct gint_image_box
{
	/* Target location of top-left corner */
	int x, y;
	/* Width and height of rendered sub-image */
	int w, h;
	/* Source bounding box (low included, high excluded) */
	int left, top;
};

/* Clip the provided box against the input. If, after clipping, the box no
   longer intersects the output (whose size is specified as out_w/out_h),
   returns false. Otherwise, returns true. */
bool gint_image_clip_input(image_t const *img, struct gint_image_box *box,
    int out_w, int out_h);

/* Clip the provided box against the output. */
void gint_image_clip_output(struct gint_image_box *b, int out_w, int out_h);

//---
// Internal image rendering routines
//
// The following functions (or non-functions) are implemented in assembler and
// make up the internal interface of the image renderer. If you just want to
// display images, use dimage() and variations; these are only useful if you
// have a different rendering system and wish to use image rendering with
// dynamic effects in it.
//---

/* Renderer command. This structure includes most of the information used by
   the image renderer to perform blits. Some of the information on the target
   is also passed as direct arguments, which is more convenient and slightly
   faster.

   Most of the values here can be set with gint_image_mkcmd(). The last two
   members, along with the return values of the gint_image_FORMAT_loop()
   functions, are used to update the command if one needs to draw *parts* of
   the image and resume the rendering later. This is used in Azur. */
struct gint_image_cmd
{
	/* Shader ID. This is used in Azur, and ignored in gint */
	uint8_t shader_id;
	/* Dynamic effects
	    Bit 0:     VFLIP
	    Bit 1:     HFLIP
	    Bits 2-7:  0=NONE, 1=CLEARBG, 2=SWAPCOLOR, 3=DYE */
	uint8_t effect;

	/* Number of pixels to render per line. For formats that force either x
	   or width alignment (most of them), this is already adjusted to a
	   suitable multiple (usually a multiple of 2). */
	int16_t columns;

	/* Stride of the input image (number of pixels between each row), in
	   pixels, without subtracting the number of columns */
	int16_t input_stride;

	/* Number of lines in the command. This can be adjusted freely, and is
	   particularly useful in Azur for fragmented rendering. */
	uint8_t lines;

	/* [Any effect]: Offset of first edge */
	int8_t edge_1;

	/* Core loop; this is an internal label of the renderer */
	void const *loop;
	/* Output pixel array, offset by target x/y */
	void const *output;
	/* Input pixel array, offset by source x/y. For formats that force x
	   alignment, this is already adjusted. */
	void const *input;
	/* Palette, when applicable */
	uint16_t const *palette;

	/* [Any effect]: Offset of right edge */
	int16_t edge_2;
	/* [CLEARBG, SWAPCOLOR]: Source color */
	uint16_t color_1;
	/* [SWAPCOLOR]: Destination color */
	uint16_t color_2;

	/* Remaining height (for updates between fragments) */
	int16_t height;
	/* Local x position (for updates between fragments) */
	int16_t x;
};

/* gint_image_mkcmd(): Prepare a rendering command with dynamic effects

   This function crafts an image renderer command. It loads all the settings
   except for effect-dependent parameters: the [.loop] label, the color section
   of [.effect], and color effect settings. See the effect-specific functions
   to see how they are defined.

   The benefit of this approach is that the rendering code does not need to be
   linked in unless an effect is actually used, which avoids blowing up the
   size of the add-in as the number of support dynamic effects increases.

   @box         Requested on-screen box (will be clipped depending on effects)
   @img         Source image
   @effects     Set of dynamic effects to be applied, as an [IMAGE_*] bitmask
   @left_edge   Whether to force 2-alignment on the input (box->left)
   @right_edge  Whether to force 2-alignment on the width
   @cmd         Command to be filled
   @out_width   Output width (usually DWIDTH)
   @out_height  Output height (usually DHEIGHT)

   Returns false if there is nothing to render because of clipping (in which
   case [cmd] is unchanged), true otherwise. [*box] is also updated to reflect
   the final box after clipping but not accounting for edges.  */
bool gint_image_mkcmd(struct gint_image_box *box, image_t const *img,
	int effects, bool left_edge, bool right_edge,
	struct gint_image_cmd *cmd, int out_width, int out_height);

/* Entry point of the renderers. These functions can be called normally as long
   as you can build the commands (eg. by using gint_image_mkcmd() then filling
   the effect-specific information). */
void *gint_image_rgb16_loop  (int output_width, struct gint_image_cmd *cmd);
void *gint_image_p8_loop     (int output_width, struct gint_image_cmd *cmd);
void *gint_image_p4_loop     (int output_width, struct gint_image_cmd *cmd);

/* Renderer fragments. The following can absolutely not be called from C code
   as they aren't full functions (and this isn't their prototype). These are
   continuations to be specified in the [.loop] field of a command before using
   one of the functions above. */

void gint_image_rgb16_normal(void);
void gint_image_rgb16_clearbg(void);
void gint_image_rgb16_swapcolor(void);
void gint_image_rgb16_dye(void);

void gint_image_p8_normal(void);
void gint_image_p8_clearbg(void);
void gint_image_p8_swapcolor(void);
void gint_image_p8_dye(void);

void gint_image_p4_normal(void);
void gint_image_p4_clearbg(void);
void gint_image_p4_swapcolor(void);
void gint_image_p4_dye(void);

#endif /* FXCG50 */

#ifdef __cplusplus
}
#endif

#endif /* GINT_IMAGE */
