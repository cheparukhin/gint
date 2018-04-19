//---
//	gint:display - Drawing functions
//---

#ifndef GINT_DISPLAY
#define GINT_DISPLAY

/* Expose the VRAM variable if GINT_NEED_VRAM is defined. This address is used
   as the VRAM basis by all of gint's drawing functions, and must point to a
   suitable buffer:
     [fx9860g]  A 4-aligned buffer of size 1024.
     [fxcg50]   A 4-aligned buffer of size 177408.

   This variable is primarily meant to be exposed to gint functions, but
   add-ins may use it or change it freely:
   - To use another video ram area (triple buffering or more, gray engine) ;
   - To implement additional drawing functions ;
   - When not drawing, as additional RAM (especially on fxcg50). */
#ifdef GINT_NEED_VRAM
extern uint32_t *vram;
#endif

/* As you would expect, display on fx9860g and display on fxcg50 are completely
   different worlds. As a consequence, so are the headers ^^ */

#ifdef FX9860G
#include <gint/display-fx.h>
#else
#include <gint/display-cg.h>
#endif

#endif /* GINT_DISPLAY */
