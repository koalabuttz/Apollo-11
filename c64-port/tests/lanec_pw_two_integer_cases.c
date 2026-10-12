/* SPDX-License-Identifier: MIT */
/* pw-two-integer: 2INTOUT, two positive integers 0-99 (src/dsky_two_integer.c).
 * The module serves 2INTOUT as five stages split at its source call/return
 * boundaries (5BLANK, sign, first and second value, return); the run chains
 * them from the 2INTOUT entry, as the module's differential does, so one
 * case is the whole original path to DSPDCEND+2. Generator numbers 0-4095
 * are LM, 4096-8191 CM. */
#include "lanec_pw.h"
#include "dsky_two_integer.h"
#include "dsky_two_integer_cases.h"

static enum lanec_status two_integer_run(apollo_overlay_state *s, lanec_io *io, enum lanec_mission mission)
{
    static apollo_overlay_state before;
    p66_bias_io p;
    unsigned k;
    before = *s;
    p.superbank = io->superbank; p.interrupt_enabled = io->interrupt_enabled;
    for (k = 0; k < 5u; ++k)
        if (dsky_two_integer(s, &p, mission == LANEC_CM ? DSKY_NV_CM : DSKY_NV_LM, (enum dsky_two_integer_stage)k) != P66_BIAS_OK) {
            *s = before;
            return LANEC_OUT_OF_SCOPE;
        }
    io->superbank = p.superbank; io->interrupt_enabled = p.interrupt_enabled;
    return LANEC_OK;
}
#define CASES 1024u
static void make(uint32_t n, apollo_overlay_state *s, lanec_io *io, lanec_meta *m)
{
    p66_bias_io p;
    enum dsky_nv_mission mission;
    uint16_t k = (uint16_t)lanec_pw_spread(n, CASES, DSKY_TWO_INTEGER_CASES);
    lanec_fill(s, io, n);
    dsky_two_integer_input(s, &p, k, &mission);
    io->superbank = p.superbank; io->interrupt_enabled = p.interrupt_enabled;
    lanec_pw_settle(s);
    m->mission = mission == DSKY_NV_CM ? LANEC_CM : LANEC_LM;
    lanec_pw_exit_stop(m, s, io, two_integer_run);
}
const lanec_suite lanec_suite_pw_two_integer = { "pw-two-integer", CASES, make, two_integer_run };
