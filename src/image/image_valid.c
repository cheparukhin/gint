#include <gint/image.h>

bool image_valid(image_t const *img)
{
    if(!img)
        return false;

    if(IMAGE_IS_RGB16(img->format)) {
        return (img->data != NULL);
    }
    if(IMAGE_IS_P8(img->format) || IMAGE_IS_P4(img->format)) {
        return (img->data != NULL) && (img->palette != NULL);
    }

    /* Invalid format */
    return false;
}
