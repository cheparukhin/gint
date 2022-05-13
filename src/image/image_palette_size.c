#include <gint/image.h>

int image_palette_size(image_t const *img)
{
    return (img->color_count >= 0) ? img->color_count* 2 : -1;
}

