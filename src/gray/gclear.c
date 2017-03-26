#include <gray.h>

/*
	gclear()
	Clears the video ram.
*/
void gclear(void)
{
	uint32_t *lbase = gray_lightVRAM();
	uint32_t *v1 = lbase + 256;
	uint32_t *v2 = gray_darkVRAM() + 256;

	while(v1 > lbase)
	{
		*--v1 = 0;
		*--v1 = 0;
		*--v1 = 0;
		*--v1 = 0;
		*--v1 = 0;
		*--v1 = 0;
		*--v1 = 0;
		*--v1 = 0;

		*--v2 = 0;
		*--v2 = 0;
		*--v2 = 0;
		*--v2 = 0;
		*--v2 = 0;
		*--v2 = 0;
		*--v2 = 0;
		*--v2 = 0;
	}
}
