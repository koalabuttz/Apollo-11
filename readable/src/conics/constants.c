/* SPDX-License-Identifier: MIT */
/* CONIC_SUBROUTINES.agc, the conics' constants in bank 4 (LM p. 1185, CM
 * p. 1288): "DO NOT DISTURB THE ORDER OF THESE CDS, OVERLAYS HAVE BEEN
 * MADE." The interpretive code reads them in the rope; these are their
 * words, as yaYUL assembles them, which the side-by-side test holds to the
 * rope. BEE17's single zero and the high word of D1/8 after it read as a
 * double word 1.0 B-17; BEE19 and BEE22 (1.0 B-19 and B-22) are the words
 * before D1/32 and D1/256, by EQUALS, as KEPZERO is LO6ZEROS. */
#include "conics.h"
#include "../agc/names.h"

static const word constant_words[] = {
    /* BEE17 */ NUMBER(000000), /* DEC 0 */
    /* D1/8 */ NUMBER(004000),  /* 2DEC 1.0 B-3 */
    NUMBER(000000),
    /* D1/128 */ NUMBER(000200), /* 2DEC 1.0 B-7 */
    NUMBER(000000),
    /* D1/64 */ NUMBER(000400), /* 2DEC 1.0 B-6 */
    NUMBER(000000),
    /* D1/4 */ NUMBER(010000), /* 2DEC 1.0 B-2 */
    NUMBER(000000),
    /* D1/16 */ NUMBER(002000), /* 2DEC 1.0 B-4 */
    NUMBER(000000),
    /* D1/32 */ NUMBER(001000), /* 2DEC 1.0 B-5 */
    NUMBER(000000),
    /* D1/1024 */ NUMBER(000020), /* 2DEC 1.0 B-10 */
    NUMBER(000000),
    /* D1/256 */ NUMBER(000100), /* 2DEC 1.0 B-8 */
    NUMBER(000000),
    /* DP9/10 */ NUMBER(034631), /* 2DEC .9 */
    NUMBER(023146),
    /* -50SC */ NUMBER(077467), /* 2DEC -50.0 B-12 */
    NUMBER(077777),
    /* 2PISC */ NUMBER(003110), /* 2DEC 6.28318530 B-6 */
    NUMBER(017665),
    /* ONEBIT */ NUMBER(000000), /* 2DEC 1.0 B-28 */
    NUMBER(000001),
    /* COGUPLIM */ NUMBER(037767), /* 2DEC .999511597 */
    NUMBER(037737),
    /* COGLOLIM */ NUMBER(040010), /* 2DEC -.999511597 */
    NUMBER(040040),
};

const struct interpretive_code CONIC_CONSTANTS = {I_BEE17 >> 10, (word)(I_BEE17 % 02000u + 02000u),
                                                  COUNT(constant_words), constant_words};
