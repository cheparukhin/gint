#include <screen.h>
#include <stdint.h>
#include <mpu.h>

/*
	screen_setBacklight()
	On compatible models, turns on or turns off the backlight.
*/
void screen_setBacklight(int on)
{
	if(isSH3())
	{
		volatile uint8_t *PGDR = (void *)0xa400012c;
		if(on)	*PGDR |=  0x80;
		else	*PGDR &= ~0x80;
	}
	else
	{
		volatile uint8_t *PNDR = (void *)0xa4050138;
		if(on)	*PNDR |=  0x10;
		else	*PNDR &= ~0x10;
	}
}

/*
	screen_toggleBacklight()
	Changes the backlight state, regardless of its current state.
*/
void screen_toggleBacklight(void)
{
	if(isSH3())
	{
		volatile uint8_t *PGDR = (void *)0xa400012c;
		*PGDR ^= 0x80;
	}
	else
	{
		volatile uint8_t *PNDR = (void *)0xa4050138;
		*PNDR ^= 0x10;
	}
}
