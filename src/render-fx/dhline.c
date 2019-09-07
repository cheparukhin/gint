#include <gint/display.h>

/* dhline(): Full-width horizontal line */
void dhline(int y, color_t color)
{
	dline(0, y, 127, y, color);
}
