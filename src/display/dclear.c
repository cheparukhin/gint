#include <internals/display.h>
#include <display.h>

/*
	dclear()
	Clears the whole vram.
*/
void dclear(void)
{
	int i;
	for(i = 0; i < 256; i++) vram[i] = 0;
}
