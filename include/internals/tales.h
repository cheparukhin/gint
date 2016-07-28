//---
//
//	gint drawing module: tales
//
//	Text displaying. Does some good optimization, though requires dynamic
//	allocation.
//
//---

#ifndef	_INTERNALS_TALES_H
#define	_INTERNALS_TALES_H	1

#include <tales.h>
#include <stdint.h>

#define	OPERATE_ARGS	uint32_t *operators, int height, int x, int y

extern struct Font *font;
extern enum Color color;

/*
	tales_init()
	Configures tales with the default font (which is part of gint).
*/
void tales_init(void)				__attribute__((constructor));

/*
	getCharacterIndex()
	Returns the index of a character in a font data area depending on the
	font format and the size of the characters. Returns the index in the
	data area, as long array, or -1 when the character does not belong to
	the font format set.
*/
int getCharacterIndex(int c);

/*
	operate()
	Operates on the vram using the given operators. The x-coordinate should
	be a multiple of 32. There should be `height` operators.
*/
void operate_mono(OPERATE_ARGS);
void operate_gray(OPERATE_ARGS);

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
int update(uint32_t *operators, int height, int available, uint32_t *glyph);

/*
	render()
	Renders text without any formatting analysis, using the given operation
	function.
*/
void render(int x, int y, const char *str, void (*op)(OPERATE_ARGS));

#endif	// _INTERNALS_TALES_H
