#include <gint/image.h>
#include "fixed.h"

void image_rotate_around_scale(image_t const *src, float angle, int gamma,
    bool resize, int *center_x, int *center_y, struct image_linear_map *map)
{
    if(!image_valid(src))
        return;

    map->src_w = src->width;
    map->src_h = src->height;

    /* Don't try to resize cleanly; just add a √2 factor in both dimensions if
       [resize=true] to make sure everything fits */
    ;
}
