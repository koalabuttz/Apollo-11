/* SPDX-License-Identifier: MIT */
#ifndef LM_VN1645_H
#define LM_VN1645_H
#include "lm_midgim.h"
/* Luminary099 P34-35_P74-75 VN1645's computation head (bank 35
 * 03606-03634) with P3XORP7X (35,3704-3711): +MGA set to -.01 or -.02 and,
 * for a P3X program with REFSMMAT good, the middle gimbal angle of DELVSIN
 * through GET+MGA (the lm-midgim step). From the NEWOPS frontier at VN1645
 * (after its callers' GOTO) to the basic frontier at GET45 +1 (35,3635,
 * TC COMPTGO), which this milestone does not port. 0 unsupported (image
 * unchanged), 1 at GET45 +1, 2 the UNIT abort frontier (alarm 1302), 3 the
 * ARCSIN argument-too-large frontier (alarm 1301). */
uint8_t lm_vn1645(apollo_overlay_state *,p66_bias_io *);

/* Steps between original NEWOPS frontiers; GET+MGA keeps its lm-midgim
 * number (LMG_GETMGA) and end states (LMG_RETURNED, LMG_ABORTED,
 * LMG_ARCSIN1301, LMG_FAILED). */
enum lm_vn1645_phase {
 LVN_VN1645=LMG_FAILED+1, /* VN1645 35,3606 through BOFF EXIT, P3XORP7X and TC INTPRET */
 LVN_MGA,                 /* 35,3630 VLOAD PUSH DELVSIN; CALL GET+MGA */
 LVN_GET45                /* GET45 35,3634 EXIT */
};
#define LM_VN1645_DONE(phase) ((phase)>=LMG_RETURNED&&(phase)<=LMG_FAILED)
uint8_t lm_vn1645_entry(const apollo_overlay_state *,const p66_bias_io *);
uint8_t lm_vn1645_step(apollo_overlay_state *,p66_bias_io *,uint8_t phase);
/* GET+MGA's return to GET45 continues VN1645. */
uint8_t lvn_remap(const apollo_overlay_state *,uint8_t next);
uint8_t lvn_vn1645(apollo_overlay_state *,p66_bias_io *);
uint8_t lvn_mga(apollo_overlay_state *,p66_bias_io *);
uint8_t lvn_get45(apollo_overlay_state *,p66_bias_io *);
#endif
