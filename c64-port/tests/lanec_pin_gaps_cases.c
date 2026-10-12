/* SPDX-License-Identifier: MIT */
/* Cases for Pinball verb 07 and CHANLOAD: both MIXBR branches, NOUNADD over
 * unswitched and switched erasable (pairs that overlap MPAC or the editing
 * registers included),
 * both return points, a channel load for every channel 0-077, DSP2DEC from
 * its entry (Q with and without overflow) and from each return, and the
 * NVSUBB/LOADLV1 lead-ins. */
#include "lanec_suites.h"
#include "lanec_pin_gaps.h"
#include "lanec_symbols.h"

#define ENTRIES 18u

static void range(lanec_meta *m, uint16_t bank, uint16_t lo, uint16_t hi)
{
    m->range[m->ranges].bank = bank; m->range[m->ranges].lo = lo; m->range[m->ranges].hi = hi; ++m->ranges;
}

static uint16_t nounadd(uint32_t *r, uint32_t v)
{
    uint16_t k;
    switch (v % 8u) {
    case 0: return (uint16_t)(LANEC_LM_MPAC - 1u + lanec_rand(r) % 3u);  /* pair overlaps MPAC */
    case 1: case 5: return (uint16_t)(01400u + lanec_rand(r) % 0377u);  /* switched, 01400-01776 */
    case 2: return (uint16_t)(017u + lanec_rand(r) % 5u);               /* CYR, SR, CYL, EDOP */
    default:
        do k = (uint16_t)(010u + lanec_rand(r) % 01367u); while (k == 01377u);
        return k;                                                       /* unswitched, 010-01376 */
    }
}

static void make(uint32_t n, apollo_overlay_state *s, lanec_io *io, lanec_meta *m)
{
    uint16_t *w = s->words, entry = (uint16_t)(n % ENTRIES), x, y, channel;
    uint32_t r = n * 747796405u + 1u, v = n / ENTRIES / 2u;
    lanec_fill(s, io, n);
    m->mission = (n / ENTRIES) & 1u ? LANEC_CM : LANEC_LM;
    x = m->mission == LANEC_LM ? LANEC_LM_DSPDPDEC : LANEC_CM_DSPDPDEC;
    y = m->mission == LANEC_LM ? LANEC_LM_CHANLOAD : LANEC_CM_CHANLOAD;
    switch (entry) {
    case 0: case 1: case 2: case 3:  /* verb 07: normal (MIXBR 1) or mixed (MIXBR 2) noun */
        lanec_enter(s, io, 040u, x);
        w[LANEC_LM_MIXBR] = entry == 3u ? 2u : 1u;
        w[LANEC_LM_NOUNADD] = nounadd(&r, v);
        range(m, 040u, x, (uint16_t)(x + 12u));
        break;
    case 4: case 5:                  /* TPAGREE returns to +13, DSP2DEC to ENDDPDEC */
        lanec_enter(s, io, 040u, (uint16_t)(x + 9u + entry));
        range(m, 040u, w[LANEC_Z], w[LANEC_Z]);
        break;
    case 10: case 11: case 12: case 13: case 14: {   /* DSP2DEC +0, +6, +8, +9, END2DEC */
        static const uint8_t at[5] = { 0, 6, 8, 9, 10 }, last[5] = { 5, 7, 8, 10, 10 };
        uint16_t d = m->mission == LANEC_LM ? LANEC_LM_DSP2DEC : LANEC_CM_DSP2DEC;
        lanec_enter(s, io, 040u, (uint16_t)(d + at[entry - 10u]));
        if (entry == 10u && (v & 1u)) w[LANEC_Q] = (uint16_t)(w[LANEC_Q] ^ 040000u);   /* Q overflowed */
        range(m, 040u, w[LANEC_Z], (uint16_t)(d + last[entry - 10u]));
        break;
    }
    case 15: case 16: case 17:       /* NVSUBB, LOADLV1, GOALMCYC */
        lanec_enter(s, io, 041u, entry == 17u ? LANEC_LM_GOALMCYC : (uint16_t)(LANEC_LM_NVSUBB + entry - 15u));
        range(m, 041u, w[LANEC_Z], w[LANEC_Z]);
        break;
    default:                         /* CHANLOAD: every channel, high NOUNCADR bits */
        channel = (uint16_t)((v + (entry - 6u) * 16u) % 0100u);
        lanec_enter(s, io, 041u, y);
        if (channel == 7u) w[LANEC_LM_NOUNCADR] = 7u;
        else w[LANEC_LM_NOUNCADR] = (uint16_t)(channel | ((entry & 1u) ? (lanec_rand(&r) & 077000u) : 0u));
        range(m, 041u, y, (uint16_t)(y + 10u));
        break;
    }
}

const lanec_suite lanec_suite_pin_gaps = { "pin-gaps", ENTRIES * 2u * 64u * 4u, make, lanec_pin_gaps_dispatch };
