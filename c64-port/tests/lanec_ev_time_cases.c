/* SPDX-License-Identifier: MIT */
/* Cases for the V55 clock update and the ATTCK2 task: every entry of both
 * missions, the loaded verb in MPAC (V23 or another), random TIME2/TIME1
 * and increments (sums that carry and overflow), random THETAD and channel
 * 14, and each axis index at ATTCK2 +1. */
#include "lanec_suites.h"
#include "lanec_ev_time.h"
#include "lanec_symbols.h"

#define ENTRIES 18u

static void make(uint32_t n, apollo_overlay_state *s, lanec_io *io, lanec_meta *m)
{
    uint16_t *w = s->words, lm_n, cm_n, e = (uint16_t)(n % ENTRIES), z;
    const uint16_t *lm = lanec_ev_time_entries(LANEC_LM, &lm_n), *cm = lanec_ev_time_entries(LANEC_CM, &cm_n);
    uint32_t r = n * 747796405u + 1u, v = n / ENTRIES;
    lanec_fill(s, io, n);
    m->mission = e < lm_n ? LANEC_LM : LANEC_CM;
    z = e < lm_n ? lm[e] : cm[e - lm_n];
    lanec_enter(s, io, m->mission == LANEC_LM ? 042u : 043u, z);
    w[LANEC_Q] = (uint16_t)(04000u + lanec_rand(&r) % 04000u);
    /* MPAC holds the verb that loaded the time: V23 in three variants of four */
    if (v & 3u) w[LANEC_LM_MPAC] = LANEC_LM_DEC23_WORD;
    /* small clock words in half the variants, so carries and no-carries both occur */
    if (v & 4u) {
        w[LANEC_LM_TIME2] = (uint16_t)(lanec_rand(&r) & 077u);
        w[LANEC_LM_DSPTEM2 + 1u] = (uint16_t)(lanec_rand(&r) & 077u);
    }
    if (z == LANEC_LM_ATTCK2 + 1u && m->mission == LANEC_LM) w[LANEC_A] = (uint16_t)(v % 3u);
    /* the CDU drive is off when the task starts (the oracle keeps Virtual
     * AGC's drive simulation away from CDUXCMD..CDUZCMD; see oracle.c) */
    if (m->mission == LANEC_LM && (z == LANEC_LM_ATTCK2 || z == LANEC_LM_ATTCK2 + 1u)) io->channel[014] &= (uint16_t)~070000u;
    m->ranges = 1;
    if (m->mission == LANEC_LM) {
        m->range[0].bank = 042u; m->range[0].lo = LANEC_LM_R33; m->range[0].hi = LANEC_LM_R33 + 051u;
    } else {
        m->range[0].bank = 043u; m->range[0].lo = LANEC_CM_ALINTIME; m->range[0].hi = LANEC_CM_ALINTIME + 031u;
    }
}

const lanec_suite lanec_suite_ev_time = { "ev-time", ENTRIES * 64u, make, lanec_ev_time_dispatch };
