#include "gintdemo.h"
#include <gray.h>
#include <keyboard.h>
#include <mpu.h>

/*
	test_gray()
	Lets the user set the gray delays and see the results.
*/

static void draw(int delay1, int delay2, int selected)
{
	extern Image res_opt_gray_start;
	int *vl = gray_lightVRAM();
	int *vd = gray_darkVRAM();

	gclear();

	for(int i = 0; i < 63; i++)
	{
		int o = (i << 2) + 2;
		vl[o] = vl[o + 1] = -((i & 31) < 16);
		vd[o] = vd[o + 1] = -(i < 32);
	}

	locate(3, 2, "light");
	print(3, 3, "%d", delay1);

	locate(3, 5, "dark");
	print(3, 6, "%d", delay2);

	locate(2, selected ? 6 : 3, "\x02");

	gimage(0, 56, &res_opt_gray_start);
	gupdate();
}

void test_gray(void)
{
	int delays[2]; // { light, dark }
	int key, changed = 1;
	int selected = 0;

	gray_getDelays(delays, delays + 1);
	gray_start();

	while(1)
	{
		if(changed)
		{
			gray_setDelays(delays[0], delays[1]);
			draw(delays[0], delays[1], selected);
		}
		changed = 0;

		key = getkey_opt(Getkey_RepeatArrowKeys, 1);
		if(key == KEY_EXIT) break;

		changed = 1;

		switch(key)
		{
		case KEY_F1:
			selected = !selected;
			break;
		case KEY_F2:
			delays[0] = isSH3() ? 985 : 994;
			delays[1] = 1609;
			break;
		case KEY_F3:
			delays[0] = 860;
			delays[1] = 1298;
			break;
		case KEY_UP:
			delays[selected] += 10;
			break;
		case KEY_DOWN:
			if(delays[selected] >= 110) delays[selected] -= 10;
			break;
		case KEY_RIGHT:
			delays[selected]++;
			break;
		case KEY_LEFT:
			if(delays[selected] >= 101) delays[selected]--;
			break;
		default:
			changed = 0;
			break;
		}
	}

	gray_stop();
}
