/* SPDX-License-Identifier: MIT */
/* pw-sign-output: +ON / -ON for the data rows (src/dsky_sign_output.c), the
 * sign chosen by Z as the generator does. Generator numbers 0-8191 are LM,
 * 8192-16383 CM. */
#include "lanec_pw_e.h"
#include "dsky_sign_output.h"
#include "dsky_sign_output_cases.h"

/* The oracle stops where Z first reaches the caller (Q at entry). A caller
 * the original path itself passes first cannot be told apart there, so the
 * run refuses callers inside the bank-40 code +ON/-ON and 11DSPIN execute:
 * the addresses the oracle traced for this suite (LANEC_TRACE). */
static const uint16_t path[2][4][2] = {
    {{02324, 02327}, {02413, 02442}, {03347, 03377}, {03404, 03411}},
    {{02225, 02230}, {02314, 02343}, {03254, 03304}, {03311, 03316}}};
static enum lanec_status sign_output_run(apollo_overlay_state *s, lanec_io *io, enum lanec_mission mission)
{
    enum dsky_sign_output_route route;
    uint16_t base = mission == LANEC_CM ? 02314u : 02413u, z = s->words[LANEC_Z], q = s->words[LANEC_Q];
    unsigned i;
    p66_bias_io p;
    if (z != base && z != base + 020u) return LANEC_OUT_OF_SCOPE;
    for (i = 0; i < 4u; ++i)
        if (q >= path[mission == LANEC_CM][i][0] && q <= path[mission == LANEC_CM][i][1]) return LANEC_OUT_OF_SCOPE;
    p.superbank = io->superbank; p.interrupt_enabled = io->interrupt_enabled;
    if (dsky_sign_output(s, &p, mission == LANEC_CM ? DSKY_NV_CM : DSKY_NV_LM, (uint8_t)(z != base), &route) != P66_BIAS_OK)
        return LANEC_OUT_OF_SCOPE;
    io->superbank = p.superbank; io->interrupt_enabled = p.interrupt_enabled;
    return LANEC_OK;
}
#define CASES 1024u
static void make(uint32_t n, apollo_overlay_state *s, lanec_io *io, lanec_meta *m)
{
    p66_bias_io p;
    enum dsky_nv_mission mission;
    uint8_t negative;
    uint16_t k = (uint16_t)lanec_pw_spread(n, CASES, DSKY_SIGN_OUTPUT_CASES);
    lanec_fill(s, io, n);
    dsky_sign_output_input(s, &p, k, &mission, &negative);
    io->superbank = p.superbank; io->interrupt_enabled = p.interrupt_enabled;
    lanec_pw_settle(s);
    m->mission = mission == DSKY_NV_CM ? LANEC_CM : LANEC_LM;
    LANEC_PW_E_STOP(m, s, io, sign_output_run);
}
const lanec_suite lanec_suite_pw_sign_output = { "pw-sign-output", CASES, make, sign_output_run };
