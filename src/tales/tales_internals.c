#include <internals/tales.h>
#include <ctype.h>

struct Font *font;

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
	int offset;
	int width, bits;

	c &= 0x7f;



	// Getting the character index in the glyph array.

	if(font->format == FontFormat_Ascii) index = c;
	else if(font->format == FontFormat_Print) index = c - 32;

	else switch(font->format)
	{
	case FontFormat_Numeric:
		if(!isdigit(c)) return -1;
		index = c - '0';
		break;
	case FontFormat_LowerCase:
		if(!islower(c)) return -1;
		index = c - 'a';
		break;
	case FontFormat_UpperCase:
		if(!isupper(c)) return -1;
		index = c - 'A';
		break;
	case FontFormat_Letters:
		if(!isalpha(c)) return -1;
		index = c - 'A' - ('a' - 'z') * (c >= 'a');
		break;
	case FontFormat_Common:
		if(!isalnum(c)) return -1;
		index = c - '0' - ('A' - '9') * (c >= 'A') -
			('a' - 'z') * (c >= 'a');
		break;
	case FontFormat_Unknown:
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
void operate(uint32_t *operators, int height, int x, int y)
{
	int *vram = display_getCurrentVRAM();
	int vram_offset = (x >> 5) + (y << 2);
	int i;

	for(i = 0; i < height; i++)
	{
		// TODO BLENDING MODES //
		vram[vram_offset] |= operators[i];

		vram_offset += 4;
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
int update(uint32_t *operators, int height, int available,
	uint32_t *glyph)
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
		// intersect two different longs.
		line = data & glyph_mask;
		line = (shift >= 0) ? (line >> shift) : (line << -shift);
		operators[i] |= line;

		data <<= width;
		bits_available -= width;

		// Continue until they do.
		if(bits_available >= 0) continue;

		// Computing a special mask that extracts just the number of
		// bits missing, and loading a new data longword.
		partial_mask = 0xffffffff << (32 + bits_available);
		data = glyph[data_index++];
		shift += width + bits_available;

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
