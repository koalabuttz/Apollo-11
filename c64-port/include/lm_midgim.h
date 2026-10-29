/* SPDX-License-Identifier: MIT */
#ifndef LM_MIDGIM_H
#define LM_MIDGIM_H
#include "lm_interp_orders.h"
/* Luminary099 P34-35_P74-75 midcourse display subroutines: GET+MGA and
 * GET.LVC with HALFREV (bank 6 03715-03752) and SELECTMU (bank 10
 * 02000-02027). From the original interpretive entry frontier (NEWOPS at the
 * routine after one of its Luminary callers' CALL, STCALL or switch branch)
 * to the caller's NEWOPS frontier at C(QPRET) for GET+MGA and GET.LVC, or
 * SELECTMU's GOTO VN1645 frontier (NEWOPS at 35,3606). 0 unsupported (image
 * unchanged), 1 returned, 2 the UNIT abort frontier (alarm 1302), 3 the
 * ARCSIN argument-too-large frontier (alarm 1301, 03723 in bank 0). */
uint8_t lm_midgim(apollo_overlay_state *,p66_bias_io *);

/* One step per routine (no inner frontiers), then an end state. */
enum lm_midgim_phase {
 LMG_GETMGA=1,  /* GET+MGA 06,3717 through either RVQ */
 LMG_GETLVC,    /* GET.LVC 06,3733 through SET RVQ */
 LMG_SELECTMU,  /* SELECTMU 10,2000 through GOTO VN1645 */
 LMG_RETURNED,
 LMG_ABORTED,
 LMG_ARCSIN1301,
 LMG_FAILED
};
#define LM_MIDGIM_DONE(phase) ((phase)>=LMG_RETURNED)
uint8_t lm_midgim_entry(const apollo_overlay_state *,const p66_bias_io *);
uint8_t lm_midgim_step(apollo_overlay_state *,p66_bias_io *,uint8_t phase);
uint8_t lmg_getmga(apollo_overlay_state *,p66_bias_io *);
uint8_t lmg_getlvc(apollo_overlay_state *,p66_bias_io *);
uint8_t lmg_selectmu(apollo_overlay_state *,p66_bias_io *);
#endif
