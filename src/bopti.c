#include <display.h>
#include <stdint.h>
#include <gray.h>

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

// This pointer is set by bopti().
static int *vram;

/*
	bopti_op()
	Operates on a vram long. The operator will often not contain 32 bits of
	image information. In this case, the bits outside the image must be set
	to 0 for Or and Invert operations... 1 for And operations. Which means
	that the calling procedure must indicate what part of the operator
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

	Draws a layer whose length is a multiple of 32.
	The need for bopti_grid_a32() is not only linked to optimization,
	because one of the bit shifts in bopti_grid() will reach 32 when x is
	a multiple of 32, which is undefined behavior.

	@arg	layer		Raw column data (column data is located at the
				beginning of layer data).
	@arg	x
	@arg	y
	@arg	column_number
	@arg	height
	@arg	mode
*/

static void bopti_grid_a32(const uint32_t *layer, int x, int y,
	int column_number, int height, enum BlendingMode mode)
{
	int vram_column_offset = (y << 2) + (x >> 5);
	int vram_offset = vram_column_offset;
	int column, row;

	for(column = 0; column < column_number; column++)
	{
		for(row = 0; row < height; row++)
		{
			bopti_op(vram_offset, *layer, 0xffffffff, mode);
			layer++;
			vram_offset += 4;
		}

		vram_column_offset++;
		vram_offset = vram_column_offset;
	}
}

static void bopti_grid(const uint32_t *layer, int x, int y, int column_number,
	int height, enum BlendingMode mode)
{
	const uint32_t *p1, *p2;
	uint32_t l1, l2;
	int right_column, line;

	int vram_column_offset = (y << 2) + (x >> 5);
	int vram_offset = vram_column_offset;

	int shift1 = 32 - (x & 31);
	int shift2 = (x & 31);

	uint32_t operator, and_mask;
	uint32_t and_mask_0 = 0xffffffff >> shift2;
	uint32_t and_mask_1 = 0xffffffff << shift1;

	if(!column_number) return;
	if(!(x & 31))
	{
		bopti_grid_a32(layer, x, y, column_number, height, mode);
		return;
	}

	// Initializing two pointers. Since the columns are written one after
	// another, they will be updated directly to parse the whole grid.
	p1 = layer - height;
	p2 = layer;

	// Drawing vram longwords, using pairs of columns.
	for(right_column = 0; right_column <= column_number; right_column++)
	{
		and_mask = 0xffffffff;
		if(right_column == 0)			and_mask &= and_mask_0;
		if(right_column == column_number)	and_mask &= and_mask_1;

		for(line = 0; line < height; line++)
		{
			l1 = (right_column > 0) ? (*p1) : (0);
			l2 = (right_column < column_number) ? (*p2) : (0);
			p1++, p2++;

			operator = (l1 << shift1) | (l2 >> shift2);
			bopti_op(vram_offset, operator, and_mask, mode);
			vram_offset += 4;
		}

		vram_column_offset++;
		vram_offset = vram_column_offset;
	}
}

/*
	bopti_rest_get()
	Returns the line of a bitmap, whose width is lower than 32. The given
	pointer is read and set according to the bitmap width.

	@arg	ptr	Address of data pointer.
	@arg	size	Element size (should be 1, 2, or 4 bytes).
*/
static uint32_t bopti_rest_get(const unsigned char **data, int size)
{
	uint32_t line;

	if(size == 4)
	{
		line = *((uint32_t *)*data);
		*data += 4;
		return line;
	}

	else if(size == 2)
	{
		line = *((uint16_t *)*data);
		*data += 2;
		return line;
	}

	else
	{
		line = **data;
		(*data)++;
		return line;
	}
}

/*
	bopti_rest()		-- general form
	bopti_rest_nover()	-- when the bitmap does not overlap two longs

	Draws a bitmap, whose width is lower than 32. It is called the 'rest'
	since the biggest part will be drawn by bopti_grid().

	@arg	rest	Ending data. Encoded on 1, 2 or 4 bytes per line
			depending on the rest width.
	@arg	x
	@arg	y
	@arg	width	Rest width.
	@arg	height
	@arg	mode
*/

static void bopti_rest_nover(const unsigned char *rest, int x, int y,
	int width, int height, enum BlendingMode mode)
{
	int element_size = (width > 16) ? (4) : (width > 8) ? (2) : (1);
	int vram_offset = (y << 2) + (x >> 5);
	int row;

	// We *have* shift > 0 because of this function's 'no overlap'
	// requirement.
	int shift_base = (4 - element_size) << 3;
	int shift = shift_base - (x & 31);

	uint32_t and_mask = (0xffffffff << (32 - width)) >> (x & 31);
	uint32_t operator;

	for(row = 0; row < height; row++)
	{
		operator = bopti_rest_get(&rest, element_size);
		operator <<= shift;

		bopti_op(vram_offset, operator, and_mask, mode);
		vram_offset += 4;
	}
}

static void bopti_rest(const unsigned char *rest, int x, int y, int width,
	int height, enum BlendingMode mode)
{
	if((x & 31) + width <= 32)
	{
		bopti_rest_nover(rest, x, y, width, height, mode);
		return;
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

static void bopti(const unsigned char *layer, int x, int y, int width,
	int height, enum BlendingMode mode)
{
	int column_number	= width >> 5;
	int rest_width		= width & 31;

	vram = display_getCurrentVRAM();

	// Skipping 'column_number' columns of 'height' longs.
	const unsigned char *rest = layer + ((column_number * height) << 2);
	int rest_x = x + (width - rest_width);

	bopti_grid((const uint32_t *)layer, x, y, column_number, height, mode);
	if(!rest_width) return;
	bopti_rest(rest, rest_x, y, rest_width, height, mode);
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
