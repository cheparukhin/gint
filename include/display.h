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



#endif	// _DISPLAY_H
