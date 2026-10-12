/* SPDX-License-Identifier: MIT */
/* pw-jam-control: fixed-fixed JAMPROC/JAMTERM through the SUPER write,
 * POSTJUMP and VBPROC/VBTERM, RECALTST and ENDOFJOB (src/dsky_jam_control.c).
 * Generator numbers 0-4095 are LM, 4096-8191 CM. */
#include "lanec_pw_c.h"
#include "dsky_jam_control.h"
#include "dsky_jam_control_cases.h"

static enum lanec_status jam_control_run(apollo_overlay_state *s, lanec_io *io, enum lanec_mission mission)
{
    uint16_t wrote_super = 0;
    if (mission == LANEC_LM) {
        dsky_charin_io p; lanec_pw_charin_in(io, &p);
        if (dsky_jam_control(s, &p, &wrote_super) != P66_BIAS_OK) return LANEC_OUT_OF_SCOPE;
        lanec_pw_charin_out(io, &p);
    } else {
        cm_native_io c = {0}; uint16_t ev = 0; lanec_pw_cm_in(io, &c);
        if (cm_jam_control(s, &c, &ev, &wrote_super) != EXEC_OK) return LANEC_OUT_OF_SCOPE;
        lanec_pw_cm_out(io, &c);
    }
    return LANEC_OK;
}
#define CASES 1024u
LANEC_PW_MAKE(make, dsky_charin_io, dsky_jam_control_input, lanec_pw_charin_out, jam_control_run, 8192u, CASES)
const lanec_suite lanec_suite_pw_jam_control = { "pw-jam-control", CASES, make, jam_control_run };
