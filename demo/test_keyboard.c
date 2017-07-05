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
	uint32_t key;
	int repeats;
} history_key_t;

typedef struct {
	history_key_t *data;
	int size;
	int pressed;
} history_t;

static int pressed_keys = 0;
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
	int y_limit = 8 - (h->pressed > h->size);

	extern font_t res_font_modern;
	text_configure(&res_font_modern, color_black);
	dtext(49, 12, "Key");
	dtext(89, 12, "Rep.");
	// dprint(49, 7, "%d %d %d", h->pressed, pressed_keys, releases);
	text_configure(NULL, color_black);
	dline(45, 18, 120, 18, color_black);

	for(int i = 0; i < h->size && y < y_limit; i++) if(h->data[i].key)
	{
		dtext(49, y * 8 - 4, key_names[key_id(h->data[i].key)]);
		if(h->data[i].repeats > 1)
			dprint(89, y * 8 - 4, "%d", h->data[i].repeats);
		y++;
	}

	if(h->pressed > h->size) dtext(49, 52, "(more)");
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

	// There may be keys pressed when this test starts. We need to detect
	// which and create a valid history for them.
	volatile const uint8_t *buffer = keyboard_stateBuffer();

	// For the purpose of this function we don't need to calculate the
	// other fields.
	event_t push_event = {
		.type		= event_key_press,
		.key.code	= KEY_AC_ON,
	};

	// Finding all the pressed keys and pushing them in the history.
	if(buffer[0] & 1) history_push(&history, push_event);
	for(int row = 1; row <= 9; row++) for(int col = 0; col < 8; col++)
	{
		// Eliminate non-existing keys.
		if(col > 6 || col <= (row <= 4)) continue;

		if(buffer[row] & (1 << col))
		{
			push_event.key.code = (col << 4) | row;
			history_push(&history, push_event);
		}
	}

	while(1)
	{
		dclear();
		locate(1, 1, "Keyboard and events");

		draw_keyboard(keyboard_stateBuffer());

		draw_events(&history);
		dupdate();

		event = waitevent();
		if(event.type == event_key_release) releases++;
		if(event.type == event_key_press && event.key.code == KEY_EXIT)
			break;
		history_push(&history, event);
	}
}
