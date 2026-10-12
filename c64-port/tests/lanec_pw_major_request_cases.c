/* SPDX-License-Identifier: MIT */
/* pw-major-request: MMCHANG, its operator return or REQMM through the
 * ENDINST/NVSBENDL/MONBACK, V37 and ALMCYCLE handoffs
 * (src/dsky_major_request.c). Generator numbers 0-4095 are LM, 4096-8191 CM. */
#include "lanec_pw_c.h"
#include "dsky_major_request.h"
#include "dsky_major_request_cases.h"

static enum lanec_status major_request_run(apollo_overlay_state *s, lanec_io *io, enum lanec_mission mission)
{
    enum dsky_major_request_route r = DSKY_MAJOR_REQUEST_END;
    if (mission == LANEC_LM) {
        dsky_charin_io p; lanec_pw_charin_in(io, &p);
        if (dsky_major_request(s, &p, &r) != P66_BIAS_OK) return LANEC_OUT_OF_SCOPE;
        lanec_pw_charin_out(io, &p);
    } else {
        cm_native_io c = {0}; uint16_t ev = 0; lanec_pw_cm_in(io, &c);
        if (cm_major_request(s, &c, &ev, &r) != EXEC_OK) return LANEC_OUT_OF_SCOPE;
        lanec_pw_cm_out(io, &c);
    }
    return LANEC_OK;
}
#define CASES 1024u
LANEC_PW_MAKE(make, dsky_charin_io, dsky_major_request_input, lanec_pw_charin_out, major_request_run, 8192u, CASES)
const lanec_suite lanec_suite_pw_major_request = { "pw-major-request", CASES, make, major_request_run };
