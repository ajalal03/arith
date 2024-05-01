#include "compress40.h"
#include "arith40.h"
#include "convert.h"
#include "Pnm_ybr.h"
#include "pnm.h"
#include <inttypes.h>
#include <stdio.h>

#define DENOM 255
#define PR_LSB 0
#define PB_LSB 4
#define D_LSB 8
#define C_LSB 13
#define B_LSB 18
#define A_LSB 23

struct pix_block {
    int pb, pr, a, b, c, d;
} *pix_block;

void compress40(FILE *fp)
{
    assert(fp != NULL);
    A2Methods_T methods = uarray2_methods_plain;
    A2Methods_mapfun *map = methods->map_default;
    Pnm_ppm rgb = Pnm_ppmread(fp, methods);
    rgb->height = rgb->height - (rgb->height % 2)
    rgb->width = rgb->width - (rgb->width % 2)
    A2Methods_UArray2 v_img = rgb_to_v(rgb, map, methods);
    printf("COMP40 Compressed image format 2\n%u %u", v_img->width,
        v_img->height);
    map(v_img->pixels, compress_to_words, &v_img);
    Pnm_ppmfree(&rgb);
    Pnm_ppmfree(&v_img);
}

void decompress40(FILE *fp)
{
    assert(fp != NULL);
    unsigned width, height;
    int read = fscanf(fp, "COMP40 Compressed image format2\n%u %u", &width,
        &height);
    assert(read == 2);
    int c = getc(input);
    assert(c == '\n');
    A2Methods_T methods = uarray2_methods_plain;
    A2Methods_UArray2 arr = methods->new(width, height, sizeof(struct Pnm_rgb));
    struct Pnm_ppm pix = {
        .width = width;
        .height = height;
        .denominator = DENOM;
        .pixels = arr;
        .methods = methods;
    };
    methods->map(pix.pixels, decompress_words, fp);
    methods->map(pix.pixels, v_to_rgb, methods);
    Pnm_ppmwrite(stdout, &pix);
    methods->free(&arr);
}

pix_block unpack(FILE *fp)
{
    assert(fp != NULL);
    uint64_t word = 0;
    unsigned char c = getc(fp);
    word = Bitpack_newu(word, 8, 0, (uin64_t)c);
    c = getc(fp);
    word = Bitpack_newu(word, 8, 8, (uin64_t)c);
    c = getc(fp);
    word = Bitpack_newu(word, 8, 16, (uin64_t)c);
    c = getc(fp);
    word = Bitpack_newu(word, 8, 24, (uin64_t)c);
    pix_block ret;
    ret.pr = Bitcpack_getu(word, 4, PR_LSB);
    ret.pr = Bitcpack_getu(word, 4, PB_LSB);
    ret.pr = Bitcpack_gets(word, 4, D_LSB);
    ret.pr = Bitcpack_gets(word, 4, C_LSB);
    ret.pr = Bitcpack_gets(word, 4, B_LSB);
    ret.pr = Bitcpack_getu(word, 4, A_LSB);
    return ret;
}

void decompress_words(int col, int row, A2Methods_UArray2 arr, void *el,
    void *cl)
{
    (void)el;
    if (i % 2 == 0 && j % 2 == 0) {
        pix_block block = unpack((FILE *) cl);
        A2Methods_T methods = uarray2_methods_plain;
        Pnm_ybr one, two, three, four;
        float a = (float) block.a / (float) 511;
        float b = (float) (block.b / 50);
        float c = (float) (block.c / 50);
        float d = (float) (block.d / 50);
        one.y = fmin(fmax(a - b - c + d, 0), 1);
        two.y = fmin(fmax(a - b + c - d, 0), 1);
        three.y = fmin(fmax(a + b - c - d, 0), 1);
        four.y = fmin(fmax(a + b + c + d, 0), 1);
        float pb = Arith40_chroma_of_index(block.pb);
        float pr = Arith40_chroma_of_index(block.pr);
        one.pb = pb;
        two.pb = pb;
        three.pb = pb;
        four.pb = pb;
        one.pr = pr;
        two.pr = pr;
        three.pr = pr;
        four.pr = pr;
    }
}

int float_to_dct(float n)
{
    n = 50 * range_helper(n, -0.3, 0.3);
}

pix_block pack(Pnm_ybr one, Pnm_ybr two, Pnm_ybr three, Pnm_ybr four)
{
    float avg_pb, avg_pr, a, b, c, d;
    avg_pb = (one.pb + two.pb + three.pb + four.pb) / 4;
    avg_pr = (one.pr + two.pr + three.pr + four.pr) / 4;
    avg_pb = Arith40_index_of_chroma(avg_pb);
    avg_pr = Arith40_index_of_chroma(avg_pr);
    a = (one.y + two.y + three.y + four.y) / 4;
    b = (four.y + three.y - two.y - one.y) / 4;
    c = (two.y - one.y - three.y + four.y) / 4;
    d = (one.y - two.y - three.y + four.y) / 4;
    int a_ret = float_to_dct(a);
    int b_ret = float_to_dct(b);
    int c_ret = float_to_dct(c);
    int d_ret = float_to_dct(d);
    pix_block ret;
    ret.a = a_ret;
    ret.b = b_ret;
    ret.c = c_ret;
    ret.d = d_ret;
    ret.pb = pb;
    ret.pr = pr;
    return ret;
}

void word_out(pix_block word)
{
    uint64_t out = 0;
    out = Bitpack_newu(out, 4, PR_LSB, (uint64_t)out.pr);
    out = Bitpack_newu(out, 4, PB_LSB, (uint64_t)out.pb);
    out = Bitpack_news(out, 5, D_LSB, (uint64_t)out.d);
    out = Bitpack_news(out, 5, C_LSB, (uint64_t)out.c);
    out = Bitpack_news(out, 5, B_LSB, (uint64_t)out.b);
    out = Bitpack_newu(out, 9, A_LSB, (uint64_t)out.a);
    putchar(Bitpack_getu(out, 8, 0));
    putchar(Bitpack_getu(out, 8, 8));
    putchar(Bitpack_getu(out, 8, 16));
    putchar(Bitpack_getu(out, 8, 24));
}

void compress_to_words(int col, int row, A2Methods_UArray2 arr, void *el,
    void *cl)
{
    Pnm_ppm v_img = *(Pnm_ppm *) cl;
    const struct A2Methods_T *methods = v_img->methods;
    float denom = v_img->denominator;
    pix_block word;
    (void)el;
    (void)arr;
    if (row % 2 == 0 && col % 2 == 0) { // ?
        Pnm_ybr one = v_img->methods->at(v_img->pixels, i, j);
        Pnm_ybr two = v_img->methods->at(v_img->pixels, i + 1, j);
        Pnm_ybr three = v_img->methods->at(v_img->pixels, i, j + 1);
        Pnm_ybr four = v_img->methods->at(v_img->pixels, i + 1, j + 1);
        word = pack(one, two, three, four);
        word_out(word);
    }
}
