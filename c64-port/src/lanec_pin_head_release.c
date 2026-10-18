/* SPDX-License-Identifier: MIT */
/* Pinball RELDSP1 (fixed-fixed), Luminary099 and Comanche055: XCH Q,
 * TS RELRET, CCS DSPLIST, then with DSPLIST +0 RELDSP2 (RELINT, CS BIT5,
 * EXTEND, WAND DSALMOUT), and on every path CAF ZERO, TS DSPLOCK,
 * TC RELRET, whose TC takes the caller's return. Monitor set-up (MONIT2)
 * and UNSUSPEN call it. It runs to the caller's return and leaves as the
 * original does there. See docs/LANEC-PIN-HEADS.md. */
#include "lanec_pin_heads.h"
#include "lanec_symbols.h"
#include "lanec_ev_internal.h"

typedef char
    same_words[(LANEC_LM_RELRET == LANEC_CM_RELRET && LANEC_LM_DSPLIST == LANEC_CM_DSPLIST &&
                LANEC_LM_DSPLOCK == LANEC_CM_DSPLOCK && LANEC_LM_RELDSP2 == LANEC_LM_RELDSP1 - 7u &&
                LANEC_CM_RELDSP2 == LANEC_CM_RELDSP1 - 7u)
                   ? 1
                   : -1];
#define BIT5     020u
#define DSALMOUT 011u

enum lanec_status lanec_pin_head_release_run(apollo_overlay_state *s, lanec_io *io,
                                             enum lanec_mission m)
{
    uint16_t *w = s->words, q = w[LANEC_Q];
    LANEC_ONE_MISSION(m);
    if (!lanec_registers_valid(s) ||
        w[LANEC_Z] != (m == LANEC_LM ? LANEC_LM_RELDSP1 : LANEC_CM_RELDSP1))
        return LANEC_OUT_OF_SCOPE;
    /* Q a return address in fixed memory: TS RELRET stores it as the
     * TC RELRET takes it (no overflow to skip on, no erasable to run). */
    if (q < 02000u || q > 07777u) return LANEC_OUT_OF_SCOPE;
    w[LANEC_Q] = w[LANEC_A]; /* XCH Q */
    w[LANEC_A] = q;
    (void)lanec_ts(s, LANEC_LM_RELRET);
    if (lanec_ccs(s, w[LANEC_LM_DSPLIST]) == 1u) { /* +0: TC RELDSP2 */
        io->interrupt_enabled = 1;                 /* RELINT */
        lanec_ev_cs(s, BIT5);
        lanec_ev_wand(s, io, DSALMOUT);
    }
    w[LANEC_A] = 0; /* CAF ZERO (-0: CCS left +0) */
    (void)lanec_ts(s, LANEC_LM_DSPLOCK);
    lanec_tc(s, q, LANEC_LM_RELRET + 1u); /* TC RELRET: its TC to the caller */
    return LANEC_OK;
}
