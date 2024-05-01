#include "bitpack.h"
#include <stdio.h>
#include <stdlib.h>
#include <except.h>
#include <inttypes.h>
#include <assert.h>
#include <stdbool.h>
#include "shift.h"

Except_T Bitpack_Overflow = { "Overflow packing bits" };
#define MAX_WIDTH 64

/* Bitpack_fitsu
 * returns true if unsigned n can be represented by width bits
 * note: expects that n is a valid binary uint64_t
 */
bool Bitpack_fitsu(uint64_t n, unsigned width)
{
    assert(width <= MAX_WIDTH);
    if (width == MAX_WIDTH) {
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
    assert(width <= MAX_WIDTH);
    if (width == MAX_WIDTH) {
        return true;
    }
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
    if (width == MAX_WIDTH) {
        return word;
    }
    assert(width + lsb <= MAX_WIDTH && width <= MAX_WIDTH);
    if (width == 0) {
        return 0;
    }
    word = shift_leftu(word, MAX_WIDTH - (width + lsb));
    return shift_rightu(word, MAX_WIDTH - width);
}

/* Bitpack_gets
 * returns an int64_t of digits held in word that is of width width and has
 * a least significant bit at position lsb
 * note: expects that width <= MAX_WIDTH and width + lsb <= MAX_WIDTH
 */
int64_t Bitpack_gets(uint64_t word, unsigned width, unsigned lsb)
{
    if (width == MAX_WIDTH) {
        return word;
    }
    assert(width + lsb <= MAX_WIDTH && width <= MAX_WIDTH);
    if (width == 0) {
        return 0;
    }
    uint64_t val = Bitpack_getu(word, width, lsb);
    /* check for negative */
    if (Bitpack_getu(word, 1, lsb + width - 1) == 1) {
        /* make preceding 1s */
        uint64_t mask = shift_leftu(shift_leftu((uint64_t) 0, width) - 1, lsb);
        word = shift_rightu(word & mask, lsb);
        return (int64_t) shift_leftu(~((uint64_t) 0), width) | word;
    }
    return (int64_t) val;
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
    /* 0 out word at correct bit positions */
    word = word ^ (shift_leftu(Bitpack_getu(word, width, lsb), lsb));
    /* insert value */
    return word | (shift_leftu(value, lsb));
}

uint64_t Bitpack_news(uint64_t word, unsigned width, unsigned lsb,
                                                        int64_t value)
{
    assert(width <= MAX_WIDTH && width + lsb <= MAX_WIDTH);
    if (Bitpack_fitss(value, width) == false) {
        RAISE(Bitpack_Overflow);
    }
    /* 0 out word at correct bit positions */
    word = word ^ (shift_leftu(Bitpack_getu(word, width, lsb), lsb));
    /* insert value */
    return word | (shift_leftu(Bitpack_getu(value, width, 0), lsb));
}
