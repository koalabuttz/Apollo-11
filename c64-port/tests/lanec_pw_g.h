/* SPDX-License-Identifier: MIT */
#ifndef LANEC_PW_G_H
#define LANEC_PW_G_H
/* Batch g of the pw-* suites (tools/mk/lanec-pw-g.mk): the LM Pinball
 * modules whose generators are `<module>_prepare(s, n)` (LM only). Their
 * own checks run each number n with SUPER 0100, the interrupt state n mod 2
 * and, for the CHARIN modules, channel 11 from `<module>_channel11(n)`; the
 * entry variant (key, rounding, DSPIN/11DSPIN/2BLANK, DPnOUTSF) is the one
 * the generator put in Z, so run() reads it back from Z. Test-only. */
#include "lanec_pw.h"
#include "dsky_charin_number.h"

/* LANEC_PW_G_P66_RUN(name, CALL): CALL is the LM routine on the
 * p66_bias_io `p`; CM cases are refused. */
#define LANEC_PW_G_P66_RUN(name, CALL)                                                             \
    static enum lanec_status name(apollo_overlay_state *s, lanec_io *io, enum lanec_mission mission) \
    {                                                                                              \
        p66_bias_io p;                                                                             \
        if (mission != LANEC_LM) return LANEC_OUT_OF_SCOPE;                                        \
        p.superbank = io->superbank; p.interrupt_enabled = io->interrupt_enabled;                  \
        if ((CALL) != P66_BIAS_OK) return LANEC_OUT_OF_SCOPE;                                      \
        io->superbank = p.superbank; io->interrupt_enabled = p.interrupt_enabled;                  \
        return LANEC_OK;                                                                           \
    }
/* The same on the dsky_charin_io `p` (channel 11 passes through). */
#define LANEC_PW_G_CHARIN_RUN(name, CALL)                                                          \
    static enum lanec_status name(apollo_overlay_state *s, lanec_io *io, enum lanec_mission mission) \
    {                                                                                              \
        dsky_charin_io p;                                                                          \
        if (mission != LANEC_LM) return LANEC_OUT_OF_SCOPE;                                        \
        p.control.superbank = io->superbank; p.control.interrupt_enabled = io->interrupt_enabled; \
        p.channel11 = io->channel[011]; p.wrote_channel11 = 0;                                     \
        if ((CALL) != P66_BIAS_OK) return LANEC_OUT_OF_SCOPE;                                      \
        io->superbank = p.control.superbank; io->interrupt_enabled = p.control.interrupt_enabled;  \
        io->channel[011] = p.channel11;                                                            \
        return LANEC_OK;                                                                           \
    }
/* make() over prepare(s, k) for k spread over 0..domain-1; CH11 is an
 * expression on k giving channel 11. */
#define LANEC_PW_G_MAKE(name, prepare, domain, cases, run, CH11)                                   \
    static void name(uint32_t n, apollo_overlay_state *s, lanec_io *io, lanec_meta *m)            \
    {                                                                                              \
        uint16_t k = (uint16_t)lanec_pw_spread(n, cases, domain);                                  \
        lanec_fill(s, io, n);                                                                      \
        prepare(s, k);                                                                             \
        io->superbank = 0100u; io->interrupt_enabled = (uint16_t)(k % 2u);                         \
        io->channel[011] = (CH11);                                                                 \
        lanec_pw_settle(s);                                                                        \
        m->mission = LANEC_LM;                                                                     \
        lanec_pw_exit_stop(m, s, io, run);                                                         \
    }
#endif
