#include <internals/tales.h>
#include <tales.h>

/*
	gtext()
	Prints the given raw string.
*/
void gtext(int x, int y, const char *str)
{
	render(x, y, str, operate_gray);
}
