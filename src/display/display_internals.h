//---
//
//	gint drawing module: display
//
//	Handles vram manipulation and drawing.
//
//
//	:: Rectangle masks
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

#ifndef	_DISPLAY_INTERNALS_H
#define	_DISPLAY_INTERNALS_H

extern int *vram;

/*
	adjustRectangle()
	Adjusts the given rectangle coordinates to ensure that :
	- the rectangle is entirely contained in the screen
	- x1 < x2
	- y1 < y2
	which is needed when working with screen rectangles.
*/
static void adjustRectangle(int *x1, int *y1, int *x2, int *y2);

/*
	getMasks()
	Computes the rectangle masks needed to affect pixels located between x1
	and x2 (both included). The four masks are stored in the third argument
	(seen as an array).
	See this module's internals header file for more information.
*/
static void getMasks(int x1, int x2, unsigned int *masks);

#endif	// _DISPLAY_INTERNALS_H
