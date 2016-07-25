#include <internals/display.h>
#include <display.h>
#include <screen.h>
#include <gray.h>

/*
	dupdate()
	Displays the vram on the physical screen.
*/
void dupdate(void)
{
	if(gray_runs()) return;
	screen_display((const void *)vram);
}
