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
#include "lanec_pw_g_run.h"
#include "dsky_charin_number.h"

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
