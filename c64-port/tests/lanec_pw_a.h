/* SPDX-License-Identifier: MIT */
#ifndef LANEC_PW_A_H
#define LANEC_PW_A_H
/* Batch a of the pw-* suites (tools/mk/lanec-pw-a.mk): adapters shared by
 * its CHARIN-family modules. Test-only. */
#include "lanec_pw.h"
#include "lanec_pw_a_run.h"
#include "dsky_charin_nv.h"
#include "dsky_nv_admission.h"

/* make() for a CHARIN-family generator gen(s, &dsky_charin_io, k) whose
 * numbers with k mod 8192 below 4096 are LM and the rest CM. */
#define LANEC_PW_CHARIN_MAKE(name, gen, domain, cases, run)                                        \
    static void name(uint32_t n, apollo_overlay_state *s, lanec_io *io, lanec_meta *m)            \
    {                                                                                              \
        dsky_charin_io c;                                                                          \
        uint16_t k = (uint16_t)lanec_pw_spread(n, cases, domain);                                  \
        lanec_fill(s, io, n);                                                                      \
        gen(s, &c, k);                                                                             \
        io->superbank = c.control.superbank; io->interrupt_enabled = c.control.interrupt_enabled; \
        io->channel[011] = c.channel11;                                                            \
        lanec_pw_settle(s);                                                                        \
        m->mission = k % 8192u >= 4096u ? LANEC_CM : LANEC_LM;                                     \
        lanec_pw_exit_stop(m, s, io, run);                                                         \
    }
/* The decimal formatters return to the caller in Q, which their generators
 * draw from all of 02000-03777 in the formatter's own bank 40. The oracle
 * stops the first time Z reaches the return address, so a caller inside
 * the code the formatter itself executes (GETINREL, +ON/-ON/SGNCOM and
 * DSPSIGN through 11DSPIN) would stop the original part-way. Such a caller
 * moves to the same offset in 02000-02177, which the formatter never
 * executes; the formatter treats every caller alike. */
static inline void lanec_pw_decimal_caller(apollo_overlay_state *s, enum dsky_nv_mission mission)
{
    static const uint16_t code[2][3][2] = {
        { { 02324u, 02327u }, { 02413u, 02442u }, { 03200u, 03417u } },   /* LM */
        { { 02225u, 02230u }, { 02314u, 02343u }, { 03105u, 03324u } } }; /* CM */
    uint16_t q = s->words[LANEC_Q], i;
    for (i = 0; i < 3u; ++i)
        if (q >= code[mission][i][0] && q <= code[mission][i][1])
            s->words[LANEC_Q] = (uint16_t)(02000u | (q & 0177u));
}
#endif
