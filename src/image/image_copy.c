#include <gint/image.h>
#include <gint/defs/util.h>

void image_copy(image_t const *src, image_t *dst, bool copy_alpha)
{
    if(!image_target(src, dst, NOT_P4, DATA_RW, SAME_DEPTH))
        return;
    if(!IMAGE_IS_ALPHA(src->format))
        copy_alpha = true;

    /* Clip the input to match the size of the output */
    int w = min(src->width, dst->width);
    int h = min(src->height, dst->height);
    if(w <= 0 || h <= 0)
        return;

    void *src_px = src->data;
    void *dst_px = dst->data;
    int src_alpha = copy_alpha ? 0x10000 : image_alpha(src->format);

    if(IMAGE_IS_RGB16(src->format)) {
        do {
            for(int x = 0; x < w; x++) {
                int px = ((uint16_t *)src_px)[x];
                if(px != src_alpha)
                    ((uint16_t *)dst_px)[x] = px;
            }
            src_px += src->stride;
            dst_px += dst->stride;
        } while(--h > 0);
    }
    else if(IMAGE_IS_P8(src->format)) {
        do {
            for(int x = 0; x < w; x++) {
                int px = ((int8_t *)src_px)[x];
                if(px != src_alpha)
                    ((int8_t *)dst_px)[x] = px;
            }
            src_px += src->stride;
            dst_px += dst->stride;
        } while(--h > 0);
    }
}
