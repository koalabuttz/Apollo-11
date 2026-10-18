/* SPDX-License-Identifier: MIT */
/* Pinball operator requests (bank 41), Luminary099 and Comanche055:
 * VBRQEXEC (CAF BIT1, MASK DSPTEM1, CCS A, TC SETVAC, CAF TCNOVAC, into
 * REQEX1) and VBRQWAIT (CAF TCWAIT, TS MPAC, CA DSPTEM1, TC REQUESTC -1).
 * Each runs to the next call or to REQEX1 and leaves as the original does
 * there, so SETVAC, REQEX1 and REQUESTC -1 run as their own routines. See
 * docs/LANEC-PIN-HEADS.md. */
#include "lanec_pin_heads.h"
#include "lanec_symbols.h"
#include "lanec_ev_internal.h"

typedef char same_words[(LANEC_LM_VBRQEXEC == LANEC_CM_VBRQEXEC && LANEC_LM_VBRQWAIT == LANEC_CM_VBRQWAIT &&
    LANEC_LM_DSPTEM1 == LANEC_CM_DSPTEM1 && LANEC_LM_SETVAC == LANEC_CM_SETVAC && LANEC_LM_REQEX1 == LANEC_CM_REQEX1 &&
    LANEC_LM_MPAC == LANEC_CM_MPAC && LANEC_LM_VBRQEXEC_BANK == 041u && LANEC_LM_SETVAC == LANEC_LM_VBRQEXEC + 022u &&
    LANEC_LM_REQEX1 == LANEC_LM_VBRQEXEC + 5u) ? 1 : -1];

enum lanec_status lanec_pin_head_request_run(apollo_overlay_state *s, lanec_io *io, enum lanec_mission m)
{
    uint16_t *w = s->words, z = w[LANEC_Z];
    LANEC_ONE_MISSION(m);
    if (!lanec_registers_valid(s) || lanec_current_bank(s, io) != 041u) return LANEC_OUT_OF_SCOPE;
    if (z == LANEC_LM_VBRQEXEC) {
        lanec_ev_ca(s, 1u);
        lanec_mask(s, w[LANEC_LM_DSPTEM1]);
        if (lanec_ccs_a(s) == 0) {          /* bit 1: A = 0, TC SETVAC */
            lanec_tc(s, LANEC_LM_SETVAC, (uint16_t)(z + 4u));
        } else {                            /* CAF TCNOVAC, on into REQEX1 */
            lanec_ev_ca(s, m == LANEC_LM ? LANEC_LM_NOVAC : LANEC_CM_NOVAC);
            w[LANEC_Z] = LANEC_LM_REQEX1;
        }
        return LANEC_OK;
    }
    if (z == LANEC_LM_VBRQWAIT) {           /* CAF TCWAIT, TS MPAC, CA DSPTEM1, TC REQUESTC -1 */
        lanec_ev_ca(s, m == LANEC_LM ? LANEC_LM_WAITLIST : LANEC_CM_WAITLIST);
        (void)lanec_ts(s, LANEC_LM_MPAC);
        lanec_ev_ca(s, w[LANEC_LM_DSPTEM1]);
        lanec_tc(s, (uint16_t)(LANEC_LM_REQEX1 + 3u), (uint16_t)(z + 4u));
        return LANEC_OK;
    }
    return LANEC_OUT_OF_SCOPE;
}
