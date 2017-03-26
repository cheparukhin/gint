#include <gray.h>
#include <display.h>

/*
	gline()
	Draws a line in the vram. Automatically optimizes special cases. This
	function does not support the light operators.
*/
void gline(int x1, int y1, int x2, int y2, color_t operator)
{
	color_t op1, op2;

	if(operator == color_invert) op1 = op2 = color_invert;
	else if(operator >= color_none) return;
	else
	{
		op1 = 3 * (operator & 1);
		op2 = 3 * (operator >> 1);
	}

	display_useVRAM(gray_lightVRAM());
	dline(x1, y1, x2, y2, op1);
	display_useVRAM(gray_darkVRAM());
	dline(x1, y1, x2, y2, op2);
}
