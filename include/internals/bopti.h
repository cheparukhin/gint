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

#ifndef	_INTERNALS_BOPTI_H
#define	_INTERNALS_BOPTI_H	1

#include <stdint.h>
#include <display.h>

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

/*
	struct Structure
	Describes an image's structure.
*/
struct Structure
{
	int width, height;
	int layer_size;

	const unsigned char *data;
	int columns;
	int end_size, end_bytes;
};

/*
	struct Command
	Contains a drawing operation's parameters.
*/
struct Command
{
	// Channel being drawn.
	enum Channel channel;
	// Operation used (whether bopti_op_mono() or bopti_op_gray()).
	void (*op)(int offset, uint32_t operator, struct Command *command);
	// Portion of the bitmap which is drawn. 'top' and 'bottom' refer to
	// lines where 'left' and 'right' refer to column ids.
	int left, right, top, bottom;
	// Position of the bitmap on the screen.
	int x, y;
	// Rectangle masks.
	uint32_t masks[4];
};

// The video ram addresses are set by the public functions and used internally
// by the module.
// Monochrome video ram, light and dark buffers (in this order).
extern int *bopti_vram, *bopti_v1, *bopti_v2;



//---
//	Some bopti routines.
//---

/*
	bopti_op()
	Operates on a vram long. The operator will often not contain 32 bits of
	image information. Since neutral bits are not the same for all
	operations, a mask is used to indicate which bits should be used for
	the operation. This mask is taken for the image's rectangle masks (see
	module display for more information on rectangle masks).
	Which operation is performed is determined by the channel setting.
*/
void bopti_op_mono(int offset, uint32_t operator, struct Command *c);
void bopti_op_gray(int offset, uint32_t operator, struct Command *c);

/*
	bopti_grid()		-- general form
	bopti_grid_a32()	-- when x is a multiple of 32

	Draws the grid at the beginning of a layer's data. The length of this
	grid is always a multiple of 32.
	The need for bopti_grid_a32() is not only linked to optimization,
	because bopti_grid() will perform a 32-bit shift when x is a multiple
	of 32, which is undefined behavior.
	bopti_grid() calls bopti_grid_32() by default.
*/
void bopti_grid_a32(const uint32_t *layer, int columns, int height,
	struct Command *c);
void bopti_grid(const uint32_t *layer, int columns, int height,
	struct Command *c);
/*
	bopti_end_get()
	Returns an operator for the end of a line, whose width is lower than 32
	(by design: otherwise, it would have been a column). The given pointer
	is read and updated so that it points to the next line at the end of
	the operation.
*/
uint32_t bopti_end_get1(const unsigned char **data);
uint32_t bopti_end_get2(const unsigned char **data);

/*
	bopti_rest()		-- general form
	bopti_rest_nover()	-- when the end does not overlap two vram longs

	Draws the end of a layer, which can be considered as a whole layer
	whose with is lower than 32. (Actually is it lower or equal to 16;
	otherwise it would have been a column and the end would be empty). The
	'size' arguments is in bytes.
	Unlike bopti_grid_a32(), bopti_end_nover() is not called automatically
	by bopti_end().
*/
void bopti_end_nover(const unsigned char *end, int size, struct Command *c);
void bopti_end(const unsigned char *end, int size, struct Command *c);

/*
	bopti()
	Draws a layer in the video ram.
*/
void bopti(const unsigned char *layer, struct Structure *s, struct Command *c);

/*
	getStructure()
	Determines the image size and data pointer.
*/
void getStructure(struct Image *img, struct Structure *structure);

#endif	// _INTERNALS_BOPTI_H
