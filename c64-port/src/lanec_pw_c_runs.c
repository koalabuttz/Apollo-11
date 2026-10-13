/* SPDX-License-Identifier: MIT */
/* The lanec_run adapters of the pw-* suites of batch c (include/lanec_pw_c_run.h). */
#include "lanec_pw_c_run.h"
#include "dsky_input_store.h"
#include "dsky_input_time.h"
#include "dsky_jam_control.h"
#include "dsky_lamp_lifecycle.h"
#include "dsky_lamp_release.h"
#include "dsky_lamp_test.h"
#include "dsky_load_complete.h"
#include "dsky_load_control.h"
#include "dsky_load_request.h"
#include "dsky_load_store.h"
#include "dsky_machine_cadr.h"
#include "dsky_major_request.h"
#include "dsky_mixed_noun.h"
#include "dsky_mode_admit.h"

LANEC_PW_LAMP_RUN(lanec_pw_input_store_run, dsky_input_store, cm_input_store, enum dsky_input_store_route, DSKY_INPUT_STORE_CALLER)

LANEC_PW_LAMP_RUN(lanec_pw_input_time_run, dsky_input_time, cm_input_time, enum dsky_input_time_route, DSKY_INPUT_TIME_LOADED)

enum lanec_status lanec_pw_jam_control_run(apollo_overlay_state *s, lanec_io *io, enum lanec_mission mission)
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

LANEC_PW_LAMP_RUN(lanec_pw_lamp_lifecycle_run, dsky_lamp_lifecycle, cm_lamp_lifecycle, enum dsky_lamp_lifecycle_route, DSKY_LAMP_LIFECYCLE_END)

LANEC_PW_LAMP_RUN(lanec_pw_lamp_release_run, dsky_lamp_release, cm_lamp_release, enum dsky_lamp_release_route, DSKY_LAMP_RELEASE_END)

LANEC_PW_LAMP_RUN(lanec_pw_lamp_test_run, dsky_lamp_test, cm_lamp_test, enum dsky_lamp_route, DSKY_LAMP_WAIT)

enum lanec_status lanec_pw_load_complete_run(apollo_overlay_state *s, lanec_io *io, enum lanec_mission mission)
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

enum lanec_status lanec_pw_load_control_run(apollo_overlay_state *s, lanec_io *io, enum lanec_mission mission)
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

LANEC_PW_LAMP_RUN(lanec_pw_load_request_run, dsky_load_request, cm_load_request, enum dsky_load_request_route, DSKY_LOAD_REQUEST_END)

LANEC_PW_LAMP_RUN(lanec_pw_load_store_run, dsky_load_store, cm_load_store, enum dsky_load_store_route, DSKY_LOAD_STORE_COMPONENT)

LANEC_PW_ROUTE_RUN(lanec_pw_machine_cadr_run, dsky_machine_cadr, cm_machine_cadr, enum dsky_machine_route, DSKY_MACHINE_VERB_FAN)

enum lanec_status lanec_pw_major_request_run(apollo_overlay_state *s, lanec_io *io, enum lanec_mission mission)
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

LANEC_PW_ROUTE_RUN(lanec_pw_mixed_noun_run, dsky_mixed_noun, cm_mixed_noun, enum dsky_mixed_route, DSKY_MIXED_VERB)

LANEC_PW_ROUTE_RUN(lanec_pw_mode_admit_run, dsky_mode_admit, cm_mode_admit, enum dsky_mode_admit_route, DSKY_MODE_ADMIT_CALLER)
