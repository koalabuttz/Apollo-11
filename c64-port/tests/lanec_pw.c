/* SPDX-License-Identifier: MIT */
#include "lanec_pw.h"

void lanec_pw_exit_stop(lanec_meta *m, const apollo_overlay_state *s, const lanec_io *io, lanec_run run)
{
#ifdef __mos__  /* the stop is for the oracle; the C64 compares frozen hashes */
    (void)m; (void)s; (void)io; (void)run;
#else
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
    m->stop_q = 1u;   /* the program may pass the exit address earlier with another Q */
    m->stop_q_value = t.words[LANEC_Q];
#endif
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
    if (n >= cases) return domain + (n - cases);    /* a suite's own targeted cases */
    return n * step + (step > 1u ? (n * 2654435761u >> 20) % step : 0u);
}

/* DECTOBIN's carry out of MPAC+1 (ADS MPAC; TC DECEND): a decimal register
 * holding 1638, 3276, 4915, 6553 or 9830 takes a fifth digit that carries
 * ten times it past 2^14. Case i of LANEC_PW_FIFTH_CHAR over a CHARIN image. */
void lanec_pw_fifth_char(apollo_overlay_state *s, unsigned i)
{
    static const uint16_t value[] = { 3276, 3276, 4915, 6553, 1638, 9830, 4915, 6553 };
    static const uint16_t key[] = { 8, 9, 2, 6, 4, 7, 5, 9 };
    static const uint16_t count[] = { 0, 5, 10, 0, 5, 10, 5, 10 };
    uint16_t *w = s->words, relative = (uint16_t)(count[i] < 5 ? 4 : count[i] < 10 ? 3 : 2);
    w[0154] = key[i];
    w[0777] = count[i];
    w[01000] = (uint16_t)(1u + i % 2u);
    w[01001u + relative] = value[i];
    if (w[01023u + count[i] / 2u] == 0) w[01023u + count[i] / 2u] = 073777;
}
