/* SPDX-License-Identifier: MIT */
/* ORBITAL_INTEGRATION.agc, the constants of ORBITAL1 (bank 12 in the LM,
 * 11 in the CM) and ORBITAL2 (bank 13) (LM pp. 1246-1247, CM pp.
 * 1353-1354). The interpretive code reads them in the rope; these are
 * their words, as yaYUL assembles them, which the side-by-side test holds
 * to the rope.
 *
 * In ORBITAL2, "IT IS VITAL THAT THE FOLLOWING CONSTANTS NOT BE SHUFFLED":
 * ACCOMP reads the shifts before ASCALE by X1 (ASCALE,1), and the body's
 * constants by X2, the central body (0 the earth, 2 the moon; indexing
 * subtracts, so MUEARTH,2 is the moon's MU, MUM). The CM keeps here the
 * sun's and the moon's MU, MUEARTH, and the earth's and the moon's
 * oblateness constants (J4REQ/J3, 2J3RE/J2, J2REQSQ), and 3J22R2MU; the
 * LM keeps them in CONTROLLED_CONSTANTS. */
#include "orbital_integration.h"
#include "../agc/names.h"

static const word orbital1_words[] = {
    /* 3/5 */ NUMBER(004631), /* 2DEC .6 B-2 */
    NUMBER(023146),
    /* THREE/8 */ NUMBER(014000), /* 2DEC .375 */
    NUMBER(000000),
    /* .3D */ NUMBER(002314), /* 2DEC .3 B-2 */
    NUMBER(031463),
    /* 3/64 */ NUMBER(001400), /* 2DEC 3 B-6 */
    NUMBER(000000),
    /* DP1/4 */ NUMBER(010000), /* 2DEC .25 */
    NUMBER(000000),
    /* 3/32 */ NUMBER(003000), /* 2DEC 3 B-5 */
    NUMBER(000000),
    /* 15/16 */ NUMBER(036000), /* 2DEC 15. B-4 */
    NUMBER(000000),
    /* 3/4 */ NUMBER(030000), /* 2DEC 3.0 B-2 */
    NUMBER(000000),
    /* 7/12 */ NUMBER(022525), /* 2DEC .5833333333 */
    NUMBER(012525),
    /* 9/16 */ NUMBER(022000), /* 2DEC 9 B-4 */
    NUMBER(000000),
    /* 5/128 */ NUMBER(001200), /* 2DEC 5 B-7 */
    NUMBER(000000),
    /* DP2/3 */ NUMBER(025252), /* 2DEC .6666666667 */
    NUMBER(025253),
    /* OCT27 */ NUMBER(000027), /* OCT 27 */
};

static const word orbital2_words[] = {
    NUMBER(077764),              /* DEC -11 */
    NUMBER(077775),              /* DEC -2 */
    NUMBER(077766),              /* DEC -9 */
    NUMBER(077771),              /* DEC -6 */
    NUMBER(077775),              /* DEC -2 */
    NUMBER(077775),              /* DEC -2 */
    NUMBER(000000),              /* DEC 0 */
    NUMBER(077763),              /* DEC -12 */
    NUMBER(077766),              /* DEC -9 */
    NUMBER(077773),              /* DEC -4 */
    /* ASCALE */ NUMBER(077770), /* DEC -7 */
    NUMBER(077771),              /* DEC -6 */
#if COMANCHE055
    NUMBER(027446), /* 2DEC* 1.32715445 E16 B-54* (S) */
    NUMBER(014620),
    NUMBER(016471), /* 2DEC* 4.9027780 E8 B-30* (M) */
    NUMBER(001352),
    /* MUEARTH */ NUMBER(022437), /* 2DEC* 3.986032 E10 B-36* */
    NUMBER(016067),
    NUMBER(000000), /* 2DEC 0 */
    NUMBER(000000),
    /* J4REQ/J3 */ NUMBER(002302), /* 2DEC* .4991607391 E7 B-26* */
    NUMBER(024736),
    NUMBER(077651), /* 2DEC -176236.02 B-25 */
    NUMBER(076237),
    /* 2J3RE/J2 */ NUMBER(077776), /* 2DEC* -.1355426363 E5 B-27* */
    NUMBER(053032),
    NUMBER(010407), /* 2DEC* .3067493316 E18 B-60* */
    NUMBER(005344),
    /* J2REQSQ */ NUMBER(013710), /* 2DEC* 1.75501139 E21 B-72* */
    NUMBER(035320),
    /* 3J22R2MU */ NUMBER(012160), /* 2DEC* 9.20479048 E16 B-58* */
    NUMBER(012124),
#endif
    /* 5/8 */ NUMBER(024000), /* 2DEC 5 B-3 */
    NUMBER(000000),
    /* -1/12 */ NUMBER(074631), /* 2DEC -.1 */
    NUMBER(063145),
    /* RECRATIO */ NUMBER(000243), /* 2DEC .01 */
    NUMBER(032703),
    /* RSPHERE */ NUMBER(003654), /* 2DEC 64373.76 E3 B-29 */
    NUMBER(021000),
    /* RDM */ NUMBER(003654), /* 2DEC 16093.44 E3 B-27 */
    NUMBER(021000),
    /* RDE */ NUMBER(004627), /* 2DEC 80467.20 E3 B-29 */
    NUMBER(025200),
};

const struct interpretive_code ORBITAL1_CONSTANTS = {CODE_BANK(I_3_5), CODE_Z(I_3_5),
                                                     COUNT(orbital1_words), orbital1_words};

/* From the first DEC, ten words before ASCALE. */
const struct interpretive_code ORBITAL2_CONSTANTS = {
    CODE_BANK(I_ASCALE), (word)(CODE_Z(I_ASCALE) - 10u), COUNT(orbital2_words), orbital2_words};
