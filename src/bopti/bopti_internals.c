#include <internals/bopti.h>

// Monochrome video ram, light and dark buffers (in this order).
uint32_t *bopti_vram, *bopti_v1, *bopti_v2;

/*
	bopti_op()
	Operates on a vram long. The operator will often not contain 32 bits of
	image information. Since neutral bits are not the same for all
	operations, a mask is used to indicate which bits should be used for
	the operation. This mask is taken for the image's rectangle masks (see
	module display for more information on rectangle masks).
	Which operation is performed is determined by the channel setting.
*/
void bopti_op_mono(int offset, uint32_t operator, struct Command *c)
{
	operator &= c->masks[offset & 3];

	switch(c->channel)
	{
	case Channel_FullAlpha:
		bopti_vram[offset] &= ~operator;
		break;

	case Channel_Mono:
		bopti_vram[offset] |= operator;
		break;

	default:
		break;
	}
}
void bopti_op_gray(int offset, uint32_t operator, struct Command *c)
{
	operator &= c->masks[offset & 3];

	switch(c->channel)
	{
	case Channel_FullAlpha:
		bopti_v1[offset] &= ~operator;
		bopti_v2[offset] &= ~operator;
		break;

	case Channel_LightAlpha:
	case Channel_DarkAlpha:
		break;

	case Channel_Mono:
		bopti_v1[offset] |= operator;
		bopti_v2[offset] |= operator;
		break;

	case Channel_Light:
		bopti_v1[offset] |= operator;
		break;

	case Channel_Dark:
		bopti_v2[offset] |= operator;
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
void bopti_grid_a32(const uint32_t *layer, int column_count, int height,
	struct Command *c)
{
	int vram_column_offset = (c->y << 2) + (c->x >> 5);
	int vram_offset = vram_column_offset;
	int column, row;

	for(column = 0; column < column_count; column++)
	{
		for(row = c->top; row < c->bottom; row++)
		{
		 	(c->op)(vram_offset, layer[row], c);
			vram_offset += 4;
		}

		vram_column_offset++;
		vram_offset = vram_column_offset;
		layer += height;
	}
}
void bopti_grid(const uint32_t *layer, int column_count, int height,
	struct Command *c)
{
	if(!column_count) return;
	if(!(c->x & 31))
	{
		bopti_grid_a32(layer, column_count, height, c);
		return;
	}

	const uint32_t *p1, *p2;
	uint32_t l1, l2, operator;
	int right_column, line;
	int actual_column_count;

	int vram_column_offset = (c->y << 2) + (c->x >> 5) + (c->x < 0);
	int vram_offset = vram_column_offset;

	int shift2 = (c->x & 31);
	int shift1 = 32 - shift2;

	// Initializing two pointers. They will read two adjacent columns at
	// the same time (p2 is column ahead of p1). Since the columns are
	// written one after another, incrementing them will suffice when
	// reaching the end of two columns, to move them to the next ones.
	p1 = layer - height;
	p2 = layer;

	// We don't want to write the first vram column when x is negative
	// because it's outside the screen.
	if(c->x < 0) p1 += height, p2 += height;
	right_column = (c->x < 0);
	// For the same reason, we don't to draw the additional rightmost
	// column when it begins after 96.
	if(c->x + (column_count << 5) > 128)
		actual_column_count = column_count - 1;
	else
		actual_column_count = column_count;

	// Drawing vram longwords, using pairs of columns.
	while(right_column <= actual_column_count)
	{
		for(line = c->top; line < c->bottom; line++)
		{
			l1 = (right_column > 0) ? p1[line] : (0);
			l2 = (right_column < column_count) ? p2[line] : (0);

			operator = (l1 << shift1) | (l2 >> shift2);
		 	(c->op)(vram_offset, operator, c);

			vram_offset += 4;
		}

		p1 += height;
		p2 += height;
		vram_column_offset++;
		vram_offset = vram_column_offset;
		right_column++;
	}
}

/*
	bopti_end_get()
	Returns an operator for the end of a line, whose width is lower than 32
	(by design: otherwise, it would have been a column). The given pointer
	is read and updated so that it points to the next line at the end of
	the operation.
*/
uint32_t bopti_end_get1(const unsigned char **data)
{
	uint32_t operator = **data;
	*data += 1;
	return operator;
}
uint32_t bopti_end_get2(const unsigned char **data)
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
void bopti_end_nover(const unsigned char *end, int size, struct Command *c)
{
	uint32_t (*get)(const unsigned char **data) =
		(size == 2) ? bopti_end_get2 : bopti_end_get1;

	// We *have* shift >= 0 because of this function's 'no overlap'
	// requirement.
	int shift = (32 - (size << 3)) - (c->x & 31);
	int vram_offset = (c->y << 2) + (c->x >> 5);
	uint32_t operator;
	int row;

	// Skipping c->top lines (because get() function only allows sequential
	// access).
	end += c->top * size;

	for(row = c->top; row < c->bottom; row++)
	{
		operator = (*get)(&end);
		operator <<= shift;

		 (c->op)(vram_offset, operator, c);

		vram_offset += 4;
	}
}
void bopti_end(const unsigned char *end, int size, struct Command *c)
{
	uint32_t (*get)(const unsigned char **data) =
		(size == 2) ? (bopti_end_get2) : (bopti_end_get1);

	int vram_offset = (c->y << 2) + (c->x >> 5);
	uint32_t row_data, operator;
	int row;

	int shift_base = (32 - (size << 3));
	int shift1 = (c->x & 31) - shift_base;
	int shift2 = shift_base + 32 - (c-> x & 31);

	// Skipping c->top lines (because get() function only allows sequential
	// access).
	end += c->top * size;

	for(row = c->top; row < c->bottom; row++)
	{
		row_data = (*get)(&end);

		operator = row_data >> shift1;
		 (c->op)(vram_offset, operator, c);


		operator = row_data << shift2;
		 (c->op)(vram_offset + 1, operator, c);


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
void bopti(const unsigned char *layer, struct Structure *s, struct Command *c)
{
	const unsigned char *grid, *end;
	int grid_columns, has_end;

	// Skipping columns at the beginning.
	grid = layer + ((c->left * s->height) << 2);

	// Updating the command arguments to eliminate some information about
	// parts that are not being drawn.
	c->x += (c->left << 5);
	c->y += c->top;

	// Columns are identified by ids 0 to s->columns - 1, and the end has
	// id s->columns. So the end is drawn if this last column is included.
	has_end = (c->right == s->columns);
	// Computing number of grid columns to draw.
	grid_columns = c->right - c->left + 1 - has_end;

	bopti_grid((const uint32_t *)grid, grid_columns, s->height, c);

	if(has_end)
	{
		end = layer + ((s->columns * s->height) << 2);
		c->x += (grid_columns << 5);

		if((c->x & 31) + s->end_size <= 32)
			bopti_end_nover(end, s->end_bytes, c);
		else
			bopti_end(end, s->end_bytes, c);
	}
}

/*
	getStructure()
	Determines the image size and data pointer.
*/
void getStructure(image_t *img, struct Structure *s)
{
	int column_count, end, end_bytes, layer;

	// Large images.
	if(!img->width && !img->height)
	{
		s->width	= img->data[0] >> 16;
		s->height	= img->data[0] & 0xffff;
		s->data		= (uint8_t *)img->data + 4;

		column_count = (s->width + 31) >> 5;
		end = 0;
		end_bytes = 0;
	}
	else
	{
		s->width	= img->width;
		s->height	= img->height;
		s->data		= (uint8_t *)img->data;

		column_count = img->width >> 5;
		end = img->width & 31;
		end_bytes =
			!end		? 0 :
			end <= 8	? 1 :
			end <= 16	? 2 :
			4;

		if(end_bytes == 4)
		{
			column_count++;
			end = 0;
			end_bytes = 0;
		}
	}

	// The layer size must be rounded to a multiple of 4.
	layer = s->height * ((column_count << 2) + end_bytes);
	layer = (layer + 3) & ~3;

	s->columns	= column_count;
	s->end_bytes	= end_bytes;
	s->end_size	= end;
	s->layer_size	= layer;
}
