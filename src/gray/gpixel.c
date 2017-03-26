#include <gray.h>

/*
	gpixel()
	Puts a pixel in the vram.
*/
void gpixel(size_t x, size_t y, color_t operator)
{
	if(x > 127 || y > 63) return;

	uint32_t *light	= gray_lightVRAM() + (y << 2) + (x >> 5);
	uint32_t *dark	= gray_darkVRAM()  + (y << 2) + (x >> 5);
	uint32_t mask	= 0x80000000 >> (x & 31);

	switch(operator)
	{
	case color_white:
		*light &= ~mask;
		*dark  &= ~mask;
		break;
	case color_light:
		*light |= mask;
		*dark  &= ~mask;
		break;
	case color_dark:
		*light &= ~mask;
		*dark  |= mask;
		break;
	case color_black:
		*light |= mask;
		*dark  |= mask;
		break;
	case color_none:
		return;

	case color_invert:
		*light ^= mask;
		*dark  ^= mask;
		break;
	case color_lighten:;
		uint32_t old_light_1 = *light;
		*light &= *dark | ~mask;
		*dark  = (old_light_1 | ~mask) & (mask ^ *dark);
		break;
	case color_lighten2:
		*dark  &= *light | ~mask;
		*light &= ~mask;
		break;
	case color_darken:;
		uint32_t old_light_2 = *light;
		*light |= *dark & mask;
		*dark  = (old_light_2 & mask) | (mask ^ *dark);
		break;
	case color_darken2:
		*dark  |= *light | mask;
		*light |= mask;
		break;
	}
}
