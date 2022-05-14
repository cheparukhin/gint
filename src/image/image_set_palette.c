#include <gint/image.h>
#include <stdlib.h>

void image_set_palette(image_t *img, uint16_t *palette, int size, bool owns)
{
    if(!img || !IMAGE_IS_INDEXED(img))
        return;
    if(img->flags & IMAGE_FLAGS_PALETTE_OWN)
        free(img->palette);

    img->palette = palette;
    img->color_count = size;

    if(owns)
        img->flags |= IMAGE_FLAGS_PALETTE_OWN;
    else
        img->flags &= ~IMAGE_FLAGS_PALETTE_OWN;
}
