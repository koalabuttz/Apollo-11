/* SPDX-License-Identifier: MIT */
/* The lanec_run adapters of the pw-* suites of batch e (include/lanec_pw_e_run.h). */
#include "lanec_pw_e_run.h"
#include "dsky_octal_verb.h"
#include "dsky_operator_release.h"
#include "dsky_operator_request.h"
#include "dsky_operator_response.h"
#include "dsky_operator_schedule.h"
#include "dsky_read_pair.h"
#include "dsky_request_control.h"
#include "dsky_response_recall.h"
#include "dsky_sign_output.h"
#include "dsky_source_alarm.h"

enum lanec_status lanec_pw_octal_verb_run(apollo_overlay_state *s, lanec_io *io, enum lanec_mission mission)
{ LANEC_ONE_MISSION(mission);
    enum dsky_octal_verb_route route;
    enum dsky_nv_mission nv = mission == LANEC_CM ? DSKY_NV_CM : DSKY_NV_LM;
    enum p66_bias_status st;
    uint16_t z = s->words[LANEC_Z], ch = (uint16_t)(s->words[01017] & 0777u);
    p66_bias_io p; p.superbank = io->superbank; p.interrupt_enabled = io->interrupt_enabled;
    if (z == 02504u) {
        if (ch > 077u) return LANEC_OUT_OF_SCOPE;
        st = dsky_octal_verb_channel(s, &p, nv, io->channel[ch], &route);
    } else if (z == 02422u) st = dsky_octal_verb_return(s, &p, nv, &route);
    else st = dsky_octal_verb_begin(s, &p, nv, &route);
    if (st != P66_BIAS_OK) return LANEC_OUT_OF_SCOPE;
    io->superbank = p.superbank; io->interrupt_enabled = p.interrupt_enabled;
    return LANEC_OK;
}

static enum dsky_operator_release_route operator_release_route;
LANEC_PW_LAMP_RUN(lanec_pw_operator_release_run, dsky_operator_release(s, &p, &operator_release_route),
                  cm_operator_release(s, &c, &wrote, &restart, &operator_release_route))

static enum dsky_request_control_route operator_request_route;
LANEC_PW_CHARIN_RUN(lanec_pw_operator_request_run, dsky_operator_request(s, &p, &operator_request_route),
                    cm_operator_request(s, &c, &wrote, &operator_request_route))

LANEC_PW_LAMP_RUN(lanec_pw_operator_response_run, dsky_operator_response(s, &p),
                  cm_operator_response(s, &c, &wrote, &restart))

static exec_address operator_schedule_request_pair(const apollo_overlay_state *s)
{
    exec_address a;
    a.location = s->words[01046]; a.bank = s->words[01047];
    return a;
}
static enum dsky_operator_schedule_route operator_schedule_route;
LANEC_PW_CHARIN_RUN(lanec_pw_operator_schedule_run, dsky_operator_schedule(s, &p, operator_schedule_request_pair(s), &operator_schedule_route),
                    cm_operator_schedule(s, &c, &wrote, operator_schedule_request_pair(s), &operator_schedule_route))

/* The oracle stops where Z first reaches the caller (Q at entry). A caller
 * the original path itself passes first cannot be told apart there, so the
 * run refuses it: TEM4 (0144, where READLO's TC TEM4 lands), READLO's own
 * body (LM 02747-02770, CM 02655-02676, bank 40) and SETEBANK (LM
 * 04313-04316, CM 04327-04332): the addresses the oracle traced for this
 * suite (LANEC_TRACE). */
enum lanec_status lanec_pw_read_pair_run(apollo_overlay_state *s, lanec_io *io, enum lanec_mission mission)
{ LANEC_ONE_MISSION(mission);
    uint16_t q = s->words[LANEC_Q], entry = mission == LANEC_CM ? 02655u : 02747u;
    uint16_t sete = mission == LANEC_CM ? 04327u : 04313u, sete_end = mission == LANEC_CM ? 04332u : 04316u;
    p66_bias_io p; p.superbank = io->superbank; p.interrupt_enabled = io->interrupt_enabled;
    if (q == 0144u || (q >= entry && q <= entry + 021u) || (q >= sete && q <= sete_end)) return LANEC_OUT_OF_SCOPE;
    if (dsky_read_pair(s, &p, mission == LANEC_CM ? DSKY_NV_CM : DSKY_NV_LM) != P66_BIAS_OK) return LANEC_OUT_OF_SCOPE;
    io->superbank = p.superbank; io->interrupt_enabled = p.interrupt_enabled;
    return LANEC_OK;
}

static enum dsky_request_control_route request_control_route;
LANEC_PW_CHARIN_RUN(lanec_pw_request_control_run, dsky_request_control(s, &p, &request_control_route),
                    cm_request_control(s, &c, &wrote, &request_control_route))

LANEC_PW_LAMP_RUN(lanec_pw_response_recall_run, dsky_response_recall(s, &p),
                  cm_response_recall(s, &c, &wrote, &restart))

/* The oracle stops where Z first reaches the caller (Q at entry). A caller
 * the original sign_output_path itself passes first cannot be told apart there, so the
 * run refuses callers inside the bank-40 code +ON/-ON and 11DSPIN execute:
 * the addresses the oracle traced for this suite (LANEC_TRACE). */
static const uint16_t sign_output_path[2][4][2] = {
    {{02324, 02327}, {02413, 02442}, {03347, 03377}, {03404, 03411}},
    {{02225, 02230}, {02314, 02343}, {03254, 03304}, {03311, 03316}}};
enum lanec_status lanec_pw_sign_output_run(apollo_overlay_state *s, lanec_io *io, enum lanec_mission mission)
{ LANEC_ONE_MISSION(mission);
    enum dsky_sign_output_route route;
    uint16_t base = mission == LANEC_CM ? 02314u : 02413u, z = s->words[LANEC_Z], q = s->words[LANEC_Q];
    unsigned i;
    p66_bias_io p;
    if (z != base && z != base + 020u) return LANEC_OUT_OF_SCOPE;
    for (i = 0; i < 4u; ++i)
        if (q >= sign_output_path[mission == LANEC_CM][i][0] && q <= sign_output_path[mission == LANEC_CM][i][1]) return LANEC_OUT_OF_SCOPE;
    p.superbank = io->superbank; p.interrupt_enabled = io->interrupt_enabled;
    if (dsky_sign_output(s, &p, mission == LANEC_CM ? DSKY_NV_CM : DSKY_NV_LM, (uint8_t)(z != base), &route) != P66_BIAS_OK)
        return LANEC_OUT_OF_SCOPE;
    io->superbank = p.superbank; io->interrupt_enabled = p.interrupt_enabled;
    return LANEC_OK;
}

static enum dsky_alarm_route source_alarm_route;
LANEC_PW_CHARIN_RUN(lanec_pw_source_alarm_run, dsky_source_alarm(s, &p, &source_alarm_route),
                    cm_source_alarm(s, &c, &wrote, &source_alarm_route))
