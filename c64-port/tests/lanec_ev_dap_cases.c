/* SPDX-License-Identifier: MIT */
/* Cases for the V48 DAP data load bodies: every entry of both missions with
 * random DAPBOOLS, DAPDATR1, APSFLAG and FLAGWRD6 bits, masses above and
 * below their minimums (and overflowing sums into MASS), masses exactly at
 * the minimums (sums of -0), and random EB. */
#include "lanec_suites.h"
#include "lanec_ev_dap.h"
#include "lanec_symbols.h"

#define ENTRIES 59u

static const uint16_t banks[4][2] = { { LANEC_LM, 020u }, { LANEC_LM, 01u }, { LANEC_CM, 043u }, { LANEC_CM, 042u } };

static void range(lanec_meta *m, uint16_t bank, uint16_t lo, uint16_t hi)
{
    m->range[m->ranges].bank = bank; m->range[m->ranges].lo = lo; m->range[m->ranges].hi = hi; ++m->ranges;
}

static void make(uint32_t n, apollo_overlay_state *s, lanec_io *io, lanec_meta *m)
{
    uint16_t *w = s->words, e = (uint16_t)(n % ENTRIES), i, count = 0;
    const uint16_t *entry = 0;
    uint32_t r = n * 747796405u + 1u, v = n / ENTRIES;
    lanec_fill(s, io, n);
    for (i = 0; i < 4u; ++i) {
        entry = lanec_ev_dap_entries((enum lanec_mission)banks[i][0], banks[i][1], &count);
        if (e < count) break;
        e = (uint16_t)(e - count);
    }
    m->mission = (enum lanec_mission)banks[i][0];
    lanec_enter(s, io, banks[i][1], entry[e]);
    w[LANEC_Q] = (uint16_t)(04000u + lanec_rand(&r) % 04000u);
    /* masses near the minimums: small positive LEMMASS/CSMMASS in half the variants */
    if (v & 1u) {
        w[LANEC_LM_LEMMASS] = (uint16_t)(lanec_rand(&r) & 07777u);
        w[LANEC_LM_CSMMASS] = (uint16_t)(lanec_rand(&r) & 07777u);
    }
    /* masses exactly at the minimums, where the sums are -0: LEMMASS at the
     * descent-plus-ascent minimum (02357, -MINLMD - MINMINLM) or the ascent
     * minimum (01046, -MINMINLM), CSMMASS at MINCSM (02000) */
    if (v % 8u == 2u || v % 8u == 4u) w[LANEC_LM_LEMMASS] = 02357u;
    if (v % 8u == 6u) w[LANEC_LM_LEMMASS] = 01046u;
    if (v % 8u >= 4u) w[LANEC_LM_CSMMASS] = 02000u;
    if (m->mission == LANEC_LM) {
        range(m, 020u, LANEC_LM_DAPDATA1, LANEC_LM_DAPDATA1 + 0110u);
        range(m, 01u, LANEC_LM_STIKLOAD, LANEC_LM_STIKLOAD + 0135u);
    } else {
        range(m, 043u, LANEC_CM_DAPDISP, LANEC_CM_DAPDISP + 7u);
        range(m, 042u, LANEC_CM_DAPDISP1, LANEC_CM_DAPDISP1 + 061u);
    }
}

const lanec_suite lanec_suite_ev_dap = { "ev-dap", ENTRIES * 48u, make, lanec_ev_dap_dispatch };
