/* SPDX-License-Identifier: MIT */
/* Pinball NVSUB and NVMONOPT (fixed-fixed), Luminary099 and Comanche055:
 * LXCH 7, TS NVTEMP, CAF BIT14, MASK MONSAVE1, AD DSPLOCK, CCS A, then
 * TC Q (display busy) or NVSBCOM (AD Q, TS NVQTEM, LXCH MONSAVE2) to
 * TC KILMONON, which returns to NVSUBCOM. Each runs to the next call and
 * leaves as the original does there. See docs/LANEC-PIN-HEADS.md. */
#include "lanec_pin_heads.h"
#include "lanec_symbols.h"
#include "lanec_ev_internal.h"

#define S(m, x) ((m) == LANEC_LM ? LANEC_LM_##x : LANEC_CM_##x)
typedef char same_words[(LANEC_LM_NVMONOPT == LANEC_LM_NVSUB + 1u && LANEC_CM_NVMONOPT == LANEC_CM_NVSUB + 1u &&
    LANEC_LM_NVSUBCOM == LANEC_LM_NVSUB + 014u && LANEC_CM_NVSUBCOM == LANEC_CM_NVSUB + 014u &&
    LANEC_LM_NVTEMP == LANEC_CM_NVTEMP && LANEC_LM_MONSAVE1 == LANEC_CM_MONSAVE1 &&
    LANEC_LM_MONSAVE2 == LANEC_CM_MONSAVE2 && LANEC_LM_NVQTEM == LANEC_CM_NVQTEM &&
    LANEC_LM_DSPLOCK == LANEC_CM_DSPLOCK) ? 1 : -1];
#define BIT14 020000u

enum lanec_status lanec_pin_head_nvsub_run(apollo_overlay_state *s, lanec_io *io, enum lanec_mission m)
{
    uint16_t *w = s->words, z = w[LANEC_Z], nvsub = S(m, NVSUB), a = w[LANEC_A], q = w[LANEC_Q], t;
    LANEC_ONE_MISSION(m);
    (void)io;
    if (!lanec_registers_valid(s) || (z != nvsub && z != nvsub + 1u)) return LANEC_OUT_OF_SCOPE;
    /* A not overflowed (TS NVTEMP would skip CAF BIT14) and Q a return
     * address in fixed memory (TC Q, AD Q). */
    if ((a & 0140000u) == 0040000u || (a & 0140000u) == 0100000u || q < 02000u || q > 07777u) return LANEC_OUT_OF_SCOPE;
    if (z == nvsub) w[LANEC_L] = 0;                     /* LXCH 7 */
    (void)lanec_ts(s, LANEC_LM_NVTEMP);
    lanec_ev_ca(s, BIT14);
    lanec_mask(s, w[LANEC_LM_MONSAVE1]);
    lanec_ad(s, w[LANEC_LM_DSPLOCK]);
    switch (lanec_ccs_a(s)) {
    case 0:                                             /* TC Q */
        lanec_tc(s, q, 3u);
        return LANEC_OK;
    case 1: lanec_ev_ca(s, 1u); /* fall through */      /* CAF ONE, NVSBCOM */
    case 2: lanec_ad(s, q); break;                      /* NVSBCOM: AD Q */
    default: break;                                     /* -0: on at TS NVQTEM */
    }
    if (!lanec_ts(s, LANEC_LM_NVQTEM)) {                /* LXCH MONSAVE2 */
        t = w[LANEC_LM_MONSAVE2];
        w[LANEC_LM_MONSAVE2] = lanec_ev_corrected(w[LANEC_L]);
        w[LANEC_L] = lanec_a16(t);
    }
    lanec_tc(s, S(m, KILMONON), S(m, NVSUBCOM));
    return LANEC_OK;
}
