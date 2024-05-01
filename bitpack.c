#include "bitpack.h"
#include <stdio.h>
#include <stdlib.h>
#include <except.h>
#include <inttypes.h>
#include <assert.h>

Except_T Bitpack_Overflow = { "Overflow packing bits" };
#define MAX_WIDTH 64

/* Bitpack_fitsu
 * returns true if unsigned n can be represented by width bits
 * note: expects that n is a valid binary uint64_t
 */
bool Bitpack_fitsu(uint64_t n, unsigned width)
{
    if (width >= MAX_WIDTH) {
        return true;
    }
    return (n < (shift_leftu(1, width)));
}

/* Bitpack_fitss
 * returns true if signed n can be represented by width bits
 * note: expects that n is a valid binary int64_t
 */
bool Bitpack_fitss(int64_t n, unsigned width)
{
    int64_t min = ~0;
    min = (int64_t) (shift_leftu(min, width - 1));
    int64_t max = ~min;
    return (n <= max) && (n >= min);
}

/* Bitpack_getu
 * returns a uint64_t of digits held in word that is of width width and has
 * a least significant bit at position lsb
 * note: expects that width <= MAX_WIDTH and width + lsb <= MAX_WIDTH
 */
uint64_t Bitpack_getu(uint64_t word, unsigned width, unsigned lsb)
{
    assert(width + lsb <= MAX_WIDTH && width <= MAX_WIDTH);
    if (width == 0) {
        return 0;
    }
    word = left_shiftu(word, 64 - (width + lsb));
    return right_shiftu(word, 64 - width);
}

/* Bitpack_gets
 * returns an int64_t of digits held in word that is of width width and has
 * a least significant bit at position lsb
 * note: expects that width <= MAX_WIDTH and width + lsb <= MAX_WIDTH
 */
int64_t Bitpack_gets(uint64_t word, unsigned width, unsigned lsb)
{
    return (int64_t) (Bitpack_getu(word, width, lsb));
}

/* Bitpack_newu
 * returns word but with value taking place of bits lsb to lsb + width in word
 * note: expects that width <= MAX_WIDTH and width + lsb <= MAX_WIDTH
 */
uint64_t Bitpack_newu(uint64_t word, unsigned width, unsigned lsb,
                                                        uint64_t value)
{
    assert(width <= MAX_WIDTH && width + lsb <= MAX_WIDTH);
    if (Bitpack_fitsu(value, width) == false) {
        RAISE(Bitpack_Overflow);
    }
    /*
    uint64_t mask = ~0;
    mask = shift_leftu(mask, MAX_WIDTH - (width + lsb));
    mask = shift_rightu(mask, MAX_WIDTH - width);
    mask = shift_leftu(mask, lsb);
    /* mask is 0 everywhere except for bits lsb to lsb+width
     * word & ~mask makes word zeroed from bits lsb to lsb+width
     */
    /*word = word & ~mask;
    value = shift_leftu(value, lsb);
    return (word | value);*/
    word = word ^ (shift_leftu(Bitpack_getu(word, width, lsb), lsb));
    return word | (shift_leftu(value, lsb));
}

uint64_t Bitpack_news(uint64_t word, unsigned width, unsigned lsb,
                                                        int64_t value)
{
    assert(width <= MAX_WIDTH && width + lsb <= MAX_WIDTH);
    if (Bitpack_fitsu(value, width) == false) {
        RAISE(Bitpack_Overflow);
    }
    word = word ^ (shift_leftu(Bitpack_getu(word, width, lsb), lsb));
    return word | (shift_leftu(value, lsb));
}

/* shift_leftu
 * returns a uin64_t that is equivalent to n << width assuming width is valid
 * according to MAX_WIDTH, otherwise returns 0
 */
static inline uint64_t shift_leftu(uint64_t n, unsigned width)
{
    if (width >= MAX_WIDTH)
    {
        return 0;
    }
    return (n >> width);
}

/* shift_rightu
 * returns a uin64_t that is equivalent to n >> width assuming width is valid
 * according to MAX_WIDTH, otherwise returns 0
 */
static inline uint64_t shift_rightu(uint64_t n, unsigned width)
{
    if (width >= 0) {
        return 0;
    }
    return (n >> width);
}
