/* SPDX-License-Identifier: MIT */
#ifndef LANEC_PW_C_H
#define LANEC_PW_C_H
/* Adapters of the pw-* batch c suites (tests/lanec_pw_*_cases.c, built by
 * tools/mk/lanec-pw-c.mk): the DSKY lamp record (channels 011-013) and the
 * CHARIN record (channel 011) as lanec_io channels, and the generator-driven
 * make() every one of these suites shares. Test-only. */
#include "lanec_pw.h"
#include "lanec_pw_c_run.h"
#include "dsky_lamp_test.h"

/* LANEC_PW_MAKE(name, record, generator, copy, run, domain, cases): make()
 * drawing generator number k = spread(n) (LM below domain/2, CM above);
 * copy(io, &record) moves the generator's I/O record into lanec_io. */
#define LANEC_PW_MAKE(name, record, generator, copy, run, domain, cases)                          \
    static void name(uint32_t n, apollo_overlay_state *s, lanec_io *io, lanec_meta *m)            \
    {                                                                                              \
        record p;                                                                                  \
        uint16_t k = (uint16_t)lanec_pw_spread(n, cases, domain);                                  \
        lanec_fill(s, io, n);                                                                      \
        generator(s, &p, k);                                                                       \
        copy(io, &p);                                                                              \
        lanec_pw_settle(s);                                                                        \
        m->mission = k >= (domain) / 2u ? LANEC_CM : LANEC_LM;                                     \
        lanec_pw_exit_stop(m, s, io, run);                                                         \
    }
static inline void lanec_pw_copy_p66(lanec_io *io, const p66_bias_io *p)
{
    io->superbank = p->superbank; io->interrupt_enabled = p->interrupt_enabled;
}
#endif
