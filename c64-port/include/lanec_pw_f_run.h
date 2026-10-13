/* SPDX-License-Identifier: MIT */
#ifndef LANEC_PW_F_RUN_H
#define LANEC_PW_F_RUN_H
/* The lanec_run adapters of the pw-* suites of batch f: each runs one earlier
 * Pinball module (src/dsky_*.c, src/cm_*.c) on a lanec_io, for the suites
 * (tests/lanec_pw_*_cases.c) and the Pinball transfer-ABI routines
 * (src/lanec_pin_routines.c). */
#include "lanec_pw_run.h"
#include <string.h>

static inline void lanec_pw_f_from_cm(lanec_io *io, const cm_native_io *c)
{
    io->channel[011] = c->channel11; io->channel[012] = c->channel12;
    io->channel[013] = c->channel13; io->channel[014] = c->channel14;
    io->channel[015] = c->main_keys; io->channel[016] = c->navigation_keys;
    io->superbank = c->superbank; io->interrupt_enabled = c->interrupt_enabled;
}

static inline void lanec_pw_f_to_cm(cm_native_io *c, const lanec_io *io)
{
    memset(c, 0, sizeof *c);
    c->channel11 = io->channel[011]; c->channel12 = io->channel[012];
    c->channel13 = io->channel[013]; c->channel14 = io->channel[014];
    c->main_keys = io->channel[015]; c->navigation_keys = io->channel[016];
    c->superbank = io->superbank; c->interrupt_enabled = io->interrupt_enabled;
}

/* A lanec_run around one CM call: `call` is an expression on `s` and `c`
 * that is nonzero when the native routine reached its boundary. */
#define LANEC_PW_F_CM_RUN(name, call)                                                              \
    enum lanec_status name(apollo_overlay_state *s, lanec_io *io, enum lanec_mission mission) \
    { LANEC_ONE_MISSION(mission);                                                                  \
        cm_native_io c;                                                                            \
        if (mission != LANEC_CM) return LANEC_OUT_OF_SCOPE;                                        \
        lanec_pw_f_to_cm(&c, io);                                                                  \
        if (!(call)) return LANEC_OUT_OF_SCOPE;                                                    \
        lanec_pw_f_from_cm(io, &c);                                                                \
        return LANEC_OK;                                                                           \
    }
/* A lanec_run around a staged CM module: the image's Z selects the stage,
 * so each of the module's `count` stages is tried on a copy and the first
 * that reaches its boundary is kept. */
#define LANEC_PW_F_CM_STEPS(name, step, count)                                                     \
    enum lanec_status name(apollo_overlay_state *s, lanec_io *io, enum lanec_mission mission)      \
    { LANEC_ONE_MISSION(mission);                                                                  \
        static apollo_overlay_state t;                                                             \
        cm_native_io c;                                                                            \
        uint8_t r;                                                                                 \
        if (mission != LANEC_CM) return LANEC_OUT_OF_SCOPE;                                        \
        for (r = 0; r < (count); ++r) {                                                            \
            t = *s;                                                                                \
            lanec_pw_f_to_cm(&c, io);                                                              \
            if (step(&t, &c, r)) { *s = t; lanec_pw_f_from_cm(io, &c); return LANEC_OK; }          \
        }                                                                                          \
        return LANEC_OUT_OF_SCOPE;                                                                 \
    }
#endif
