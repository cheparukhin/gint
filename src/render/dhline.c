#include <gint/display.h>

/* dhline(): Full-width horizontal line */
void dhline(int y, int color)
{
	dline(0, y, DWIDTH - 1, y, color);
}
