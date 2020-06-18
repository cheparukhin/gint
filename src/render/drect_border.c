#include <gint/defs/util.h>
#include <gint/display.h>

/* drect_border(): Rectangle with border */
void drect_border(int x1, int y1, int x2, int y2, int fill, int width,
	int border)
{
	if(x1 > x2) swap(x1, x2);
	if(y1 > y2) swap(y1, y2);

	/* Order and bounds */
	if(x1 >= 396 || x2 < 0 || y1 >= 224 || y2 < 0) return;
	if(x1 < 0) x1 = 0;
	if(x2 >= 396) x2 = 395;
	if(y1 < 0) y1 = 0;
	if(y2 >= 224) y2 = 223;

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
