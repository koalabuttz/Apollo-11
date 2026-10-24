/* SPDX-License-Identifier: MIT */
/* pw-two-digit: Luminary DSPDECVN (40,3306) through SHORTMP and two DSPIN passes to
 * the caller (src/dsky_two_digit.c). Past the spread, A is -0: 2INTOUT passes
 * MPAC's -0 (LM P65's display, docs/LANEC-PIN-CERT.md), which shows as 00. */
#include "lanec_pw_g.h"
#include "dsky_two_digit.h"
#include "dsky_two_digit_cases.h"

#define CASES 1024u
#define MINUS_ZERO 32u
static void make(uint32_t n, apollo_overlay_state *s, lanec_io *io, lanec_meta *m)
{
    uint16_t k = n < CASES ? (uint16_t)lanec_pw_spread(n, CASES, DSKY_TWO_DIGIT_CASE_COUNT)
                           : (uint16_t)((n - CASES) * 97u % DSKY_TWO_DIGIT_CASE_COUNT);
    lanec_fill(s, io, n);
    dsky_two_digit_prepare(s, k);
    if (n >= CASES) s->words[0] = 0177777u;
    io->superbank = 0100u; io->interrupt_enabled = (uint16_t)(k % 2u);
    lanec_pw_settle(s);
    m->mission = LANEC_LM;
    lanec_pw_exit_stop(m, s, io, lanec_pw_two_digit_run);
}
const lanec_suite lanec_suite_pw_two_digit = { "pw-two-digit", CASES + MINUS_ZERO, make, lanec_pw_two_digit_run };
