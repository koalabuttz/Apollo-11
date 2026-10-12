/* SPDX-License-Identifier: MIT */
/* pw-load-complete: LOADLV through RELDSP, POSTJUMP/RECALTST and ENDOFJOB
 * (src/dsky_load_complete.c). Generator numbers 0-4095 are LM, 4096-8191 CM. */
#include "lanec_pw_c.h"
#include "dsky_load_complete.h"
#include "dsky_load_complete_cases.h"

static enum lanec_status load_complete_run(apollo_overlay_state *s, lanec_io *io, enum lanec_mission mission)
{
    if (mission == LANEC_LM) {
        dsky_lamp_io p; lanec_pw_lamp_in(io, &p);
        if (dsky_load_complete(s, &p) != P66_BIAS_OK) return LANEC_OUT_OF_SCOPE;
        lanec_pw_lamp_out(io, &p);
    } else {
        cm_native_io c = {0}; uint16_t ev = 0, rl = 0; lanec_pw_cm_in(io, &c);
        if (cm_load_complete(s, &c, &ev, &rl) != EXEC_OK) return LANEC_OUT_OF_SCOPE;
        lanec_pw_cm_out(io, &c);
    }
    return LANEC_OK;
}
#define CASES 1024u
LANEC_PW_MAKE(make, dsky_lamp_io, dsky_load_complete_input, lanec_pw_lamp_out, load_complete_run, 8192u, CASES)
const lanec_suite lanec_suite_pw_load_complete = { "pw-load-complete", CASES, make, load_complete_run };
