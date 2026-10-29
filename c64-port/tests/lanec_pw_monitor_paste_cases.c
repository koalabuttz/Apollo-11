/* SPDX-License-Identifier: MIT */
/* pw-monitor-paste: PASTEVB/PASTEOPT control stages through UPDATVB-1,
 * BLANKSUB or ENDOFJOB (src/dsky_monitor_paste.c). The generator chooses
 * mission and stage; each stage is accepted only at its own Z, so run()
 * takes the first stage the module accepts. The second half of each
 * mission's generator numbers repeats the first with PASTEVB entered from
 * another fixed bank (as after HMSOUT in bank 42 for noun 36) and BANKCALL's
 * saved bank (BUF2+1) to match. */
#include "lanec_pw.h"
#include "dsky_monitor_paste.h"
#include "dsky_monitor_paste_cases.h"

#define CASES 1024u
static void make(uint32_t n, apollo_overlay_state *s, lanec_io *io, lanec_meta *m)
{
    p66_bias_io p;
    enum dsky_nv_mission mission;
    enum dsky_monitor_paste_stage stage;
    static const uint16_t fbank[8] = {064000u, 060000u, 076000u, 040000u, 002000u, 000000u, 066000u, 026000u};
    uint16_t k = (uint16_t)lanec_pw_spread(n, CASES, 2u * DSKY_MONITOR_PASTE_CASES), j = (uint16_t)(k % 8192u);
    lanec_fill(s, io, n);
    dsky_monitor_paste_input(s, &p, (uint16_t)(k / 16384u * 8192u + j), &mission, &stage);
    if (k % 16384u >= 8192u) {
        uint16_t fb = fbank[(j / 3u) % 8u];
        if (stage == DSKY_MONITOR_PASTE_UPDATED) s->words[0134] = (uint16_t)(fb | (s->words[3] >> 8));
        else { s->words[4] = fb; s->words[6] = (uint16_t)(fb | (s->words[3] >> 8)); }
    }
    io->superbank = p.superbank; io->interrupt_enabled = p.interrupt_enabled;
    lanec_pw_settle(s);
    m->mission = mission == DSKY_NV_CM ? LANEC_CM : LANEC_LM;
    lanec_pw_exit_stop(m, s, io, lanec_pw_monitor_paste_run);
}
const lanec_suite lanec_suite_pw_monitor_paste = { "pw-monitor-paste", CASES, make, lanec_pw_monitor_paste_run };
