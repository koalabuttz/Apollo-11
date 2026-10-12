/* SPDX-License-Identifier: MIT */
/* pw-test-noun: TESTNN with a fresh noun-table read and the normal/mixed
 * continuation (src/dsky_test_noun.c). Generator numbers 0-8191 are LM,
 * 8192-16383 CM. */
#include "lanec_pw.h"
#include "dsky_test_noun.h"
#include "dsky_test_noun_cases.h"

#define CASES 1024u
static void make(uint32_t n, apollo_overlay_state *s, lanec_io *io, lanec_meta *m)
{
    p66_bias_io p;
    enum dsky_nv_mission mission;
    uint16_t k = (uint16_t)lanec_pw_spread(n, CASES, DSKY_TEST_NOUN_CASES);
    lanec_fill(s, io, n);
    dsky_test_noun_input(s, &p, k, &mission);
    io->superbank = p.superbank; io->interrupt_enabled = p.interrupt_enabled;
    lanec_pw_settle(s);
    m->mission = mission == DSKY_NV_CM ? LANEC_CM : LANEC_LM;
    lanec_pw_exit_stop(m, s, io, lanec_pw_test_noun_run);
}
const lanec_suite lanec_suite_pw_test_noun = { "pw-test-noun", CASES, make, lanec_pw_test_noun_run };
