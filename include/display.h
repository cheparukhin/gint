#ifndef	_DISPLAY_H
#define	_DISPLAY_H	1

//---
//	Included submodules.
//---

#include <tales.h>



//---
//	Types and constants.
//---

enum Color
{
	Color_White	= 0,	// White	(AND 0)
	Color_Black	= 1,	// Black	(OR 1)
	Color_None	= 2,	// Transparent	(NOP)
	Color_Invert	= 3,	// Invert	(XOR 1)
};

/*
	enum BlendingMode
	Describes the various blending modes available for drawing images.
	Blending modes may be combined for special effects! For instance:
	- Or		Only sets black pixels.
	- And		Only erases white pixels.
	- Or | And	Fully draws the bitmap.
	- Invert	Inverts pixels where the bitmap is black.
	- Or | Invert	Erases black pixels.
	Adding Checker to an existing combination limits the operation to
	pixels (x, y) that satisfy (x + y) & 1 == 1.
	Operations are done in the following order : Or - Invert - And.
*/
enum BlendingMode
{
	Blend_Or	= 0x01,
	Blend_Invert	= 0x02,
	Blend_And	= 0x04,

	Blend_Checker	= 0x10,
};

/*
	enum ImageFormat
	Describes the various image formats available. Colors may be encoded
	as monochrome (1 layer) or gray (2 layers). Whatever the color map, any
	bitmap may also have an additional alpha layer.
*/
enum ImageFormat
{
	ImageFormat_Mono	= 0x01,
	ImageFormat_Gray	= 0x02,

	ImageFormat_Alpha	= 0x10,

	ImageFormat_ColorMask	= 0x0f,
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

	unsigned char width;
	unsigned char height;
	unsigned char format;

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
	Returns the local video ram. This function always return the same
	address.
	The buffer returned by this function should not be used directly when
	running the gray engine.

	@return	Video ram address of the monochrome display module.
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
	Changes the current video ram address. The argument *MUST* be a
	4-aligned 1024 buffer ; otherwise any drawing operation will crash the
	program.
	This function will most likely have no effect when running the gray
	engine.

	@arg	New video ram address.
*/
void display_useVRAM(void *vram);



//---
//	Global drawing functions.
//---

/*
	dupdate()
	Displays the vram on the physical screen.
*/
void dupdate(void);

/*
	dclear()
	Clears the whole video ram.
*/
void dclear(void);

/*
	dclear_area()
	Clears an area of the video ram.

	@arg	x1
	@arg	y1
	@arg	x2
	@arg	y2
*/
void dclear_area(int x1, int y1, int x2, int y2);

/*
	dreverse_area()
	Reverses an area of the screen.

	@arg	x1
	@arg	y1
	@arg	x2
	@arg	y2
*/
void dreverse_area(int x1, int y1, int x2, int y2);



//---
//	Local drawing functions.
//---

/*
	dpixel()
	Puts a pixel on the screen.

	@arg	x
	@arg	y
	@arg	color
*/
void dpixel(int x, int y, enum Color color);

/*
	dline()
	Draws a line on the screen. Automatically optimizes horizontal and
	vertical lines.

	Uses an algorithm written by PierrotLL for MonochromeLib.

	@arg	x1
	@arg	y1
	@arg	x2
	@arg	y2
	@arg	color
*/
void dline(int x1, int y1, int x2, int y2, enum Color color);



//---
//	Image drawing.
//---

/*
	dimage()
	Displays an image in the vram. Does a real lot of optimization.

	@arg	image
	@arg	x
	@arg	y
	@arg	mode
*/
void dimage(struct Image *image, int x, int y, enum BlendingMode mode);



#endif	// _DISPLAY_H
