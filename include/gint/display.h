//---
//	gint:display - Drawing functions
//---

#ifndef GINT_DISPLAY
#define GINT_DISPLAY

#include <gint/defs/types.h>

/* Expose the VRAM variable if GINT_NEED_VRAM is defined. This address is used
   as the VRAM basis by all of gint's drawing functions, and must point to a
   suitable buffer:
     [fx9860g]  A 4-aligned buffer of size 1024.
     [fxcg50]   A 4-aligned buffer of size 177408.

   This variable is primarily meant to be exposed to gint functions, but
   add-ins may use it or change it freely:
   - To use another video ram area (triple buffering or more, gray engine);
   - To implement additional drawing functions;
   - When not drawing, as additional RAM (especially on fxcg50). */
#ifdef GINT_NEED_VRAM
extern uint32_t *vram;
#endif

/* As you would expect, display on fx9860g and display on fxcg50 are completely
   different worlds. As a consequence, so are the rendering functions ^^ */

#ifdef FX9860G
#include <gint/display-fx.h>
#else
#include <gint/display-cg.h>
#endif

//---
//  Parameter access
//---

/* dinfo_t - summary of information provided by this module */
typedef struct
{
	/* Screen width, in pixels */
	int width;
	/* Screen height, in pixels */
	int height;
	/* Color depth (is 1 on fx9860g regardless of the gray engine) */
	int bpp;
	/* Current rendering font */
	font_t const * font;

} dinfo_t;

/* dinfo() - retrieve information from the display module
   This function returns the value of most parameters of the display module.

   @info  Pointer to allocated dinfo_t structure, will be filled. */
void dinfo(dinfo_t *info);

#endif /* GINT_DISPLAY */
