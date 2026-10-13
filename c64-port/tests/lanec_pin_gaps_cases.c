/* SPDX-License-Identifier: MIT */
/* Cases for Pinball verb 07 and CHANLOAD: both MIXBR branches, NOUNADD over
 * unswitched and switched erasable (pairs that overlap MPAC or the editing
 * registers included),
 * both return points, a channel load for every channel 0-077, DSP2DEC from
 * its entry (Q with and without overflow) and from each return, and the
 * NVSUBB/LOADLV1 lead-ins, and DSPDCWD1 from both entries and from DSPIN's
 * return (each digit, every CCS branch), and the POSTJUMP and BANKJUMP
 * targets GOVNUPDT, DSPOCTIN, UPDAT1 +2 and OCTBACK with their lead-ins
 * (OCTBACK over every WDCNT branch, DSPCOUNT positive, blanking and
 * negative, any CYL). */
#include "lanec_suites.h"
#include "lanec_pin_gaps.h"
#include "lanec_symbols.h"

#define ENTRIES 26u
#define FIRST (ENTRIES * 2u * 64u * 4u)    /* the jump-target cases follow the first suite's */
#define JUMPS 6u

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

static void make_jump(uint32_t n, apollo_overlay_state *s, lanec_io *io, lanec_meta *m)
{
    static const uint16_t wdcnt[8] = { 0, 1, 2, 3, 4, 077777u, 077776u, 070000u };
    static const uint16_t count[8] = { 0, 1, 5, 021u, 020004u, 020021u, 077776u, 077777u };
    uint16_t *w = s->words, entry = (uint16_t)(n % JUMPS), g, d;
    uint32_t r = n * 2891336453u + 7u, v = n / JUMPS / 2u;
    lanec_fill(s, io, FIRST + n);
    m->mission = (n / JUMPS) & 1u ? LANEC_CM : LANEC_LM;
    g = m->mission == LANEC_LM ? LANEC_LM_GOVNUPDT : LANEC_CM_GOVNUPDT;
    d = m->mission == LANEC_LM ? LANEC_LM_DSPOCTIN : LANEC_CM_DSPOCTIN;
    switch (entry) {
    case 0: case 1:                  /* GOVNUPDT, +1 (DSPDECVN returns) */
        lanec_enter(s, io, 040u, (uint16_t)(g + entry));
        range(m, 040u, w[LANEC_Z], w[LANEC_Z]);
        break;
    case 2: case 3:                  /* DSPOCTIN, +1 (DSPIN returns) */
        lanec_enter(s, io, 040u, (uint16_t)(d + entry - 2u));
        range(m, 040u, w[LANEC_Z], (uint16_t)(d + 2u));
        break;
    case 4:                          /* UPDAT1 +2 (POSTJUMP from GOVNUPDT) */
        lanec_enter(s, io, 041u, (uint16_t)(LANEC_LM_UPDAT1 + 2u));
        range(m, 041u, w[LANEC_Z], w[LANEC_Z]);
        break;
    default:                         /* OCTBACK (BANKJUMP from DSPOCTIN) */
        lanec_enter(s, io, 041u, LANEC_LM_OCTBACK);
        w[LANEC_LM_WDCNT] = v % 16u == 15u ? (uint16_t)(lanec_rand(&r) & 077777u) : wdcnt[v % 8u];
        w[LANEC_LM_DSPCOUNT] = count[(v / 8u) % 8u];
        w[022] = (uint16_t)(lanec_rand(&r) & 077777u);
        range(m, 041u, LANEC_LM_WDAGAIN, (uint16_t)(LANEC_LM_OCTBACK + 4u));
        break;
    }
}

static void make(uint32_t n, apollo_overlay_state *s, lanec_io *io, lanec_meta *m)
{
    uint16_t *w = s->words, entry = (uint16_t)(n % ENTRIES), x, y, channel;
    uint32_t r = n * 747796405u + 1u, v = n / ENTRIES / 2u;
    if (n >= FIRST) { make_jump(n - FIRST, s, io, m); return; }
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
    case 18: case 19: case 20: {     /* LODSAMPT, FLASHON, FLASHOFF: TC Q to a caller in switched fixed */
        uint16_t f = entry == 18u ? (m->mission == LANEC_LM ? LANEC_LM_LODSAMPT : LANEC_CM_LODSAMPT)
                   : entry == 19u ? (m->mission == LANEC_LM ? LANEC_LM_FLASHON : LANEC_CM_FLASHON)
                                  : (m->mission == LANEC_LM ? LANEC_LM_FLASHOFF : LANEC_CM_FLASHOFF);
        lanec_enter(s, io, (uint16_t)(1u + lanec_rand(&r) % 043u), f);
        w[LANEC_Q] = (uint16_t)(02000u + lanec_rand(&r) % 02000u);
        if (v % 8u == 7u) w[LANEC_Q] = (uint16_t)(lanec_rand(&r) % 02000u);   /* refused: not fixed */
        range(m, LANEC_ANY_BANK, f, (uint16_t)(f + 3u));
        range(m, LANEC_ANY_BANK, LANEC_Q, LANEC_Q);                       /* TC Q runs Q */
        break;
    }
    case 21: case 22:                /* WAITLIST and NOVAC return to the lamp test */
        lanec_enter(s, io, 041u, entry == 21u ? (uint16_t)(LANEC_LM_FULLDSP - 1u) : (uint16_t)(LANEC_LM_TSTLTS3 - 1u));
        range(m, 041u, w[LANEC_Z], w[LANEC_Z]);
        break;
    case 23: case 24: case 25: {     /* DSPDCWD1 -1, +0, and +16 where DSPIN returns */
        uint16_t d = m->mission == LANEC_LM ? LANEC_LM_DSPDCWD1 : LANEC_CM_DSPDCWD1;
        uint16_t sh = m->mission == LANEC_LM ? LANEC_LM_SHORTMP : LANEC_CM_SHORTMP;
        uint16_t dm = m->mission == LANEC_LM ? LANEC_LM_DMPSUB2 : LANEC_CM_DMPSUB2;
        static const uint16_t count[8] = { 0, 1, 4, 9, 14, 21, 077777u, 077776u };
        lanec_enter(s, io, 040u, entry == 23u ? (uint16_t)(d - 1u) : entry == 24u ? d : (uint16_t)(d + 16u));
        w[LANEC_A] = (uint16_t)(v % 8u == 7u ? lanec_rand(&r) & 0177777u : v % 5u);
        if (v % 8u != 6u) w[LANEC_LM_MPAC] &= 037777u;                     /* a magnitude, as DSPDECWD leaves it */
        w[LANEC_LM_DSPCOUNT] = count[(v / 8u) % 8u];
        w[LANEC_LM_WDCNT] = count[(v / 3u) % 8u];
        if (w[LANEC_Q] == d + 18u) ++w[LANEC_Q];
        range(m, 040u, (uint16_t)(d - 1u), (uint16_t)(d + 20u));
        range(m, LANEC_ANY_BANK, sh, (uint16_t)(sh + 6u));
        range(m, LANEC_ANY_BANK, dm, (uint16_t)(dm + 7u));
        range(m, LANEC_TC_CELL, LANEC_Q, LANEC_Q);                         /* SHORTMP's TC Q */
        m->stop[0].bank = 040u; m->stop[0].address = d;                    /* the next digit */
        m->stops = 1u; m->stop_q = 1u; m->stop_q_value = (uint16_t)(d + 18u);
        break;
    }
    default:                         /* CHANLOAD: every channel, high NOUNCADR bits */
        channel = (uint16_t)((v + (entry - 6u) * 16u) % 0100u);
        lanec_enter(s, io, 041u, y);
        if (channel == 7u) w[LANEC_LM_NOUNCADR] = 7u;
        else w[LANEC_LM_NOUNCADR] = (uint16_t)(channel | ((entry & 1u) ? (lanec_rand(&r) & 077000u) : 0u));
        range(m, 041u, y, (uint16_t)(y + 10u));
        break;
    }
}

const lanec_suite lanec_suite_pin_gaps = { "pin-gaps", FIRST + JUMPS * 2u * 128u, make, lanec_pin_gaps_dispatch };
