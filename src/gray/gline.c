#include <gint/gray.h>
#include <gint/defs/util.h>
#include <display/common.h>

/* gline(): Bresenham line drawing algorithm
   Remotely adapted from MonochromeLib code by Pierre "PerriotLL" Le Gall.
   Relies on grect() for optimized situations.
   @x1 @y1 @x2 @y2  Coordinates of endpoints of line (included)
   @color           Any color */
void gline(int x1, int y1, int x2, int y2, color_t color)
{
	/* Trivial optimizations */
	if(x1 == x2 || y1 == y2)
	{
		grect(x1, y1, x2, y2, color);
		return;
	}

	/* Brensenham line drawing algorithm */

	int i, x = x1, y = y1, cumul;
	int dx = x2 - x1, dy = y2 - y1;
	int sx = sgn(dx), sy = sgn(dy);

	dx = abs(dx), dy = abs(dy);

	gpixel(x1, y1, color);

	if(dx >= dy)
	{
		/* Start with a non-zero cumul to even the overdue between the
		   two ends of the line (for more regularity) */
		cumul = dx >> 1;
		for(i = 1; i < dx; i++)
		{
			x += sx;
			cumul += dy;
			if(cumul > dx) cumul -= dx, y += sy;
			gpixel(x, y, color);
		}
	}
	else
	{
		cumul = dy >> 1;
		for(i = 1; i < dy; i++)
		{
			y += sy;
			cumul += dx;
			if(cumul > dy) cumul -= dy, x += sx;
			gpixel(x, y, color);
		}
	}

	gpixel(x2, y2, color);
}
