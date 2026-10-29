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

#define CASES 1024u
static void make(uint32_t n, apollo_overlay_state *s, lanec_io *io, lanec_meta *m)
{
    p66_bias_io p;
    enum dsky_nv_mission mission;
    /* past the spread: the module's own states with the pair in low erasable */
    uint16_t k = n < CASES ? (uint16_t)lanec_pw_spread(n, CASES, DSKY_TWO_INTEGER_CASES)
                           : (uint16_t)((n - CASES) * 127u % DSKY_TWO_INTEGER_CASES);    /* then integers past 99 */
    lanec_fill(s, io, n);
    dsky_two_integer_input(s, &p, k, &mission);
    if (n >= CASES && n < CASES + LANEC_PW_LOW_NOUN) lanec_pw_low_noun(s, n - CASES);
    else if (n >= CASES) {    /* integers past 99 (DSPDECVN shows them modulo 100), and -0 (00: LM P65) */
        static const uint16_t big[9] = {100u, 101u, 199u, 999u, 1000u, 9999u, 16383u, 037777u, 077777u};
        uint16_t *w = s->words, j = (uint16_t)(n - CASES - LANEC_PW_LOW_NOUN), at;
        at = w[0140] == 2u ? (uint16_t)(w[0150u + (w[0117] <= 2u ? w[0117] : 0u)] & 03777u)
                           : w[0145] < 01400u ? w[0145] : (uint16_t)((w[3] & 03400u) + (w[0145] & 0377u));
        if (j % 2u) w[0154] = big[(j / 2u) % 9u];
        if (at + 1u < 2048u) w[at + 1u] = big[(j / 2u + 3u) % 9u];
    }
    io->superbank = p.superbank; io->interrupt_enabled = p.interrupt_enabled;
    lanec_pw_settle(s);
    m->mission = mission == DSKY_NV_CM ? LANEC_CM : LANEC_LM;
    lanec_pw_exit_stop(m, s, io, lanec_pw_two_integer_run);
}
const lanec_suite lanec_suite_pw_two_integer = { "pw-two-integer", CASES + 2u * LANEC_PW_LOW_NOUN, make, lanec_pw_two_integer_run };
