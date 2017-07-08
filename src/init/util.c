#include <internals/init.h>
#include <string.h>
#include <clock.h>
#include <gint.h>
#include <math.h>

/* init_version() -- get a version string */
const char *init_version(void)
{
	static char data[14];
	uint32_t s = (uint32_t)&GINT_VERSION;

	/* Force the string constant to reside in ROM because we haven't
	   initialized the data section yet */
	memcpy(data, "gint #0.0-000", 14);

	/* Quickly get the three digits of the build number */
	qdiv_t x = qdiv_10(s & 0xffff);
	qdiv_t y = qdiv_10(x.q);

	data[5]   = (s & 0xff000000) >> 24;
	data[6]  += ((s & 0x00f00000) >> 20);
	data[8]  += ((s & 0x000f0000) >> 16);
	data[10] += y.q;
	data[11] += y.r;
	data[12] += x.r;

	return data;
}

/* init_stage() -- change the current init stage */
void init_stage(const char *name)
{
	drect(85, 0, 127, 7, color_white);
	print(15, 1, name);
}

/* init_halt() -- halt the program */
void init_halt(void)
{
	while(1) sleep();
}

/* print_dec() -- print a number in base 10 */
void print_dec(int x, int y, int n, int digits)
{
	char str[20];
	str[digits] = 0;

	while(--digits >= 0)
	{
		qdiv_t d = qdiv_10(n);
		str[digits] = '0' + d.r;
		n = d.q;
	}

	print(x, y, str);
}

/* print_hex() -- print a number in base 16 */
void print_hex(int x, int y, uint32_t n, int digits)
{
	char str[20];
	str[digits] = 0;

	while(--digits >= 0)
	{
		str[digits] = (n & 0xf) + '0' + 39 * ((n & 0xf) > 9);
		n >>= 4;
	}

	print(x, y, str);
}
