/* SPDX-License-Identifier: MIT */
/* Cases for the Luminary099 EXTENDED_VERBS interpretive words: every entry
 * with random MOONTHIS/MOONOTH switch bits, a VAC area in FIXLOC (above the interpreter's
 * work area, in unswitched erasable), NEWJOB +0,
 * and at NEWOPS EDOP +0 with BANKSET = BB and INTBIT15 its bit 15, as the interpreter leaves them. */
#include "lanec_suites.h"
#include "lanec_ev_lmi.h"
#include "lanec_symbols.h"

static void make(uint32_t n, apollo_overlay_state *s, lanec_io *io, lanec_meta *m)
{
    uint16_t *w = s->words, count, bank, z;
    uint32_t r = n * 747796405u + 1u;
    const uint16_t *at;
    (void)lanec_ev_lmi_entry(0, &bank, &z, &count);
    at = lanec_ev_lmi_entry((uint16_t)(n % count), &bank, &z, &count);
    lanec_fill(s, io, n);
    m->mission = LANEC_LM;
    lanec_enter(s, io, bank, z);
    w[067] = 0;                                                     /* NEWJOB */
    w[0120] = (uint16_t)(0200u + lanec_rand(&r) % (01400u - 060u - 0200u)); /* FIXLOC */
    if (z == 06037u) {
        w[LANEC_Q] = *at;
    } else {
        w[0164] = *at; w[023] = 0; w[0165] = w[LANEC_BB];           /* LOC, EDOP, BANKSET */
        w[0115] = w[0165] & 040000u;                                /* INTBIT15 */
        /* RRLOSDSP lines of sight along Y (UNIT's alarm), and with only a
         * low-order Z left (UNIT's small-vector path, a DV with L = -0) */
        if (*at == LANEC_LM_RRLOSDSP + 6u && (n / count) % 4u == 0u) w[0154] = w[0155] = w[0161] = w[0162] = 0;
        if (*at == LANEC_LM_RRLOSDSP + 6u && (n / count) % 4u == 1u) w[0154] = w[0155] = w[0160] = w[0161] = 0;
    }
    /* the interpreter: fixed-fixed 06000-07777, banks 0 and 1, the TC in Q
     * and A (registers 0 and 2) and SWCALL for RTB */
    m->ranges = 5;
    m->range[0].bank = LANEC_ANY_BANK; m->range[0].lo = 06000u; m->range[0].hi = 07777u;
    m->range[3].bank = LANEC_ANY_BANK; m->range[3].lo = 0u; m->range[3].hi = 0177u;
    m->range[4].bank = LANEC_ANY_BANK; m->range[4].lo = 04620u; m->range[4].hi = 04632u;
    m->range[1].bank = 0; m->range[1].lo = 02000u; m->range[1].hi = 03777u;
    m->range[2].bank = 1; m->range[2].lo = 02000u; m->range[2].hi = 03777u;
    m->newops = 06070u;
    m->locs = 5;
    m->loc[4].bank = 040u; m->loc[4].lo = LANEC_LM_RRLOSDSP + 4u; m->loc[4].hi = LANEC_LM_RRLOSDSP + 046u;
    m->loc[0].bank = 042u; m->loc[0].lo = LANEC_LM_TRMTRAK1 + 7u; m->loc[0].hi = LANEC_LM_TRMTRAK1 + 9u;
    m->loc[1].bank = 043u; m->loc[1].lo = LANEC_LM_ATTACHIT + 1u; m->loc[1].hi = LANEC_LM_ATTACHIT + 9u;
    m->loc[2].bank = 043u; m->loc[2].lo = LANEC_LM_ATTACHIT + 16u; m->loc[2].hi = LANEC_LM_ATTACHIT + 21u;
    m->loc[3].bank = 043u; m->loc[3].lo = LANEC_LM_TCPIN; m->loc[3].hi = LANEC_LM_TCPIN + 1u;
}

const lanec_suite lanec_suite_ev_lmi = { "ev-lmi", 15u * 64u, make, lanec_ev_lmi_dispatch };
