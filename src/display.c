/*
	display

	Handles vram manipulation and drawing.

	:: Rectangle masks

	The concept of 'rectangle masks' is used several times in this module.
	It consists in saying that an operation that affects a rectangle acts
	the same on all the lines (considering that only the lines that
	intersect the rectangle are changed) and therefore it is possible to
	represent the behavior on a single line using 'masks' that indicate
	whether a pixel is affected (1) or not (0).

	For example when clearing the screen rectangle (16, 16, 112, 48), the
	masks will represent information '16 to 112 on x-axis', and will hold
	the following values : 0000ffff, ffffffff, ffffffff and ffff0000. These
	masks can then be used by setting vram[offset] &= ~masks[i]. This
	appears to be very flexible : for instance, reversing a rectangle of
	vram only needs vram[offset] ^= masks[i].

	This technique can also be used in more subtle cases with more complex
	patterns, but within this module it is unlikely to happen.
*/

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
	- The rectangle is entirely contained in the screen,
	- x1 < x2 and y1 < y2,
	which is needed when working with screen rectangles.

	@arg	x1
	@arg	y1
	@arg	x2
	@arg	y2
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
	and x2 (both included).

	@arg	x1
	@arg	x2
	@arg	masks	Four-integer-array pointer.
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
	// power of 2 (actually it creates positive bits at the left but those
	// ones are removed by the bitwise-and mask).
	x2 = ~x2 & 31;

	// Setting the last masks.
	masks[l1] &= (0xffffffff >> x1);
	masks[l2] &= (0xffffffff << x2);
}



//---
//	Generic functions.
//---

/*
	display_getLocalVRAM()
	Returns the local video ram.

	@return	Video ram address.
*/
void *display_getLocalVRAM(void)
{
	return (void *)local_vram;
}

/*
	display_getCurrentVRAM()
	Returns the current vido ram.

	@return	Video ram address.
*/
void *display_getCurrentVRAM(void)
{
	return (void *)vram;
}

/*
	display_useVRAM()
	Changes the current video ram address. Expects a *4-aligned* 1024-byte
	buffer.

	@arg	New video ram address.
*/
void display_useVRAM(void *ptr)
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
	Clears an area of the vram using rectangle masks.

	@arg	x1
	@arg	y1
	@arg	x2
	@arg	y2
*/
void dclear_area(int x1, int y1, int x2, int y2)
{
	unsigned int masks[4];
	adjust(&x1, &y1, &x2, &y2);
	getmasks(x1, x2, masks);

	int offset = y1 << 2;
	int end = (y2 + 1) << 2;
	int i;

	for(i = 0; i < 4; i++) masks[i] = ~masks[i];
	while(offset < end) vram[offset] &= masks[offset & 3], offset++;
}

/*
	dreverse_area()
	Reverses an area of the vram. This function is a simple application of
	the rectangle masks concept.

	@arg	x1
	@arg	y1
	@arg	x2
	@arg	y2
*/
void dreverse_area(int x1, int y1, int x2, int y2)
{
	unsigned int masks[4];
	adjust(&x1, &y1, &x2, &y2);
	getmasks(x1, x2, masks);

	int offset = y1 << 2;
	int end = (y2 + 1) << 2;

	while(offset < end) vram[offset] ^= masks[offset & 3], offset++;
}



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

	case Color_None:
		return;

	case Color_Invert:
		vram[offset] ^= mask;
		break;
	}
}

/*
	dline()
	Draws a line on the screen. Automatically optimizes horizontal and
	vertical lines.

	@arg	x1
	@arg	y1
	@arg	x2
	@arg	y2
	@arg	color
*/

static void dhline(int x1, int x2, int y, enum Color color)
{
	unsigned int masks[4];
	int offset = y << 2;
	int i;

	getmasks(x1, x2, masks);

	switch(color)
	{
	case Color_White:
		for(i = 0; i < 4; i++) vram[offset + i] &= ~masks[i];
		break;

	case Color_Black:
		for(i = 0; i < 4; i++) vram[offset + i] |= masks[i];
		break;

	case Color_None:
		return;

	case Color_Invert:
		for(i = 0; i < 4; i++) vram[offset + i] ^= masks[i];
		break;
	}
}

static void dvline(int y1, int y2, int x, enum Color color)
{
	int offset = (y1 << 2) + (x >> 5);
	int end = (y2 << 2) + (x >> 5);
	int mask = 0x80000000 >> (x & 31);

	switch(color)
	{
	case Color_White:
		while(offset <= end) vram[offset] &= ~mask, offset += 4;
		break;

	case Color_Black:
		while(offset <= end) vram[offset] |= mask, offset += 4;
		break;

	case Color_None:
		return;

	case Color_Invert:
		while(offset <= end) vram[offset] ^= mask, offset += 4;
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



//---
//	Image drawing. There is only one public function dimage(), but there
//	are lots of local methods and optimizations.
//
//	Some expressions may look nonsense sometimes. The procedure is always
//	the same : get a part of the image in an operator, shift it depending
//	on the drawing x-coordinate, compute a mask that indicates which bits
//	of the operator contain information, and modify a vram long using the
//	operator and the mask.
//---

/*
	bopti_op()
	Operates on a vram long. The operator will often not contain 32 bits of
	image information. In this case, the bits outside the image must be set
	to 0 for Or and Invert operations... 1 for And operations. Which means
	that the calling produre must indicate what part of the operator
	belongs to the image, which is done through the image_mask argument.

	@arg	offset		Vram offset where edition is planned.
	@arg	operator	Longword to operate with.
	@arg	image_mask	Part of the operator that is inside the image.
	@arg	mode		Operation mode.
*/
static void bopti_op(int offset, uint32_t operator, uint32_t image_mask,
	enum BlendingMode mode)
{
	if(mode & Blend_Checker)	operator &= 0x55555555;
	if(mode & Blend_Or)		vram[offset] |= operator;
	if(mode & Blend_Invert)		vram[offset] ^= operator;
	operator |= ~image_mask;
	if(mode & Blend_And)		vram[offset] &= operator;
}

/*
	bopti_grid()		-- general form
	bopti_grid_a32()	-- when x is a multiple of 32

	Draws a layer, whose width is a multiple of 32, in the vram.
	The need for bopti_grid_a32() is not only linked to optimization,
	because one of the bit shifts in bopti_grid() will reach 32 when x is
	a multiple of 32, which is undefined behavior.

	@arg	layer		Raw column data (column data is located at the
				beginning of layer data).
	@arg	column_number
	@arg	width
	@arg	height
	@arg	x
	@arg	y
	@arg	mode
*/

static void bopti_grid_a32(const uint32_t *layer, int column_number, int width,
	int height, int x, int y, enum BlendingMode mode)
{
	int vram_column_offset = (y << 2) + (x >> 5);
	int vram_offset = vram_column_offset;

	int column, line;
	uint32_t operator, and_mask;
	uint32_t rightest_and_mask;

	if(width & 31) rightest_and_mask = ~(0xffffffff >> (width & 31));
	else rightest_and_mask = 0xffffffff;

	for(column = 0; column < column_number; column++)
	{
		for(line = 0; line < height; line++)
		{
			operator = *layer++;

			and_mask = (column < column_number - 1) ?
				(0xffffffff) : (rightest_and_mask);
			bopti_op(vram_offset, operator, and_mask, mode);
			vram_offset += 4;
		}

		vram_column_offset++;
		vram_offset = vram_column_offset;
	}
}

static void bopti_grid(const uint32_t *layer, int column_number, int width,
	int height, int x, int y, enum BlendingMode mode)
{
	const uint32_t *p1, *p2;
	uint32_t l1, l2;
	int right_column, line;

	int vram_column_offset = (y << 2) + (x >> 5);
	int vram_offset = vram_column_offset;

	int shift1 = 32 - (x & 31);
	int shift2 = (x & 31);
	int combined_shift_last = shift1 + 32 - (width & 31);

	uint32_t operator, and_mask;
	uint32_t and_mask_0 = 0xffffffff >> shift2;
	uint32_t and_mask_1 = (0xffffffff) << combined_shift_last;

	if(!column_number) return;
	if(!(x & 31))
	{
		bopti_grid_a32(layer, column_number, width, height, x, y,
			mode);
		return;
	}

	// Initializing two pointers. Since the columns are written one after
	// another, they will be updated directly to parse the whole grid.
	p1 = layer - height;
	p2 = layer;

	// Drawing vram longwords, using pairs of columns.
	for(right_column = 0; right_column <= column_number; right_column++)
	{
		for(line = 0; line < height; line++)
		{
			l1 = (right_column > 0) ? (*p1) : (0);
			l2 = (right_column < column_number) ? (*p2) : (0);
			p1++, p2++;

			operator = (l1 << shift1) | (l2 >> shift2);

			and_mask = 0xffffffff;
			if(!right_column) and_mask &= and_mask_0;
			if(right_column == column_number)
				and_mask &= and_mask_1;

			bopti_op(vram_offset, operator, and_mask, mode);
			vram_offset += 4;
		}

		vram_column_offset++;
		vram_offset = vram_column_offset;
	}
}

/*
	bopti_rest8()		-- general form, width below 8
	bopti_rest8_nover()	-- when the rest does not meet two longs
	bopti_rest16()		-- general form, width below 16
	bopti_rest16_nover()	-- when the rest does not meet two longs

	Draw rests of row size of 8 and 16 bits, respectively.

	@arg	rest	Rest data, located at the end of the layer data.
	@arh	width
	@arg	height
	@arg	x
	@arg	y
	@arg	mode
*/

static void bopti_rest8_nover(const uint8_t *rest, int width, int height,
	int x, int y, enum BlendingMode mode)
{
	int vram_offset = (y << 2) + (x >> 5);
	int shift = x & 31;

	uint32_t operator;
	uint32_t and_mask = ~(0xffffffff >> width) >> shift;
	int line;

	for(line = 0; line < height; line++)
	{
		operator = *rest++;
		// Optimization possible ? Probably not.
		operator <<= 24;
		operator >>= shift;

		bopti_op(vram_offset, operator, and_mask, mode);
		vram_offset += 4;
	}
}

static void bopti_rest8(const uint8_t *rest, int width, int height, int x,
	int y, enum BlendingMode mode)
{
	if((x & 31) + width < 32)
	{
		bopti_rest8_nover(rest, width, height, x, y, mode);
		return;
	}

	int vram_offset = (y << 2) + (x >> 5);
	int shift1 = (x & 31) - 24;
	int shift2 = 56 - (x & 31);
	uint32_t and_mask_1 = 0xffffffff >> (x & 31);
	uint32_t and_mask_2 = ~(0xffffffff >> ((x & 31) + width - 32));

	uint32_t operator;
	int line;

	for(line = 0; line < height; line++)
	{
		operator = *rest++;

		bopti_op(vram_offset, operator >> shift1, and_mask_1, mode);
		bopti_op(vram_offset + 1, operator << shift2, and_mask_2,
			mode);
		vram_offset += 4;
	}
}

static void bopti_rest16_nover(const uint16_t *rest, int width, int height,
	int x, int y, enum BlendingMode mode)
{
	int vram_offset = (y << 2) + (x >> 5);
	int shift = x & 31;

	uint32_t operator;
	uint32_t and_mask = ~(0xffffffff >> width) >> shift;
	int line;

	for(line = 0; line < height; line++)
	{
		operator = *rest++;
		// As far as I know, no, we can't optimize this into a single
		// shift.
		operator <<= 16;
		operator >>= shift;

		bopti_op(vram_offset, operator, and_mask, mode);
		vram_offset += 4;
	}
}

static void bopti_rest16(const uint16_t *rest, int width, int height, int x,
	int y, enum BlendingMode mode)
{
	if((x & 31) + width < 32)
	{
		bopti_rest16_nover(rest, width, height, x, y, mode);
		return;
	}

	int vram_offset = (y << 2) + (x >> 5);
	int shift1 = (x & 31) - 16;
	int shift2 = 48 - (x & 31);
	uint32_t and_mask_1 = 0xffffffff >> (x & 31);
	uint32_t and_mask_2 = ~(0xffffffff >> ((x & 31) + width - 32));

	uint32_t operator;
	int line;

	for(line = 0; line < height; line++)
	{
		operator = *rest++;

		bopti_op(vram_offset, operator >> shift1, and_mask_1, mode);
		bopti_op(vram_offset + 1, operator << shift2, and_mask_2,
			mode);
		vram_offset += 4;
	}
}

/*
	bopti()
	Draws an image layer in the video ram.

	@arg	bitmap	Raw layer data.
	@arg	x
	@arg	y
	@arg	width
	@arg	height
	@arg	mode
*/
void bopti(const unsigned char *layer, int x, int y, int width, int height,
	enum BlendingMode mode)
{
	int column_number	= width >> 5;
	int rest_width		= width & 31;
	int grid_width		= width & ~31;

	if(rest_width > 16)
	{
		column_number++;
		rest_width = 0;
		grid_width = width;
	}

	const unsigned char *rest = layer + ((column_number * height) << 2);
	int rest_x = x + (width - rest_width);

	bopti_grid((const uint32_t *)layer, column_number, grid_width, height,
		x, y, mode);
	if(!rest_width) return;

	if(rest_width <= 8)
		bopti_rest8((const uint8_t *)rest, rest_width, height, rest_x,
			y, mode);
	else
		bopti_rest16((const uint16_t *)rest, rest_width, height,
			rest_x, y, mode);
}

/*
	dimage()
	Displays an image in the vram.

	@arg	image
	@arg	x
	@arg	y
	@arg	mode
*/

void dimage(struct Image *image, int x, int y, enum BlendingMode mode)
{
	int width = image->width;
	int height = image->height;
	const unsigned char *data = (const unsigned char *)&(image->data);

	// Computing the layer size.
	int columns = image->width >> 5;
	int rest = image->width & 31;
	int rest_size =
		!rest		? 0 :
		rest <= 8	? 1 :
		rest <= 16	? 2 :
		4;
	int layer_size = ((columns << 2) + rest_size) * image->height;
	// The layer size must be a multiple of 4.
	if(layer_size & 3) layer_size += 4 - (layer_size & 3);

	switch(image->format & ImageFormat_ColorMask)
	{
	case ImageFormat_Mono:
		if(image->format & ImageFormat_Alpha)
		{
			bopti(data + layer_size, x, y, width, height,
				Blend_And);
		}
		bopti(data, x, y, width, height, mode);
		break;

	case ImageFormat_Gray:
		if(image->format & ImageFormat_Alpha)
		{
			bopti(data + 2 * layer_size, x, y, width, height,
				Blend_And);
		}

		display_useVRAM(gray_darkVRAM());
		bopti(data, x, y, width, height, mode);
		display_useVRAM(gray_lightVRAM());
		bopti(data + layer_size, x, y, width, height, mode);
		break;
	}
}
