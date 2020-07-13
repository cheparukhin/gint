#include <gint/display.h>

/* dvline(): Full-height vertical line */
void dvline(int x, int color)
{
	dline(x, 0, x, DHEIGHT - 1, color);
}
