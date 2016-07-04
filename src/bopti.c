//---
//
//	gint drawing module: bopti
//
//	bopti does every job related to display images. There is only one
//	public function, but there are lots of internal optimizations.
//
//	Some bit-manipulation expressions may look written out of nowhere. The
//	idea is always the same: get a part of the image in an 'operator',
//	which is a 32-bit variable, shift this operator so that its bits
//	correspond to the desired position for the bitmap on the screen, and
//	edit the video-ram long entry which correspond to this position using
//	a 'mask' that indicates which bits of the operator contain information.
//
//---

#include <display.h>
#include <stdint.h>
#include <gray.h>


//---
//	Heading declarations.
//---

/*
	enum Channel
	Determines the kind of information written into a layer. Every image is
	made of one or more channels.
*/
enum Channel
{
	Channel_Mono		= 0x01,
	Channel_Light		= 0x02,
	Channel_Dark		= 0x04,

	Channel_FullAlpha	= 0x08,
	Channel_LightAlpha	= 0x10,
	Channel_DarkAlpha	= 0x20,
};

/*
	enum Format
	Describes the various combination of channels allowed by bopti.
*/
enum Format
{
	Format_Mono		= Channel_Mono,
	Format_MonoAlpha	= Format_Mono | Channel_FullAlpha,
	Format_Gray		= Channel_Light | Channel_Dark,
	Format_GrayAlpha	= Format_Gray | Channel_FullAlpha,
	Format_GreaterAlpha	= Format_Mono | Channel_LightAlpha |
				  Channel_DarkAlpha
};

// These pointers are set by dimage() to avoid having to repeatedly determine
// which video ram to use.
static int *vram, *v1, *v2;
// The following variables refer to parameters that do not change during the
// drawing operation (at least for the time of a layer). They could be passed
// on by every function from the module, but this would be heavy and useless.
static enum Channel channel;
static int height;
static void (*op)(int offset, uint32_t operator, uint32_t op_mask);



//---
//	Drawing operation.
//---

/*
	bopti_op()
	Operates on a vram long. The operator will often not contain 32 bits of
	image information. Since neutral bits are not the same for all
	operations, the op_mask argument indicates which bits should be used
	for the operation. Which operation has to be done is determined by the
	channel setting.
*/
static void bopti_op_mono(int offset, uint32_t operator, uint32_t op_mask)
{
	operator &= op_mask;

	switch(channel)
	{
	case Channel_Mono:
		vram[offset] |= operator;
		break;

	case Channel_FullAlpha:
		vram[offset] &= ~operator;
		break;

	default:
		break;
	}
}
static void bopti_op_gray(int offset, uint32_t operator, uint32_t op_mask)
{
	operator &= op_mask;

	switch(channel)
	{
	case Channel_Mono:
		v1[offset] |= operator;
		v2[offset] |= operator;
		break;

	case Channel_Light:
		v1[offset] |= operator;
		break;

	case Channel_Dark:
		v2[offset] |= operator;
		break;

	case Channel_FullAlpha:
		v1[offset] &= ~operator;
		v2[offset] &= ~operator;
		break;

	case Channel_LightAlpha:
	case Channel_DarkAlpha:
		break;
	}
}

/*
	bopti_grid()		-- general form
	bopti_grid_a32()	-- when x is a multiple of 32

	Draws the grid at the beginning of a layer's data. The length of this
	grid is always a multiple of 32.
	The need for bopti_grid_a32() is not only linked to optimization,
	because bopti_grid() will perform a 32-bit shift when x is a multiple
	of 32, which is undefined behavior.
*/
static void bopti_grid_a32(const uint32_t *layer, int x, int y,
	int column_count)
{
	int vram_column_offset = (y << 2) + (x >> 5);
	int vram_offset = vram_column_offset;
	int column, row;

	for(column = 0; column < column_count; column++)
	{
		for(row = 0; row < height; row++)
		{
			(*op)(vram_offset, *layer, 0xffffffff);
			layer++;
			vram_offset += 4;
		}

		vram_column_offset++;
		vram_offset = vram_column_offset;
	}
}
static void bopti_grid(const uint32_t *layer, int x, int y, int column_count)
{
	if(!column_count) return;
	if(!(x & 31))
	{
		bopti_grid_a32(layer, x, y, column_count);
		return;
	}

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

	// Initializing two pointers. Since the columns are written one after
	// another, they will be updated directly to parse the whole grid.
	p1 = layer - height;
	p2 = layer;

	// Drawing vram longwords, using pairs of columns.
	for(right_column = 0; right_column <= column_count; right_column++)
	{
		and_mask = 0xffffffff;
		if(right_column == 0)			and_mask &= and_mask_0;
		if(right_column == column_count)	and_mask &= and_mask_1;

		for(line = 0; line < height; line++)
		{
			l1 = (right_column > 0) ? (*p1) : (0);
			l2 = (right_column < column_count) ? (*p2) : (0);
			p1++, p2++;

			operator = (l1 << shift1) | (l2 >> shift2);
			(*op)(vram_offset, operator, and_mask);
			vram_offset += 4;
		}

		vram_column_offset++;
		vram_offset = vram_column_offset;
	}
}

/*
	bopti_end_get()
	Returns an operator for the end of a line, whose width is lower than 32
	(by design: otherwise, it would have been a column). The given pointer
	is read and updated so that it points to the next line at the end of
	the operation.
*/
static uint32_t bopti_end_get1(const unsigned char **data)
{
	uint32_t operator = **data;
	*data += 1;
	return operator;
}
static uint32_t bopti_end_get2(const unsigned char **data)
{
	uint32_t operator = *((uint16_t *)*data);
	*data += 2;
	return operator;
}

/*
	bopti_rest()		-- general form
	bopti_rest_nover()	-- when the end does not overlap two vram longs

	Draws the end of a layer, which can be considered as a whole layer
	whose with is lower than 32. (Actually is it lower or equal to 16;
	otherwise it would have been a column and the end would be empty).
*/
static void bopti_end_nover(const unsigned char *end, int x, int y, int width)
{
	uint32_t (*get)(const unsigned char **data) =
		(width > 8) ? bopti_end_get2 : bopti_end_get1;

	int vram_offset = (y << 2) + (x >> 5);
	int row;

	// We *have* shift >= 0 because of this function's 'no overlap'
	// requirement.
	int shift_base = (width > 8) ? 16 : 24;
	int shift = shift_base - (x & 31);

	uint32_t and_mask = (0xffffffff << (32 - width)) >> (x & 31);
	uint32_t operator;

	for(row = 0; row < height; row++)
	{
		operator = (*get)(&end);
		operator <<= shift;

		(*op)(vram_offset, operator, and_mask);
		vram_offset += 4;
	}
}
static void bopti_end(const unsigned char *end, int x, int y, int width)
{
	if((x & 31) + width <= 32)
	{
		bopti_end_nover(end, x, y, width);
		return;
	}

	uint32_t (*get)(const unsigned char **data) =
		(width > 8) ? (bopti_end_get2) : (bopti_end_get1);

	int vram_offset = (y << 2) + (x >> 5);
	int row;

	int shift_base = (width > 8) ? 16 : 24;
	int shift1 = (x & 31) - shift_base;
	int shift2 = shift_base + 32 - (x & 31);

	uint32_t and_mask_0 = 0xffffffff >> (x & 31);
	uint32_t and_mask_1 = 0xffffffff << (64 - width - (x & 31));

	uint32_t row_data, operator;

	for(row = 0; row < height; row++)
	{
		row_data = (*get)(&end);

		operator = row_data >> shift1;
		(*op)(vram_offset, operator, and_mask_0);

		operator = row_data << shift2;
		(*op)(vram_offset + 1, operator, and_mask_1);

		vram_offset += 4;
	}
}



//---
//	Wrappers and various functions.
//---

/*
	bopti()
	Draws a layer in the video ram.
*/

static void bopti(const unsigned char *layer, int x, int y, int columns,
	int end_size)
{
	const unsigned char *end = layer + ((columns * height) << 2);
	int end_x = x + (columns << 5);

	bopti_grid((const uint32_t *)layer, x, y, columns);
	if(end_size) bopti_end(end, end_x, y, end_size);
}

/*
	getStructure()
	Determines the image size and data pointer.
*/
static void getStructure(struct Image *img, int *width, int *height,
	int *layer_size, const unsigned char **data, int *columns,
	int *end_size)
{
	int column_count, end, end_bytes, layer;

	// Large images.
	if(!img->width && !img->height)
	{
		if(width)	*width  = (img->data[0] << 8) | img->data[1];
		if(height)	*height = (img->data[2] << 8) | img->data[3];
		if(data)	*data   = img->data + 4;

		column_count = (*width + 31) >> 5;
		end = end_bytes = 0;
	}
	else
	{
		if(width)	*width  = img->width;
		if(height)	*height = img->height;
		if(data)	*data   = img->data;

		column_count = img->width >> 5;
		end = img->width & 31;
		end_bytes =
			!end		? 0 :
			end <= 8	? 1 :
			end <= 16	? 2 :
			4;
	}

	// The layer size must be rounded to a multiple of 4.
	layer = img->height * ((column_count << 2) + end_bytes);
	if(layer & 3) layer += 4 - (layer & 3);

	if(columns)	*columns    = column_count;
	if(end_size)	*end_size   = end;
	if(layer_size)	*layer_size = layer;
}

/*
	dimage()
	Displays a monochrome image in the video ram.
*/
void dimage(struct Image *img, int x, int y)
{
	int width, layer_size, columns, end;
	int format = img->format, i = 0;
	const unsigned char *data;

	if(img->magic != 0xb7) return;
	if(img->format != Format_Mono && img->format != Format_MonoAlpha)
		return;
	op = bopti_op_mono;

	// 'height' refers to a static variable for this file.
	getStructure(img, &width, &height, &layer_size, &data, &columns, &end);

	vram = display_getCurrentVRAM();

	while(format)
	{
		// Drawing every layer, in order of formats.
		if(format & 1)
		{
			channel = (1 << i);
			bopti(data, x, y, columns, end);
			data += layer_size;
		}

		format >>= 1;
		i++;
	}

}

/*
	gimage()
	Displays a gray image in the video ram.
*/
void gimage(struct Image *img, int x, int y)
{
	int width, layer_size, columns, end;
	int format = img->format, i = 0;
	const unsigned char *data;

	if(img->magic != 0xb7) return;
	op = bopti_op_gray;

	// 'height' refers to a static variable for this file.
	getStructure(img, &width, &height, &layer_size, &data, &columns, &end);

	v1 = gray_lightVRAM();
	v2 = gray_darkVRAM();

	while(format)
	{
		// Drawing every layer, in order of formats.
		if(format & 1)
		{
			channel = (1 << i);
			bopti(data, x, y, columns, end);
			data += layer_size;
		}

		format >>= 1;
		i++;
	}

}
