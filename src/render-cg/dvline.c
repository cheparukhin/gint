#include <gint/display.h>

/* dvline(): Full-height vertical line */
void dvline(int x, color_t color)
{
	dline(x, 0, x, 223, color);
}
