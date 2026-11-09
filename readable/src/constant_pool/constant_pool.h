/* SPDX-License-Identifier: MIT */
/* The constant pools, laid out as the listings have them:
 *
 *   FIXED_FIXED_CONSTANT_POOL.agc (LM pp. 1095-1099, CM pp. 1200-1204),
 *     the constants every bank reaches with a CAF
 *     (fixed_fixed_constant_pool.c);
 *   INTERPRETIVE_CONSTANT.agc (LM pp. 1100-1101; the CM's
 *     INTERPRETIVE_CONSTANTS.agc, pp. 1205-1206), the vectors and double
 *     words interpretive code reaches, once in each half of the rope
 *     (interpretive_constants.c). */
#ifndef READABLE_CONSTANT_POOL_H
#define READABLE_CONSTANT_POOL_H

#include <stdbool.h>
#include "../agc/agc.h"

/* A word of a pool: its label in the listing ("" for a word with none,
 * such as a double word's second) and its value. */
struct pool_word {
    const char *label;
    word value;
};

/* How the pools write their words, in the listing's notation. */
/* clang-format off */
#define WORD(label, value)          {label, (word)(value)}
#define DOUBLE(label, major, minor) {label, (word)(major)}, {"", (word)(minor)}
#define MINUS(n)                    (077777u - (n))
#define VN(verb, noun)              ((verb) * 0200u + (noun))
#define COUNT(array)                (sizeof(array) / sizeof((array)[0]))
/* clang-format on */

/* The pool's word at a fixed-fixed address, false where the pool has none
 * (the listing's commented-out lines take no address). */
bool fixed_fixed_constant(word address, word *value);

/* The word's label in the listing, or NULL off the pool. */
const char *fixed_fixed_constant_label(word address);

/* The interpretive constants' word at a rope address (the bank * 02000
 * plus the word's offset in the bank, as I_ names give it), false off
 * them. */
bool interpretive_constant(word address, word *value);

/* The word's label in the listing ("" for none), or NULL off them. */
const char *interpretive_constant_label(word address);

#endif
