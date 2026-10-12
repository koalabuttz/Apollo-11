/* SPDX-License-Identifier: MIT */
#ifndef LANEC_PW_F_H
#define LANEC_PW_F_H
/* Batch f of the pw-* suites (tools/mk/lanec-pw-f.mk): the CM Pinball
 * modules that take a cm_native_io, and LM NVSUBUSY. A cm_native_io carries
 * channels 11-16, the superbank and the interrupt state; these helpers move
 * them between it and a lanec_io. Test-only. */
#include <string.h>
#include "lanec_pw.h"

static inline void lanec_pw_f_from_cm(lanec_io *io, const cm_native_io *c)
{
    io->channel[011] = c->channel11; io->channel[012] = c->channel12;
    io->channel[013] = c->channel13; io->channel[014] = c->channel14;
    io->channel[015] = c->main_keys; io->channel[016] = c->navigation_keys;
    io->superbank = c->superbank; io->interrupt_enabled = c->interrupt_enabled;
}
static inline void lanec_pw_f_to_cm(cm_native_io *c, const lanec_io *io)
{
    memset(c, 0, sizeof *c);
    c->channel11 = io->channel[011]; c->channel12 = io->channel[012];
    c->channel13 = io->channel[013]; c->channel14 = io->channel[014];
    c->main_keys = io->channel[015]; c->navigation_keys = io->channel[016];
    c->superbank = io->superbank; c->interrupt_enabled = io->interrupt_enabled;
}
/* The oracle stops the first time Z reaches the caller the native routine
 * returned to. The DSPIN/DSPDECWD family itself executes these CM addresses
 * before it returns: the return cells DSEXIT (0114), WDRET (0115) and
 * DSPWDRET (0144), which it reaches by TC, and the fixed-fixed SLEFT5
 * (04354-04361), DMPSUB (07075-07104) and SHORTMP (07256-07264) code it calls.
 * A generator caller Q at one of them is no ordinary caller and cannot be
 * told apart from the routine's own passage, so those generator numbers are
 * skipped. */
static inline int lanec_pw_f_caller_reentered(uint16_t q)
{
    return q == 0114u || q == 0115u || q == 0144u || (q >= 04354u && q <= 04361u)
        || (q >= 07075u && q <= 07104u) || (q >= 07256u && q <= 07264u);
}
/* lanec_pw_exit_stop for a route that may stop at the CM interpreter's
 * NEWOPS (06037) with LOC at the next interpretive word: the original passes
 * NEWOPS once per interpretive instruction, so the oracle stops there only
 * when LOC (in the bank FB selects) is the native routine's final LOC, or is
 * in a bank other than the target bank and the interpretive bank the case
 * starts in (FB, or BANKSET when the case starts in fixed-fixed code). */
static inline void lanec_pw_f_exit_stop(lanec_meta *m, const apollo_overlay_state *s, const lanec_io *io, lanec_run run)
{
    static apollo_overlay_state t;
    lanec_io u = *io;
    uint16_t loc, tb, sb[2], i, k;
    lanec_pw_exit_stop(m, s, io, run);
    if (m->mission != LANEC_CM || m->stops != 1u || m->stop[0].address != 06037u) return;
    t = *s;
    if (run(&t, &u, m->mission) != LANEC_OK) return;
    loc = t.words[0164];
    tb = lanec_current_bank(&t, &u);
    sb[0] = lanec_current_bank(s, io);
    sb[1] = (uint16_t)((s->words[0165] >> 10) & 037u);
    if (sb[1] >= 030u && (io->superbank & 0100u)) sb[1] = (uint16_t)(sb[1] + 010u);
    m->stops = 0; m->newops = 06037u; m->locs = 0;
    if (loc > 02000u) { m->loc[m->locs].bank = tb; m->loc[m->locs].lo = 02000u; m->loc[m->locs++].hi = (uint16_t)(loc - 1u); }
    if (loc < 03777u) { m->loc[m->locs].bank = tb; m->loc[m->locs].lo = (uint16_t)(loc + 1u); m->loc[m->locs++].hi = 03777u; }
    for (i = 0; i < 2u; ++i) {
        int seen = sb[i] == tb || sb[i] == 0u;
        for (k = 0; k < i; ++k) seen |= sb[k] == sb[i];
        if (!seen) { m->loc[m->locs].bank = sb[i]; m->loc[m->locs].lo = 02000u; m->loc[m->locs++].hi = 03777u; }
    }
}
/* A lanec_run around one CM call: `call` is an expression on `s` and `c`
 * that is nonzero when the native routine reached its boundary. */
#define LANEC_PW_F_CM_RUN(name, call)                                                              \
    static enum lanec_status name(apollo_overlay_state *s, lanec_io *io, enum lanec_mission mission) \
    {                                                                                              \
        cm_native_io c;                                                                            \
        if (mission != LANEC_CM) return LANEC_OUT_OF_SCOPE;                                        \
        lanec_pw_f_to_cm(&c, io);                                                                  \
        if (!(call)) return LANEC_OUT_OF_SCOPE;                                                    \
        lanec_pw_f_from_cm(io, &c);                                                                \
        return LANEC_OK;                                                                           \
    }
#endif
