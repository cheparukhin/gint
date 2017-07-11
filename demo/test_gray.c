#include "gintdemo.h"
#include <gray.h>
#include <keyboard.h>
#include <mpu.h>

/*
	test_gray()
	Lets the user set the gray delays and see the results.
*/

static void draw(int delay1, int delay2, int selected, int gran)
{
	extern image_t res_opt_gray;
	uint32_t *vl = gray_lightVRAM();
	uint32_t *vd = gray_darkVRAM();

	gclear();
	locate(1, 1, "Gray engine");

	for(int i = 0; i < 64; i++)
	{
		int offset = (i << 2) + 3;
		vl[offset] = -(!(i & 16));
		vd[offset] = -(i < 32);
	}

	print(2, 3, "light  %d", delay1);
	print(2, 4, " dark  %d", delay2);
	print(2, 6, "Mode: \x04%d", gran);

	int lengths[2] = {
		4 - (delay1 < 1000),
		4 - (delay2 < 1000)
	};
	print(8, 3 + selected, "\x01");
	print(9 + lengths[selected], 3 + selected, "\x02");

	gimage(0, 56, &res_opt_gray);
	gupdate();
}

void test_gray(void)
{
	int delays[2]; // { light, dark }
	int key, changed = 1;
	int selected = 0;
	int gran = 1;

	gray_getDelays(delays, delays + 1);
	gray_start();

	while(1)
	{
		if(changed)
		{
			gray_setDelays(delays[0], delays[1]);
			draw(delays[0], delays[1], selected, gran);
		}
		changed = 0;

		key = getkey_opt(getkey_default, 25);
		if(key == KEY_EXIT) break;

		changed = 1;

		switch(key)
		{
		case KEY_F1:
			delays[0] = 912;
			delays[1] = 1343;
			break;
		case KEY_F2:
			delays[0] = 993;
			delays[1] = 1609;
			break;
		case KEY_F3:
			delays[0] = 860;
			delays[1] = 1298;
			break;
		case KEY_F5:
			if(gran >= 100) gran = 1;
			else gran *= 10;
		case KEY_UP:
			selected = 0;
			break;
		case KEY_DOWN:
			selected = 1;
			break;
		case KEY_LEFT:
			if(delays[selected] - gran >= 100)
				delays[selected] -= gran;
			break;
		case KEY_RIGHT:
			if(delays[selected] + gran < 10000)
				delays[selected] += gran;
			break;
		default:
			changed = 0;
			break;
		}
	}

	gray_stop();
}
