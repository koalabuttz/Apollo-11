/* SPDX-License-Identifier: MIT */
/* INTERPRETIVE_CONSTANT.agc -- LM pp. 1100-1101; the CM's
 * INTERPRETIVE_CONSTANTS.agc, pp. 1205-1206. The faithful port reads these
 * words from its compiled listing data (c64-port/tools/fixed_data_generate.py).
 *
 * The vectors and double words that interpretive code names as operands.
 * The interpreter reaches a fixed operand only in the half of the rope its
 * program runs in, so the listing lays the constants out twice: once at
 * INTPRET1 (bank 23 in the LM, 26 in the CM) and again, "INTERPRETIVE
 * CONSTANTS IN THE OTHER HALF-MEMORY", at INTPRET2 (bank 12 in the LM, 04
 * in the CM).
 *
 * The unit vectors share their words. A vector is three double words, so
 * from UNITZ the words read 0, 0, .5 (the Z axis), from UNITY 0, .5, 0
 * and from UNITX .5, 0, 0, its last two double words the first of
 * ZEROVECS, the zero vector. The other half has the same as ZUNIT, YUNIT,
 * XUNIT and ZEROVEC.
 *
 * Each word is written as the listing has it: a 2DEC or 2OCT as
 * DOUBLE(label, major, minor), its two words, with the 2DEC in the
 * comment; an OCT as the octal number; MINUS(n) for a negative DEC (its
 * one's complement). A word with no label has "". */
#include <stddef.h>
#include "constant_pool.h"
#include "../agc/names.h"

/* ---- INTPRET1, from DP1/4TH (p. 1100; CM p. 1205) ---- */

static const struct pool_word intpret1[] = {
    DOUBLE("DP1/4TH", 010000, 0), /* 2DEC .25 */
    DOUBLE("UNITZ", 0, 0),        /* 2DEC 0 */
    DOUBLE("UNITY", 0, 0),        /* 2DEC 0 */
    DOUBLE("UNITX", 020000, 0),   /* 2DEC .5 */
    DOUBLE("ZEROVECS", 0, 0),     /* 2DEC 0 */
    DOUBLE("", 0, 0),             /* 2DEC 0 */
    DOUBLE("", 0, 0),             /* 2DEC 0 */
    /* DPHALF = UNITX */
    WORD("DPPOSMAX", 037777), /* OCT 37777 */
    WORD("", 037777),         /* OCT 37777 */
};

/* ---- INTPRET2, from ZUNIT (p. 1101; CM p. 1206) ---- */

static const struct pool_word intpret2[] = {
    DOUBLE("ZUNIT", 0, 0),      /* 2DEC 0 */
    DOUBLE("YUNIT", 0, 0),      /* 2DEC 0 */
    DOUBLE("XUNIT", 020000, 0), /* 2DEC .5 */
    DOUBLE("ZEROVEC", 0, 0),    /* 2DEC 0 */
    DOUBLE("", 0, 0),           /* 2DEC 0 */
    DOUBLE("", 0, 0),           /* 2DEC 0 */
    /* "-0, -6, -12 MUST REMAIN IN THIS ORDER" */
    WORD("", 077777), /* OCT 77777 */
#if LUMINARY099
    WORD("DFC-6", MINUS(6)),   /* DEC -6 */
    WORD("DFC-12", MINUS(12)), /* DEC -12 */
#else
    WORD("DEC-6", MINUS(6)),   /* DEC -6 */
    WORD("DEC-12", MINUS(12)), /* DEC -12 */
#endif
    /* "THESE TWO CONSTANTS MUST REMAIN" "ADJACENT AND THE SAME FOR INTEGRATION" */
    DOUBLE("LODPMAX", 037777, 037777),  /* 2OCT 3777737777 */
    DOUBLE("LODPMAX1", 037777, 037777), /* 2OCT 3777737777 */
    /* ZERODP = ZEROVEC */
    /* HALFDP = XUNIT */
};

/* The word at a rope address, or NULL. */
static const struct pool_word *pool_word_at(word address)
{
    unsigned in1 = (unsigned)address - I_DP1_4TH;
    unsigned in2 = (unsigned)address - I_ZUNIT;

    if (address >= I_DP1_4TH && in1 < COUNT(intpret1)) return &intpret1[in1];
    if (address >= I_ZUNIT && in2 < COUNT(intpret2)) return &intpret2[in2];
    return NULL;
}

bool interpretive_constant(word address, word *value)
{
    const struct pool_word *at = pool_word_at(address);

    if (!at) return false;
    *value = at->value;
    return true;
}

const char *interpretive_constant_label(word address)
{
    const struct pool_word *at = pool_word_at(address);

    return at ? at->label : NULL;
}
