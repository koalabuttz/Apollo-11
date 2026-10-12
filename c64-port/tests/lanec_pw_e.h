/* SPDX-License-Identifier: MIT */
#ifndef LANEC_PW_E_H
#define LANEC_PW_E_H
/* Adapters shared by the pw-* suites of batch e (tools/mk/lanec-pw-e.mk):
 * lanec_io to the DSKY lamp record (dsky_lamp_io, channels 11-13) and to the
 * CHARIN record (dsky_charin_io, channel 11), with their CM cm_native_io
 * twins. LM_CALL / CM_CALL are the native calls, written over the locals
 * p (LM record), c (cm_native_io), wrote and restart. The RESTART lamp latch
 * is not part of lanec_io (the oracle does not model it): it enters off and
 * its result, like the wrote-channel flags, is not compared. Results are
 * copied back only when the module accepts the case. Test-only. */
#include "lanec_pw.h"
#include "lanec_pw_e_run.h"

/* lanec_pw_exit_stop for suites whose run refuses some generated cases: a
 * refused case has no stop, so the oracle is given no range either and
 * leaves after one instruction (diff.py --refusals does not compare it). */
#define LANEC_PW_E_STOP(m, s, io, run)                                                             \
    do { lanec_pw_exit_stop(m, s, io, run); if (!(m)->stops) (m)->ranges = 0; } while (0)

/* make() for a lamp / CHARIN generator over 8192 numbers (4096-8191 CM). */
#define LANEC_PW_LAMP_MAKE(name, input, run, cases)                                                \
    static void name(uint32_t n, apollo_overlay_state *s, lanec_io *io, lanec_meta *m)            \
    {                                                                                              \
        dsky_lamp_io p;                                                                            \
        uint16_t k = (uint16_t)lanec_pw_spread(n, cases, 8192u);                                   \
        lanec_fill(s, io, n);                                                                      \
        input(s, &p, k);                                                                           \
        io->superbank = p.control.superbank; io->interrupt_enabled = p.control.interrupt_enabled;  \
        io->channel[011] = p.channel11; io->channel[012] = p.channel12; io->channel[013] = p.channel13; \
        lanec_pw_settle(s);                                                                        \
        m->mission = k >= 4096u ? LANEC_CM : LANEC_LM;                                             \
        lanec_pw_exit_stop(m, s, io, run);                                                         \
    }
#define LANEC_PW_CHARIN_MAKE(name, input, run, cases)                                              \
    static void name(uint32_t n, apollo_overlay_state *s, lanec_io *io, lanec_meta *m)            \
    {                                                                                              \
        dsky_charin_io p;                                                                          \
        uint16_t k = (uint16_t)lanec_pw_spread(n, cases, 8192u);                                   \
        lanec_fill(s, io, n);                                                                      \
        input(s, &p, k);                                                                           \
        io->superbank = p.control.superbank; io->interrupt_enabled = p.control.interrupt_enabled;  \
        io->channel[011] = p.channel11;                                                            \
        lanec_pw_settle(s);                                                                        \
        m->mission = k >= 4096u ? LANEC_CM : LANEC_LM;                                             \
        lanec_pw_exit_stop(m, s, io, run);                                                         \
    }
#endif
