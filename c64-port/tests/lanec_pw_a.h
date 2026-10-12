/* SPDX-License-Identifier: MIT */
#ifndef LANEC_PW_A_H
#define LANEC_PW_A_H
/* Batch a of the pw-* suites (tools/mk/lanec-pw-a.mk): adapters shared by
 * its CHARIN-family modules. Test-only. */
#include "lanec_pw.h"
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
    static enum lanec_status name(apollo_overlay_state *s, lanec_io *io, enum lanec_mission mission) \
    {                                                                                              \
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
