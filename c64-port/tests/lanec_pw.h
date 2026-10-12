/* SPDX-License-Identifier: MIT */
#ifndef LANEC_PW_H
#define LANEC_PW_H
/* Shared pieces of the pw-* suites (tests/lanec_pw_*.c): the earlier Pinball
 * modules (src/dsky_*.c and their CM twins) run through the Lane C harness,
 * so the Virtual AGC oracle compares their complete results and records
 * which original words they cover. Each suite draws its states from the
 * module's own input generator, spread over the generator's whole domain.
 * The oracle runs the original program from the entry until Z first reaches
 * the point (bank and address) where the native module stopped with the
 * same Q; the native module claims nothing past it. Test-only. */
#include "lanec_suites.h"
#include "p66_bias.h"
#include "cm_native_image.h"

void lanec_pw_exit_stop(lanec_meta *, const apollo_overlay_state *, const lanec_io *, lanec_run);
/* A generator's image in the forms the oracle can hold (L sign-extended,
 * OPTY/OPTX and GYROCMD..OPTXCMD idle, as lanec_fill leaves them). */
void lanec_pw_settle(apollo_overlay_state *);
/* Generator number of case n of `cases` spread evenly over 0..domain-1. */
uint32_t lanec_pw_spread(uint32_t n, uint32_t cases, uint32_t domain);

/* LANEC_PW_P66_RUN(name, lm, cm): a lanec_run around an LM p66_bias_io
 * routine and its CM cm_native_io twin (either may be 0 for one mission). */
#define LANEC_PW_P66_RUN(name, lm, cm)                                                             \
    static enum lanec_status name(apollo_overlay_state *s, lanec_io *io, enum lanec_mission mission) \
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
