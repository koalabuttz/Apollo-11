/* SPDX-License-Identifier: MIT */
/* pw-decimal-load: DECDSP's load of the noun component (src/dsky_decimal_load.c).
 * The generator's lower half is LM, the upper half CM. */
#include "lanec_pw_a.h"
#include "dsky_decimal_load.h"
#include "dsky_decimal_load_cases.h"

#define CASES 1024u
static void make(uint32_t n, apollo_overlay_state *s, lanec_io *io, lanec_meta *m)
{
    p66_bias_io p; enum dsky_nv_mission mission;
    /* past the spread: the module's own states with the noun at the top of
     * erasable (01776, 01777: the last words come from fixed memory, as the
     * LM's TCDH for noun 13) */
    uint32_t i = n - CASES;
    lanec_fill(s, io, n);
    dsky_decimal_load_input(s, &p, (uint16_t)(n < CASES ? lanec_pw_spread(n, CASES, DSKY_DECIMAL_LOAD_CASES)
                                                        : i * 127u % DSKY_DECIMAL_LOAD_CASES), &mission);
    if (n >= CASES) {
        uint16_t *w = s->words, f = w[0140] == 1u ? 0147u : 0146u;
        w[0145] = (uint16_t)(01776u + (i & 1u));
        w[f] = (uint16_t)((w[f] & ~06000u) | ((1u + (i / 2u) % 2u) << 10));
    }
    io->superbank = p.superbank; io->interrupt_enabled = p.interrupt_enabled;
    lanec_pw_settle(s);
    m->mission = mission == DSKY_NV_CM ? LANEC_CM : LANEC_LM;
    lanec_pw_exit_stop(m, s, io, lanec_pw_decimal_load_run);
}
const lanec_suite lanec_suite_pw_decimal_load = { "pw-decimal-load", CASES + 16u, make, lanec_pw_decimal_load_run };
