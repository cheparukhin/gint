#include <internals/display.h>
#include <display.h>
#include <screen.h>

/*
	dupdate()
	Displays the vram on the physical screen.
*/
void dupdate(void)
{
	screen_display((const void *)vram);
}
