/* SPDX-License-Identifier: MIT */
/* Pinball release steps, Luminary099 and Comanche055: VBPROC +3, +4, +5
 * (bank 40), the calls after KILMONON returns (TC RELDSP, TC FLASHOFF,
 * TC RECALTST), each run to its call; and RELDSP1 (fixed-fixed): XCH Q,
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
                LANEC_CM_RELDSP2 == LANEC_CM_RELDSP1 - 7u && LANEC_LM_VBPROC_BANK == 040u &&
                LANEC_CM_VBPROC_BANK == 040u && LANEC_LM_RECALTST == LANEC_LM_VBPROC + 0105u &&
                LANEC_CM_RECALTST == LANEC_CM_VBPROC + 0105u)
                   ? 1
                   : -1];
#define BIT5     020u
#define DSALMOUT 011u

enum lanec_status lanec_pin_head_release_run(apollo_overlay_state *s, lanec_io *io,
                                             enum lanec_mission m)
{
    uint16_t *w = s->words, q = w[LANEC_Q], z = w[LANEC_Z], proc;
    LANEC_ONE_MISSION(m);
    proc = m == LANEC_LM ? LANEC_LM_VBPROC : LANEC_CM_VBPROC;
    if (!lanec_registers_valid(s)) return LANEC_OUT_OF_SCOPE;
    if (z >= proc + 3u && z <= proc + 5u) { /* TC RELDSP, TC FLASHOFF, TC RECALTST */
        static const uint16_t lm[3] = {LANEC_LM_RELDSP, LANEC_LM_FLASHOFF, LANEC_LM_RECALTST};
        static const uint16_t cm[3] = {LANEC_CM_RELDSP, LANEC_CM_FLASHOFF, LANEC_CM_RECALTST};
        if (lanec_current_bank(s, io) != 040u) return LANEC_OUT_OF_SCOPE;
        lanec_tc(s, (m == LANEC_LM ? lm : cm)[z - proc - 3u], (uint16_t)(z + 1u));
        return LANEC_OK;
    }
    if (z != (m == LANEC_LM ? LANEC_LM_RELDSP1 : LANEC_CM_RELDSP1)) return LANEC_OUT_OF_SCOPE;
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
