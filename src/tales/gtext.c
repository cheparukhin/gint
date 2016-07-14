#include <tales.h>
#include <gray.h>

/*
	gtext()
	Prints the given raw string.
*/
void gtext(const char *str, int x, int y)
{
	display_useVRAM(gray_lightVRAM());
	dtext(str, x, y);
	display_useVRAM(gray_darkVRAM());
	dtext(str, x, y);
}
