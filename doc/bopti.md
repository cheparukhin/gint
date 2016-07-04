
# gint documentation: bitmap rendering #


*Warning: this is a draft. The current implementation of bopti is different*
*from this description, though similar.*



## Basics

The bitmap drawing module, *bopti*, is based on video-ram (vram) bitwise
operations. The images are made of layers that describe (more or less) which
pixels of the image an operation applies to. Rendering the image consists in
applying an operation function to the existing vram pixels.

*bopti* makes an extensive use of longword operations and 4-alignment to take
advantage of the bit-based structure of the monochrome vram and enhance
performance. Among all possible optimizations, avoiding direct pixel access has
proven to be the most efficient.



---



## Operations

Operations are functions applied to update a vram longword in accordance with
an operation mask. Bits that are set in the mask indicate pixels which have to
be updated by the operation. Bits that are reset indicate pixels that must not
be changed.

All the point is, the functions must not access the bit information in the mask
or the vram data individually. They must operate globally using longword
bitwise instructions, so that performance is maintained.

Consider for instance a logical and operation (`(a, b) -> a & b`).
Operating on pixels would need to move some data, test the value of a bit in
the mask, edit the vram data, and eventually shift both the data and the mask, for all of the 32 pixels.
One could not expect this from happening in less than 150 processor cycles
(in comparison, using generic-purpose `setPixel()`-like functions would be at
least 10 times as long). The smarter method operates directly on the longword
parameters, and performs `data = data & ~mask`, which is 2 processor cycles
long.

The following operations are defined by *bopti*:

- `Draw    `: Draws black pixels.
- `Alpha   `: Erases non-transparent pixels.
- `Change  `: Changes the pixels' color.
- `Lighten `: Lightens gray pixels.
- `Lighten2`: Lightens gray pixels more.
- `Darken  `: Darkens gray pixels.
- `Darken2 `: Darkens gray pixels more.

To perform an operation, *bopti* uses the mask data, which is taken from a
layer, and calls the associated operation function. Every operation has its
default layer mask (except `change`), but this setting may be overridden.
*bopti* allows user programs to use any monochrome image as a mask for an
operation. For instance, a black rectangle may be drawn by any of the operation
functions, resulting in various results.

An additional operation, `fill`, is defined by the library. It does all the job
necessary to render the full image, which often falls back to performing the
operations that correspond to the kind of image.



---



## Operation on gray pixels

*Detailed article: [Gray engine](gray-engine)*

Gray pixels are made of one four colors, each of which is represented by two
bits. Arguments `light` and `dark` of gray operation functions are longwords
containing the least significant and most significant of these bits,
respectively.

	white     = 0 [00]
	lightgray = 1 [01]
	darkgray  = 2 [10]
	black     = 3 [11]

The `Lighten` operation affects pixels as if decrementing their value (white
pixels are not changed), and `darken` does the opposite (black pixels are not
changed).
Operations `Lighten2` and `darken2` do the same two times.

From this description, and considering two bits `light` and `dark`, it follows
that:

```c
lighten2 (light, dark) = (0, light & dark)
lighten  (light, dark) = (light & dark, light & ~dark)
darken   (light, dark) = (light | dark, light | ~dark)
darken2  (light, dark) = (1, light | dark)
```

This does not take account of a possible operation mask. See section
[Operation functions](#operation-functions) for more flexible functions.



---



## Partial transparency

*bopti* allows monochrome images to have semi-transparent pixels. Consider for
example a white background. An opaque black pixel will render black, while a
1/3-transparent black pixel will render dark gray, and a 2/3-transparent black
pixel will render light gray. Which means that:

* 1/3-transparent white pixels form the mask for `lighten2`
* 2/3-transparent white pixels form the mask for `lighten`
* 2/3-transparent black pixels form the mask for `darken`
* 1/3-transparent black pixels form the mask for `darken2`

Partial transparency on gray pixels is not allowed. Apart from the complexity
of the generic partial transparency rendering operation, semi-transparent gray
pixels are not of any use.



---



## Operation functions

Operations on monochrome buffers are defined as functions of two parameters:
the vram data longword to update, `data`, and the operation mask, `x`. Every
of these functions must satisfy `f(data, 0) = data`.

Operations on gray buffers take three arguments: `light` and `dark`, which are
longwords from the [gray buffers](gray-engine), and the operation mask `x`.
They update both longwords and return them. These functions must satisfy
`f(light, dark, 0) = (light, dark)`.

The functions for each of the operations are the following:

~~~c
# Draw function
draw(data, x) = data | x

# Alpha function
alpha(data, x) = data & ~x

# Change function
change(data, x) = data ^ x

# Lighten function
lighten(light, dark, x) = (light & (dark | ~x), (light | ~x) & (x ^ dark))

# Lighten2 function
lighten2(light, dark, x) = (light & ~x, (light | ~x) & dark)

# Darken function
darken(light, dark, x) = (light | (dark & x), (light & x) | (x ^ dark))

# Darken2 function
darken2(light, dark, x) = (light | x, (light & x) | dark)
~~~

One could easily check that these functions do their jobs when `x  = 1` and
leave the data unchanged when `x = 0`.



---



## Image format

Images are made of *layers*, each of which describe the mask for an operation.
When an image is rendered, *bopti* draws some of those layers in the vram
using the operation functions.

* Non-transparent monochrome images only have one layer, which describes the mask for the `draw` operation.
* Transparent monochrome images have two layers. The first describes the mask
for the `draw` operation, while the other is the `alpha` operation mask (which
means that it indicates which pixels are transparent).
* Non-transparent gray images also have two layers: one for each
[gray buffer](gray-engine). Both are for the `draw` operation.
* Transparent gray images have three layers. Two of them constitute the two-bit
color information for the `draw` operation, and the third is the `alpha`
operation mask.
* Semi-transparent monochrome images also have three layers. Two are used to
store the two-bit transparency level information (0 is opaque, 3 is fully
transparent), and the third indicates the color.

Layers are encoded as a bit map. The image is split into a *grid*, which is
made of 32-pixel *columns*, and an *end*.

	   32     32     32    e
	+------+------+------+---+
	|      |      |      |   |
	|      |      |      |   |
	|      |      |      |   |
	+------+------+------+---+
	
	          Bitmap

The first bytes of the layer data is the column data. Each column is encoded
as a 32-bit integer array from top to bottom. Columns are written from left to
right. The end is encoded as an 8-bit or 16-bit integer array depending on its
size, and written from top to bottom. Additionally, 0 to 3 NUL (0x00) bytes are
added to make the layer size a multiple of 4 (to allow 32-bit access to the
column data of the following layer).

In case of big images (see the image structure below), the end is expanded to
a 32-pixel column to improve performance.

The image itself is a structure of the following kind (in case of small
images):

```c
struct Image
{
	unsigned char magic;
	unsigned char format;

	unsigned char width;
	unsigned char height;

	const uint32_t data[];

} __attribute__((aligned(4)));
```

For bigger images (`width` > 255 or `height` > 255), both `width` and `height`
are set to `0` and the actual size information is written on two shorts just
where the data resides:

```c
struct BigImage
{
	unsigned char magic;
	unsigned char format;

	unsigned char null_width;       /* contains 0 */
	unsigned char null_height;      /* contains 0 */

	unsigned short width;
	unsigned short height;

	const uint32_t data[];

} __attribute__((aligned(4)));
```

This does not create a memory loss because a two-byte gap was needed to make
the data 4-aligned.

* The `magic` number, which is common to all the file formats of *gint*,
identifies the file type and version of the structure. *bopti* will not render
an image which is not encoded for its specific version.

* The `format` attribute describes the layer distribution, as specified by the
following enum:

	```c
	enum ImageFormat
	{
		ImageFormat_Mono                = 0x01,
		ImageFormat_MonoAlpha           = 0x09,
		ImageFormat_Gray                = 0x06,
		ImageFormat_GrayAlpha           = 0x0e,
		ImageFormat_GreaterAlpha        = 0x31,

		ImageFormat_ColorMask           = 0x07,
		ImageFormat_AlphaMask           = 0x38,
	};
	```

	`Alpha` refers to uniform transparency. The only format that supports
	partial transparency is `GreaterAlpha`, and it is always encoded as
	monochrome (because using gray pixels would lead to 9 different colors,
	which is rather unoptimized). Gray images with partial transparency
	will be refused by *fxconv*.


* The `width` and `height` attributes are exactly what you expect.

* The `data` is simply made of all the layers put one after another. Layers are
put in the following order:

		[0]  Monochrome `draw` layer
		[1]  Dark gray `draw` layer
		[2]  Light gray `draw` layer
		[3]  Uniform `alpha` layer
		[4]  First semi-`alpha` layer
		[5]  Second semi-`alpha` layer

	Not every format uses the six layers, of course. The layers used by
	each format may be found by reading the position of the `1`'s in the
	corresponding `enum ImageFormat` entry. Layers that are not needed are
	skipped.
