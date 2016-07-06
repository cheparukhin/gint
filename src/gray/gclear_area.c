#include <gray.h>
#include <display.h>

/*
	gclear_area()
	Clears an area of the video ram. End points (x1, y1) and (x2, y2) are
	included.
*/
void gclear_area(int x1, int y1, int x2, int y2)
{
	display_useVRAM(gray_lightVRAM());
	dclear_area(x1, y1, x2, y2);
	display_useVRAM(gray_darkVRAM());
	dclear_area(x1, y1, x2, y2);
}
