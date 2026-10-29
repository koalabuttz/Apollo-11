/* SPDX-License-Identifier: MIT */
/* Cases for the tracking and state-vector verb glue: every entry of both
 * missions with random flag words, EXTVBACT and EB, and a VAC area address
 * in FIXLOC (FIXLOC + QPRET below 02000, as the executive assigns it). */
#include "lanec_suites.h"
#include "lanec_ev_track.h"
#include "lanec_symbols.h"

#define ENTRIES 48u

static const uint16_t banks[4][2] = { { LANEC_LM, 040u }, { LANEC_LM, 042u }, { LANEC_LM, 043u }, { LANEC_CM, 043u } };

static void range(lanec_meta *m, uint16_t bank, uint16_t lo, uint16_t hi)
{
    m->range[m->ranges].bank = bank; m->range[m->ranges].lo = lo; m->range[m->ranges].hi = hi; ++m->ranges;
}

static void make(uint32_t n, apollo_overlay_state *s, lanec_io *io, lanec_meta *m)
{
    uint16_t *w = s->words, e = (uint16_t)(n % ENTRIES), i, count = 0, bank;
    const uint16_t *entry = 0;
    uint32_t r = n * 747796405u + 1u;
    lanec_fill(s, io, n);
    for (i = 0; i < 4u; ++i) {
        entry = lanec_ev_track_entries((enum lanec_mission)banks[i][0], banks[i][1], &count);
        if (e < count) break;
        e = (uint16_t)(e - count);
    }
    m->mission = (enum lanec_mission)banks[i][0];
    bank = banks[i][1];
    lanec_enter(s, io, bank, entry[e]);
    w[LANEC_Q] = (uint16_t)(04000u + lanec_rand(&r) % 04000u);
    w[LANEC_LM_FIXLOC] = (uint16_t)(lanec_rand(&r) % (02000u - LANEC_LM_QPRET));
    m->stops = 1;
    m->stop[0].bank = 043u;
    if (m->mission == LANEC_LM) {
        m->stop[0].address = LANEC_LM_VERB69;
        if (bank == 040u) range(m, 040u, LANEC_LM_DSPRRLOS, LANEC_LM_RRLOSDSP + 46u);
        else if (bank == 042u) range(m, 042u, LANEC_LM_TRMTRAK1, LANEC_LM_TRMTRAK1 + 22u);
        else { range(m, 043u, LANEC_LM_VERB69, LANEC_LM_VERB69); range(m, 043u, LANEC_LM_ATTACHIT, LANEC_LM_ATTACHIT + 30u); }
    } else {
        m->stop[0].address = LANEC_CM_VERB69;
        range(m, 043u, LANEC_CM_VERB69, LANEC_CM_VERB69);
        range(m, 043u, LANEC_CM_TRACKTRM, LANEC_CM_CLEANOUT + 2u);
        range(m, 043u, LANEC_CM_SETVHFLG, LANEC_CM_LMTOCM + 14u);
    }
}

const lanec_suite lanec_suite_ev_track = { "ev-track", ENTRIES * 48u, make, lanec_ev_track_dispatch };
