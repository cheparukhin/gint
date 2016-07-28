#include <gray.h>
#include <display.h>

/*
	gline()
	Draws a line in the vram. Automatically optimizes special cases.
*/
void gline(int x1, int y1, int x2, int y2, enum Color color)
{
	enum Color c1, c2;

	if(color == Color_None) return;
	else if(color == Color_Invert) c1 = c2 = Color_Invert;
	else
	{
		c1 = 3 * (color & 1);
		c2 = 3 * (color >> 1);
	}

	display_useVRAM(gray_lightVRAM());
	dline(x1, y1, x2, y2, c1);
	display_useVRAM(gray_darkVRAM());
	dline(x1, y1, x2, y2, c2);
}
