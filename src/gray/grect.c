#include <gint/defs/util.h>
#include <gint/gray.h>
#include <display/fx.h>

/* grect(): Fill a rectangle on the screen */
void grect(int x1, int y1, int x2, int y2, color_t color)
{
	if(x1 > x2) swap(x1, x2);
	if(y1 > y2) swap(y1, y2);

	/* Argument checking */
	if(x1 >= 128 || x2 < 0 || y1 >= 64 || y2 < 0) return;
	if(x1 < 0) x1 = 0;
	if(x2 >= 128) x2 = 127;
	if(y1 < 0) y1 = 0;
	if(y2 >= 64) y2 = 63;

	/* Use masks to get the work done fast! */
	uint32_t m[4];
	masks(x1, x2, m);

	uint32_t *light, *dark;
	gvram(&light, &dark);

	light += (y1 << 2);
	dark  += (y1 << 2);
	int h = y2 - y1 + 1;

	if(color == C_WHITE) while(h--)
	{
		light[0] &= ~m[0];
		light[1] &= ~m[1];
		light[2] &= ~m[2];
		light[3] &= ~m[3];

		dark[0] &= ~m[0];
		dark[1] &= ~m[1];
		dark[2] &= ~m[2];
		dark[3] &= ~m[3];

		light += 4;
		dark += 4;
	}
	else if(color == C_LIGHT) while(h--)
	{
		light[0] |= m[0];
		light[1] |= m[1];
		light[2] |= m[2];
		light[3] |= m[3];

		dark[0] &= ~m[0];
		dark[1] &= ~m[1];
		dark[2] &= ~m[2];
		dark[3] &= ~m[3];

		light += 4;
		dark += 4;
	}
	else if(color == C_DARK) while(h--)
	{
		light[0] &= ~m[0];
		light[1] &= ~m[1];
		light[2] &= ~m[2];
		light[3] &= ~m[3];

		dark[0] |= m[0];
		dark[1] |= m[1];
		dark[2] |= m[2];
		dark[3] |= m[3];

		light += 4;
		dark += 4;
	}
	else if(color == C_BLACK) while(h--)
	{
		light[0] |= m[0];
		light[1] |= m[1];
		light[2] |= m[2];
		light[3] |= m[3];

		dark[0] |= m[0];
		dark[1] |= m[1];
		dark[2] |= m[2];
		dark[3] |= m[3];

		light += 4;
		dark += 4;
	}
	else if(color == C_INVERT) while(h--)
	{
		light[0] ^= m[0];
		light[1] ^= m[1];
		light[2] ^= m[2];
		light[3] ^= m[3];

		dark[0] ^= m[0];
		dark[1] ^= m[1];
		dark[2] ^= m[2];
		dark[3] ^= m[3];

		light += 4;
		dark += 4;
	}
	else if(color == C_LIGHTEN) for(int i = 0; i < (h << 2); i++)
	{
		int j = i & 3;
		uint32_t tmp = *dark, x = m[j];

		*dark &= *light | ~x;
		*light = (*light ^ x) & (tmp | ~x);

		light++, dark++;
	}
	else if(color == C_DARKEN) for(int i = 0; i < (h << 2); i++)
	{
		int j = i & 3;
		uint32_t tmp = *dark, x = m[j];

		*dark |= *light & x;
		*light = (*light ^ x) | (tmp & x);

		light++, dark++;
	}
}
