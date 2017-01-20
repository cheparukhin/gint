#include "gintdemo.h"
#include <display.h>
#include <gray.h>
#include <keyboard.h>

#include <internals/bopti.h>

#include <stddef.h>

/*
	test_bopti()
	Displays and moves many kinds of bitmaps. Here are the images used:

	Name		Size		Color		Alpha
	---------------------------------------------------------
	items.bmp	266 * 124	Gray		-
	sprites.bmp	 66 * 33	Gray		-
	swords.bmp	 88 * 16	Gray		Full
	---------------------------------------------------------
	zelda.bmp	 86 * 280	Mono		-
	isometric.bmp	 37 * 27	Mono		Full
			 		Mono		Greater
	--------------------------------------------------------- 
*/

static void getwh(Image *img, int *width, int *height)
{
	const uint8_t *data;

	if(!img)
	{
		*width = 0;
		*height = 0;
		return;
	}
	*width	= img->width;
	*height	= img->height;
	if(*width && *height) return;

	data = (uint8_t *)img->data;
	*width	= (data[0] << 8) | data[1];
	*height	= (data[2] << 8) | data[3];
}

static void getxy(Image *img, int *x, int *y)
{
	int width, height;

	getwh(img, &width, &height);
	*x = 64 - (width >> 1);
	*y = 28 - (height >> 1);
}

static Image *select(Image *current)
{
	extern Image res_bopti_thumbs;
	extern Image
		res_items,
		res_sprites,
		res_swords,
		res_zelda,
		res_isometric;

	struct {
		Image *img;
		const char *name;
		const char *info;
	} images[] = {
		{ &res_items,		"Items",	"Gray"		},
		{ &res_sprites,		"Sprites",	"Gray"		},
		{ &res_swords,		"Swords",	"Gray Alpha"	},
		{ &res_zelda,		"Zelda",	"Mono"		},
		{ &res_isometric,	"Isometric",	"Mono Alpha"	},
		{ NULL,			NULL,		NULL		}
	};

	Image *thumbs = &res_bopti_thumbs;
	int items = 0;
	static int row = 0;
	int leave = 1, i;

	while(images[items].img) items++;

	gray_start();

	while(1)
	{
		gclear();
		locate(1, 1, "Select an image:");

		for(i = 0; i < items && i < 7; i++)
		{
			locate(2, 2 + i + (i > row), images[i].name);
			gimage_part(100, 8 + 8 * (i + (i > row)), thumbs, 0,
				8 * i, 7, 7);
			if(i == row)
			{
				int width, height;
				getwh(images[i].img, &width, &height);
				print(2, 2 + i + 1, "%d\x04%d", width, height);
				locate(10, 2 + i + 1, images[i].info);
			}
		}

		greverse_area(0, 8 * row + 8, 128, 8 * row + 23);
		gupdate();

		do
		{
			leave = 1;

			switch(getkey())
			{
			case KEY_UP:
				row = (row + items - 1) % items;
				break;
			case KEY_DOWN:
				row = (row + 1) % items;
				break;
			case KEY_EXE:
				return images[row].img;
			case KEY_EXIT:
				return current;
			default:
				leave = 0;
				break;
			}
		}
		while(!leave);
	}

	gray_stop();
}

void test_bopti(void)
{
	extern Image res_opt_bitmap;
	Image *img = NULL;

	int leave = 1;
	int black_bg = 0;
	int x = 0, y = 0;

	while(1)
	{
		if(img && (img->format & Channel_Light))
		{
			gray_start();
			gclear();

			if(black_bg) greverse_area(0, 0, 127, 63);
			if(img) gimage(x, y, img);

			gclear_area(0, 55, 127, 63);
			gimage(0, 56, &res_opt_bitmap);
			gupdate();
		}
		else if(img)
		{
			gray_stop();
			dclear();

			if(black_bg) dreverse_area(0, 0, 127, 63);
			if(img) dimage(x, y, img);

			dclear_area(0, 55, 127, 63);
			dimage(0, 56, &res_opt_bitmap);
			dupdate();
		}
		else
		{
			gray_stop();

			dclear();
			locate(3, 3, "No image selected");

			dimage(0, 56, &res_opt_bitmap);
			dupdate();
		}

		do
		{
			leave = 1;

			switch(getkey())
			{
			case KEY_EXIT:
				gray_stop();
				return;

			case KEY_F1:
				img = select(img);
				getxy(img, &x, &y);
				break;
			case KEY_F5:
				black_bg = !black_bg;
				break;

			case KEY_UP:
				y--;
				break;
			case KEY_DOWN:
				y++;
				break;
			case KEY_LEFT:
				x--;
				break;
			case KEY_RIGHT:
				x++;
				break;

			default:
				leave = 0;
			}
		}
		while(!leave);
	}

	gray_stop();
	return;
}
