/* SPDX-License-Identifier: MIT */
#ifndef LANEC_PW_B_RUN_H
#define LANEC_PW_B_RUN_H
/* The lanec_run adapters of the pw-* suites of batch b: each runs one earlier
 * Pinball module (src/dsky_*.c, src/cm_*.c) on a lanec_io, for the suites
 * (tests/lanec_pw_*_cases.c) and the Pinball transfer-ABI routines
 * (src/lanec_pin_routines.c). */
#include "lanec_pw_run.h"
#include "dsky_lamp_test.h"
#include "dsky_nv_admission.h"

/* LM p66_bias_io routine with a route out-parameter and its CM
 * cm_native_io twin. */
#define LANEC_PW_ROUTE_RUN(name, lm, cm, route_type)                                               \
    enum lanec_status name(apollo_overlay_state *s, lanec_io *io, enum lanec_mission mission) \
    {                                                                                              \
        int ok = 0; route_type r;                                                                  \
        if (mission == LANEC_LM) {                                                                 \
            p66_bias_io p; p.superbank = io->superbank; p.interrupt_enabled = io->interrupt_enabled; \
            ok = lm(s, &p, &r) == P66_BIAS_OK;                                                     \
            if (ok) { io->superbank = p.superbank; io->interrupt_enabled = p.interrupt_enabled; } \
        } else {                                                                                   \
            cm_native_io c = {0}; c.superbank = io->superbank; c.interrupt_enabled = io->interrupt_enabled; \
            ok = cm(s, &c, &r) == EXEC_OK;                                                         \
            if (ok) { io->superbank = c.superbank; io->interrupt_enabled = c.interrupt_enabled; }  \
        }                                                                                          \
        return ok ? LANEC_OK : LANEC_OUT_OF_SCOPE;                                                 \
    }

/* LM dsky_lamp_io routine (route out-parameter) and its CM twin taking
 * cm_native_io plus wrote_channels/restart_light. Channels 011-013 pass
 * through; the restart light (not a channel the oracle holds) starts off. */
#define LANEC_PW_LAMP_RUN(name, lm, cm, route_type)                                                \
    enum lanec_status name(apollo_overlay_state *s, lanec_io *io, enum lanec_mission mission) \
    {                                                                                              \
        int ok = 0; route_type r;                                                                  \
        if (mission == LANEC_LM) {                                                                 \
            dsky_lamp_io p = {{0, 0}, 0, 0, 0, 0, 0};                                              \
            p.control.superbank = io->superbank; p.control.interrupt_enabled = io->interrupt_enabled; \
            p.channel11 = io->channel[011]; p.channel12 = io->channel[012]; p.channel13 = io->channel[013]; \
            ok = lm(s, &p, &r) == P66_BIAS_OK;                                                     \
            if (ok) {                                                                              \
                io->superbank = p.control.superbank; io->interrupt_enabled = p.control.interrupt_enabled; \
                io->channel[011] = p.channel11; io->channel[012] = p.channel12; io->channel[013] = p.channel13; \
            }                                                                                      \
        } else {                                                                                   \
            cm_native_io c = {0}; uint16_t wrote = 0, restart = 0;                                 \
            c.superbank = io->superbank; c.interrupt_enabled = io->interrupt_enabled;              \
            c.channel11 = io->channel[011]; c.channel12 = io->channel[012]; c.channel13 = io->channel[013]; \
            ok = cm(s, &c, &wrote, &restart, &r) == EXEC_OK;                                       \
            if (ok) {                                                                              \
                io->superbank = c.superbank; io->interrupt_enabled = c.interrupt_enabled;          \
                io->channel[011] = c.channel11; io->channel[012] = c.channel12; io->channel[013] = c.channel13; \
            }                                                                                      \
        }                                                                                          \
        return ok ? LANEC_OK : LANEC_OUT_OF_SCOPE;                                                 \
    }

static inline enum dsky_nv_mission lanec_pw_nv(enum lanec_mission m)
{
    return m == LANEC_LM ? DSKY_NV_LM : DSKY_NV_CM;
}
#endif
