#include <gint/image.h>
#include <stdlib.h>
#include <string.h>

uint16_t *image_copy_palette(image_t const *img)
{
    int size = image_palette_size(img);
    if(size < 0 || !img->palette)
        return NULL;

    void *palette = malloc(size);
    if(!palette)
        return NULL;

    return memcpy(palette, img->palette, size);
}
