/* SPDX-License-Identifier: MIT */
/* STAR_TABLES.agc of the CM -- Comanche055 pp. 1389-1393. The faithful
 * port reads these words from its compiled listing data
 * (c64-port/tools/fixed_data_generate.py).
 *
 * The navigation stars, 37 unit vectors, for the sightings of P51-P53.
 * The listing assembles them at STARTAB (bank 14, TAGS_FOR_RELATIVE_SETLOC)
 * from star 37 down to star 1, three double words a star (X, Y, Z), with
 * CATLOG, a single DEC 6970, just after star 1. So star n's vector starts
 * 6n words below CATLOG, which is how the interpretive code reaches it:
 * with n times 6 in index register 1, a VLOAD* of CATLOG,1 (an indexed
 * address is the address less the index). PICAPAR, which picks a pair of
 * stars, starts its index at "X1 = 37 X 6 + 6" and counts down by 6.
 *
 * Each component is a 2DEC scaled B-1: the word holds half the value, so
 * that a unit vector's components, up to 1, fit a fraction below 1. The
 * stars are written below as the listing has them, in decimal;
 * star_table_word() makes each word from them as the assembler does. */
#include "star_tables.h"
#include "../agc/names.h"

#if COMANCHE055

/* clang-format off */
const struct star STARS[STAR_COUNT] = {
    /* number, X, Y, Z (p. 1389) */
    {37, +.8342971408, -.2392481515, -.4966976975},
    {36, +.8139832631, -.5557243189, +.1691204557},
    {35, +.4541086270, -.5392368197, +.7092312789},
    {34, +.3201817378, -.4436021946, -.8370786986},
    {33, +.5520184464, -.7933187400, -.2567508745},
    {32, +.4537196908, -.8779508801, +.1527766153},
    {31, +.2069525789, -.8719885748, -.4436288486},
    {30, +.1217293692, -.7702732847, +.6259880410},
    {29, -.1124304773, -.9694934200, +.2178116072},
    {28, -.1146237858, -.3399692557, -.9334250333},
    {27, -.3516499609, -.8240752703, -.4441196390},
    {26, -.5326876930, -.7160644554, +.4511047742},
    {25, -.7861763936, -.5217996305, +.3311371675},
    {24, -.6898393233, -.4182330640, -.5909338474},
    {23, -.5812035376, -.2909171294, +.7599800468},
    {22, -.9170097662, -.3502146628, -.1908999176},
    {21, -.4523440203, -.0493710140, -.8904759346},
    {20, -.9525211695, -.0593434796, -.2986331746},
    {19, -.9656605484, +.0525933156, +.2544280809},
    {18, -.8608205219, +.4636213989, +.2098647835},
    {17, -.7742591356, +.6152504197, -.1482892839},
    {16, -.4657947941, +.4774785033, +.7450164351},
    {15, -.3612508532, +.5747270840, -.7342932655},
    {14, -.4118589524, +.9065485360, +.0924226975},
    {13, -.1820751783, +.9404899869, -.2869271926},
    {12, -.0614937230, +.6031563286, -.7952489957},
    {11, +.1371725575, +.6813721061, +.7189685267},
    {10, +.2011399589, +.9690337941, -.1432348512},
    { 9, +.3507315038, +.8926333307, +.2831839492},
    { 8, +.4105636020, +.4988110001, +.7632988371},
    { 7, +.7032235469, +.7075846047, +.0692868685},
    { 6, +.5450107404, +.5314955466, -.6484410356},
    { 5, +.0130968840, +.0078062795, +.9998837600},
    { 4, +.4917678276, +.2204887125, -.8423473935},
    { 3, +.4775639450, +.1166004340, +.8708254803},
    { 2, +.9342640400, +.1735073142, -.3115219339},
    { 1, +.8748658918, +.0260879174, +.4836621670},
};
/* clang-format on */

/* A 2DEC scaled B-1, half the value as a double word: a fraction of 28
 * bits, rounded, its upper 14 bits in the first word and its lower 14 in
 * the second, both words carrying the sign (one's complement). */
static word two_dec_b1(double value, unsigned half)
{
    double magnitude = (value < 0.0 ? -value : value) / 2.0;
    unsigned long bits = (unsigned long)(magnitude * 268435456.0 + 0.5); /* 2^28 */
    word part = (word)(half == 0u ? bits >> 14 : bits & 037777u);

    return value < 0.0 ? (word)(MINUS_ZERO - part) : part;
}

bool star_table_word(word address, word *value)
{
    unsigned first = I_CATLOG - 6u * STAR_COUNT;
    unsigned offset, component;
    const struct star *star;
    double x;

    if (address == I_CATLOG) {
        *value = (word)CATLOG_WORD;
        return true;
    }
    if (address < first || address > I_CATLOG) return false;
    offset = address - first;
    star = &STARS[offset / 6u];
    component = offset % 6u / 2u;
    x = component == 0u ? star->x : component == 1u ? star->y : star->z;
    *value = two_dec_b1(x, offset % 2u);
    return true;
}

#endif
