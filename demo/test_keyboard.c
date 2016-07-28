#include "gintdemo.h"
#include <display.h>
#include <keyboard.h>
#include <stdlib.h>

/*
	test_keyboard()
	Displays a real-time multigetkey() and the keyboard state.
*/

static void draw(volatile unsigned char *state)
{
	int i, j, k, l;
	int x, y;

	for(i = 0; i < 10; i++) for(j = 1; j < 8; j++)
	{
		// Eliminating keys that do not exist.
		if(!i && j != 7) continue;
		if(i && j == 7) continue;
		if(i <= 4 && j == 6) continue;
		if(i == 4 && j == 5) continue;

		x = 5 * j + 1;
		y = 61 - 5 * i;
		// Moving the [AC/ON] key.
		if(!i) x = 5 * (5) + 1, y = 61 - 5 * (4);

		// Drawing a filled shape when the key is pressed.
		if(state[i] & (128 >> j))
		{
			for(k = -2; k <= 2; k++) for(l = -2; l <= 2; l++)
				if(abs(k) + abs(l) <= 2)
					dpixel(x + k, y + l, Color_Black);
		}
		// Drawing a square border otherwise.
		else
		{
			for(k = -1; k <= 1; k++) for(l = -1; l <= 1; l++)
				if(k || l) dpixel(x + k, y + l, Color_Black);
		}
	}
}

void test_keyboard(void)
{
	const char *key_names[] = {
		"F1",       "F2",            "F3",   "F4",   "F5",    "F6",
		"SHIFT",    "OPTN",          "VARS", "MENU", "Left",  "Up",
		"ALPHA",    "x^2",           "^",    "EXIT", "Down",  "Right",
		"X,\x1d,T", "log",           "ln",   "sin",  "cos",   "tan",
		"[frac]",   "F\x0f\x09" "D", "(",    ")",    ",",     "\x09",
		"7",        "8",             "9",    "DEL",  "AC/ON", NULL,
		"4",        "5",             "6",    "\x04", "\x05",  NULL,
		"1",        "2",             "3",    "+",    "-",     NULL,
		"0",        ".",             "\x08", "(-)",  "EXE",   NULL
	};

	volatile unsigned char *state = keystate();
	int keys[4] = { 0 };
	int i;

	while(1)
	{
		multigetkey(keys, 4, 1);
		if(keys[0] == KEY_EXIT && keys[1] == KEY_NONE) break;

		dclear();
		locate(1, 1, "Keyboard driver");
		locate(8, 3, "Pressed keys:");
		draw(state);

		if(keys[0] == KEY_NONE) locate(9, 4, ":None");
		else for(i = 0; i < 4 && keys[i] != KEY_NONE; i++)
		{
			locate( 9, i + 4, ":");
			locate(10, i + 4, key_names[keyid(keys[i])]);
		}

		dupdate();
	}
}
