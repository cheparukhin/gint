//---
//
//	gint drawing module: display
//
//	Handles vram manipulation and drawing.
//
//---

#ifndef	_INTERNALS_DISPLAY_H
#define	_INTERNALS_DISPLAY_H	1

#include <stdint.h>

extern int *vram;

//---
//	Rectangle masks.
//
//	The concept of 'rectangle masks' is used several times in this module.
//	It is based on the fact that an operation that affects a rectangle acts
//	the same on all its lines. Therefore the behavior of the operation is
//	determined by its behavior on a single line, which is represented using
//	'masks' whose bits indicate whether a pixel is affected (1) or not (0).
//
//	For example when clearing the screen rectangle (16, 16, 112, 48), the
//	masks will represent information '16 to 112 on x-axis', and will hold
//	the following values : 0000ffff, ffffffff, ffffffff and ffff0000. These
//	masks can then be used by setting vram[offset] &= ~masks[i]. This
//	appears to be very flexible : for instance, vram[offset] ^= masks[i]
//	will reverse the pixels in the same rectangle.
//
//	This technique can also be used in more subtle cases with more complex
//	patterns, but within this module it is unlikely to happen.
//
//---

/*
	adjustRectangle()
	Adjusts the given rectangle coordinates to ensure that :
	- the rectangle is entirely contained in the screen
	- x1 < x2
	- y1 < y2
	which is needed when working with screen rectangles.
	Returns non-zero if the rectangle is outside the screen.
*/
int adjustRectangle(int *x1, int *y1, int *x2, int *y2);

/*
	getMasks()
	Computes the rectangle masks needed to affect pixels located between x1
	and x2 (both included). The four masks are stored in the third argument
	(seen as an array).
*/
void getMasks(int x1, int x2, uint32_t *masks);

#endif	// _INTERNALS_DISPLAY_H
