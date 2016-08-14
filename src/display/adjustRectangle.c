#include <internals/display.h>

/*
	adjustRectangle()
	Adjusts the given rectangle coordinates to ensure that :
	- the rectangle is entirely contained in the screen
	- x1 < x2
	- y1 < y2
	which is needed when working with screen rectangles.
	Returns non-zero if the rectangle is outside the screen.
*/
int adjustRectangle(int *x1, int *y1, int *x2, int *y2)
{
	#define	swap(a, b)	tmp = a, a = b, b = tmp
	int tmp;

	if(*x2 < *x1) swap(*x1, *x2);
	if(*y2 < *y1) swap(*y1, *y2);

	if(*x1 > 127 || *y1 > 63 || *x2 < 0 || *y2 < 0) return 1;

	if(*x1 < 0) *x1 = 0;
	if(*y1 < 0) *y1 = 0;
	if(*x2 > 127) *x2 = 127;
	if(*y2 > 63) *y2 = 63;

	return 0;
	#undef	swap
}
