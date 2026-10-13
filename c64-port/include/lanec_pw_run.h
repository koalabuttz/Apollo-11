/* SPDX-License-Identifier: MIT */
#ifndef LANEC_PW_RUN_H
#define LANEC_PW_RUN_H
/* The lanec_run adapters of the pw-* suites: each runs one earlier
 * Pinball module (src/dsky_*.c, src/cm_*.c) on a lanec_io, for the suites
 * (tests/lanec_pw_*_cases.c) and the Pinball transfer-ABI routines
 * (src/lanec_pin_routines.c). */
#include "lanec.h"
#include "lanec_pw_runs.h"
#include "p66_bias.h"
#include "cm_native_image.h"

/* LANEC_PW_P66_RUN(name, lm, cm): a lanec_run around an LM p66_bias_io
 * routine and its CM cm_native_io twin (either may be 0 for one mission). */
#define LANEC_PW_P66_RUN(name, lm, cm)                                                             \
    enum lanec_status name(apollo_overlay_state *s, lanec_io *io, enum lanec_mission mission) \
    {                                                                                              \
        int ok = 0;                                                                                \
        if (mission == LANEC_LM) {                                                                 \
            p66_bias_io p; p.superbank = io->superbank; p.interrupt_enabled = io->interrupt_enabled; \
            ok = lm(s, &p) == P66_BIAS_OK;                                                         \
            if (ok) { io->superbank = p.superbank; io->interrupt_enabled = p.interrupt_enabled; } \
        } else {                                                                                   \
            cm_native_io c = {0}; c.superbank = io->superbank; c.interrupt_enabled = io->interrupt_enabled; \
            ok = cm(s, &c) == EXEC_OK;                                                             \
            if (ok) { io->superbank = c.superbank; io->interrupt_enabled = c.interrupt_enabled; }  \
        }                                                                                          \
        return ok ? LANEC_OK : LANEC_OUT_OF_SCOPE;                                                 \
    }
#endif
