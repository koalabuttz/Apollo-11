/* SPDX-License-Identifier: MIT */
/* pw-hours-output: HMSOUT and its helper arithmetic, eight explicit bank 42
 * stages from the fresh entry to the saved ENTRET (src/dsky_hours_output.c).
 * The stage is the one whose source entry Z holds. Each case starts from a
 * generated fresh state (8192 numbers, the generator returns the mission)
 * moved n%8 stages along with the native module, then runs one stage.
 * Generator numbers 0-8191 are LM, 8192-16383 CM; in each half the second
 * 4096 repeat the first with the time in low erasable (TIME2/TIME1 of noun
 * 36 and other unswitched words from 010), where the read bound 010 is at
 * its edge. */
#include "lanec_pw_b.h"
#include "dsky_hours_output.h"
#include "dsky_hours_output_cases.h"

#define CASES 2048u
static void make(uint32_t n, apollo_overlay_state *s, lanec_io *io, lanec_meta *m)
{
    p66_bias_io p; enum dsky_nv_mission mission;
    static const uint16_t low[8] = {024u, 010u, 026u, 030u, 034u, 046u, 060u, 076u};
    uint16_t k = (uint16_t)lanec_pw_spread(n, CASES, 2u * DSKY_HOURS_OUTPUT_CASES), j = (uint16_t)(k % 4096u);
    lanec_fill(s, io, n);
    dsky_hours_output_input(s, &p, (uint16_t)(k / 8192u * 4096u + j), &mission);
    if (k % 8192u >= 4096u) {
        uint16_t old = s->words[0145], at = low[(j / 16u) % 8u];
        s->words[at] = s->words[old]; s->words[at + 1u] = s->words[old + 1u];
        s->words[0145] = s->words[0150u + s->words[0117]] = at;
    }
    io->superbank = p.superbank; io->interrupt_enabled = p.interrupt_enabled;
    lanec_pw_settle(s);
    m->mission = mission == DSKY_NV_CM ? LANEC_CM : LANEC_LM;
    lanec_pw_advance(s, io, m->mission, lanec_pw_hours_output_run, n % 8u);
    lanec_pw_exit_stop(m, s, io, lanec_pw_hours_output_run);
}
const lanec_suite lanec_suite_pw_hours_output = { "pw-hours-output", CASES, make, lanec_pw_hours_output_run };
