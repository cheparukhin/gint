#include <display_internals.h>
#include <display.h>

/*
	dpixel()
	Puts a pixel in the vram.
*/
void dpixel(int x, int y, enum Color color)
{
	if((unsigned int)x > 127 || (unsigned int)y > 63) return;

	int offset = (y << 2) + (x >> 5);
	int mask = 0x80000000 >> (x & 31);

	switch(color)
	{
	case Color_White:
		vram[offset] &= ~mask;
		break;

	case Color_Black:
		vram[offset] |= mask;
		break;

	case Color_Invert:
		vram[offset] ^= mask;
		break;

	default:
		break;
	}
}
