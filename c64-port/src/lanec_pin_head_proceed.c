/* SPDX-License-Identifier: MIT */
/* Pinball proceed and terminate (bank 40), Luminary099 and Comanche055:
 * VBPROC (CAF ONE, TS LOADSTAT, TC KILMONON), VBTERM (CS ONE,
 * TC VBPROC +1), PROCKEY (CAF ZERO, TS REQRET, CS VD1, TS DSPCOUNT,
 * TC VBPROC), VBRESEQ (CS ZERO, TC VBPROC +1) and RECALTST (CCS CADRSTOR,
 * TC RECAL1, TC ENDOFJOB, or on into RECAL1; CADRSTOR -0, which holds no
 * CADR, is out of scope). Each runs to the next call and leaves as the original does
 * there. See docs/LANEC-PIN-HEADS.md. */
#include "lanec_pin_heads.h"
#include "lanec_symbols.h"
#include "lanec_ev_internal.h"

#define S(m, x) ((m) == LANEC_LM ? LANEC_LM_##x : LANEC_CM_##x)
typedef char same_words[(LANEC_LM_VBTERM == LANEC_LM_VBPROC + 6u && LANEC_CM_VBTERM == LANEC_CM_VBPROC + 6u &&
    LANEC_LM_PROCKEY == LANEC_LM_VBPROC + 010u && LANEC_CM_PROCKEY == LANEC_CM_VBPROC + 010u &&
    LANEC_LM_VBRESEQ == LANEC_LM_VBPROC + 015u && LANEC_CM_VBRESEQ == LANEC_CM_VBPROC + 015u &&
    LANEC_LM_RECAL1 == LANEC_LM_RECALTST + 3u && LANEC_CM_RECAL1 == LANEC_CM_RECALTST + 3u &&
    LANEC_LM_VBPROC_BANK == 040u && LANEC_CM_VBPROC_BANK == 040u && LANEC_LM_RECALTST_BANK == 040u &&
    LANEC_CM_RECALTST_BANK == 040u && LANEC_LM_LOADSTAT == LANEC_CM_LOADSTAT && LANEC_LM_REQRET == LANEC_CM_REQRET &&
    LANEC_LM_DSPCOUNT == LANEC_CM_DSPCOUNT && LANEC_LM_CADRSTOR == LANEC_CM_CADRSTOR) ? 1 : -1];
#define VD1 023u

enum lanec_status lanec_pin_head_proceed_run(apollo_overlay_state *s, lanec_io *io, enum lanec_mission m)
{
    uint16_t *w = s->words, z = w[LANEC_Z], proc = S(m, VBPROC), recal = S(m, RECALTST);
    LANEC_ONE_MISSION(m);
    if (!lanec_registers_valid(s) || lanec_current_bank(s, io) != 040u) return LANEC_OUT_OF_SCOPE;
    if (z == proc) {                                     /* CAF ONE, TS LOADSTAT, TC KILMONON */
        lanec_ev_ca(s, 1u);
        (void)lanec_ts(s, LANEC_LM_LOADSTAT);
        lanec_tc(s, S(m, KILMONON), (uint16_t)(z + 3u));
    } else if (z == proc + 6u || z == proc + 015u) {    /* CS ONE / CS ZERO, TC VBPROC +1 */
        w[LANEC_A] = z == proc + 6u ? 0177776u : 0177777u;
        lanec_tc(s, (uint16_t)(proc + 1u), (uint16_t)(z + 2u));
    } else if (z == proc + 010u) {                       /* CAF ZERO, TS REQRET, CS VD1, TS DSPCOUNT, TC VBPROC */
        w[LANEC_A] = 0;
        (void)lanec_ts(s, LANEC_LM_REQRET);
        lanec_ev_cs(s, VD1);
        (void)lanec_ts(s, LANEC_LM_DSPCOUNT);
        lanec_tc(s, proc, (uint16_t)(z + 5u));
    } else if (z == recal && w[LANEC_LM_CADRSTOR] != 077777u) {   /* -0 goes on past RECAL1: not a CADR */
        switch (lanec_ccs(s, w[LANEC_LM_CADRSTOR])) {
        case 0: lanec_tc(s, (uint16_t)(recal + 3u), (uint16_t)(z + 2u)); break;   /* TC RECAL1 */
        case 1: lanec_tc(s, S(m, ENDOFJOB), (uint16_t)(z + 3u)); break;            /* TC ENDOFJOB */
        default: w[LANEC_Z] = (uint16_t)(recal + 3u); break;                      /* on into RECAL1 */
        }
    } else {
        return LANEC_OUT_OF_SCOPE;
    }
    return LANEC_OK;
}
