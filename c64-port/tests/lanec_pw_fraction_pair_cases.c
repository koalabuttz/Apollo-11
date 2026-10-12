/* SPDX-License-Identifier: MIT */
/* pw-fraction-pair: DSPDC2NR (LM 40,3266, CM 40,3173) to the caller or the
 * CCSHOLE frontier (src/dsky_fraction_pair.c). The generator (16384
 * numbers) returns the mission.
 * Known: the generator's caller Q (0100-7577) sometimes equals an address
 * the original path passes before it returns (bank 40 code, the fixed-fixed
 * helpers, erasable 0115/0144); the module's own reference stops only after
 * the return (Q 0116), the oracle at the first arrival, so those 33 cases
 * differ. */
#include "lanec_pw_b.h"
#include "dsky_fraction_pair.h"
#include "dsky_fraction_pair_cases.h"

#define CASES 1024u
static void make(uint32_t n, apollo_overlay_state *s, lanec_io *io, lanec_meta *m)
{
    p66_bias_io p; enum dsky_nv_mission mission;
    uint16_t k = (uint16_t)lanec_pw_spread(n, CASES, DSKY_FRACTION_PAIR_CASES);
    lanec_fill(s, io, n);
    dsky_fraction_pair_input(s, &p, k, &mission);
    io->superbank = p.superbank; io->interrupt_enabled = p.interrupt_enabled;
    lanec_pw_settle(s);
    m->mission = mission == DSKY_NV_CM ? LANEC_CM : LANEC_LM;
    lanec_pw_exit_stop(m, s, io, lanec_pw_fraction_pair_run);
}
const lanec_suite lanec_suite_pw_fraction_pair = { "pw-fraction-pair", CASES, make, lanec_pw_fraction_pair_run };
