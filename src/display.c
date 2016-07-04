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

#include <screen.h>
#include <display.h>
#include <string.h>
#include <stdint.h>
#include <gray.h>

// Program video ram. It resides in .bss section, therefore it is cleared at
// program initialization and stripped from the executable file.
static int local_vram[256];
static int *vram = local_vram;

#define sgn(x)	((x) < 0 ? -1 : 1)
#define	abs(x)	((x) < 0 ? -(x) : (x))
#define	rnd(x)	((int)((x) + 0.5))



//---
//	Local functions.
//---

/*
	adjust()
	Adjusts the given rectangle coordinates to ensure that :
	- the rectangle is entirely contained in the screen
	- x1 < x2
	- y1 < y2
	which is needed when working with screen rectangles.
*/
static void adjust(int *x1, int *y1, int *x2, int *y2)
{
	#define	swap(a, b)	tmp = a, a = b, b = tmp
	int tmp;

	if(*x2 < *x1) swap(*x1, *x2);
	if(*y2 < *y1) swap(*y1, *y2);

	if(*x1 < 0) *x1 = 0;
	if(*y1 < 0) *y1 = 0;
	if(*x2 > 127) *x2 = 127;
	if(*y2 > 63) *y2 = 63;
	#undef	swap
}

/*
	getmasks()

	Computes the rectangle masks needed to affect pixels located between x1
	and x2 (both included). The four masks are stored in the third argument
	(seen as an array).
*/
static void getmasks(int x1, int x2, unsigned int *masks)
{
	// Indexes of the first and last longs that are non-blank.
	int l1 = x1 >> 5;
	int l2 = x2 >> 5;
	int i = 0;

	// Setting the base masks. Those are the final values, except for the
	// longs with indexes l1 and l2, that still need to be adjusted.
	while(i < l1)  masks[i++] = 0x00000000;
	while(i <= l2) masks[i++] = 0xffffffff;
	while(i < 4)   masks[i++] = 0x00000000;

	// Removing the long number information in x1 and x2 (that is, the
	// multiples of 32) to keep only the interesting information -- the
	// number of null bits to add in l1 and l2.
	x1 &= 31;
	// Inverting x2 is here the same as computing 32 - x, since 32 is a
	// power of 2 (positive bits at the left are removed by the mask).
	x2 = ~x2 & 31;

	// Setting the first and last masks.
	masks[l1] &= (0xffffffff >> x1);
	masks[l2] &= (0xffffffff << x2);
}



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
inline void *display_getLocalVRAM(void)
{
	return (void *)local_vram;
}

/*
	display_getCurrentVRAM()
	Returns the current video ram. This function usually returns the
	parameter of the last call to display_useVRAM(), unless the gray engine
	is running (in which case the result is undefined). Returns the local
	vram address by default.
*/
inline void *display_getCurrentVRAM(void)
{
	return (void *)vram;
}

/*
	display_useVRAM()
	Changes the current video ram address. The argument MUST be a 4-
	aligned 1024-byte buffer ; otherwise any drawing operation will crash
	the program.
	This function will most likely have no effect when running the gray
	engine.
*/
inline void display_useVRAM(void *ptr)
{
	vram = (int *)ptr;
}



//---
//	Global drawing functions.
//---

/*
	dupdate()
	Displays the vram on the physical screen.
*/
void dupdate(void)
{
	if(gray_runs()) return;
	screen_display((const void *)local_vram);
}

/*
	dclear()
	Clears the whole vram.
*/
void dclear(void)
{
	int i;
	for(i = 0; i < 256; i++) vram[i] = 0;
}

/*
	dclear_area()
	Clears an area of the vram using rectangle masks. Both (x1, y1) and
	(x2, y2) are cleared.
*/
void dclear_area(int x1, int y1, int x2, int y2)
{
	unsigned int masks[4];
	adjust(&x1, &y1, &x2, &y2);
	getmasks(x1, x2, masks);

	int begin = y1 << 2;
	int end = (y2 + 1) << 2;
	int i;

	for(i = 0; i < 4; i++) masks[i] = ~masks[i];
	for(i = begin; i < end; i++) vram[i] &= masks[i & 3];
}

/*
	dreverse_area()
	Reverses an area of the vram. This function is a simple application of
	the rectangle masks concept. (x1, y1) and (x2, y2) are reversed as
	well.
*/
void dreverse_area(int x1, int y1, int x2, int y2)
{
	unsigned int masks[4];
	adjust(&x1, &y1, &x2, &y2);
	getmasks(x1, x2, masks);

	int begin = y1 << 2;
	int end = (y2 + 1) << 2;
	int i;

	if(gray_runs())
	{
		int *v1 = gray_lightVRAM();
		int *v2 = gray_darkVRAM();

		for(i = begin; i < end; i++)
		{
			v1[i] ^= masks[i & 3];
			v2[i] ^= masks[i & 3];
		}
	}
	else for(i = begin; i < end; i++)
	{
		vram[i] ^= masks[i & 3];
	}
}



//---
//	Local drawing functions.
//---

/*
	dpixel()
	Puts a pixel in the vram.
*/
void dpixel(int x, int y, enum Color color)
{
	if((unsigned int)x > 127 || (unsigned int)y > 63) return;

	int offset = (y << 2) + (x >> 5);
	int mask = 0x80000000 >> (x & 31);

	switch(color)
	{
	case Color_White:
		vram[offset] &= ~mask;
		break;

	case Color_Black:
		vram[offset] |= mask;
		break;

	case Color_Invert:
		vram[offset] ^= mask;
		break;

	default:
		break;
	}
}

/*
	dline()
	Draws a line on the screen. Automatically optimizes horizontal and
	vertical lines.
*/

static void dhline(int x1, int x2, int y, enum Color color)
{
	unsigned int masks[4];
	int offset = y << 2;
	int i;

	// Swapping x1 and x2 if needed.
	if(x1 > x2) x1 ^= x2, x2 ^= x1, x1 ^= x2;
	getmasks(x1, x2, masks);

	int *v1 = gray_lightVRAM();
	int *v2 = gray_darkVRAM();

	if(gray_runs()) switch(color)
	{
	case Color_White:
		for(i = 0; i < 4; i++)
		{
			v1[offset + i] &= ~masks[i];
			v2[offset + i] &= ~masks[i];
		}
		break;

	case Color_Light:
		for(i = 0; i < 4; i++)
		{
			v1[offset + i] |= masks[i];
			v2[offset + i] &= ~masks[i];
		}
		break;

	case Color_Dark:
		for(i = 0; i < 4; i++)
		{
			v1[offset + i] &= ~masks[i];
			v2[offset + i] |= masks[i];
		}
		break;

	case Color_Black:
		for(i = 0; i < 4; i++)
		{
			v1[offset + i] |= masks[i];
			v2[offset + i] |= masks[i];
		}
		break;

	case Color_Invert:
		for(i = 0; i < 4; i++)
		{
			v1[offset + i] ^= masks[i];
			v2[offset + i] ^= masks[i];
		}
		break;

	default:
		break;
	}

	else switch(color)
	{
	case Color_White:
		for(i = 0; i < 4; i++) vram[offset + i] &= ~masks[i];
		break;

	case Color_Black:
		for(i = 0; i < 4; i++) vram[offset + i] |= masks[i];
		break;

	case Color_Invert:
		for(i = 0; i < 4; i++) vram[offset + i] ^= masks[i];
		break;

	default:
		break;
	}
}

static void dvline(int y1, int y2, int x, enum Color color)
{
	int offset = (y1 << 2) + (x >> 5);
	int end = (y2 << 2) + (x >> 5);
	int mask = 0x80000000 >> (x & 31);

	int *v1 = gray_lightVRAM();
	int *v2 = gray_darkVRAM();

	if(gray_runs()) switch(color)
	{
	case Color_White:
		while(offset <= end)
		{
			v1[offset] &= ~mask;
			v2[offset] &= ~mask;
			offset += 4;
		}
		break;

	case Color_Light:
		while(offset <= end)
		{
			v1[offset] |= mask;
			v2[offset] &= ~mask;
			offset += 4;
		}
		break;

	case Color_Dark:
		while(offset <= end)
		{
			v1[offset] &= ~mask;
			v2[offset] |= mask;
			offset += 4;
		}
		break;

	case Color_Black:
		while(offset <= end)
		{
			v1[offset] |= mask;
			v2[offset] |= mask;
			offset += 4;
		}
		break;

	case Color_Invert:
		while(offset <= end)
		{
			v1[offset] ^= mask;
			v2[offset] ^= mask;
			offset += 4;
		}
		break;

	default:
		break;
	}

	else switch(color)
	{
	case Color_White:
		while(offset <= end) vram[offset] &= ~mask, offset += 4;
		break;

	case Color_Black:
		while(offset <= end) vram[offset] |= mask, offset += 4;
		break;

	case Color_Invert:
		while(offset <= end) vram[offset] ^= mask, offset += 4;
		break;

	default:
		break;
	}
}

void dline(int x1, int y1, int x2, int y2, enum Color color)
{
	adjust(&x1, &y1, &x2, &y2);

	// Possible optimizations.
	if(y1 == y2)
	{
		dhline(x1, x2, y1, color);
		return;
	}
	if(x1 == x2)
	{
		dvline(y1, y2, x1, color);
		return;
	}

	int i, x = x1, y = y1, cumul;
	int dx = x2 - x1, dy = y2 - y1;
	int sx = sgn(dx), sy = sgn(dy);

	dx = abs(dx), dy = abs(dy);

	dpixel(x1, y1, color);

	if(dx >= dy)
	{
		cumul = dx >> 1;
		for(i = 1; i < dx; i++)
		{
			x += sx;
			cumul += dy;
			if(cumul > dx) cumul -= dx, y += sy;
			dpixel(x, y, color);
		}
	}
	else
	{
		cumul = dy >> 1;
		for(i = 1; i < dy; i++)
		{
			y += sy;
			cumul += dx;
			if(cumul > dy) cumul -= dy, x += sx;
			dpixel(x, y, color);
		}
	}

	dpixel(x2, y2, color);
}
