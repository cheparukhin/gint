#include <internals/display.h>
#include <display.h>

/*
	dclear()
	Clears the whole vram, making all pixels white.
*/
void dclear(void)
{
	// I tend to use pre-decrement more than post-increment.
	uint32_t *index = vram + 256;

	while(index > vram)
	{
		*--index = 0;
		*--index = 0;
		*--index = 0;
		*--index = 0;
		*--index = 0;
		*--index = 0;
		*--index = 0;
		*--index = 0;
		*--index = 0;
		*--index = 0;
		*--index = 0;
		*--index = 0;
		*--index = 0;
		*--index = 0;
		*--index = 0;
		*--index = 0;
	}
}
