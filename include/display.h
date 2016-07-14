//---
//
//	gint drawing module: display
//
//	Handles vram manipulation and drawing for plain monochrome display.
//
//---

#ifndef	_DISPLAY_H
#define	_DISPLAY_H	1


//---
//	Heading declarations.
//---

#include <stdint.h>
#include <tales.h>

enum Color
{
	Color_White	= 0,
	Color_Light	= 1,
	Color_Dark	= 2,
	Color_Black	= 3,
	Color_None	= 4,
	Color_Invert	= 5,
};

/*
	struct Image
	This structure holds information about a bitmap encoded with fxconv.
	Data is accessed using longword operations, which *requires* many
	sizes to be multiples of 4 (structure alignment, data alignment, layer
	size, ...).
*/
struct Image
{
	unsigned char magic;
	unsigned char format;

	unsigned char width;
	unsigned char height;

	const unsigned char __attribute__((aligned(4))) data[];

} __attribute__((aligned(4)));
// Useful shorthand for user code.
typedef struct Image Image;



// A few other constants.
#define	DISPLAY_WIDTH	128
#define	DISPLAY_HEIGHT	64



//---
//	Generic functions.
//---

/*
	display_getLocalVRAM()
	Returns the local video ram address. This function always return the
	same address.
	The buffer returned by this function should not be used directly when
	running the gray engine.
*/
void *display_getLocalVRAM(void);

/*
	display_getCurrentVRAM()
	Returns the current video ram. This function usually returns the
	parameter of the last call to display_useVRAM(), unless the gray engine
	is running (in which case the result is undefined). Returns the local
	vram address by default.
*/
void *display_getCurrentVRAM(void);

/*
	display_useVRAM()
	Changes the current video ram address. The argument MUST be a 4-
	aligned 1024-byte buffer; otherwise any drawing operation will crash
	the program.
	This function will most likely have no effect when running the gray
	engine.
*/
void display_useVRAM(void *vram);



//---
//	Global drawing functions.
//---

/*
	dupdate()
	Displays the vram on the physical screen. Does nothing when the gray
	engine is running.
*/
void dupdate(void);

/*
	dclear()
	Clears the whole video ram.
*/
void dclear(void);

/*
	dclear_area()
	Clears an area of the video ram. Both (x1, y1) and (x2, y2) are
	cleared.
*/
void dclear_area(int x1, int y1, int x2, int y2);

/*
	dreverse_area()
	Reverses an area of the vram. (x1, y1) and (x2, y2) are reversed as
	well.
*/
void dreverse_area(int x1, int y1, int x2, int y2);



//---
//	Local drawing functions.
//---

/*
	dpixel()
	Puts a pixel in the vram.
*/
void dpixel(int x, int y, enum Color color);

/*
	dline()
	Draws a line in the vram. Automatically optimizes horizontal and
	vertical lines.

	Uses an algorithm written by PierrotLL for MonochromeLib.
*/
void dline(int x1, int y1, int x2, int y2, enum Color color);

/*
	dimage()
	Displays a monochrome image in the vram. Does a real lot of
	optimization.
*/
void dimage(struct Image *image, int x, int y);



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
*/
void adjustRectangle(int *x1, int *y1, int *x2, int *y2);

/*
	getMasks()
	Computes the rectangle masks needed to affect pixels located between x1
	and x2 (both included). The four masks are stored in the third argument
	(seen as an array).
*/
void getMasks(int x1, int x2, uint32_t *masks);



#endif	// _DISPLAY_H
