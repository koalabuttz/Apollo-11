/* SPDX-License-Identifier: MIT */
/* The lanec_run adapters of the pw-* suites (include/lanec_pw_run.h). */
#include "lanec_pw_run.h"
#include "lm_pinball_busy.h"
#include "dsky_blank.h"
#include "dsky_blanksub.h"
#include "dsky_charin_reset.h"
#include "dsky_extended_dispatch.h"
#include "dsky_minutes_output.h"
#include "dsky_mode_job.h"
#include "dsky_monitor.h"
#include "dsky_monitor_consumer.h"
#include "dsky_monitor_paste.h"
#include "dsky_noun_check.h"
#include "dsky_nv_admission.h"
#include "dsky_nv_blank.h"
#include "dsky_nv_busy.h"
#include "dsky_nv_consumer.h"
#include "dsky_nv_continuation.h"
#include "dsky_nv_return.h"
#include "dsky_nv_update.h"
#include "dsky_nv_update_general.h"
#include "dsky_octal_consumer.h"
#include "dsky_octal_display.h"
#include "dsky_scalar_output.h"
#include "dsky_test_noun.h"
#include "dsky_two_integer.h"
#include "dsky_verb_dispatch.h"

enum lanec_status lanec_pw_blank_run(apollo_overlay_state *s, lanec_io *io, enum lanec_mission mission)
{ LANEC_ONE_MISSION(mission);
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

LANEC_PW_P66_RUN(lanec_pw_blanksub_run, dsky_blanksub, cm_blanksub)

enum lanec_status lanec_pw_charin_reset_run(apollo_overlay_state *s, lanec_io *io, enum lanec_mission mission)
{ LANEC_ONE_MISSION(mission);
    int ok;
    uint16_t c11, c13;
    if (mission == LANEC_LM) {
        dsky_reset_io r;
        r.control.superbank = io->superbank; r.control.interrupt_enabled = io->interrupt_enabled;
        r.channel11 = io->channel[011]; r.channel13 = io->channel[013]; r.wrote_channels = 0;
        r.restart_light = 0;
        ok = dsky_charin_reset(s, &r) == P66_BIAS_OK;
        if (ok) { io->superbank = r.control.superbank; io->interrupt_enabled = r.control.interrupt_enabled; }
        c11 = r.channel11; c13 = r.channel13;
    } else {
        cm_native_io c = {0};
        uint16_t wrote = 0, light = 0;
        c.superbank = io->superbank; c.interrupt_enabled = io->interrupt_enabled;
        c.channel11 = io->channel[011]; c.channel13 = io->channel[013];
        ok = cm_charin_reset(s, &c, &wrote, &light) == EXEC_OK;
        if (ok) { io->superbank = c.superbank; io->interrupt_enabled = c.interrupt_enabled; }
        c11 = c.channel11; c13 = c.channel13;
    }
    if (!ok) return LANEC_OUT_OF_SCOPE;
    io->channel[011] = c11; io->channel[013] = c13;
    return LANEC_OK;
}

LANEC_PW_P66_RUN(lanec_pw_extended_dispatch_run, dsky_extended_dispatch, cm_extended_dispatch)

enum lanec_status lanec_pw_lm_pinball_busy_run(apollo_overlay_state *s, lanec_io *io, enum lanec_mission mission)
{ LANEC_ONE_MISSION(mission);
    p66_bias_io p;
    agc_word channel11;
    enum lm_busy_status st;
    if (mission != LANEC_LM) return LANEC_OUT_OF_SCOPE;
    p.superbank = io->superbank; p.interrupt_enabled = io->interrupt_enabled;
    channel11 = io->channel[011];
    st = lm_pinball_busy(s, &p, &channel11);
    if (st == LM_BUSY_OUT_OF_SCOPE) return LANEC_OUT_OF_SCOPE;
    io->superbank = p.superbank; io->interrupt_enabled = p.interrupt_enabled;
    io->channel[011] = channel11;
    return LANEC_OK;
}

enum lanec_status lanec_pw_minutes_output_run(apollo_overlay_state *s, lanec_io *io, enum lanec_mission mission)
{ LANEC_ONE_MISSION(mission);
    static const uint16_t pcs[2][6] = { { 03303, 03343, 03347, 03351, 03361, 03371 },
                                        { 03216, 03256, 03262, 03264, 03274, 03304 } };
    static apollo_overlay_state saved;
    enum dsky_nv_mission m = mission == LANEC_LM ? DSKY_NV_LM : DSKY_NV_CM;
    p66_bias_io p;
    unsigned k;
    p.superbank = io->superbank; p.interrupt_enabled = io->interrupt_enabled;
    for (k = 0; k < 6u && s->words[LANEC_Z] != pcs[mission][k]; ++k) {}
    if (k == 6u) return LANEC_OUT_OF_SCOPE;
    saved = *s;
    for (; k < 6u; ++k)
        if (s->words[LANEC_Z] != pcs[mission][k] || dsky_minutes_output(s, &p, m, (enum dsky_minutes_stage)k) != P66_BIAS_OK) {
            *s = saved;
            return LANEC_OUT_OF_SCOPE;
        }
    io->superbank = p.superbank; io->interrupt_enabled = p.interrupt_enabled;
    return LANEC_OK;
}

enum lanec_status lanec_pw_mode_job_run(apollo_overlay_state *s, lanec_io *io, enum lanec_mission mission)
{ LANEC_ONE_MISSION(mission);
    enum dsky_mode_job_route route;
    int ok;
    if (mission == LANEC_LM) {
        p66_bias_io p; p.superbank = io->superbank; p.interrupt_enabled = io->interrupt_enabled;
        ok = dsky_mode_job(s, &p, &route) == P66_BIAS_OK;
        if (ok) { io->superbank = p.superbank; io->interrupt_enabled = p.interrupt_enabled; }
    } else {
        cm_native_io c = {0}; c.superbank = io->superbank; c.interrupt_enabled = io->interrupt_enabled;
        ok = cm_mode_job(s, &c, &route) == EXEC_OK;
        if (ok) { io->superbank = c.superbank; io->interrupt_enabled = c.interrupt_enabled; }
    }
    return ok ? LANEC_OK : LANEC_OUT_OF_SCOPE;
}

enum lanec_status lanec_pw_monitor_run(apollo_overlay_state *s, lanec_io *io, enum lanec_mission mission)
{ LANEC_ONE_MISSION(mission);
    dsky_charin_io c;
    enum dsky_monitor_route route;
    unsigned e;
    c.control.superbank = io->superbank; c.control.interrupt_enabled = io->interrupt_enabled;
    c.channel11 = io->channel[011]; c.wrote_channel11 = 0;
    for (e = DSKY_MONITOR_SETUP; e <= DSKY_MONITOR_BUSY; ++e)
        if (dsky_monitor(s, &c, (enum dsky_nv_mission)mission, (enum dsky_monitor_stage)e, &route) == P66_BIAS_OK) {
            io->superbank = c.control.superbank; io->interrupt_enabled = c.control.interrupt_enabled;
            io->channel[011] = c.channel11;
            return LANEC_OK;
        }
    return LANEC_OUT_OF_SCOPE;
}

enum lanec_status lanec_pw_monitor_consumer_run(apollo_overlay_state *s, lanec_io *io, enum lanec_mission mission)
{ LANEC_ONE_MISSION(mission);
    dsky_charin_io c;
    enum dsky_nv_mission nm = (enum dsky_nv_mission)mission;
    enum dsky_monitor_consumer_stage stage, next;
    unsigned step, ok = 0;
    c.control.superbank = io->superbank; c.control.interrupt_enabled = io->interrupt_enabled;
    c.channel11 = io->channel[011]; c.wrote_channel11 = 0;
    for (step = 0; step < 64u; ++step) {
        agc_word value;
        stage = dsky_monitor_consumer_entry(s, nm);
        if (stage == DSKY_MONC_FRONTIER) break;
        if (stage >= DSKY_MONC_OCTAL_BEGIN && stage <= DSKY_MONC_OCTAL_RETURN) {
            uint16_t ch = s->words[01017] & 0777u;
            value = ch < LANEC_CHANNELS ? (agc_word)(io->channel[ch] & AGC_MASK) : 0u;
        } else value = (agc_word)(io->channel[033] & AGC_MASK);
        if (dsky_monitor_consumer(s, &c, nm, stage, value, &next) != P66_BIAS_OK) break;
        ok = 1;
    }
    if (!ok) return LANEC_OUT_OF_SCOPE;
    io->superbank = c.control.superbank; io->interrupt_enabled = c.control.interrupt_enabled;
    io->channel[011] = c.channel11;
    return LANEC_OK;
}

enum lanec_status lanec_pw_monitor_paste_run(apollo_overlay_state *s, lanec_io *io, enum lanec_mission mission)
{ LANEC_ONE_MISSION(mission);
    p66_bias_io p;
    unsigned e;
    p.superbank = io->superbank; p.interrupt_enabled = io->interrupt_enabled;
    for (e = DSKY_MONITOR_PASTE_BEGIN; e <= DSKY_MONITOR_PASTE_BLANKED; ++e)
        if (dsky_monitor_paste(s, &p, (enum dsky_nv_mission)mission, (enum dsky_monitor_paste_stage)e) == P66_BIAS_OK) {
            io->superbank = p.superbank; io->interrupt_enabled = p.interrupt_enabled;
            return LANEC_OK;
        }
    return LANEC_OUT_OF_SCOPE;
}

enum lanec_status lanec_pw_noun_check_run(apollo_overlay_state *s, lanec_io *io, enum lanec_mission mission)
{ LANEC_ONE_MISSION(mission);
    enum dsky_noun_check_route route;
    int ok;
    if (mission == LANEC_LM) {
        dsky_lamp_io p = {{0, 0}, 0, 0, 0, 0, 0};
        p.control.superbank = io->superbank; p.control.interrupt_enabled = io->interrupt_enabled;
        p.channel11 = io->channel[011]; p.channel12 = io->channel[012]; p.channel13 = io->channel[013];
        ok = dsky_noun_check(s, &p, &route) == P66_BIAS_OK;
        if (ok) { io->superbank = p.control.superbank; io->interrupt_enabled = p.control.interrupt_enabled; }
    } else {
        cm_native_io c = {0};
        uint16_t wrote = 0, restart = 0;
        c.superbank = io->superbank; c.interrupt_enabled = io->interrupt_enabled;
        ok = cm_noun_check(s, &c, &wrote, &restart, &route) == EXEC_OK;
        if (ok) { io->superbank = c.superbank; io->interrupt_enabled = c.interrupt_enabled; }
    }
    return ok ? LANEC_OK : LANEC_OUT_OF_SCOPE;
}

enum lanec_status lanec_pw_nv_admission_run(apollo_overlay_state *s, lanec_io *io, enum lanec_mission mission)
{ LANEC_ONE_MISSION(mission);
    p66_bias_io p;
    enum dsky_nv_route route;
    unsigned e;
    p.superbank = io->superbank; p.interrupt_enabled = io->interrupt_enabled;
    for (e = DSKY_NV_NVSUB; e <= DSKY_NV_NVSUB2; ++e)
        if (dsky_nv_admit(s, &p, (enum dsky_nv_mission)mission, (enum dsky_nv_entry)e, &route) == P66_BIAS_OK) {
            io->superbank = p.superbank; io->interrupt_enabled = p.interrupt_enabled;
            return LANEC_OK;
        }
    return LANEC_OUT_OF_SCOPE;
}

enum lanec_status lanec_pw_nv_blank_run(apollo_overlay_state *s, lanec_io *io, enum lanec_mission mission)
{ LANEC_ONE_MISSION(mission);
    p66_bias_io p;
    agc_word channel11 = io->channel[011];
    p.superbank = io->superbank; p.interrupt_enabled = io->interrupt_enabled;
    if (dsky_nv_blank(s, &p, &channel11, (enum dsky_nv_mission)mission) != P66_BIAS_OK) return LANEC_OUT_OF_SCOPE;
    io->superbank = p.superbank; io->interrupt_enabled = p.interrupt_enabled; io->channel[011] = channel11;
    return LANEC_OK;
}

enum lanec_status lanec_pw_nv_busy_run(apollo_overlay_state *s, lanec_io *io, enum lanec_mission mission)
{ LANEC_ONE_MISSION(mission);
    p66_bias_io p;
    agc_word channel11 = io->channel[011];
    enum dsky_nv_busy_route route;
    unsigned e;
    p.superbank = io->superbank; p.interrupt_enabled = io->interrupt_enabled;
    for (e = DSKY_NV_ENDIDLE; e <= DSKY_NV_BUSY_INNER; ++e)
        if (dsky_nv_busy(s, &p, &channel11, (enum dsky_nv_mission)mission, (enum dsky_nv_busy_entry)e, &route) == P66_BIAS_OK) {
            io->superbank = p.superbank; io->interrupt_enabled = p.interrupt_enabled; io->channel[011] = channel11;
            return LANEC_OK;
        }
    return LANEC_OUT_OF_SCOPE;
}

enum lanec_status lanec_pw_nv_consumer_run(apollo_overlay_state *s, lanec_io *io, enum lanec_mission mission)
{ LANEC_ONE_MISSION(mission);
    p66_bias_io p;
    agc_word channel11 = io->channel[011];
    enum dsky_nv_mission nm = (enum dsky_nv_mission)mission;
    unsigned phase;
    p.superbank = io->superbank; p.interrupt_enabled = io->interrupt_enabled;
    if (dsky_nv_consumer_step(s, &p, &channel11, nm, DSKY_NV_CONSUMER_CALL) != P66_BIAS_OK &&
        dsky_nv_consumer_step(s, &p, &channel11, nm, DSKY_NV_CONSUMER_MONITOR) != P66_BIAS_OK)
        return LANEC_OUT_OF_SCOPE;
    for (phase = 1; phase < 5u; ++phase)
        if (dsky_nv_consumer_step(s, &p, &channel11, nm,
                                  (phase & 1u) ? DSKY_NV_CONSUMER_SERVICE : DSKY_NV_CONSUMER_CONTINUE) != P66_BIAS_OK)
            break;
    io->superbank = p.superbank; io->interrupt_enabled = p.interrupt_enabled; io->channel[011] = channel11;
    return LANEC_OK;
}

enum lanec_status lanec_pw_nv_continuation_run(apollo_overlay_state *s, lanec_io *io, enum lanec_mission mission)
{ LANEC_ONE_MISSION(mission);
    p66_bias_io p;
    unsigned e;
    p.superbank = io->superbank; p.interrupt_enabled = io->interrupt_enabled;
    for (e = DSKY_NV_COMMON; e <= DSKY_NV_PAS0; ++e)
        if (dsky_nv_continue(s, &p, (enum dsky_nv_mission)mission, (enum dsky_nv_continuation_entry)e) == P66_BIAS_OK) {
            io->superbank = p.superbank; io->interrupt_enabled = p.interrupt_enabled;
            return LANEC_OK;
        }
    return LANEC_OUT_OF_SCOPE;
}

enum lanec_status lanec_pw_nv_return_run(apollo_overlay_state *s, lanec_io *io, enum lanec_mission mission)
{ LANEC_ONE_MISSION(mission);
    p66_bias_io p;
    p.superbank = io->superbank; p.interrupt_enabled = io->interrupt_enabled;
    if (dsky_nv_return(s, &p, (enum dsky_nv_mission)mission) != P66_BIAS_OK) return LANEC_OUT_OF_SCOPE;
    io->superbank = p.superbank; io->interrupt_enabled = p.interrupt_enabled;
    return LANEC_OK;
}

enum lanec_status lanec_pw_nv_update_run(apollo_overlay_state *s, lanec_io *io, enum lanec_mission mission)
{ LANEC_ONE_MISSION(mission);
    p66_bias_io p;
    unsigned e;
    p.superbank = io->superbank; p.interrupt_enabled = io->interrupt_enabled;
    for (e = DSKY_NV_VERB_STORE; e <= DSKY_NV_NOUN_UPDATE; ++e)
        if (dsky_nv_update(s, &p, (enum dsky_nv_mission)mission, (enum dsky_nv_update_entry)e) == P66_BIAS_OK) {
            io->superbank = p.superbank; io->interrupt_enabled = p.interrupt_enabled;
            return LANEC_OK;
        }
    return LANEC_OUT_OF_SCOPE;
}

enum lanec_status lanec_pw_nv_update_general_run(apollo_overlay_state *s, lanec_io *io, enum lanec_mission mission)
{ LANEC_ONE_MISSION(mission);
    p66_bias_io p;
    enum dsky_nv_update_route route;
    unsigned e;
    p.superbank = io->superbank; p.interrupt_enabled = io->interrupt_enabled;
    for (e = DSKY_NV_VERB_STORE; e <= DSKY_NV_NOUN_UPDATE; ++e)
        if (dsky_nv_update_general(s, &p, (enum dsky_nv_mission)mission, (enum dsky_nv_update_entry)e, &route) == P66_BIAS_OK) {
            io->superbank = p.superbank; io->interrupt_enabled = p.interrupt_enabled;
            return LANEC_OK;
        }
    return LANEC_OUT_OF_SCOPE;
}

enum lanec_status lanec_pw_octal_consumer_run(apollo_overlay_state *s, lanec_io *io, enum lanec_mission mission)
{ LANEC_ONE_MISSION(mission);
    p66_bias_io p;
    enum dsky_octal_consumer_route route = DSKY_OCTAL_NEXT;
    unsigned step, e, ok = 0;
    p.superbank = io->superbank; p.interrupt_enabled = io->interrupt_enabled;
    for (step = 0; step < 64u && route == DSKY_OCTAL_NEXT; ++step) {
        uint16_t ch = s->words[01017] & 0777u;
        agc_word value = ch < LANEC_CHANNELS ? (agc_word)(io->channel[ch] & AGC_MASK) : 0u;
        for (e = DSKY_OCTAL_BEGIN; e <= DSKY_OCTAL_RETURN; ++e)
            if (dsky_octal_consumer(s, &p, (enum dsky_nv_mission)mission, (enum dsky_octal_consumer_stage)e, value, &route) == P66_BIAS_OK)
                break;
        if (e > DSKY_OCTAL_RETURN) break;
        ok = 1;
    }
    if (!ok) return LANEC_OUT_OF_SCOPE;
    io->superbank = p.superbank; io->interrupt_enabled = p.interrupt_enabled;
    return LANEC_OK;
}

enum lanec_status lanec_pw_octal_display_run(apollo_overlay_state *s, lanec_io *io, enum lanec_mission mission)
{ LANEC_ONE_MISSION(mission);
    enum dsky_octal_route route;
    int ok;
    if (mission == LANEC_LM) {
        p66_bias_io p; p.superbank = io->superbank; p.interrupt_enabled = io->interrupt_enabled;
        ok = dsky_octal_display(s, &p, &route) == P66_BIAS_OK;
        if (ok) { io->superbank = p.superbank; io->interrupt_enabled = p.interrupt_enabled; }
    } else {
        cm_native_io c = {0}; c.superbank = io->superbank; c.interrupt_enabled = io->interrupt_enabled;
        ok = cm_octal_display(s, &c, &route) == EXEC_OK;
        if (ok) { io->superbank = c.superbank; io->interrupt_enabled = c.interrupt_enabled; }
    }
    return ok ? LANEC_OK : LANEC_OUT_OF_SCOPE;
}

enum lanec_status lanec_pw_scalar_output_run(apollo_overlay_state *s, lanec_io *io, enum lanec_mission mission)
{ LANEC_ONE_MISSION(mission);
    static const uint16_t pc[2][6] = {{02615, 02677, 02704, 02623, 02635, 0}, {02516, 02603, 02610, 02524, 0, 02653}};
    unsigned e, mi = mission == LANEC_CM;
    p66_bias_io p;
    for (e = 0; e < 6u && pc[mi][e] != s->words[LANEC_Z]; ++e) {}
    if (e == 6u) return LANEC_OUT_OF_SCOPE;
    p.superbank = io->superbank; p.interrupt_enabled = io->interrupt_enabled;
    if (dsky_scalar_output(s, &p, mi ? DSKY_NV_CM : DSKY_NV_LM, (enum dsky_scalar_output_entry)e, io->channel[033]) != P66_BIAS_OK)
        return LANEC_OUT_OF_SCOPE;
    io->superbank = p.superbank; io->interrupt_enabled = p.interrupt_enabled;
    return LANEC_OK;
}

enum lanec_status lanec_pw_test_noun_run(apollo_overlay_state *s, lanec_io *io, enum lanec_mission mission)
{ LANEC_ONE_MISSION(mission);
    enum dsky_enter_zero_route route;
    p66_bias_io p; p.superbank = io->superbank; p.interrupt_enabled = io->interrupt_enabled;
    if (dsky_test_noun(s, &p, mission == LANEC_CM ? DSKY_NV_CM : DSKY_NV_LM, &route) != P66_BIAS_OK) return LANEC_OUT_OF_SCOPE;
    io->superbank = p.superbank; io->interrupt_enabled = p.interrupt_enabled;
    return LANEC_OK;
}

enum lanec_status lanec_pw_two_integer_run(apollo_overlay_state *s, lanec_io *io, enum lanec_mission mission)
{ LANEC_ONE_MISSION(mission);
#ifndef AGC_FLIGHT_INTERPRETER
    /* A flight run needs no copy: the Pinball trampoline (src/lanec_pin_trampoline.c)
     * gives a refused routine its entry state back, and the 4 KB stays out of the
     * module's store slot. */
    static apollo_overlay_state before;
#endif
    p66_bias_io p;
    unsigned k;
#ifndef AGC_FLIGHT_INTERPRETER
    before = *s;
#endif
    p.superbank = io->superbank; p.interrupt_enabled = io->interrupt_enabled;
    for (k = 0; k < 5u; ++k)
        if (dsky_two_integer(s, &p, mission == LANEC_CM ? DSKY_NV_CM : DSKY_NV_LM, (enum dsky_two_integer_stage)k) != P66_BIAS_OK) {
#ifndef AGC_FLIGHT_INTERPRETER
            *s = before;
#endif
            return LANEC_OUT_OF_SCOPE;
        }
    io->superbank = p.superbank; io->interrupt_enabled = p.interrupt_enabled;
    return LANEC_OK;
}

enum lanec_status lanec_pw_verb_dispatch_run(apollo_overlay_state *s, lanec_io *io, enum lanec_mission mission)
{ LANEC_ONE_MISSION(mission);
    enum dsky_verb_route route;
    int ok;
    if (mission == LANEC_LM) {
        p66_bias_io p; p.superbank = io->superbank; p.interrupt_enabled = io->interrupt_enabled;
        ok = dsky_verb_dispatch(s, &p, &route) == P66_BIAS_OK;
        if (ok) { io->superbank = p.superbank; io->interrupt_enabled = p.interrupt_enabled; }
    } else {
        cm_native_io c = {0}; c.superbank = io->superbank; c.interrupt_enabled = io->interrupt_enabled;
        ok = cm_verb_dispatch(s, &c, &route) == EXEC_OK;
        if (ok) { io->superbank = c.superbank; io->interrupt_enabled = c.interrupt_enabled; }
    }
    return ok ? LANEC_OK : LANEC_OUT_OF_SCOPE;
}
