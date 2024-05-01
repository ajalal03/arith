#include "convert.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "pnm_ybr.h"
#include <mem.h>

#define DENOM 255

/* range_helper
 * returns bottom if n is less than bottom, top if more than top, n otherwise
 */
float range_helper(float n, float bottom, float top)
{
    if (n < bottom) {
        n = bottom;
    }
    else if (n > top) {
        n = top;
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
    cl->denom = img->denominator;
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
    if ((col > new_cl->methods->width(new_cl->arr)) ||
            (row > new_cl->methods->height(new_cl->arr))) {
        return;
    }
    Pnm_ybr ybr = new_cl->methods->at(new_cl->arr, col, row);
    Pnm_rgb rgb = el;
    float r = (rgb->red * 1.0)/(new_cl->denom);
    float g = (rgb->green * 1.0)/(new_cl->denom);
    float b = (rgb->blue * 1.0)/(new_cl->denom);
    float y = .299 * r + .587 * g + .114 * b;
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
    int width = (methods->width(img));
        int height = (methods->height(img));
        //printf("HELLO\n");
        /* set denominator by ourself. We can try different denominator 
         * values. */
        unsigned denominator = DENOM;
        /* allocate memory for Pnm_ppm image. */
        Pnm_ppm rgb_image;
        NEW(rgb_image);

        /* set members values of Pnm_ppm image. */
        rgb_image->width  = width;
        rgb_image->height = height;
        rgb_image->denominator = denominator;
        rgb_image->methods = methods;
        rgb_image->pixels = methods->new(width, height, 
                                        sizeof(struct Pnm_rgb));
        
        /* closure for apply function contains 2D RGB pixels, methods and 
         * denominator. */      
        struct a2_closure *my_cl = malloc(sizeof(*my_cl));
        my_cl->methods = methods;
        my_cl->arr = rgb_image->pixels;
        my_cl->denom = denominator;

        /* use the map function to transfer each component video color pixel 
         * to RGB color pixel. */
        map(img, apply_v_to_rgb, my_cl);
        /* free the closure. */
        // if(my_cl != NULL){
        //         free(my_cl);
        //         my_cl = NULL;
        // }

        return img;
}

/* apply_v_to_rgb
 * sets Pnm_ybr el to its corresponding Pnm_rgb counterpart by saving results
 * to *cl's arr
 */
void apply_v_to_rgb(int col, int row, A2Methods_UArray2 arr, void *el,
    void *cl)
{
    (void) arr;
        struct a2_closure *mcl = cl;

        Pnm_ybr ybr_pixel = el;
        Pnm_rgb rgb_pixel = mcl->methods->at(mcl->arr, col, row);
        /* compute red, green, blue integers based on y, Pb, Pr values. */
        rgb_pixel->red =   (uint64_t)((mcl->denom) * 
                               range_helper(1.0 * (ybr_pixel->y) + 0.0 * 
                               (ybr_pixel->pb) + 1.402 * (ybr_pixel->pr),0,DENOM));
                               

        rgb_pixel->green = (uint64_t)((mcl->denom) * 
                               range_helper(1.0 * (ybr_pixel->y) - 0.344136 * 
                               (ybr_pixel->pb) - 0.714136 * (ybr_pixel->pr),0,DENOM));
        rgb_pixel->blue =  (uint64_t)((mcl->denom) * 
                               range_helper(1.0 * (ybr_pixel->y) + 1.772 * 
                               (ybr_pixel->pb) + 0.0 * (ybr_pixel->pr),0,DENOM));
}
