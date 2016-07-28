#include <internals/tales.h>
#include <tales.h>

#include <display.h>
#include <gray.h>

/*
	dtext()
	Prints the given string, without any analysis.
*/
void dtext(int x, int y, const char *str)
{
	render(x, y, str, operate_mono);
}

/*
	gtext()
	Prints the given raw string.
*/
void gtext(int x, int y, const char *str)
{
	render(x, y, str, operate_gray);
}
