#include <gint/gray.h>

/* gvline(): Full-height vertical line */
void gvline(int x, color_t color)
{
	gline(x, 0, x, 63, color);
}
