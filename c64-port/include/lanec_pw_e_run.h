/* SPDX-License-Identifier: MIT */
#ifndef LANEC_PW_E_RUN_H
#define LANEC_PW_E_RUN_H
/* The lanec_run adapters of the pw-* suites of batch e: each runs one earlier
 * Pinball module (src/dsky_*.c, src/cm_*.c) on a lanec_io, for the suites
 * (tests/lanec_pw_*_cases.c) and the Pinball transfer-ABI routines
 * (src/lanec_pin_routines.c). */
#include "lanec_pw_run.h"

#define LANEC_PW_LAMP_RUN(name, LM_CALL, CM_CALL)                                                  \
    enum lanec_status name(apollo_overlay_state *s, lanec_io *io, enum lanec_mission mission) \
    {                                                                                              \
        int ok;                                                                                    \
        if (mission == LANEC_LM) {                                                                 \
            dsky_lamp_io p;                                                                        \
            p.control.superbank = io->superbank; p.control.interrupt_enabled = io->interrupt_enabled; \
            p.channel11 = io->channel[011]; p.channel12 = io->channel[012]; p.channel13 = io->channel[013]; \
            p.wrote_channels = 0; p.restart_light = 0;                                             \
            ok = (LM_CALL) == P66_BIAS_OK;                                                         \
            if (ok) {                                                                              \
                io->superbank = p.control.superbank; io->interrupt_enabled = p.control.interrupt_enabled; \
                io->channel[011] = p.channel11; io->channel[012] = p.channel12; io->channel[013] = p.channel13; \
            }                                                                                      \
        } else {                                                                                   \
            cm_native_io c = {0}; uint16_t wrote = 0, restart = 0;                                 \
            c.superbank = io->superbank; c.interrupt_enabled = io->interrupt_enabled;             \
            c.channel11 = io->channel[011]; c.channel12 = io->channel[012]; c.channel13 = io->channel[013]; \
            ok = (CM_CALL) == EXEC_OK;                                                             \
            if (ok) {                                                                              \
                io->superbank = c.superbank; io->interrupt_enabled = c.interrupt_enabled;          \
                io->channel[011] = c.channel11; io->channel[012] = c.channel12; io->channel[013] = c.channel13; \
            }                                                                                      \
        }                                                                                          \
        return ok ? LANEC_OK : LANEC_OUT_OF_SCOPE;                                                 \
    }

#define LANEC_PW_CHARIN_RUN(name, LM_CALL, CM_CALL)                                                \
    enum lanec_status name(apollo_overlay_state *s, lanec_io *io, enum lanec_mission mission) \
    {                                                                                              \
        int ok;                                                                                    \
        if (mission == LANEC_LM) {                                                                 \
            dsky_charin_io p;                                                                      \
            p.control.superbank = io->superbank; p.control.interrupt_enabled = io->interrupt_enabled; \
            p.channel11 = io->channel[011]; p.wrote_channel11 = 0;                                 \
            ok = (LM_CALL) == P66_BIAS_OK;                                                         \
            if (ok) {                                                                              \
                io->superbank = p.control.superbank; io->interrupt_enabled = p.control.interrupt_enabled; \
                io->channel[011] = p.channel11;                                                    \
            }                                                                                      \
        } else {                                                                                   \
            cm_native_io c = {0}; uint16_t wrote = 0;                                              \
            c.superbank = io->superbank; c.interrupt_enabled = io->interrupt_enabled;             \
            c.channel11 = io->channel[011];                                                        \
            ok = (CM_CALL) == EXEC_OK;                                                             \
            if (ok) {                                                                              \
                io->superbank = c.superbank; io->interrupt_enabled = c.interrupt_enabled;          \
                io->channel[011] = c.channel11;                                                    \
            }                                                                                      \
        }                                                                                          \
        return ok ? LANEC_OK : LANEC_OUT_OF_SCOPE;                                                 \
    }
#endif
