/* SPDX-License-Identifier: MIT */
#ifndef LANEC_PW_A_RUN_H
#define LANEC_PW_A_RUN_H
/* The lanec_run adapters of the pw-* suites of batch a: each runs one earlier
 * Pinball module (src/dsky_*.c, src/cm_*.c) on a lanec_io, for the suites
 * (tests/lanec_pw_*_cases.c) and the Pinball transfer-ABI routines
 * (src/lanec_pin_routines.c). */
#include "lanec_pw_run.h"
#include "dsky_charin_nv.h"
#include "dsky_nv_admission.h"

/* The DECDSP modules take the mission as an enum dsky_nv_mission. */
static inline enum dsky_nv_mission lanec_pw_nv(enum lanec_mission mission)
{
    return mission == LANEC_CM ? DSKY_NV_CM : DSKY_NV_LM;
}

/* Z at the rounded entry of the decimal formatters (LM 03226, CM 03133;
 * the unrounded entries are 03262 and 03167). */
static inline uint8_t lanec_pw_rounded(const apollo_overlay_state *s)
{
    return (uint8_t)(s->words[LANEC_Z] == 03226u || s->words[LANEC_Z] == 03133u);
}

/* LANEC_PW_CHARIN_RUN(name, lm, cm): a lanec_run around an LM routine on a
 * dsky_charin_io `c` and its CM twin on a cm_native_io `x` (plus the
 * wrote-channel-11 flag `w`). `lm` and `cm` are the complete calls; channel
 * 11 passes through io->channel[011]. */
#define LANEC_PW_CHARIN_RUN(name, lm, cm)                                                          \
    enum lanec_status name(apollo_overlay_state *s, lanec_io *io, enum lanec_mission mission) \
    { LANEC_ONE_MISSION(mission);                                                                  \
        int ok;                                                                                    \
        if (mission == LANEC_LM) {                                                                 \
            dsky_charin_io c;                                                                      \
            c.control.superbank = io->superbank; c.control.interrupt_enabled = io->interrupt_enabled; \
            c.channel11 = io->channel[011]; c.wrote_channel11 = 0;                                 \
            ok = (lm) == P66_BIAS_OK;                                                              \
            if (ok) { io->superbank = c.control.superbank; io->interrupt_enabled = c.control.interrupt_enabled; \
                      io->channel[011] = c.channel11; }                                            \
        } else {                                                                                   \
            cm_native_io x = {0}; uint16_t w = 0;                                                  \
            x.superbank = io->superbank; x.interrupt_enabled = io->interrupt_enabled;              \
            x.channel11 = io->channel[011];                                                        \
            ok = (cm) == EXEC_OK;                                                                  \
            (void)w;                                                                               \
            if (ok) { io->superbank = x.superbank; io->interrupt_enabled = x.interrupt_enabled;    \
                      io->channel[011] = x.channel11; }                                            \
        }                                                                                          \
        return ok ? LANEC_OK : LANEC_OUT_OF_SCOPE;                                                 \
    }
#endif
