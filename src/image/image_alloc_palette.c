#include <gint/image.h>
#include <gint/defs/util.h>
#include <stdlib.h>

bool image_alloc_palette(image_t *img, int size)
{
    if(!img || !IMAGE_IS_INDEXED(img))
        return;
    if(img->flags & IMAGE_FLAGS_PALETTE_OWN)
        free(img->palette);

    if(IMAGE_IS_P8(img)) {
        size = (size <= 0) ? 256 : min(size, 256);
    }
    if(IMAGE_IS_P4(img)) {
        size = 16;
    }

    img->palette = calloc(size, 2);
    img->color_count = 0;
    img->flags &= ~IMAGE_FLAGS_PALETTE_OWN;

    if(!img->palette)
        return false;

    memset(img->palette, 0, 2*size);
    img->color_count = size;
    img->flags |= IMAGE_FLAGS_PALETTE_OWN;
    return true;
}
