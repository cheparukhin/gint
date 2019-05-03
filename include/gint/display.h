//---
//	gint:display - Drawing functions
//---

#ifndef GINT_DISPLAY
#define GINT_DISPLAY

#include <gint/defs/types.h>

/* As you would expect, display on fx9860g and display on fxcg50 are completely
   different worlds. As a consequence, so are the rendering functions ^^ */

#ifdef FX9860G
#include <gint/display-fx.h>
#endif

#ifdef FXCG50
#include <gint/display-cg.h>
#endif

#if 0
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
#endif

#endif /* GINT_DISPLAY */
