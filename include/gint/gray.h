//---
//	gint:gray - Gray engine and rendering functions
//---

#ifndef GINT_GRAY
#define GINT_GRAY

#include <gint/defs/types.h>
#include <gint/display.h>

//---
//	Engine control
//---

/* gray_start(): Start the gray engine

   The control of the screen is transferred to the engine; you should not use
   dupdate() after this function, only gupdate().

   The engine uses timer number GRAY_TIMER (equal to 0 by default) to run. If
   the timer is unavailable, this function will fail. Otherwise, the gray
   engine is started and any attempt to use timer 0 while it is running will be
   denied by the timer module. */
void gray_start(void);

/* gray_stop(): Stop the gray engine

   Safe to call if the engine was not running. The gray engine returns the
   control of the screen and dupdate() is safe to use again.

   This function will leave the screen in whatever state it is, which is most
   probably one of the gray buffers being displayed. You should dupdate()
   quickly after this call to avoid visual artifacts. If the next monochrome
   frame is slow to render, consider rendering it before stopping the gray
   engine, and calling dupdate() immediately after. */
void gray_stop(void);

/* gray_delays(): Set the gray engine delays

   The gray engine works by swapping two images at a fast pace. Pixels that are
   white on both or black or both will appear as such, but pixels that are
   black on only one of the images will look gray.

   If both images stay on-screen for the same amount on time, there will be
   only one shade of gray. But if one stays longer, then pixels that are only
   black here will look darker than their counterparts, making two shades of
   gray. This is the default.

   Since the gray engine has its default settings, you don't need to set the
   delays before using gray drawing functions. But you can do it to customize
   the appearance of the gray to your application. Be aware that most values
   will just produce visual artifacts and will not look good. There are three
   characteristics that you'll want to control:

   * The stability of the shades; depending on the frequency gray areas can
     appear to blink.
   * Stripes. If the refresh timing coincides with the screen's display
     duration, stripes will appear. They can be of various sizes, visibility
     and speed; but overall they're the *one* thing you want to avoid.
   * And the color of the shades themselves.

   Here are values from an older version of gint, which may not look good now
   but provide bases to search for good settings:

     LIGHT    DARK    BLINKING     STRIPES       COLORS
     --------------------------------------------------
       860    1298        none    terrible       decent
       912    1343       heavy        none         good
       993    1609      medium       light       decent
      1325    1607       heavy       light    excellent
     --------------------------------------------------

  Here are values for this version of gint (for the Graph 35+E II only):

     LIGHT    DARK    BLINKING     STRIPES       COLORS
     --------------------------------------------------
       680    1078
       762    1311         low        some     too light  [default]
       869    1097      medium        some    excellent
       869    1311      medium        none         good
       937    1425      medium        none         good
     --------------------------------------------------

   @light  New light delay
   @dark   New dark delay */
void gray_delays(uint32_t light, uint32_t dark);

/* gray_config(): Get the current configuration of the engine

   Provides the value of the current light and dark delays, measured in timer
   ticks of prescaler P_phi/64. See <gint/clock.h> on how to obtain this value.
   Both pointers may be NULL.

   @light  Set to the current light delay setting
   @dark   Set to the current dark delay setting
   Returns non-zero if the engine is currently running, 0 otherwise. */
int gray_config(uint32_t *light, uint32_t *dark);

//---
//	Area rendering functions
//---

/* gclear(): Fill the screen with a single color
   Clears the VRAM and paints all the pixels in the same color. Optimized for
   opaque colors; use grect() for other colors.

   @color  white, light, dark, black */
void gclear(color_t color);

/* grect(): Fill a rectangle on the screen
   Applies a color or an operator to the rectangle enclosed by (x1 y1) and
   (x2 y2), both included.

   @x1 @x2 @y1 @y2  Bounding rectangle
   @color           white, light, dark, black, none, invert, lighten, darken */
void grect(int x1, int y1, int x2, int y2, color_t color);

//---
//	Point drawing functions
//---

/* gpixel(): Change a pixel's color
   Paints the specified pixel. Use lines or area rendering when possible
   because painting pixels individually is slow.

   @x @y   Coordinates of the pixel to paint
   @color  white, light, dark, black, none, invert, lighten, darken */
void gpixel(int x, int y, color_t color);

/* gline(): Render a straight line
   Draws a line without anti-aliasing, using a Bresenham-style algorithm. Much
   like dline(), has optimizations for vertical and horizontal lines but is not
   able to handle clipping.

   @x1 @y1 @x2 @y2  End points of the line (both included)
   @color           white, light, dark, black, none, invert, lighten, darken */
void gline(int x1, int y1, int x2, int y2, color_t color);

/* ghline(): Full-width horizontal line
   @y      Line number
   @color  white, light, dark, black, none, invert, lighten, darken */
void ghline(int y, color_t color);

/* gvline(): Full-height vertical line
   @x      Column number
   @color  white, light, dark, black, none, invert, lighten, darken */
void gvline(int x, color_t color);

//---
//	Text rendering
//---

/* dfont() and dsize() still work with the gray engine. */

/* gtext(): Display a string of text
   Exactly like dtext(), except that the rendering is done on the two gray
   VRAMs and all colors are supported.

   @x @y   Coordinates of top-left corner of the rendered string
   @str    String to display
   @fg     Foreground: white, light, dark, black, none, invert, lighten, darken
   @bg     Background, same colors as fg */
void gtext(int x, int y, const char *str, int fg, int bg);

//---
//	Image rendering
//---

/* gimage(): Render a full image
   This function is exactly like dimage(), but it draws gray image instead.

   @x @y   Coordinates of the top-left corner of the image
   @image  Pointer to gray image encoded with [fxconv] */
void gimage(int x, int y, image_t const *image);

/* gsubimage(): Render a section of an image
   Like dsubimage() for gray images. Same options apply.

   @x @y           Coordinates on screen of the rendered subrectangle
   @image          Pointer to image encoded with [fxconv]
   @left @top      Top-left coordinates of the subrectangle within [image]
   @width @height  Subrectangle dimensions
   @flags          OR-combination of DIMAGE_* flags */
void gsubimage(int x, int y, image_t const *image, int left, int top,
	int width, int height, int flags);

//---
//	VRAM management
//---

/* gupdate(): Push the current VRAMs to the screen

   This function swaps pairs of VRAM buffers in the gray engine so that the
   gray and light VRAMs that were being drawn to are now available to be
   displayed on the screen at the next refresh, while the VRAMs that were
   previously being displayed are now available for drawing.

   Unlike dupdate(), gupdate() does not interact with the screen as this
   interaction is done very frequently by the engine itself. It performs an
   atomic operation to swap buffers, but nothing on the screen will change
   until the engine's timer actually fires. */
void gupdate(void);


/* gvram(): Get the current VRAM pointers

   This function stores the current VRAM pointers to the specified locations.
   This operation needs to be done atomically because the engine might kick in
   and sway buffers at any time; calling gvraml() then gvramd() is unsafe.

   @light  Set to the current light VRAM pointer
   @dark   Set to the current dark VRAM pointer */
void gvram(uint32_t **light, uint32_t **dark);

/* gvraml(): Shorthand to retrieve the current light VRAM pointer */
uint32_t *gvraml(void);

/* gvramd(): Shorthand to retrieve the current dark VRAM pointer */
uint32_t *gvramd(void);

#endif /* GINT_GRAY */
