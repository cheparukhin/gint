#include "gintdemo.h"
#include <display.h>
#include <keyboard.h>
#include <stdlib.h>
#include <events.h>

static int draw_keyboard(volatile uint8_t *state)
{
	int pressed_keys = 0;
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
			pressed_keys++;
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

	return pressed_keys;
}

typedef struct {
	uint32_t key;
	int repeats;
} history_key_t;

typedef struct {
	history_key_t *data;
	int size;
	int pressed;
} history_t;

static int pressed_keys = -1;
static int releases = 0;

static void history_push(history_t *h, event_t event)
{
	// Only reset the number of pressed keys in the history when the actual
	// number of pressed keys reaches 0.

	if(event.type == event_key_release)
	{
		pressed_keys--;
		if(!pressed_keys) h->pressed = 0;
		return;
	}

	// Now we're sure a key was pressed or repeated. Check if it's in the
	// last pressed elements of the history array, if so, update it.
	// Otherwise make space for a new entry and add it.

	if(event.type == event_key_press) pressed_keys++;

	for(int i = h->size - h->pressed; i < h->size; i++)
	{
		if(h->data[i].key == event.key.code)
		{
			h->data[i].repeats++;
			return;
		}
	}

	// If there are already h->size keys pressed, do not add more.
	if(event.type == event_key_press) h->pressed++;
	if(h->pressed > h->size) return;

	for(int i = 0; i < h->size - 1; i++) h->data[i] = h->data[i + 1];
	h->data[h->size - 1].key = event.key.code;
	h->data[h->size - 1].repeats = 1;
}

static void draw_events(history_t *h)
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

	int y = 3;

	print(8, 2, "%d %d %d", pressed_keys, h->pressed, releases);

	for(int i = 0; i < h->size; i++)
	{
		if(!h->data[i].key) continue;
		print(8, y, "%s", key_names[key_id(h->data[i].key)]);
		if(h->data[i].repeats > 1)
			print(18, y, "%d", h->data[i].repeats);
		y++;
	}

	if(h->pressed > h->size) print(8, 8, "(more)");
}

/*
	test_keyboard_events()
	Real-time keyboard management with events.
*/
void test_keyboard_events(void)
{
	history_key_t history_data[5] = { 0 };
	history_t history = {
		.data		= history_data,
		.size		= 5,
		.pressed	= 0,
	};
	event_t event;

	while(1)
	{
		dclear();
		locate(1, 1, "Keyboard and events");

		// There may be more than zero keys pressed when this test
		// starts. We need to detect this count automatically.
		int x = draw_keyboard(keyboard_stateBuffer());
		if(pressed_keys < 0) pressed_keys = x;

		draw_events(&history);
		dupdate();

		event = waitevent();
		if(event.type == event_key_release) releases++;
		if(event.type == event_key_press && event.key.code == KEY_EXIT)
			break;
		history_push(&history, event);
	}
}
