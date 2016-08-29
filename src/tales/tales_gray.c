#include <internals/tales.h>
#include <display.h>
#include <gray.h>

void operate_gray(OPERATE_ARGS)
{
	int *vl = gray_lightVRAM();
	int *vd = gray_darkVRAM();
	int vram_offset = (x >> 5) + (y << 2);
	uint32_t op;
	int i;

	for(i = 0; i < height; i++)
	{
		op = operators[i];

		switch(color)
		{
		case Color_White:
			vl[vram_offset] &= ~op;
			vd[vram_offset] &= ~op;
			break;
		case Color_Light:
			vl[vram_offset] |= op;
			vd[vram_offset] &= ~op;
			break;
		case Color_Dark:
			vl[vram_offset] &= ~op;
			vd[vram_offset] |= op;
			break;
		case Color_Black:
			vl[vram_offset] |= op;
			vd[vram_offset] |= op;
			break;
		case Color_Invert:
			vl[vram_offset] ^= op;
			vd[vram_offset] ^= op;
			break;
		default:
			break;
		}

		vram_offset += 4;
	}
}
