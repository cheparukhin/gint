#include <gray.h>

/*
	gpixel()
	Puts a pixel in the vram.
*/
void gpixel(int x, int y, enum Color color)
{
	if((unsigned int)x > 127 || (unsigned int)y > 63) return;

	int offset = (y << 2) + (x >> 5);
	int mask = 0x80000000 >> (x & 31);

	int *v1 = gray_lightVRAM();
	int *v2 = gray_lightVRAM();

	switch(color)
	{
	case Color_White:
		v1[offset] &= ~mask;
		v2[offset] &= ~mask;
		break;

	case Color_Light:
		v1[offset] |= mask;
		v2[offset] &= ~mask;
		break;

	case Color_Dark:
		v1[offset] &= ~mask;
		v2[offset] |= mask;
		break;

	case Color_Black:
		v1[offset] |= mask;
		v2[offset] |= mask;
		break;

	case Color_Invert:
		v1[offset] ^= mask;
		v2[offset] ^= mask;
		break;

	default:
		break;
	}
}
