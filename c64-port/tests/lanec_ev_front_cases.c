/* SPDX-License-Identifier: MIT */
/* Cases for the EXTENDED_VERBS front end: every fan slot, every CCS class of
 * EXTVBACT and of the masked FLAGWRD4, TS overflow on both SETXTACT stores,
 * both NVSUB and FALTON return points, P00/P00- and other MODREG values. */
#include "lanec_suites.h"
#include "lanec_ev_front.h"
#include "lanec_symbols.h"

#define ENTRIES 12u
#define B 043u

static void range(lanec_meta *m, uint16_t bank, uint16_t lo, uint16_t hi)
{
    m->range[m->ranges].bank = bank; m->range[m->ranges].lo = lo; m->range[m->ranges].hi = hi; ++m->ranges;
}

static uint16_t ccs_class(uint32_t *r, uint8_t c)
{
    uint16_t v = (uint16_t)(lanec_rand(r) & 037777u);
    switch (c & 3u) {
    case 0: return v ? v : 1u;
    case 1: return 0;
    case 2: return (uint16_t)(040000u | (v == 037777u ? 0u : v));
    default: return 077777u;
    }
}

static void make(uint32_t n, apollo_overlay_state *s, lanec_io *io, lanec_meta *m)
{
    uint16_t *w = s->words, entry = (uint16_t)(n % ENTRIES);
    uint32_t r = n * 747796405u + 1u;
    uint16_t gx = LANEC_LM_GOEXTVB, tx = LANEC_LM_TESTXACT, sx = LANEC_LM_SETXTACT, xa = LANEC_LM_XACTALM;
    uint16_t x0 = LANEC_LM_XACT0, ae = LANEC_LM_ALM_S_END, gp = LANEC_LM_GOPIN, mp = LANEC_LM_MPAC;
    lanec_fill(s, io, n);
    m->mission = (n / ENTRIES) & 1u ? LANEC_CM : LANEC_LM;
    /* Callers' Q: a return address in fixed memory. */
    w[LANEC_Q] = (uint16_t)(02000u + lanec_rand(&r) % 06000u);
    switch (entry) {
    case 0: case 1: case 2:          /* fan: every verb 40..99 in both missions */
        lanec_enter(s, io, B, gx);
        w[mp] = (uint16_t)((n / (ENTRIES * 2u)) % 60u);
        range(m, B, gx, (uint16_t)(LANEC_LM_LST2FAN + 59u));
        m->stop[0].bank = B; m->stop[0].address = 02037u; m->stops = 1; /* VERB69 */
        break;
    case 3: case 4:                  /* TESTXACT */
        lanec_enter(s, io, B, tx);
        w[LANEC_LM_EXTVBACT] = ccs_class(&r, (uint8_t)(n / (ENTRIES * 2u)));
        if ((n / (ENTRIES * 8u)) & 1u) w[LANEC_LM_FLAGWRD4] &= (uint16_t)~024100u;
        range(m, B, tx, (uint16_t)(sx + 4u)); range(m, B, ae, ae);
        break;
    case 5:                          /* SETXTACT with any A and Q, overflow included */
        lanec_enter(s, io, B, sx);
        if ((n / ENTRIES) & 2u) w[LANEC_A] = (uint16_t)(lanec_rand(&r) & 0177777u);
        if ((n / ENTRIES) & 4u) w[LANEC_Q] = (uint16_t)(lanec_rand(&r) & 0177777u);
        range(m, B, sx, (uint16_t)(sx + 4u));
        break;
    case 6:                          /* XACT0 */
        lanec_enter(s, io, B, x0);
        range(m, B, x0, (uint16_t)(x0 + 1u)); range(m, B, sx, (uint16_t)(sx + 4u));
        break;
    case 7:                          /* NVSUB returns to SETXTACT +5 or +6 */
        lanec_enter(s, io, B, (uint16_t)(sx + 5u + ((n / ENTRIES / 2u) & 1u)));
        do w[mp + 1u] = (uint16_t)(02000u + lanec_rand(&r) % 06000u);
        while (w[mp + 1u] == sx + 5u || w[mp + 1u] == sx + 6u);
        range(m, B, (uint16_t)(sx + 5u), (uint16_t)(sx + 6u)); range(m, LANEC_ANY_BANK, (uint16_t)(mp + 1u), (uint16_t)(mp + 1u));
        break;
    case 8:                          /* XACTALM, and FALTON's return to it */
        lanec_enter(s, io, B, (uint16_t)(xa + ((n / ENTRIES / 2u) & 1u)));
        range(m, B, w[LANEC_Z], w[LANEC_Z]);
        break;
    case 9:                          /* ALM/END, and FALTON's return to GOPIN */
        lanec_enter(s, io, B, (n / ENTRIES / 2u) & 1u ? gp : ae);
        range(m, B, w[LANEC_Z], w[LANEC_Z]);
        break;
    default:                         /* LM CHKPOOH; P00 (+0), P00- (-0) or any program */
        m->mission = LANEC_LM;
        lanec_enter(s, io, B, LANEC_LM_CHKPOOH);
        switch ((n / ENTRIES) % 4u) {
        case 0: w[LANEC_LM_MODREG] = 0; break;
        case 1: w[LANEC_LM_MODREG] = 077777u; break;
        default: break;
        }
        range(m, B, LANEC_LM_CHKPOOH, (uint16_t)(LANEC_LM_CHKPOOH + 3u)); range(m, B, ae, ae);
        break;
    }
}

const lanec_suite lanec_suite_ev_front = { "ev-front", 12u * 2u * 60u * 4u, make, lanec_ev_front_dispatch };
