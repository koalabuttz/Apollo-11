/* SPDX-License-Identifier: MIT */
/* pw-decimal-row: the DECDSP row call and its return (src/dsky_decimal_row.c).
 * The generator's lower half is LM, the upper half CM. The entry (CALL at
 * 02566, RETURN in fixed-fixed) is read back from Z. */
#include "lanec_pw_a.h"
#include "dsky_decimal_row.h"
#include "dsky_decimal_row_cases.h"

static enum lanec_status run(apollo_overlay_state *s, lanec_io *io, enum lanec_mission mission)
{
    p66_bias_io p; enum lanec_status r;
    enum dsky_decimal_row_entry entry = s->words[LANEC_Z] == 02566u ? DSKY_DECIMAL_ROW_CALL : DSKY_DECIMAL_ROW_RETURN;
    p.superbank = io->superbank; p.interrupt_enabled = io->interrupt_enabled;
    r = dsky_decimal_row(s, &p, lanec_pw_nv(mission), entry) == P66_BIAS_OK ? LANEC_OK : LANEC_OUT_OF_SCOPE;
    if (r == LANEC_OK) { io->superbank = p.superbank; io->interrupt_enabled = p.interrupt_enabled; }
    return r;
}
#define CASES 1024u
static void make(uint32_t n, apollo_overlay_state *s, lanec_io *io, lanec_meta *m)
{
    p66_bias_io p; enum dsky_nv_mission mission; enum dsky_decimal_row_entry entry;
    lanec_fill(s, io, n);
    dsky_decimal_row_input(s, &p, (uint16_t)lanec_pw_spread(n, CASES, DSKY_DECIMAL_ROW_CASES), &mission, &entry);
    io->superbank = p.superbank; io->interrupt_enabled = p.interrupt_enabled;
    lanec_pw_settle(s);
    m->mission = mission == DSKY_NV_CM ? LANEC_CM : LANEC_LM;
    lanec_pw_exit_stop(m, s, io, run);
}
const lanec_suite lanec_suite_pw_decimal_row = { "pw-decimal-row", CASES, make, run };
