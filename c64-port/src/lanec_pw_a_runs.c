/* SPDX-License-Identifier: MIT */
/* The lanec_run adapters of the pw-* suites of batch a (include/lanec_pw_a_run.h). */
#include "lanec_pw_a_run.h"
#include "dsky_address_request.h"
#include "dsky_alarm_cycle.h"
#include "dsky_alarm_recycle.h"
#include "dsky_charin_alarm.h"
#include "dsky_charin_clear.h"
#include "dsky_charin_release.h"
#include "dsky_charin_sign.h"
#include "dsky_decimal_consumer.h"
#include "dsky_decimal_general.h"
#include "dsky_decimal_load.h"
#include "dsky_decimal_raw.h"
#include "dsky_decimal_row.h"
#include "dsky_decimal_select.h"

static enum dsky_request_route address_request_route;
LANEC_PW_CHARIN_RUN(lanec_pw_address_request_run, dsky_address_request(s, &c, &address_request_route), cm_address_request(s, &x, &w, &address_request_route))

static enum dsky_alarm_cycle_route alarm_cycle_route;
LANEC_PW_CHARIN_RUN(lanec_pw_alarm_cycle_run, dsky_alarm_cycle(s, &c, &alarm_cycle_route), cm_alarm_cycle(s, &x, &w, &alarm_cycle_route))

static enum dsky_recycle_route alarm_recycle_route;
LANEC_PW_CHARIN_RUN(lanec_pw_alarm_recycle_run, dsky_alarm_recycle(s, &c, &alarm_recycle_route), cm_alarm_recycle(s, &x, &w, &alarm_recycle_route))

LANEC_PW_CHARIN_RUN(lanec_pw_charin_alarm_run, dsky_charin_alarm(s, &c), cm_charin_alarm(s, &x, &w))

LANEC_PW_CHARIN_RUN(lanec_pw_charin_clear_run, dsky_charin_clear(s, &c), cm_charin_clear(s, &x, &w))

static enum dsky_release_route charin_release_route;
LANEC_PW_CHARIN_RUN(lanec_pw_charin_release_run, dsky_charin_release(s, &c, &charin_release_route), cm_charin_release(s, &x, &w, &charin_release_route))

LANEC_PW_CHARIN_RUN(lanec_pw_charin_sign_run, dsky_charin_sign(s, &c), cm_charin_sign(s, &x, &w))

enum lanec_status lanec_pw_decimal_consumer_run(apollo_overlay_state *s, lanec_io *io, enum lanec_mission mission)
{
    p66_bias_io p;
    enum dsky_decimal_consumer_route route = DSKY_DECIMAL_NEXT;
    agc_word routine = (agc_word)(s->words[0140] == 1u ? (s->words[0147] >> 5) & 037u : s->words[0153] & 037u);
    unsigned phase = 0, steps;
    static apollo_overlay_state before;
    before = *s;
    p.superbank = io->superbank; p.interrupt_enabled = io->interrupt_enabled;
    for (steps = 0; steps < 64u; ++steps) {
        enum dsky_decimal_consumer_stage stage =
            phase == 0u ? DSKY_DECIMAL_LOAD : phase == 1u ? DSKY_DECIMAL_SELECT
            : phase == 2u ? (routine == 4u || routine == 5u || routine == 7u || routine == 10u ? DSKY_DECIMAL_DP : DSKY_DECIMAL_SCALAR)
            : phase == 3u ? DSKY_DECIMAL_CALL : phase == 4u ? DSKY_DECIMAL_FORMAT : DSKY_DECIMAL_RETURN;
        /* A stage refused anywhere in the chain refuses the whole case. */
        if (dsky_decimal_consumer(s, &p, lanec_pw_nv(mission), stage, io->channel[033], &route) != P66_BIAS_OK) break;
        if (route != DSKY_DECIMAL_NEXT) {
            io->superbank = p.superbank; io->interrupt_enabled = p.interrupt_enabled;
            return LANEC_OK;
        }
        phase = phase == 5u ? 1u : phase + 1u;
        if (phase == 2u && routine == 1u) phase = 3u;
    }
    *s = before;
    return LANEC_OUT_OF_SCOPE;
}

enum lanec_status lanec_pw_decimal_general_run(apollo_overlay_state *s, lanec_io *io, enum lanec_mission mission)
{
    p66_bias_io p; enum lanec_status r; enum dsky_decimal_general_route route;
    p.superbank = io->superbank; p.interrupt_enabled = io->interrupt_enabled;
    r = dsky_decimal_general(s, &p, lanec_pw_nv(mission), lanec_pw_rounded(s), &route) == P66_BIAS_OK ? LANEC_OK : LANEC_OUT_OF_SCOPE;
    if (r == LANEC_OK) { io->superbank = p.superbank; io->interrupt_enabled = p.interrupt_enabled; }
    return r;
}

enum lanec_status lanec_pw_decimal_load_run(apollo_overlay_state *s, lanec_io *io, enum lanec_mission mission)
{
    p66_bias_io p; enum lanec_status r;
    p.superbank = io->superbank; p.interrupt_enabled = io->interrupt_enabled;
    r = dsky_decimal_load(s, &p, lanec_pw_nv(mission)) == P66_BIAS_OK ? LANEC_OK : LANEC_OUT_OF_SCOPE;
    if (r == LANEC_OK) { io->superbank = p.superbank; io->interrupt_enabled = p.interrupt_enabled; }
    return r;
}

enum lanec_status lanec_pw_decimal_raw_run(apollo_overlay_state *s, lanec_io *io, enum lanec_mission mission)
{
    p66_bias_io p; enum lanec_status r; enum dsky_decimal_general_route route;
    p.superbank = io->superbank; p.interrupt_enabled = io->interrupt_enabled;
    r = dsky_decimal_raw(s, &p, lanec_pw_nv(mission), lanec_pw_rounded(s), &route) == P66_BIAS_OK ? LANEC_OK : LANEC_OUT_OF_SCOPE;
    if (r == LANEC_OK) { io->superbank = p.superbank; io->interrupt_enabled = p.interrupt_enabled; }
    return r;
}

enum lanec_status lanec_pw_decimal_row_run(apollo_overlay_state *s, lanec_io *io, enum lanec_mission mission)
{
    p66_bias_io p; enum lanec_status r;
    enum dsky_decimal_row_entry entry = s->words[LANEC_Z] == 02566u ? DSKY_DECIMAL_ROW_CALL : DSKY_DECIMAL_ROW_RETURN;
    p.superbank = io->superbank; p.interrupt_enabled = io->interrupt_enabled;
    r = dsky_decimal_row(s, &p, lanec_pw_nv(mission), entry) == P66_BIAS_OK ? LANEC_OK : LANEC_OUT_OF_SCOPE;
    if (r == LANEC_OK) { io->superbank = p.superbank; io->interrupt_enabled = p.interrupt_enabled; }
    return r;
}

enum lanec_status lanec_pw_decimal_select_run(apollo_overlay_state *s, lanec_io *io, enum lanec_mission mission)
{
    p66_bias_io p; enum lanec_status r; agc_word route = 0;
    p.superbank = io->superbank; p.interrupt_enabled = io->interrupt_enabled;
    r = dsky_decimal_select(s, &p, lanec_pw_nv(mission), &route) == P66_BIAS_OK ? LANEC_OK : LANEC_OUT_OF_SCOPE;
    if (r == LANEC_OK) { io->superbank = p.superbank; io->interrupt_enabled = p.interrupt_enabled; }
    return r;
}
