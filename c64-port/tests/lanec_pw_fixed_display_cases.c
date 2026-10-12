/* SPDX-License-Identifier: MIT */
/* pw-fixed-display: DSPFMEM (41,3353) through SUPDACAL, the passive fixed
 * word read and the octal formatter to ENDOFJOB or CCSHOLE
 * (src/dsky_fixed_display.c), three explicit stages selected by Z. The DATA
 * stage gets its word from the module's own passive reader
 * (src/dsky_fixed_display_data.c), as tools/dsky_fixed_display_snapshot.c
 * does. Each case starts from a generated state (81920 numbers, the
 * generator returns the mission) moved n%3 stages along with the native
 * module, then runs one stage. */
#include "lanec_pw_b.h"
#include "dsky_fixed_display.h"
#include "dsky_fixed_display_cases.h"

static enum lanec_status fixed_display_run(apollo_overlay_state *s, lanec_io *io, enum lanec_mission mission)
{
    unsigned stage;
    enum dsky_nv_mission nv = lanec_pw_nv(mission);
    enum dsky_fixed_display_route r;
    for (stage = 0; stage < 3u; ++stage) {
        p66_bias_io p; agc_word value = 0; uint16_t address;
        p.superbank = io->superbank; p.interrupt_enabled = io->interrupt_enabled;
        if (stage == DSKY_FIXED_DATA &&
            (!dsky_fixed_display_address(s, &p, nv, &address) || !dsky_fixed_display_data(nv, address, &value, 0)))
            continue;
        if (dsky_fixed_display(s, &p, nv, (enum dsky_fixed_display_stage)stage, value, &r) == P66_BIAS_OK) {
            io->superbank = p.superbank; io->interrupt_enabled = p.interrupt_enabled;
            return LANEC_OK;
        }
    }
    return LANEC_OUT_OF_SCOPE;
}
#define CASES 2048u
static void make(uint32_t n, apollo_overlay_state *s, lanec_io *io, lanec_meta *m)
{
    p66_bias_io p; enum dsky_nv_mission mission;
    uint32_t k = lanec_pw_spread(n, CASES, DSKY_FIXED_DISPLAY_CASES);
    lanec_fill(s, io, n);
    dsky_fixed_display_input(s, &p, k, &mission);
    io->superbank = p.superbank; io->interrupt_enabled = p.interrupt_enabled;
    lanec_pw_settle(s);
    m->mission = mission == DSKY_NV_CM ? LANEC_CM : LANEC_LM;
    lanec_pw_advance(s, io, m->mission, fixed_display_run, n % 3u);
    lanec_pw_exit_stop(m, s, io, fixed_display_run);
}
const lanec_suite lanec_suite_pw_fixed_display = { "pw-fixed-display", CASES, make, fixed_display_run };
