/* SPDX-License-Identifier: MIT */
/* Cases for the display start-up block: every basic block, every COPINDEX,
 * NVWORD of each sign including -0, PINBRANCH flag combinations, the
 * perform, flash, paste and second-part bits of TEMPOR2, DSPFLG values
 * whose ADS lands on zero, negative or positive, and A with and without
 * overflow. Variants 64-71 put COPINDEX outside 0-2: the blocks that index
 * with it refuse, the others run as the original does. */
#include "lanec_suites.h"
#include "lanec_di_nvdsp.h"
#include "lanec_symbols.h"

#define ENTRIES 25u
static const uint8_t offsets[ENTRIES] = { 0, 9, 11, 16, 17, 34, 35, 39, 40, 41, 42, 43, 44, 46, 48, 50, 52, 53,
    57, 61, 65, 69, 75, 79, 91 };

static void range(lanec_meta *m, uint16_t bank, uint16_t lo, uint16_t hi)
{
    m->range[m->ranges].bank = bank; m->range[m->ranges].lo = lo; m->range[m->ranges].hi = hi; ++m->ranges;
}

static void make(uint32_t n, apollo_overlay_state *s, lanec_io *io, lanec_meta *m)
{
    uint16_t *w = s->words, e = (uint16_t)(n % ENTRIES), pb, vp, dspflg, x, i;
    uint32_t r = n * 747796405u + 1u, v = n / ENTRIES / 2u;
    lanec_fill(s, io, n);
    m->mission = (n / ENTRIES) & 1u ? LANEC_CM : LANEC_LM;
    if (m->mission == LANEC_LM) { pb = LANEC_LM_PINBRNCH; vp = LANEC_LM_V50PASTE; dspflg = LANEC_LM_EBANKSAV; }
    else { pb = LANEC_CM_PINBRNCH; vp = LANEC_CM_V50PASTE; dspflg = LANEC_CM_EBANKSAV; }
    i = (uint16_t)(v % 3u);
    w[LANEC_LM_LOC] = i;                                                        /* COPINDEX */
    x = (uint16_t)(lanec_rand(&r) & 077777u);
    switch ((v / 3u) % 4u) {                                                   /* NVWORD */
    case 0: w[LANEC_LM_NVWORD + i] = 0; break;
    case 1: w[LANEC_LM_NVWORD + i] = 077777u; break;
    case 2: w[LANEC_LM_NVWORD + i] = (uint16_t)(040000u | x); break;
    default: w[LANEC_LM_NVWORD + i] = (uint16_t)(x & 037777u); break;
    }
    if ((v / 2u) % 3u == 0) w[LANEC_LM_NVWORD + i] &= (uint16_t)~0177u;          /* a paste (noun 00) */
    w[LANEC_LM_TEMPOR2] = (uint16_t)(lanec_rand(&r) & 077777u);
    w[LANEC_LM_TEMPOR2] &= (uint16_t)~(((v / 4u) & 1u ? 020u : 0u) | ((v / 8u) & 1u ? 010u : 0u) |
                                       ((v / 16u) & 1u ? 040u : 0u) | ((v / 5u) & 1u ? 010000u : 0u));
    w[LANEC_LM_FLAGWRD4] &= (uint16_t)~(lanec_rand(&r) & 070000u);
    if ((v / 6u) % 3u == 0) w[dspflg + i] = (uint16_t)(077777u - LANEC_LM_BIT13_WORD);   /* ADS gives -0 */
    x = (uint16_t)lanec_rand(&r);
    switch ((v / 7u) % 4u) {
    case 0: w[LANEC_A] = (uint16_t)(0040000u | (x & 037777u)); break;
    case 1: w[LANEC_A] = (uint16_t)(0100000u | (x & 037777u)); break;
    default: w[LANEC_A] = lanec_a16((uint16_t)(x & 077777u)); break;
    }
    if (v >= 64u) w[LANEC_LM_LOC] = (uint16_t)(3u + lanec_rand(&r) % 077774u);
    lanec_enter(s, io, 010u, (uint16_t)(pb + offsets[e]));
    range(m, 010u, pb, (uint16_t)(vp + 1u));
}

const lanec_suite lanec_suite_di_nvdsp = { "di-nvdsp", ENTRIES * 2u * 72u, make, lanec_di_nvdsp_dispatch };
