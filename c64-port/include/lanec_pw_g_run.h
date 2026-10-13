/* SPDX-License-Identifier: MIT */
#ifndef LANEC_PW_G_RUN_H
#define LANEC_PW_G_RUN_H
/* The lanec_run adapters of the pw-* suites of batch g: each runs one earlier
 * Pinball module (src/dsky_*.c, src/cm_*.c) on a lanec_io, for the suites
 * (tests/lanec_pw_*_cases.c) and the Pinball transfer-ABI routines
 * (src/lanec_pin_routines.c). */
#include "lanec_pw_run.h"
#include "dsky_charin_number.h"

/* LANEC_PW_G_P66_RUN(name, CALL): CALL is the LM routine on the
 * p66_bias_io `p`; CM cases are refused. */
#define LANEC_PW_G_P66_RUN(name, CALL)                                                             \
    enum lanec_status name(apollo_overlay_state *s, lanec_io *io, enum lanec_mission mission) \
    { LANEC_ONE_MISSION(mission);                                                                  \
        p66_bias_io p;                                                                             \
        if (mission != LANEC_LM) return LANEC_OUT_OF_SCOPE;                                        \
        p.superbank = io->superbank; p.interrupt_enabled = io->interrupt_enabled;                  \
        if ((CALL) != P66_BIAS_OK) return LANEC_OUT_OF_SCOPE;                                      \
        io->superbank = p.superbank; io->interrupt_enabled = p.interrupt_enabled;                  \
        return LANEC_OK;                                                                           \
    }

/* The same on the dsky_charin_io `p` (channel 11 passes through). */
#define LANEC_PW_G_CHARIN_RUN(name, CALL)                                                          \
    enum lanec_status name(apollo_overlay_state *s, lanec_io *io, enum lanec_mission mission) \
    { LANEC_ONE_MISSION(mission);                                                                  \
        dsky_charin_io p;                                                                          \
        if (mission != LANEC_LM) return LANEC_OUT_OF_SCOPE;                                        \
        p.control.superbank = io->superbank; p.control.interrupt_enabled = io->interrupt_enabled; \
        p.channel11 = io->channel[011]; p.wrote_channel11 = 0;                                     \
        if ((CALL) != P66_BIAS_OK) return LANEC_OUT_OF_SCOPE;                                      \
        io->superbank = p.control.superbank; io->interrupt_enabled = p.control.interrupt_enabled;  \
        io->channel[011] = p.channel11;                                                            \
        return LANEC_OK;                                                                           \
    }
#endif
