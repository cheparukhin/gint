#include <gint/defs/util.h>
#include <gint/display.h>

/* drect_border(): Rectangle with border */
void drect_border(int x1, int y1, int x2, int y2, int fill, int width,
	int border)
{
	if(x1 > x2) swap(x1, x2);
	if(y1 > y2) swap(y1, y2);

	if(x1 >= DWIDTH || x2 < 0 || y1 >= DHEIGHT || y2 < 0) return;

	drect(x1, y1, x2, y1 + (width-1), border);
	drect(x1, y2 - (width-1), x2, y2, border);

	y1 += width;
	y2 -= width;

	drect(x1, y1, x1 + (width-1), y2, border);
	drect(x2 - (width-1), y1, x2, y2, border);

	x1 += width;
	x2 -= width;

	if(fill != C_NONE) drect(x1, y1, x2, y2, fill);
}
