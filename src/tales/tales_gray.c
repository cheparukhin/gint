#include <internals/tales.h>
#include <display.h>
#include <gray.h>

void operate_gray(OPERATE_ARGS)
{
	if(x < 0) return;

	int start = (y < 0) ? (-y) : (0);
	size_t vram_offset = (x >> 5) + ((y + start) << 2);
	uint32_t *light = (uint32_t *)gray_lightVRAM() + vram_offset;
	uint32_t *dark  = (uint32_t *)gray_darkVRAM()  + vram_offset;
	uint32_t op, old_light;

	for(int i = start; i < height; i++)
	{
		op = operators[i];

		switch(operator)
		{
		case color_white:
			*light &= ~op;
			*dark  &= ~op;
			break;
		case color_light:
			*light |= op;
			*dark  &= ~op;
			break;
		case color_dark:
			*light &= ~op;
			*dark  |= op;
			break;
		case color_black:
			*light |= op;
			*dark  |= op;
			break;
		case color_none:
			return;

		case color_invert:
			*light ^= op;
			*dark  ^= op;
			break;
		case color_lighten:
			old_light = *light;
			*light &= *dark | ~op;
			*dark   = (old_light | ~op) & (op ^ *dark);
			break;
		case color_lighten2:
			*dark  &= *light | ~op;
			*light &= ~op;
			break;
		case color_darken:
			old_light = *light;
			*light |= *dark & op;
			*dark   = (old_light & op) | (op ^ *dark);
			break;
		case color_darken2:
			*dark  |= *light | op;
			*light |= op;
			break;
		}

		light += 4;
		dark += 4;
	}
}
