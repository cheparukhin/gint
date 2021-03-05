//---
//	gint:keydev - Generic input handling on keyboard devices
//---

#include <gint/keyboard.h>
#include <gint/drivers/keydev.h>
#include <gint/defs/types.h>
#include <gint/defs/util.h>
#include <gint/std/string.h>
#include <stdarg.h>

void keydev_init(keydev_t *d)
{
	memset(d, 0, sizeof *d);
}

//---
// Driver event generation
//---

/* queue_push(): Add an event in a device's buffer
   Returns false if the event cannot be pushed. */
static bool queue_push(keydev_t *d, keydev_event_t e)
{
	int next = (d->queue_end + 1) % KEYBOARD_QUEUE_SIZE;
	if(next == d->queue_next)
	{
		d->events_lost++;
		return false;
	}

	d->queue[d->queue_end] = e;
	d->queue_end = next;
	return true;
}

/* queue_poll(): Generate key events from the buffer
   Sets (*e) and returns true on success, otherwise false. */
static bool queue_poll(keydev_t *d, keydev_event_t *e)
{
	if(d->queue_next == d->queue_end) return false;

	*e = d->queue[d->queue_next];
	d->queue_next = (d->queue_next + 1) % KEYBOARD_QUEUE_SIZE;
	return true;
}

/* keydev_process_state(): Process the new keyboard states for events */
void keydev_process_state(keydev_t *d, uint8_t scan[12])
{
	/* Compare new data with the internal state. Push releases before
	   presses so that a key change occurring within a single analysis
	   frame can be performed. This happens all the time when going back to
	   the main MENU via gint_osmenu() on a keybind. */
	for(int row = 0; row < 12; row++)
	{
		int diff = ~scan[row] & d->state_now[row];
		if(!diff) continue;

		/* Update internal status if the event can be pushed */
		keydev_event_t e = { d->time, diff, row, KEYEV_UP };
		if(queue_push(d, e)) d->state_now[row] &= scan[row];
	}
	for(int row = 0; row < 12; row++)
	{
		int diff = scan[row] & ~d->state_now[row];
		if(!diff) continue;

		keydev_event_t e = { d->time, diff, row, KEYEV_DOWN };
		if(queue_push(d, e)) d->state_now[row] |= scan[row];
	}
}

/* keydev_process_key(): Process a new key state for events */
void keydev_process_key(keydev_t *d, int keycode, bool state)
{
	/* If the key has changed state, push an event */
	int row = (keycode >> 4) ^ 1;
	int col = 0x80 >> (keycode & 0x7);

	int prev = d->state_now[row] & col;
	if(state && !prev)
	{
		keydev_event_t e = { d->time, col, row, KEYEV_DOWN };
		if(queue_push(d, e)) d->state_now[row] |= col;
	}
	else if(!state && prev)
	{
		keydev_event_t e = { d->time, col, row, KEYEV_UP };
		if(queue_push(d, e)) d->state_now[row] &= ~col;
	}
}

void keydev_tick(keydev_t *d, uint us)
{
	d->time++;

	if(d->rep_key != 0)
	{
		if(d->rep_delay >= 0)
			d->rep_delay = max(d->rep_delay - (int)us, 0);
		d->rep_time += us;
	}
}

//---
// Keyboard event generation
//---

static bool can_repeat(keydev_t *d, int key)
{
	int tr = d->tr.enabled;
	int shift = tr & (KEYDEV_TR_DELAYED_SHIFT | KEYDEV_TR_INSTANT_SHIFT);
	int alpha = tr & (KEYDEV_TR_DELAYED_ALPHA | KEYDEV_TR_INSTANT_ALPHA);

	return !(key == KEY_SHIFT && shift) && !(key == KEY_ALPHA && alpha);
}

/* keydev_unqueue_event(): Retrieve the next keyboard event in queue */
key_event_t keydev_unqueue_event(keydev_t *d)
{
	/* Every device event is unfolded into up to 8 keyboard events, stored
	   temporarily in the driver's structure. */
	keydev_event_t e;
	key_event_t kev = { .type = KEYEV_NONE, .time = d->time };

	/* If there are no events, generate some */
	if(d->out_size == 0)
	{
		if(!queue_poll(d, &e)) return kev;
		int changed = e.changed;
		kev.type = e.kind;

		for(int code = ((e.row ^ 1) << 4) | 0x7; code & 0x7; code--)
		{
			if(changed & 1)
			{
				kev.key = code;
				d->out[d->out_size++] = kev;
			}
			changed >>= 1;
		}
	}

	/* Return one of the available events */
	kev = d->out[--(d->out_size)];

	/* Update the event state accordingly */
	int row = (kev.key >> 4) ^ 1;
	int col = 0x80 >> (kev.key & 0x7);

	if(kev.type == KEYEV_DOWN)
	{
		d->state_queue[row] |= col;
		/* Mark this key as the currently repeating one */
		if(d->rep_key == 0 && can_repeat(d, kev.key))
		{
			d->rep_key = kev.key;
			d->rep_count = -1;
			d->rep_time = 0;
			d->rep_delay = 0;
		}
	}
	if(kev.type == KEYEV_UP)
	{
		d->state_queue[row] &= ~col;
		/* End the current repeating streak */
		if(d->rep_key == kev.key)
		{
			d->rep_key = 0;
			d->rep_count = -1;
			d->rep_time = -1;
			d->rep_delay = -1;
			d->delayed_shift = 0;
			d->delayed_alpha = 0;
		}
	}

	return kev;
}

/* keydev_repeat_event(): Generate a repeat event if applicable */
key_event_t keydev_repeat_event(keydev_t *d)
{
	key_event_t e = { .type = KEYEV_NONE, .time = d->time };
	/* <Repeats> is disabled */
	if(!(d->tr.enabled & KEYDEV_TR_REPEATS)) return e;
	/* No key is being repeated, or it's too early */
	if(!d->rep_key || d->rep_delay != 0) return e;
	/* Key is blocked by transform options modified during the streak */
	if(!can_repeat(d, d->rep_key)) return e;

	/* Plan the next repeat the currently-pressed key */
	int elapsed = (int16_t)(d->time - d->rep_time);
	d->rep_delay = -1;
	d->rep_count++;

	/* Returning < 0 will block further repeats */
	if(d->tr.repeater)
		d->rep_delay = d->tr.repeater(d->rep_key,elapsed,d->rep_count);

	/* Don't return an event on the first call (it's a KEYEV_DOWN) */
	if(!d->rep_count) return e;

	e.key = d->rep_key;
	e.type = KEYEV_HOLD;
	return e;
}

/* keydev_keydown(): Check if a key is down according to generated events */
bool keydev_keydown(keydev_t *d, int key)
{
	int row = (key >> 4) ^ 1;
	int col = 0x80 >> (key & 0x7);

	return (d->state_queue[row] & col) != 0;
}

/* keydev_idle(): Check if all keys are released */
bool keydev_idle(keydev_t *d, ...)
{
	uint32_t *state = (void *)d->state_queue;
	uint32_t check[3] = { state[0], state[1], state[2] };
	int key;

	va_list args;
	va_start(args, d);
	while((key = va_arg(args, int)))
	{
		int row = (key >> 4) ^ 1;
		int col = 0x80 >> (key & 0x7);
		((uint8_t *)check)[row] &= ~col;
	}
	va_end(args);

	return (check[0] == 0) && (check[1] == 0) && (check[2] == 0);
}

//---
// Event transforms
//---

/* keydev_transform(): Obtain current transform parameters */
keydev_transform_t keydev_transform(keydev_t *d)
{
	return d->tr;
}

/* keydev_set_transform(): Set transform parameters */
void keydev_set_transform(keydev_t *d, keydev_transform_t tr)
{
	int change = d->tr.enabled ^ tr.enabled;

	if(change & KEYDEV_TR_DELAYED_SHIFT)
	{
		d->pressed_shift = 0;
		d->delayed_shift = 0;
	}
	if(change & KEYDEV_TR_DELAYED_ALPHA)
	{
		d->pressed_alpha = 0;
		d->delayed_alpha = 0;
	}

	d->tr = tr;
}

/* keydev_read(): Retrieve the next transformed event */
key_event_t keydev_read(keydev_t *d)
{
	#define opt(NAME) (d->tr.enabled & KEYDEV_TR_ ## NAME)
	key_event_t e;

	while(1)
	{
		/* Repeat at the end of every tick, or if we're late */
		bool empty = (d->queue_next == d->queue_end) && !d->out_size;
		bool end_of_tick = (d->time_repeats == d->time - 1) && empty;
		bool late_repeat = (d->time_repeats <= d->time - 2);

		if(end_of_tick || late_repeat)
			e = keydev_repeat_event(d);
		if(e.type == KEYEV_NONE)
			e = keydev_unqueue_event(d);
		if(e.type == KEYEV_NONE)
			return e;

		int k = e.key;

		// <Instant SHIFT> and <Instant ALPHA>

		if(e.type == KEYEV_DOWN || e.type == KEYEV_HOLD)
		{
			if(opt(INSTANT_SHIFT) && k != KEY_SHIFT)
				e.shift |= keydev_keydown(d, KEY_SHIFT);
			if(opt(INSTANT_ALPHA) && k != KEY_ALPHA)
				e.alpha |= keydev_keydown(d, KEY_ALPHA);

		}

		// <Delayed SHIFT> and <Delayed ALPHA>

		if(opt(DELAYED_SHIFT))
		{
			if(e.type == KEYEV_DOWN && k == KEY_SHIFT)
			{
				if(d->delayed_shift)
					d->delayed_shift = 0;
				else if(keydev_idle(d,KEY_SHIFT,0))
					d->pressed_shift = 1;
			}
			else if(e.type != KEYEV_UP && k == d->rep_key)
			{
				e.shift |= d->delayed_shift;
				d->pressed_shift = 0;
			}
			else if(e.type == KEYEV_UP && d->pressed_shift)
			{
				d->pressed_shift = 0;
				d->delayed_shift = 1;
			}
		}

		if(opt(DELAYED_ALPHA))
		{
			if(e.type == KEYEV_DOWN && k == KEY_ALPHA)
			{
				if(d->delayed_alpha)
					d->delayed_alpha = 0;
				else if(keydev_idle(d,KEY_ALPHA,0))
					d->pressed_alpha = 1;
			}
			else if(e.type != KEYEV_UP && k == d->rep_key)
			{
				e.alpha |= d->delayed_alpha;
				d->pressed_alpha = 0;
			}
			else if(e.type == KEYEV_UP && d->pressed_alpha)
			{
				d->pressed_alpha = 0;
				d->delayed_alpha = 1;
			}
		}

		// <Delete Modifiers>
		if(opt(DELETE_MODIFIERS) && !can_repeat(d, k)) continue;

		// <Delete Releases>
		if(opt(DELETE_RELEASES) && e.type == KEYEV_UP) continue;

		return e;
	}

	#undef opt
}
