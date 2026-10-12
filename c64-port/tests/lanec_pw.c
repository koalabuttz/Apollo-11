/* SPDX-License-Identifier: MIT */
#include "lanec_pw.h"

void lanec_pw_exit_stop(lanec_meta *m, const apollo_overlay_state *s, const lanec_io *io, lanec_run run)
{
    static apollo_overlay_state t;
    lanec_io u = *io;
    uint16_t z;
    t = *s;
    m->ranges = 1u;   /* every erasable and fixed address, any bank */
    m->range[0].bank = LANEC_ANY_BANK; m->range[0].lo = 0u; m->range[0].hi = 07777u;
    if (run(&t, &u, m->mission) != LANEC_OK) return;
    z = t.words[LANEC_Z];
    m->stops = 1u;
    m->stop[0].address = z;
    m->stop[0].bank = z >= 02000u && z < 04000u ? lanec_current_bank(&t, &u) : LANEC_ANY_BANK;
}

void lanec_pw_settle(apollo_overlay_state *s)
{
    uint16_t i, *w = s->words;
    w[LANEC_L] = (uint16_t)((w[LANEC_L] & 077777u) | ((w[LANEC_L] << 1) & 0100000u));
    w[035] = w[036] = 0;
    for (i = 047; i <= 054; ++i) w[i] = 0;
}

uint32_t lanec_pw_spread(uint32_t n, uint32_t cases, uint32_t domain)
{
    uint32_t step = domain / cases;
    return n * step + (step > 1u ? (n * 2654435761u >> 20) % step : 0u);
}
