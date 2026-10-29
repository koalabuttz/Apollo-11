/* SPDX-License-Identifier: MIT */
/* The lanec_run adapters of the pw-* suites of batch b (include/lanec_pw_b_run.h). */
#include "lanec_pw_b_run.h"
#include "dsky_decimal_time_consumer.h"
#include "dsky_display_release.h"
#include "dsky_enter.h"
#include "dsky_enter_zero.h"
#include "dsky_five_blank.h"
#include "dsky_fixed_display.h"
#include "dsky_fraction_pair.h"
#include "dsky_hours_output.h"
#include "dsky_input_angle.h"
#include "dsky_input_arithmetic.h"
#include "dsky_input_factor.h"
#include "dsky_input_mode.h"
#include "dsky_input_scale.h"
#include "dsky_input_select.h"

/* The DSKY decimal-time consumer stage that starts at Z (255: none), as
 * tests/dsky_decimal_time_consumer_cases.c maps it for the module's own checks. */
static uint8_t time_stage(enum dsky_nv_mission mission, agc_word pc)
{
    static const agc_word lm[19] = { 03230, 03234, 03240, 03242, 03253, 03255, 03264, 03266, 03303, 03343,
                                     03347, 03351, 03361, 03371, 02732, 02733, 02734, 02736, 02745 },
                          cm[19] = { 03143, 03147, 03153, 03155, 03166, 03170, 03177, 03201, 03216, 03256,
                                     03262, 03264, 03274, 03304, 02636, 02637, 02640, 02642, 02651 };
    unsigned i;
    if (pc == 02523u) return 0;
    if (pc == 02537u) return 1;
    if (pc == 02570u) return 21;
    for (i = 0; i < 19u; ++i)
        if (pc == (mission == DSKY_NV_LM ? lm[i] : cm[i])) return (uint8_t)(i + 2u);
    return 255;
}

enum lanec_status lanec_pw_decimal_time_consumer_run(apollo_overlay_state *s, lanec_io *io, enum lanec_mission mission)
{ LANEC_ONE_MISSION(mission);
    p66_bias_io p; enum dsky_decimal_consumer_route r;
    enum dsky_nv_mission nv = lanec_pw_nv(mission);
    p.superbank = io->superbank; p.interrupt_enabled = io->interrupt_enabled;
    if (dsky_decimal_time_consumer(s, &p, nv, time_stage(nv, s->words[LANEC_Z]), &r) != P66_BIAS_OK)
        return LANEC_OUT_OF_SCOPE;
    io->superbank = p.superbank; io->interrupt_enabled = p.interrupt_enabled;
    return LANEC_OK;
}

enum lanec_status lanec_pw_display_release_run(apollo_overlay_state *s, lanec_io *io, enum lanec_mission mission)
{ LANEC_ONE_MISSION(mission);
    int ok;
    if (mission == LANEC_LM) {
        dsky_charin_io p;
        p.control.superbank = io->superbank; p.control.interrupt_enabled = io->interrupt_enabled;
        p.channel11 = io->channel[011]; p.wrote_channel11 = 0;
        ok = dsky_display_release(s, &p) == P66_BIAS_OK;
        if (ok) { io->superbank = p.control.superbank; io->interrupt_enabled = p.control.interrupt_enabled; io->channel[011] = p.channel11; }
    } else {
        cm_native_io c = {0}; uint16_t wrote = 0;
        c.superbank = io->superbank; c.interrupt_enabled = io->interrupt_enabled; c.channel11 = io->channel[011];
        ok = cm_display_release(s, &c, &wrote) == EXEC_OK;
        if (ok) { io->superbank = c.superbank; io->interrupt_enabled = c.interrupt_enabled; io->channel[011] = c.channel11; }
    }
    return ok ? LANEC_OK : LANEC_OUT_OF_SCOPE;
}

enum lanec_status lanec_pw_enter_run(apollo_overlay_state *s, lanec_io *io, enum lanec_mission mission)
{ LANEC_ONE_MISSION(mission);
    int ok; enum dsky_enter_route r;
    if (mission == LANEC_LM) {
        dsky_charin_io p;
        p.control.superbank = io->superbank; p.control.interrupt_enabled = io->interrupt_enabled;
        p.channel11 = io->channel[011]; p.wrote_channel11 = 0;
        ok = dsky_enter(s, &p, &r) == P66_BIAS_OK;
        if (ok) { io->superbank = p.control.superbank; io->interrupt_enabled = p.control.interrupt_enabled; io->channel[011] = p.channel11; }
    } else {
        cm_native_io c = {0}; uint16_t wrote = 0;
        c.superbank = io->superbank; c.interrupt_enabled = io->interrupt_enabled; c.channel11 = io->channel[011];
        ok = cm_enter(s, &c, &wrote, &r) == EXEC_OK;
        if (ok) { io->superbank = c.superbank; io->interrupt_enabled = c.interrupt_enabled; io->channel[011] = c.channel11; }
    }
    return ok ? LANEC_OK : LANEC_OUT_OF_SCOPE;
}

LANEC_PW_ROUTE_RUN(lanec_pw_enter_zero_run, dsky_enter_zero, cm_enter_zero, enum dsky_enter_zero_route)

LANEC_PW_ROUTE_RUN(lanec_pw_five_blank_run, dsky_five_blank, cm_five_blank, enum dsky_five_blank_route)

enum lanec_status lanec_pw_fixed_display_run(apollo_overlay_state *s, lanec_io *io, enum lanec_mission mission)
{ LANEC_ONE_MISSION(mission);
    unsigned stage;
    enum dsky_nv_mission nv = lanec_pw_nv(mission);
    enum dsky_fixed_display_route r;
    for (stage = 0; stage < 3u; ++stage) {
        p66_bias_io p; agc_word value = 0; uint16_t address;
        p.superbank = io->superbank; p.interrupt_enabled = io->interrupt_enabled;
        if (stage == DSKY_FIXED_DATA &&
            (!lanec_fixed_read || !dsky_fixed_display_address(s, &p, nv, &address) || !lanec_fixed_read(mission, address, &value)))
            continue;
        if (dsky_fixed_display(s, &p, nv, (enum dsky_fixed_display_stage)stage, value, &r) == P66_BIAS_OK) {
            io->superbank = p.superbank; io->interrupt_enabled = p.interrupt_enabled;
            return LANEC_OK;
        }
    }
    return LANEC_OUT_OF_SCOPE;
}

enum lanec_status lanec_pw_fraction_pair_run(apollo_overlay_state *s, lanec_io *io, enum lanec_mission mission)
{ LANEC_ONE_MISSION(mission);
    p66_bias_io p; enum dsky_decimal_general_route r;
    p.superbank = io->superbank; p.interrupt_enabled = io->interrupt_enabled;
    if (dsky_fraction_pair(s, &p, lanec_pw_nv(mission), &r) != P66_BIAS_OK) return LANEC_OUT_OF_SCOPE;
    io->superbank = p.superbank; io->interrupt_enabled = p.interrupt_enabled;
    return LANEC_OK;
}

enum lanec_status lanec_pw_hours_output_run(apollo_overlay_state *s, lanec_io *io, enum lanec_mission mission)
{ LANEC_ONE_MISSION(mission);
    unsigned stage;
    for (stage = 0; stage < 8u; ++stage) {
        p66_bias_io p; p.superbank = io->superbank; p.interrupt_enabled = io->interrupt_enabled;
        if (dsky_hours_output(s, &p, lanec_pw_nv(mission), (enum dsky_hours_stage)stage) == P66_BIAS_OK) {
            io->superbank = p.superbank; io->interrupt_enabled = p.interrupt_enabled;
            return LANEC_OK;
        }
    }
    return LANEC_OUT_OF_SCOPE;
}

LANEC_PW_LAMP_RUN(lanec_pw_input_angle_run, dsky_input_angle, cm_input_angle, enum dsky_input_angle_route)

LANEC_PW_LAMP_RUN(lanec_pw_input_arithmetic_run, dsky_input_arithmetic, cm_input_arithmetic, enum dsky_input_arithmetic_route)

LANEC_PW_LAMP_RUN(lanec_pw_input_factor_run, dsky_input_factor, cm_input_factor, enum dsky_input_factor_route)

LANEC_PW_LAMP_RUN(lanec_pw_input_mode_run, dsky_input_mode, cm_input_mode, enum dsky_input_mode_route)

LANEC_PW_LAMP_RUN(lanec_pw_input_scale_run, dsky_input_scale, cm_input_scale, enum dsky_input_scale_route)

LANEC_PW_LAMP_RUN(lanec_pw_input_select_run, dsky_input_select, cm_input_select, enum dsky_input_select_route)

bool (*lanec_fixed_read)(enum lanec_mission, uint16_t, agc_word *);
