#ifndef QUANTIZATION_H_INCLUDED
#define QUANTIZATION_H_INCLUDED

struct a2_closure {
    A2Methods_UArray2 *arr;
    A2Methods_T methods;
    unsigned denom;
};

extern A2Methods_UArray2 rgb_to_v(Pnm_ppm img, A2Methods_mapfun *map,
    A2Methods_T methods);

extern void apply_rgb_to_v(int col, int row, A2Methods_UArray2 arr, void *el,
    void *cl);

extern Pnm_ppm v_to_rgb(A2Methods_UArray2 img, A2Methods_mapfun *map,
    A2Methods_T methods);

extern void apply_v_to_rgb(int col, int row, A2Methods_UArray2 arr, void *el,
    void *cl)l

#endif
