#include "compress40.h"
#include <arith40.h>
#include "pnm_ybr.h"
#include <assert.h>
#include "pnm.h"
#include "a2methods.h"
#include "a2plain.h"
#include <inttypes.h>
#include <bitpack.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "convert.h"
#include "shift.h"

#define DENOM 255
#define PR_LSB 0
#define PB_LSB 4
#define D_LSB 8
#define C_LSB 13
#define B_LSB 18
#define A_LSB 23

struct pix_block {
    uint64_t pb, pr, a;
    int64_t b, c, d;
};
typedef struct pix_block pix_block;

void decompress_words(int col, int row, A2Methods_UArray2 arr, void *el,
    void *cl);
void compress_to_words(int col, int row, A2Methods_UArray2 arr, void *el,
    void *cl);
void compress40(FILE *fp)
{
    assert(fp != NULL);
    A2Methods_T methods = uarray2_methods_plain;
    A2Methods_mapfun *map = methods->map_default;
    Pnm_ppm rgb = Pnm_ppmread(fp, methods);
    rgb->height = rgb->height - (rgb->height % 2) + 1;
    rgb->width = rgb->width - (rgb->width % 2) + 1;
    A2Methods_UArray2 v_img = rgb_to_v(rgb, map, methods);
    printf("COMP40 Compressed image format 2\n%u %u\n", rgb->width - 1,
        rgb->height - 1);
    map(v_img, compress_to_words, methods);
    Pnm_ppmfree(&rgb);
    methods->free(&v_img);
}

void decompress40(FILE *fp)
{
    assert(fp != NULL);
    unsigned width, height;
    int read = fscanf(fp, "COMP40 Compressed image format 2\n%u %u", &width, &height);
        //printf("width: %u height: %u \n",width,height);
    assert(read == 2);
    int c = getc(fp);
    assert(c == '\n');
    A2Methods_T methods = uarray2_methods_plain;
    A2Methods_UArray2 arr = methods->new(width, height, sizeof(struct Pnm_rgb));
    struct Pnm_ppm pix = {
        .width = width, .height = height, .denominator = DENOM, .pixels = arr,
        .methods = methods
    };
    methods->map_row_major(pix.pixels, decompress_words, fp);
    //v_to_rgb(pix.pixels, methods->map_row_major, methods);
    Pnm_ppmwrite(stdout, &pix);
    methods->free(&arr);
}

pix_block unpack(FILE *fp)
{

    assert(fp != NULL);
    uint64_t word = 0;
    unsigned char c = getc(fp);
    //printf("%uc", c);
    word = Bitpack_newu(word, 8, 0, (uint64_t)c);
    c = getc(fp);
    word = Bitpack_newu(word, 8, 8, (uint64_t)c);
    c = getc(fp);
    word = Bitpack_newu(word, 8, 16, (uint64_t)c);
    c = getc(fp);
    word = Bitpack_newu(word, 8, 24, (uint64_t)c);
    pix_block ret;
    ret.pr = Bitpack_getu(word, 4, PR_LSB);
    // printf(" pr is %lu\n",ret.pr);
    ret.pb = Bitpack_getu(word, 4, PB_LSB);
    // printf("pb is %lu\n",ret.pb);
    ret.d = Bitpack_gets(word, 5, D_LSB);
    // printf("d is %lu\n", ret.d);
    ret.c = Bitpack_gets(word, 5, C_LSB);
        // printf("c is %lu\n", ret.c);
    ret.b = Bitpack_gets(word, 5, B_LSB);
        // printf("b is %lu\n", ret.b);
    ret.a = Bitpack_getu(word, 9, A_LSB);
        // printf("a is %lu\n", ret.a);
    return ret;
}

void decompress_words(int col, int row, A2Methods_UArray2 arr, void *el,
    void *cl)
{

    (void)el;
    A2Methods_T methods = uarray2_methods_plain;
    if (col % 2 == 0 && row % 2 == 0) {
        pix_block block = unpack((FILE *) cl);
        //printf("pr is %lu\n", block.pr);
        Pnm_ybr one = malloc(sizeof(*one));
        Pnm_ybr two = malloc(sizeof(*two));
        Pnm_ybr three = malloc(sizeof(*three));
        Pnm_ybr four = malloc(sizeof(*four));
        float a = (float) block.a / (float) 511;
        //printf("a: %f\n",a);
        float b = (float) (block.b / 50);
        //printf("b: %f\n",b);
        float c = (float) (block.c / 50);
        //printf("c: %f\n",c);
        float d = (float) (block.d / 50);
        //printf("d: %f\n",d);
        one->y = fmin(fmax(a - b - c + d, 0), 1);
        two->y = fmin(fmax(a - b + c - d, 0), 1);
        three->y = fmin(fmax(a + b - c - d, 0), 1);
        four->y = fmin(fmax(a + b + c + d, 0), 1);
        float pb = Arith40_chroma_of_index(block.pb);
        // printf("one->y %f\n",one->y);
        // printf("two->y %f\n",two->y);
            //printf("float is %f\n", pb);
        float pr = Arith40_chroma_of_index(block.pr);
        //printf("%f\n",pb);
        one->pb = pb;
        //printf("%f",one->pb);
        two->pb = pb;
        three->pb = pb;
        four->pb = pb;
        one->pr = pr;
        two->pr = pr;
        three->pr = pr;
        four->pr = pr;
        
        *((Pnm_ybr *) methods->at(arr,col, row)) = one;
        *((Pnm_ybr *) methods->at(arr,col + 1, row)) = two;
        *((Pnm_ybr *) methods->at(arr,col, row + 1)) = three;
        *((Pnm_ybr *) methods->at(arr,col + 1, row + 1)) = four;
    }
}

int float_to_dct(float n)
{
    return round((int)(50 * range_helper(n, -0.3, 0.3)));
}

pix_block pack(Pnm_ybr one, Pnm_ybr two, Pnm_ybr three, Pnm_ybr four)
{
    float avg_pb, avg_pr, a, b, c, d;
    avg_pb = (one->pb + two->pb + three->pb + four->pb) / 4;
    avg_pr = (one->pr + two->pr + three->pr + four->pr) / 4;
    //printf("%f\n",avg_pb);
    avg_pb = Arith40_index_of_chroma(avg_pb);
        //printf("float is %f\n", avg_pb);
    avg_pr = Arith40_index_of_chroma(avg_pr);
    a = (one->y + two->y + three->y + four->y) / 4;
    b = (four->y + three->y - two->y - one->y) / 4;
    c = (two->y - one->y - three->y + four->y) / 4;
    d = (one->y - two->y - three->y + four->y) / 4;
    int a_ret = (int) round((511 * range_helper(a, 0, 1)));
    int b_ret = float_to_dct(b);
    int c_ret = float_to_dct(c);
    int d_ret = float_to_dct(d);
    pix_block ret;
    ret.a = a_ret;
    ret.b = b_ret;
    ret.c = c_ret;
    ret.d = d_ret;
    ret.pb = avg_pb;
    // printf("a: %f\n",a);
    // printf("b: %f\n",b);
    // printf("c: %f\n",c);
    // printf("d: %f\n",d);
    // printf("%lu\n",ret.pb);
    // 
    ret.pr = avg_pr;
    //printf("%lu\n",ret.pr);
    return ret;
}

void word_out(pix_block word)
{

    uint32_t out = 0;
    out = Bitpack_newu(out, 4, PR_LSB, word.pr);
    out = Bitpack_newu(out, 4, PB_LSB, word.pb);
    out = Bitpack_news(out, 5, D_LSB, word.d);
    out = Bitpack_news(out, 5, C_LSB, word.c);
    out = Bitpack_news(out, 5, B_LSB, word.b);
    out = Bitpack_newu(out, 9, A_LSB, word.a);
    unsigned char byte;
    for (int i = 0; i < 4; i++) {
        byte = shift_rightu(out, (24 - (8 * i)));//(out >> (24 - (8 * i)));
        putchar(byte);
    }
}

void compress_to_words(int col, int row, A2Methods_UArray2 arr, void *el,
    void *cl)
{
    A2Methods_T methods = cl;
    pix_block word;
    (void)el;
    // printf("a: %ld\n",word.a);
    // printf("b: %ld\n",word.b);
    // printf("c: %lu\n",word.c);
    // printf("d: %lu\n",word.d);
    if (row % 2 == 1 && col % 2 == 1) {
        Pnm_ybr one = methods->at(arr, col, row);
        Pnm_ybr two = methods->at(arr, col - 1, row);
        Pnm_ybr three = methods->at(arr, col, row - 1);
        Pnm_ybr four = methods->at(arr, col - 1, row - 1);
        word = pack(one, two, three, four);
        (void) word;
        word_out(word);
    }
}
