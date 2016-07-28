//---
//
//	gint drawing module: tales
//
//	Text displaying. Does some pretty good optimization, though requires
//	dynamic allocation. The stack is used.
//
//---

#ifndef	_TALES_H
#define	_TALES_H	1

#include <display.h>
#include <stdint.h>

//---
//	Types and constants.
//---

/*
	enum ImageFormat
	This type holds information about the characters in the font. Each bit
	represents various characters, and the type itself is a combination of
	several of those bits.
	Bits represent the following characters (lsb right):
	--  --  --  non-print  |  special  capitals  lower  numbers
*/
enum FontFormat
{
	FontFormat_Unknown	= 0x00,
	FontFormat_Numeric	= 0x01,
	FontFormat_LowerCase	= 0x02,
	FontFormat_UpperCase	= 0x04,
	FontFormat_Letters	= 0x06,
	FontFormat_Common	= 0x07,
	FontFormat_Print	= 0x0f,
	FontFormat_Ascii	= 0x1f,
};

/*
	struct FontGlyph
	Holds a glyph's data. The width is used for spacing, and the raw data
	is encoded line after line, from to to bottom, by appending bits
	without consideration of the byte boundaries.
	This structure is actually never used, because data is read directly
	as a longword array (hence the 4-byte alignment).
*/
struct FontGlyph
{
	unsigned char width;

	const unsigned char data[];
} __attribute__((aligned(4)));

/*
	struct Font
	Holds a font's data. Data is accessed using longword operations, hence
	the 4-alignment attributes. The line height is the one given in the
	font image header line, which may be used by applications that write
	strings on several lines. The data height is the height of the biggest
	glyph. Every glyph is encoded on 'data_height' lines, for optimization
	considerations.	
	The index field is used to reduce character access time.
	The name field may not be NUL-terminated when the name contains 28
	characters. When the name is shorter, the field is padded with zeros.
*/
struct Font
{
	unsigned char magic;

	unsigned char format;
	unsigned char line_height;
	unsigned char data_height;

	// Warning : this field may not be NUL-terminated.
	char name[28];

	uint16_t index[16];

	__attribute__((aligned(4))) const uint32_t glyphs[];

} __attribute__((aligned(4)));
// Useful shorthand for user code.
typedef struct Font Font;



//---
//	Generic functions.
//---

/*
	text_configure()
	Sets the font and color to use for subsequent text operations. Pass
	font = NULL to use the default font.
*/
void text_configure(struct Font *font, enum Color color);

/*
	dtext()
	Prints the given string, without any analysis.
*/
void dtext(int x, int y, const char *str);

/*
	gtext()
	Prints the given raw string.
*/
void gtext(int x, int y, const char *str);

/*
	dprint()
	Prints a formatted string. Works the same as printf().
*/
void dprint(int x, int y, const char *format, ...);

/*
	gprint()
	Prints a formatted string. Works the same as printf().
*/
void gprint(int x, int y, const char *format, ...);

#endif	// _TALES_H
