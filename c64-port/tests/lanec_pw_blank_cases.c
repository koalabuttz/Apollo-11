/* SPDX-License-Identifier: MIT */
/* pw-blank: BLNKSUB1 and the fixed-fixed return of the blanking body
 * (src/dsky_blank.c). Generator numbers 0-4095 are LM, 4096-8191 CM; bit
 * 2048 picks the entry, which run() reads back from Z (BLNKSUB1 is in bank
 * 40, the return in fixed-fixed). */
#include "lanec_pw.h"
#include "dsky_blank.h"
#include "dsky_blank_cases.h"

static enum lanec_status run(apollo_overlay_state *s, lanec_io *io, enum lanec_mission mission)
{
    enum dsky_blank_entry entry = s->words[LANEC_Z] >= 04000u ? DSKY_BLANK_RETURN : DSKY_BLNKSUB1;
    int ok;
    if (mission == LANEC_LM) {
        p66_bias_io p; p.superbank = io->superbank; p.interrupt_enabled = io->interrupt_enabled;
        ok = dsky_blank(s, &p, entry) == P66_BIAS_OK;
        if (ok) { io->superbank = p.superbank; io->interrupt_enabled = p.interrupt_enabled; }
    } else {
        cm_native_io c = {0}; c.superbank = io->superbank; c.interrupt_enabled = io->interrupt_enabled;
        ok = cm_blank(s, &c, entry) == EXEC_OK;
        if (ok) { io->superbank = c.superbank; io->interrupt_enabled = c.interrupt_enabled; }
    }
    return ok ? LANEC_OK : LANEC_OUT_OF_SCOPE;
}
#define CASES 1024u
static void make(uint32_t n, apollo_overlay_state *s, lanec_io *io, lanec_meta *m)
{
    p66_bias_io p;
    uint16_t k = (uint16_t)lanec_pw_spread(n, CASES, 8192u);
    lanec_fill(s, io, n);
    dsky_blank_input(s, &p, k);
    io->superbank = p.superbank; io->interrupt_enabled = p.interrupt_enabled;
    lanec_pw_settle(s);
    m->mission = k >= 4096u ? LANEC_CM : LANEC_LM;
    lanec_pw_exit_stop(m, s, io, run);
}
const lanec_suite lanec_suite_pw_blank = { "pw-blank", CASES, make, run };
