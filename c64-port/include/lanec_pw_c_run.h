/* SPDX-License-Identifier: MIT */
#ifndef LANEC_PW_C_RUN_H
#define LANEC_PW_C_RUN_H
/* The lanec_run adapters of the pw-* suites of batch c: each runs one earlier
 * Pinball module (src/dsky_*.c, src/cm_*.c) on a lanec_io, for the suites
 * (tests/lanec_pw_*_cases.c) and the Pinball transfer-ABI routines
 * (src/lanec_pin_routines.c). */
#include "lanec_pw_run.h"
#include "dsky_lamp_test.h"

static inline void lanec_pw_lamp_in(const lanec_io *io, dsky_lamp_io *p)
{
    p->control.superbank = io->superbank; p->control.interrupt_enabled = io->interrupt_enabled;
    p->channel11 = io->channel[011]; p->channel12 = io->channel[012]; p->channel13 = io->channel[013];
    p->wrote_channels = 0; p->restart_light = 0;
}

static inline void lanec_pw_lamp_out(lanec_io *io, const dsky_lamp_io *p)
{
    io->superbank = p->control.superbank; io->interrupt_enabled = p->control.interrupt_enabled;
    io->channel[011] = p->channel11; io->channel[012] = p->channel12; io->channel[013] = p->channel13;
}

static inline void lanec_pw_charin_in(const lanec_io *io, dsky_charin_io *p)
{
    p->control.superbank = io->superbank; p->control.interrupt_enabled = io->interrupt_enabled;
    p->channel11 = io->channel[011]; p->wrote_channel11 = 0;
}

static inline void lanec_pw_charin_out(lanec_io *io, const dsky_charin_io *p)
{
    io->superbank = p->control.superbank; io->interrupt_enabled = p->control.interrupt_enabled;
    io->channel[011] = p->channel11;
}

static inline void lanec_pw_cm_in(const lanec_io *io, cm_native_io *c)
{
    c->superbank = io->superbank; c->interrupt_enabled = io->interrupt_enabled;
    c->channel11 = io->channel[011]; c->channel12 = io->channel[012]; c->channel13 = io->channel[013];
}

static inline void lanec_pw_cm_out(lanec_io *io, const cm_native_io *c)
{
    io->superbank = c->superbank; io->interrupt_enabled = c->interrupt_enabled;
    io->channel[011] = c->channel11; io->channel[012] = c->channel12; io->channel[013] = c->channel13;
}

/* LANEC_PW_LAMP_RUN(name, lm, cm, route_type, route0): LM
 * lm(s, dsky_lamp_io *, route *) and CM cm(s, cm_native_io *, wrote_channels *,
 * restart_light *, route *), channels 011-013 through lanec_io. */
#define LANEC_PW_LAMP_RUN(name, lm, cm, route_type, route0)                                       \
    enum lanec_status name(apollo_overlay_state *s, lanec_io *io, enum lanec_mission mission) \
    { LANEC_ONE_MISSION(mission);                                                                  \
        route_type r = route0;                                                                     \
        if (mission == LANEC_LM) {                                                                 \
            dsky_lamp_io p; lanec_pw_lamp_in(io, &p);                                              \
            if (lm(s, &p, &r) != P66_BIAS_OK) return LANEC_OUT_OF_SCOPE;                           \
            lanec_pw_lamp_out(io, &p);                                                             \
        } else {                                                                                   \
            cm_native_io c = {0}; uint16_t ev = 0, rl = 0; lanec_pw_cm_in(io, &c);                 \
            if (cm(s, &c, &ev, &rl, &r) != EXEC_OK) return LANEC_OUT_OF_SCOPE;                     \
            lanec_pw_cm_out(io, &c);                                                               \
        }                                                                                          \
        return LANEC_OK;                                                                           \
    }

/* LANEC_PW_ROUTE_RUN(name, lm, cm, route_type, route0): LM
 * lm(s, p66_bias_io *, route *) and CM cm(s, cm_native_io *, route *). */
#define LANEC_PW_ROUTE_RUN(name, lm, cm, route_type, route0)                                      \
    enum lanec_status name(apollo_overlay_state *s, lanec_io *io, enum lanec_mission mission) \
    { LANEC_ONE_MISSION(mission);                                                                  \
        route_type r = route0;                                                                     \
        if (mission == LANEC_LM) {                                                                 \
            p66_bias_io p; p.superbank = io->superbank; p.interrupt_enabled = io->interrupt_enabled; \
            if (lm(s, &p, &r) != P66_BIAS_OK) return LANEC_OUT_OF_SCOPE;                           \
            io->superbank = p.superbank; io->interrupt_enabled = p.interrupt_enabled;              \
        } else {                                                                                   \
            cm_native_io c = {0}; c.superbank = io->superbank; c.interrupt_enabled = io->interrupt_enabled; \
            if (cm(s, &c, &r) != EXEC_OK) return LANEC_OUT_OF_SCOPE;                               \
            io->superbank = c.superbank; io->interrupt_enabled = c.interrupt_enabled;              \
        }                                                                                          \
        return LANEC_OK;                                                                           \
    }
#endif
