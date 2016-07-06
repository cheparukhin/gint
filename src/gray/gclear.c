#include <gray.h>

/*
	gclear()
	Clears the video ram.
*/
void gclear(void)
{
	int *v1 = gray_lightVRAM();
	int *v2 = gray_darkVRAM();
	int i;

	for(i = 0; i < 256; i++) v1[i] = v2[i] = 0;
}
