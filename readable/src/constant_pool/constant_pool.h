/* SPDX-License-Identifier: MIT */
/* FIXED_FIXED_CONSTANT_POOL.agc (LM pp. 1095-1099, CM pp. 1200-1204): the
 * constants every bank reaches with a CAF, laid out as the listing has
 * them (fixed_fixed_constant_pool.c). */
#ifndef READABLE_CONSTANT_POOL_H
#define READABLE_CONSTANT_POOL_H

#include <stdbool.h>
#include "../agc/agc.h"

/* The pool's word at a fixed-fixed address, false where the pool has none
 * (the listing's commented-out lines take no address). */
bool fixed_fixed_constant(word address, word *value);

/* The word's label in the listing, or NULL off the pool. */
const char *fixed_fixed_constant_label(word address);

#endif
