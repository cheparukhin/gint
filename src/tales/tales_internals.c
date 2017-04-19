#include <internals/tales.h>
#include <alloca.h>
#include <string.h>
#include <ctype.h>
#include <gray.h>

font_t *font = NULL;
color_t operator;

/*
	tales_init()
	Configures tales with the default font (which is part of gint).
*/
void tales_init(void)
{
	text_configure(NULL, color_black);
}

/*
	getCharacterIndex()
	Returns the index of a character in a font data area depending on the
	font format and the size of the characters. Returns the index in the
	data area, as long array, or -1 when the character does not belong to
	the font format set.
*/
int getCharacterIndex(int c)
{
	const char *data = (const char *)&font->glyphs;
	int index, current;
	int width, bits;
	size_t offset;

	c &= 0x7f;



	// Getting the character index in the glyph array.

	if(font->format == font_format_ascii) index = c;
	else if(font->format == font_format_print) index = c - 32;

	else switch(font->format)
	{
	case font_format_numeric:
		if(!isdigit(c)) return -1;
		index = c - '0';
		break;
	case font_format_lower:
		if(!islower(c)) return -1;
		index = c - 'a';
		break;
	case font_format_upper:
		if(!isupper(c)) return -1;
		index = c - 'A';
		break;
	case font_format_letters:
		if(!isalpha(c)) return -1;
		index = c - 'A' - ('a' - 'Z') * (c >= 'a');
		break;
	case font_format_common:
		if(!isalnum(c)) return -1;
		index = c - '0' - ('A' - '9') * (c >= 'A') -
			('a' - 'Z') * (c >= 'a');
		break;
	case font_format_unknown:
	default:
		return -1;
	}



	// Reaching the character offset.

	current = index & ~7;
	offset = font->index[current >> 3];

	while(current < index)
	{
		width = data[offset << 2];
		bits = font->data_height * width + 8;

		offset += (bits + 31) >> 5;
		current++;
	}

	return offset;
}

/*
	operate()
	Operates on the vram using the given operators. The x-coordinate should
	be a multiple of 32. There should be `height` operators.
*/
void operate_mono(OPERATE_ARGS)
{
	if(x < 0) return;

	uint32_t *vram	= display_getCurrentVRAM();
	int start = (y < 0) ? (-y) : (0);
	uint32_t *video	= vram + (x >> 5) + ((y + start) << 2);

	switch(operator)
	{
	case color_white:
		for(int i = start; i < height; i++)
		{
			*video &= ~operators[i];
			video += 4;
		}
		break;

	case color_black:
		for(int i = start; i < height; i++)
		{
			*video |= operators[i];
			video += 4;
		}
		break;

	case color_invert:
		for(int i = start; i < height; i++)
		{
			*video ^= operators[i];
			video += 4;
		}
		break;

	default: return;
	}
}

/*
	update()
	Updates the operators using the given glyph. The operation will not be
	complete if there are not enough bits available in the operator data.
	In this case the offset will become negative, which means that the
	calling procedure has to call operate() and re-call update().
	`available` represents the number of free bits in the operators (lower
	bits).
	Returns the number of bits available after the operation. If it's
	negative, call operate() and update() again.
*/
int update(uint32_t *operators, int height, int available, uint32_t *glyph)
{
	// Glyph width.
	int width = glyph[0] >> 24;
	int i;

	// The glyph mask extracts 'width' bits at the left. The partial mask
	// is used when there are less than 'width' bits available in the
	// current data longword.
	uint32_t glyph_mask = 0xffffffff << (32 - width);
	uint32_t partial_mask;

	int shift;
	uint32_t line;

	// Current data longword, next data array index, and number of bits
	// still available in 'data'.
	uint32_t data = glyph[0] << 8;
	int data_index = 1;
	int bits_available = 24;

	for(i = 0; i < height; i++)
	{
		shift = 32 - available;

		// Getting the next 'width' bits. In some cases these bits will
		// intersect two different longs...
		line = data & glyph_mask;
		line = (shift >= 0) ? (line >> shift) : (line << -shift);
		operators[i] |= line;

		data <<= width;
		bits_available -= width;

		// ... continue looping until they do.
		if(bits_available >= 0) continue;

		// Computing a special mask that extracts just the number of
		// bits missing, and loading a new data longword.
		partial_mask = 0xffffffff << (32 + bits_available);
		data = glyph[data_index++];
		shift += width + bits_available;

		// In case this condition is not verified, the program invokes
		// undefined behavior because of the bit shift. Anyway, it
		// means that the end of the end of the operators was reached,
		// in which case the function should not continue writing.
		if(shift <= 31)
		{
			line = data & partial_mask;
			line = (shift >= 0) ? (line >> shift) :
				(line << -shift);
			operators[i] |= line;
		}

		data <<= -bits_available;
		bits_available += 32;
	}

	return available - width;
}

/*
	render()
	Renders text without any formatting analysis, using the given operation
	function.
*/
void render(int x, int y, const char *str, void (*op)(OPERATE_ARGS))
{
	if(!font) return;

	// Operator data, and number of available bits in the operators (which
	// is the same for all operators, since they are treated equally).
	uint32_t *operators;
	int available;

	// Raw glyph data, each glyph being represented as one or several
	// longwords, and an index in this array.
	uint32_t *data = (uint32_t *)font->glyphs;
	int index;

	// Storage height of each glyph. This is a constant for all glyphs
	// because the format required it. It makes this routine consequently
	// faster.
	int height = font->data_height;



	// Allocating data. There will be one operator for each line.
	if(x > 127 || y > 63 || y <= -height) return;
	if(y + height > 64) height = 64 - y;

	operators = alloca(height * sizeof(uint32_t));
	if(!operators) return;
	for(int i = 0; i < height; i++) operators[i] = 0;

	// Computing the initial operator offset to have 32-aligned operators.
	// This allows to write directly video ram longs instead of having to
	// shift operators, and do all the vram operation twice.
	// I double-checked that this operation is still valid when x is
	// negative.
	available = 32 - (x & 31);
	x &= ~31;

	// Displaying character after another.
	while(*str)
	{
		index = getCharacterIndex(*str++);
		if(index < 0) continue;

		// Updating the operators.
		available = update(operators, height, available, data + index);

		// Continue until operators are full (this includes an
		// additional bit to add a space between each character).
		if(available > 1)
		{
			available--;
			continue;
		}

		// When operators are full, updating the video ram and
		// preparing the operators for another row.

		if(x >= 0) (*op)(operators, height, x, y);
		x += 32;
		if(x > 96) break;

		for(int i = 0; i < height; i++) operators[i] = 0;
		if(available >= 0)
		{
			available += 31;
			continue;
		}

		// Finishing update, in cases where it has been only partially
		// done because there was not enough bits available to fit all
		// the information. Also adding a space, assuming that
		// characters aren't more than 30 bits wide. (=p)
		available += 32 + (data[index] >> 24);
		available = update(operators, height, available, data + index);
		available--;
	}

	// Final operation. This condition allows a single bit of the operators
	// to be used - that's because the loop will add a final spacing pixel.
	if(x <= 96 && available < 31) (*op)(operators, height, x, y);
}
