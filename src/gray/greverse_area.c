#include <gray.h>
#include <display.h>

/*
	greverse_area()
	Reverses an area of the vram. End points (x1, y1) and (x2, y2) are
	included.
*/
void greverse_area(int x1, int y1, int x2, int y2)
{
	display_useVRAM(gray_lightVRAM());
	dreverse_area(x1, y1, x2, y2);
	display_useVRAM(gray_darkVRAM());
	dreverse_area(x1, y1, x2, y2);
}
