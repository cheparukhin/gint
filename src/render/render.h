//---
//	render - Internal definitions for common display functions
//---

#ifndef RENDER_COMMON
#define RENDER_COMMON

#include <gint/display.h>

/* gint_dhline(): Optimized horizontal line
   @x1 @x2 @y  Coordinates of endpoints of line (both included)
   @color      Any color suitable for dline() */
void gint_dhline(int x1, int x2, int y, color_t color);

/* gint_dvline(): Optimized vertical line
   @y1 @y2 @x  Coordinates of endpoints of line (both included)
   @color      Any color suitable for dline() */
void gint_dvline(int y1, int y2, int x, color_t color);

//---
//	Font rendering (topti)
//---

/* Current font */
extern font_t const *topti_font;
/* Default font */
extern font_t const *gint_default_font;

#endif /* RENDER_COMMON */
