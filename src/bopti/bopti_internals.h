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

#ifndef	_BOPTI_INTERNALS_H
#define	_BOPTI_INTERNALS_H	1

#include <stdint.h>

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

// The following variables refer to parameters that do not change during the
// drawing operation (at least for the time of a layer). They could be passed
// on by every function from the module, but this would be heavy and useless.
// Using global variables is not really a proper solution but it does simplify
// code much.
extern int *vram, *v1, *v2;
extern enum Channel channel;
extern int height;
extern void (*op)(int offset, uint32_t operator, uint32_t op_mask);



//---
//	Some bopti routines.
//---

/*
	bopti_op()
	Operates on a vram long. The operator will often not contain 32 bits of
	image information. Since neutral bits are not the same for all
	operations, the op_mask argument indicates which bits should be used
	for the operation. Which operation has to be done is determined by the
	channel setting.
*/
void bopti_op_mono(int offset, uint32_t operator, uint32_t op_mask);
void bopti_op_gray(int offset, uint32_t operator, uint32_t op_mask);

/*
	bopti_grid()		-- general form
	bopti_grid_a32()	-- when x is a multiple of 32

	Draws the grid at the beginning of a layer's data. The length of this
	grid is always a multiple of 32.
	The need for bopti_grid_a32() is not only linked to optimization,
	because bopti_grid() will perform a 32-bit shift when x is a multiple
	of 32, which is undefined behavior.
*/
void bopti_grid_a32(const uint32_t *layer, int x, int y, int column_count);
void bopti_grid(const uint32_t *layer, int x, int y, int column_count);

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
	otherwise it would have been a column and the end would be empty).
*/
void bopti_end_nover(const unsigned char *end, int x, int y, int width);
void bopti_end(const unsigned char *end, int x, int y, int width);

/*
	bopti()
	Draws a layer in the video ram.
*/
void bopti(const unsigned char *layer, int x, int y, int columns,
	int end_size);

/*
	getStructure()
	Determines the image size and data pointer.
*/
void getStructure(struct Image *img, int *width, int *height, int *layer_size,
	const unsigned char **data, int *columns, int *end_size);

#endif	// _BOPTI_INTERNALS_H
