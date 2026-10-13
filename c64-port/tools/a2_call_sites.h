/* SPDX-License-Identifier: MIT */
/* Row cover only (tools/a2_c64_rows.py, select(call_sites=...)): included
 * ahead of a milestone source so that every call site of an agc_image_ops
 * helper whose outcome no branch of the source shows gets that outcome as
 * gcov branch arcs on the call site's own line:
 *   DAS (and DDOUBL)  the low word's carry, borrow or neither into the high
 *                     word, and A = +1, -1 or +0 (the overflow) after it;
 *   ADS               A overflowing positive, negative or not;
 *   BZF               A = +0, -0 or nonzero.
 * The expansion only reads the image; the PRG and the test builds never
 * include this file. */
#ifndef A2_CALL_SITES_H
#define A2_CALL_SITES_H
#include "agc_image_ops.h"
static volatile unsigned a2_site_n;
static agc_register a2_site_add16(agc_register a, agc_register b) {
    uint32_t t = (uint32_t)a + (uint32_t)b;
    if (t & 0200000u) t = (t + 1u) & 0177777u;
    return (agc_register)t;
}
static agc_register a2_site_operand(const agc_image *s, unsigned a) {
    agc_word v;
    if (a < 3u) return s->words[a];
    v = ai_word(s, a);
    return (agc_register)((v & 077777u) | (((unsigned)v << 1) & 0100000u));
}
/* DAS k's low-word sum: 1 carry, 2 borrow, 0 neither. */
static unsigned a2_site_carry(const agc_image *s, unsigned k) {
    agc_register lsw = a2_site_add16(s->words[1], k == 0u ? s->words[1] : a2_site_operand(s, k + 1u));
    return (lsw & 0140000u) == 040000u ? 1u : (lsw & 0140000u) == 0100000u ? 2u : 0u;
}
#define ai_das(s, k) do { agc_image *a2_s = (s); unsigned a2_k = (k), a2_c = a2_site_carry(a2_s, a2_k); \
    (ai_das)(a2_s, a2_k); \
    if (a2_c == 1u) a2_site_n++; else if (a2_c == 2u) a2_site_n++; \
    if (a2_s->words[0] == 1u) a2_site_n++; else if (a2_s->words[0] == 0177776u) a2_site_n++; } while (0)
#define ai_ads(s, a) do { agc_image *a2_s = (s); (ai_ads)(a2_s, (a)); \
    if ((a2_s->words[0] & 0140000u) == 040000u) a2_site_n++; \
    else if ((a2_s->words[0] & 0140000u) == 0100000u) a2_site_n++; } while (0)
#define ai_bzf(s) ((s)->words[0] == 0u ? (a2_site_n++, (ai_bzf)(s)) : \
    (s)->words[0] == 0177777u ? (a2_site_n++, (ai_bzf)(s)) : (ai_bzf)(s))
#endif
