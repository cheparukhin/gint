#include "gintdemo.h"
#include <display.h>
#include <keyboard.h>
#include <stdlib.h>
#include <events.h>

static void draw_keyboard(volatile uint8_t *state)
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
		y = 59 - 5 * i;

		// Space for the horizontal line.
		y += 3 * (i < 7);

		// Moving the [AC/ON] key.
		if(!i) x = 5 * (5) + 1, y = 61 - 5 * (4) + 1;

		// Drawing a filled shape when the key is pressed.
		if(state[i] & (0x80 >> j))
		{
			for(k = -2; k <= 2; k++) for(l = -2; l <= 2; l++)
				if(abs(k) + abs(l) <= 2)
					dpixel(x + k, y + l, color_black);
		}
		// Drawing a square border otherwise.
		else
		{
			for(k = -1; k <= 1; k++) for(l = -1; l <= 1; l++)
				if(k || l) dpixel(x + k, y + l, color_black);
		}
	}

	// Binding the arrow keys together for a more visual thing.
	dpixel(28, 19, color_black); dpixel(29, 19, color_black);
	dpixel(28, 24, color_black); dpixel(29, 24, color_black);
	dpixel(26, 21, color_black); dpixel(26, 22, color_black);
	dpixel(31, 21, color_black); dpixel(31, 22, color_black);

	// An horizontal line to separate parts of the keyboard.
	dline(5, 28, 32, 28, color_black);
}

typedef struct {
	event_type_t type;
	int key;
	int repeats;

} enhanced_event_t;

static void push_history(enhanced_event_t *history, int size, event_t event)
{
	#define	event_eq(x, y) ((x).type == (y).type && (x).key == (y).key)

	// Determining where the history ends.
	int length = 0;
	while(length < size && history[length].type != event_none) length++;

	// Checking if the previous event is being repeated.
	if(length > 0 && event_eq(history[length - 1], event))
	{
		history[length - 1].repeats++;
		return;
	}

	// Making up some space if required.
	if(length == size)
	{
		for(int i = 0; i < size - 1; i++) history[i] = history[i + 1];
		length = size - 1;
	}

	// Adding a new entry to the history.
	history[length].type	= event.type;
	history[length].key	= event.key;
	history[length].repeats	= 1;

	#undef	event_eq
}

static void draw_events(enhanced_event_t *history, int size)
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
	const char *event_names[] = {
		"None ", "User ", "Press", "Rept.", "Rel. ", "Timer"
	};

	for(int i = 0; i < size && history[i].type != event_none; i++)
	{
		print(8, 3 + i, "%s %s", event_names[history[i].type],
			key_names[key_id(history[i].key)]);
		if(history[i].repeats > 1)
			print(19, 3 + i, "%d", history[i].repeats);
	}
}

/*
	test_keyboard_events()
	Real-time keyboard management with events.
*/
void test_keyboard_events(void)
{
	enhanced_event_t history[5];
	int history_size = 5;
	event_t event;

	for(int i = 0; i < history_size; i++) history[i].type = event_none;

	while(1)
	{
		dclear();
		locate(1, 1, "Keyboard and events");
		draw_keyboard(keyboard_stateBuffer());
		draw_events(history, history_size);
		dupdate();

		event = waitevent();
		if(event.type == event_key_press && event.key == KEY_EXIT)
			break;
		push_history(history, history_size, event);
	}
}
