#include <gint/image.h>
#include <stdlib.h>
#include <gint/defs/util.h>

image_t *image_alloc(int width, int height, int format,
    void *palette, int palette_size, int alpha)
{
    image_t *img = image_create(width, height, format, alpha);
    if(!img)
        return NULL;

    if(IMAGE_IS_RGB16(format)) {
        img->stride = ((width + 1) >> 1) * 4;
        palette_size = -1;
    }
    else if(IMAGE_IS_P8(format)) {
        img->stride = width;
        palette_size = max(0, min(256, palette_size));
    }
    else if(IMAGE_IS_P4(format)) {
        img->stride = ((width + 1) >> 1);
        palette_size = 32;
    }

    void *data = malloc(height * img->stride);
    if(!data) {
        image_free(img);
        return NULL;
    }

    img->data = data;
    img->flags |= IMAGE_FLAGS_DATA_ALLOC;

    if(!palette && palette_size > 0) {
        palette = malloc(palette_size * 2);
        if(!palette) {
            image_free(img);
            return NULL;
        }
    }

    img->palette = palette;
    img->color_count = palette_size;
    return img;
}
