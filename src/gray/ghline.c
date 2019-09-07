#include <gint/gray.h>

/* ghline(): Full-width horizontal line */
void ghline(int y, color_t color)
{
	gline(0, y, 127, y, color);
}
