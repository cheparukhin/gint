//---
//	gint:keysc:getkey - High-level keyboard monitoring function
//---

#include <gint/keyboard.h>
#include <gint/drivers/keydev.h>
#include <gint/gint.h>
#include <gint/defs/types.h>

#ifdef FX9860G
#include <gint/drivers/t6k11.h>
#endif

/* Delay before first repeat and subsequent repeats, in milliseconds */
static int rep_first = 400, rep_next = 40;
/* Repeat profile function */
static getkey_profile_t repeat_profile = NULL;
/* Feature function */
static getkey_feature_t feature_function = NULL;
/* Repeater specification */
static enum { NONE, ALL, ARROWS, FILTER } repeat_mode;

static int getkey_repeater(int key, int duration, int count)
{
	if(repeat_mode == NONE) return -1;
	if(repeat_mode == FILTER) return repeat_profile(key, duration, count);

	if(repeat_mode == ARROWS && key != KEY_LEFT && key != KEY_RIGHT
		&& key != KEY_UP && key != KEY_DOWN) return -1;

	return (count ? rep_next : rep_first) * 1000;
}

/* getkey(): Wait for a key press */
key_event_t getkey(void)
{
	return getkey_opt(GETKEY_DEFAULT, NULL);
}

/* getkey_opt(): Enhanced getkey() */
key_event_t getkey_opt(int opt, volatile int *timeout)
{
	keydev_t *d = keydev_std();
	keydev_transform_t tr0 = keydev_transform(d);
	key_event_t e;

	int o = KEYDEV_TR_REPEATS +
		KEYDEV_TR_DELETE_MODIFIERS +
		KEYDEV_TR_DELETE_RELEASES +
		(opt & (GETKEY_MOD_SHIFT + GETKEY_MOD_ALPHA));
	keydev_set_transform(d, (keydev_transform_t){ o, getkey_repeater });

	repeat_mode = NONE;
	if(opt & GETKEY_REP_ARROWS) repeat_mode = ARROWS;
	if(opt & GETKEY_REP_ALL) repeat_mode = ALL;
	if(opt & GETKEY_REP_PROFILE && repeat_profile) repeat_mode = FILTER;

	while(1)
	{
		e = keydev_read(d);
		if(e.type == KEYEV_NONE && timeout && *timeout) break;

		#ifdef FX9860G
		/* Backlight toggle */
		else if((opt & GETKEY_BACKLIGHT) && e.type == KEYEV_DOWN &&
			e.key == KEY_OPTN && e.shift && !e.alpha)
			t6k11_backlight(-1);
		#endif

		/* Return-to-menu */
		else if((opt & GETKEY_MENU) && e.type == KEYEV_DOWN &&
			e.key == KEY_MENU && !e.shift && !e.alpha)
			gint_osmenu();

		else if(e.type == KEYEV_DOWN || e.type == KEYEV_HOLD)
		{
			/* Custom global features */
			bool accepted = false;
			if((opt & GETKEY_FEATURES) && feature_function)
				accepted = feature_function(e);
			/* Return if the even has not been accepted yet */
			if(!accepted) break;
		}
	}

	keydev_set_transform(d, tr0);
	return e;
}

/* getkey_repeat(): Set repeat delays for getkey() */
void getkey_repeat(int first, int next)
{
	rep_first = first;
	rep_next = next;
}

/* getkey_repeat_profile(): Get the current repeat profile function */
getkey_profile_t getkey_repeat_profile(void)
{
	return repeat_profile;
}

/* getkey_set_repeat_profile(): Set the repeat profile function */
void getkey_set_repeat_profile(getkey_profile_t profile)
{
	repeat_profile = profile;
}

/* getkey_feature_function(): Get the current feature function */
getkey_feature_t getkey_feature_function(void)
{
	return feature_function;
}

/* getkey_set_feature_function(): Set the global feature function */
void getkey_set_feature_function(getkey_feature_t function)
{
	feature_function = function;
}
