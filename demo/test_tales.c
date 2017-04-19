#include "gintdemo.h"
#include <display.h>
#include <keyboard.h>
#include <gray.h>

#include <stddef.h>

/*
	test_tales()
	Displays some text using different modes and clipping options.
*/

static font_t *select(font_t *current)
{
	extern font_t res_font_modern;
	struct {
		font_t *font;
		const char *name;
	} fonts[] = {
		{ NULL,			"gint default"	},
		{ &res_font_modern,	"Modern"	},
	};
	int font_number = 2;

	static int row = 0;
	int i, leave;

	while(1)
	{
		text_configure(NULL, color_black);

		dclear();
		locate(1, 1, "Select a font:");

		for(i = 0; i < font_number && i < 6; i++)
		{
			if(fonts[i].font)
			{
				int height = fonts[i].font->line_height;
				int y = (i + 2) * 8 - 8 + ((7 - height) >> 1);

				text_configure(fonts[i].font, color_black);
				dtext(7, y, fonts[i].name);
			}
			else
			{
				text_configure(NULL, color_black);
				locate(2, i + 2, fonts[i].name);
			}

		}

		drect(0, 8 * row + 8, 128, 8 * row + 15, color_invert);
		dupdate();

		do
		{
			leave = 1;

			switch(getkey())
			{
			case KEY_UP:
				row = (row + font_number - 1) % font_number;
				break;
			case KEY_DOWN:
				row = (row + 1) % font_number;
				break;
			case KEY_EXE:
				return fonts[row].font;
			case KEY_EXIT:
				return current;
			default:
				leave = 0;
				break;
			}
		}
		while(!leave);
	}
}

void test_tales(void)
{
	color_t colors[] = { color_black, color_dark, color_light, color_white,
		color_invert };
	extern image_t res_opt_tales;
	font_t *font = NULL;

	int black_bg = 0;
	int color = 0;
	int i, x, height;
	int leave;

	gray_start();
	while(1)
	{
		gclear();
		if(black_bg) grect(0, 0, 127, 54, color_invert);

		if(font)
		{
			text_configure(font, colors[color]);
			height = font->line_height + 1;
		}
		else
		{
			text_configure(NULL, colors[color]);
			height = 8;
		}

		for(i = 0; i < 6 && 2 + (i + 1) * height < 56; i++)
		{
			char str[17];
			for(int j = 0; j < 16; j++) str[j] = 32 + (i << 4) + j;
			str[16] = 0;

			gtext(1, 1 + i * height, str);
		}

		gimage(0, 56, &res_opt_tales);

		x = 45 + 8 * color;
		gline(x, 57, x + 5, 57, color_black);
		gline(x, 57, x, 62, color_black);
		gline(x + 5, 57, x + 5, 62, color_black);
		gline(x, 62, x + 5, 62, color_black);

		gupdate();

		do
		{
			leave = 1;

			switch(getkey())
			{
			case KEY_F1:
				gray_stop();
				font = select(font);
				gray_start();
				break;
			case KEY_F2:
				color = (color + 1) % 5;
				break;
			case KEY_F5:
				black_bg = !black_bg;
				break;

			case KEY_EXIT:
				gray_stop();
				text_configure(NULL, color_black);
				return;
			default:
				leave = 0;
				break;
			}
		}
		while (!leave);
	}
}
