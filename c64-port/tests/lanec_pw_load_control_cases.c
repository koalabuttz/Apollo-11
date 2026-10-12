/* SPDX-License-Identifier: MIT */
/* pw-load-control: VBPROC/VBTERM/VBRESEQ/PROCKEY through RECALTST and
 * ENDOFJOB, or direct RECALTST (src/dsky_load_control.c). Generator numbers
 * 0-4095 are LM, 4096-8191 CM. */
#include "lanec_pw_c.h"
#include "dsky_load_control.h"
#include "dsky_load_control_cases.h"

static enum lanec_status load_control_run(apollo_overlay_state *s, lanec_io *io, enum lanec_mission mission)
{
    if (mission == LANEC_LM) {
        dsky_charin_io p; lanec_pw_charin_in(io, &p);
        if (dsky_load_control(s, &p) != P66_BIAS_OK) return LANEC_OUT_OF_SCOPE;
        lanec_pw_charin_out(io, &p);
    } else {
        cm_native_io c = {0}; uint16_t ev = 0; lanec_pw_cm_in(io, &c);
        if (cm_load_control(s, &c, &ev) != EXEC_OK) return LANEC_OUT_OF_SCOPE;
        lanec_pw_cm_out(io, &c);
    }
    return LANEC_OK;
}
#define CASES 1024u
LANEC_PW_MAKE(make, dsky_charin_io, dsky_load_control_input, lanec_pw_charin_out, load_control_run, 8192u, CASES)
const lanec_suite lanec_suite_pw_load_control = { "pw-load-control", CASES, make, load_control_run };
