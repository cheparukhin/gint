#include <internals/tales.h>
#include <tales.h>

/*
	dtext()
	Prints the given string, without any analysis.
*/
void dtext(int x, int y, const char *str)
{
	render(x, y, str, operate_mono);
}
