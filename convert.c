#include "convert.h"
#include <stdio.h>
#include <stdlib.h>

#define DENOM 255

/* range_helper
 * returns bottom if n is less than bottom, top if more than top, n otherwise
 */
static float range_helper(float n, float bottom, float top)
{
    if (val < bottom) {
        val = bottom;
    }
    else if (val > top) {
        val = top;
    }
    return n;
}

/* rgb_to_v
 * returns a UArray2 as specified by methods->new where every pixel is a
 * video encoding of img's rbg values
 */
A2Methods_UArray2 rgb_to_v(Pnm_ppm img, A2Methods_mapfun *map,
    A2Methods_T methods)
{
    int width = img->width;
    int height = img->height;
    A2Methods_UArray2 v_img = methods->new(width, height,
        sizeof(struct Pnm_ybr));
    struct a2_closure *cl = malloc(sizeof(*cl));
    cl->arr = v_img;
    cl->denominator = img->denominator;
    cl->methods = methods;
    map(img->pixels, apply_rgb_to_v, cl);
    if (cl != NULL) {
        free(cl);
    }
    return v_img;
}

/* apply_rgb_to_v
 * sets Pnm_rgb el to its corresponding Pnm_ybr counterpart by saving results
 * to *cl's arr
 */
void apply_rgb_to_v(int col, int row, A2Methods_UArray2 arr, void *el,
    void *cl)
{
    (void)arr;
    struct a2_closure *new_cl = cl;
    if ((col > new_cl->methods->width(new_cl->array2)) ||
            (row > new_cl->methods->height(new_cl->array2))) {
        return;
    }
    Pnm_ybr ybr = new_cl->methods->at(new_cl->arr, col, row);
    Pnm_rgb rgb = el;
    float r = (rgb->red * 1.0)/(new_cl->denom);
    float g = (rgb->green * 1.0)/(new_cl->denom);
    float b = (rgb->blue * 1.0)/(new_cl->denom);
    float y = .299 * r + .587 * g + .224 * b;
    float pb = -.168736 * r - .331264 * g + .5 * b;
    float pr = .5 * r + -.418688 * g - .081312 * b;
    ybr->y = range_helper(y, 0, 1);
    ybr->pb = range_helper(pb, -0.5, 0.5);
    ybr->pr = range_helper(pr, -0.5, 0.5);
}

/* v_to_rgb
 * returns a UArray2 as specified by methods->new where every pixel is a
 * rbg pixel of video encoding from img's array2
 */
Pnm_ppm v_to_rgb(A2Methods_UArray2 img, A2Methods_mapfun *map,
    A2Methods_T methods)
{
    int width = methods->width(img);
    int height = methods->height(img);
    unsigned denom = DENOM;
    Pnm_ppm rgb;
    NEW(rgb);
    rgb->width = width;
    rgb->height = height;
    rgb->denominator = denom;
    rgb->methods = methods;
    rgb->pixels = methods->new(width, height, sizeof(struct Pnm_rgb));
    struct a2_closure *cl = malloc(sizeof(*cl));
    cl->arr = rgb->pixels;
    cl->denom = denom;
    cl->methods = methods;
    map(img, apply_v_to_rgb, cl);
    if (cl != NULL) {
        free(cl);
    }
    return rgb;
}

/* apply_v_to_rgb
 * sets Pnm_ybr el to its corresponding Pnm_rgb counterpart by saving results
 * to *cl's arr
 */
void apply_v_to_rgb(int col, int row, A2Methods_UArray2 arr, void *el,
    void *cl)
{
    (void)arr;
    struct a2_closure *new_cl = cl;
    Pnm_ybr ybr = el;
    Pnm_rgb rgb = new_cl->methods->at(new_cl->arr, col, row);
    float r = 1.0 * ybr->y + 0.0 * ybr->pb + 1.402 * ybr->pr;
    float g = 1.0 * ybr->y - 0.344136 * ybr->pb - 0.714136 * ybr->pr;
    float b = 1.0 * ybr->y + 1.772 * ybr->pb + 0.0 * ybr->pr;
    rgb->red = range_helper(r * ybr->denom, 0.0, 1.0);
    rgb->green = range_helper(g * ybr->denom, 0.0, 1.0);
    rgb->blue = range_helper(b * ybr->denom, 0.0, 1.0);
}
