#include <internals/tales.h>
#include <tales.h>

#include <stdlib.h>
#include <string.h>
#include <alloca.h>

/*
	dtext()
	Prints the given string, without any analysis.
*/
void dtext(const char *str, int x, int y)
{
	// Operator data, and number of available bits in the operators (which
	// is the same for all operators, since they are treated equally).
	uint32_t *operators;
	int available;

	// Raw glyph data, each glyph being represented by one or several
	// longwords, and an index in this array.
	uint32_t *data = (uint32_t *)font->glyphs;
	int index;

	// Height of each glyph. This value is constant because the storage
	// format requires it: it allows greater optimization.
	int height;

	int i;



	if(!font) return;

	// Allocating data. There will be one operator for each line.
	height = font->data_height;
	if(x > 127 || y > 63 || y <= -height) return;

	operators = alloca(height * sizeof(uint32_t));
	for(i = 0; i < height; i++) operators[i] = 0;
	if(!operators) return;

	// Computing the initial operator offset to have 32-aligned operators.
	// This allows to write directly video ram longs instead of having to
	// shift operators, and do all the vram operation twice.
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

		operate(operators, height, x, y);
		x += 32;
		if(x > 96) break;

		memset(operators, 0, height << 2);
		if(available >= 0)
		{
			available = 31 + available;
			continue;
		}

		// Finishing update, in case it has been only partially done,
		// because there was not enough bits available to fit all the
		// information. Also adding a space, assuming that characters
		// aren't more than 30 bits wide.
		available += 32 + (data[index] >> 24);
		available = update(operators, height, available, data + index);
		available--;
	}

	// Final operation.
	if(x <= 96 && available < 32) operate(operators, height, x, y);

//	free(operators);
}
