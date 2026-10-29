/* SPDX-License-Identifier: MIT */
#ifndef GUIDANCE_THROTTLE_H
#define GUIDANCE_THROTTLE_H
#include "guidance_unit.h"
typedef struct {agc_dp mass,lag_tau,maxforce,minforce;} p66_throttle_inputs;
typedef struct {agc_dp afc,fweight,lasttpip;} p66_throttle_feedback;
enum p66_feedback_status {P66_FEEDBACK_NORMAL=0,P66_FEEDBACK_MINIMUM=1,P66_FEEDBACK_MAXIMUM=2,P66_FEEDBACK_ALARM_1302=3,P66_FEEDBACK_OUT_OF_SCOPE=4};
/* Source P66 VXSC KPIP1 at32,3434 through STODL /AFC/ at32,3507.
 * Entry vector DELVROD/MODE-1, legal owned VAC with pointerbase, gravity20/21
 * and cosine22/23. Stable VBIAS/MASS/LAG-TAU/MAXFORCE/MINFORCE are ordinary
 * historical words. Preserve source branch/pop force clamps, DP signed zeros,
 * complete numerical scratch and measured acceleration MPAC output. LASTTPIP
 * is updated from preparation.this_piptime; FWEIGHT retained. Exit pointerbase,
 * MODE0, ADDRWDbase before ITRPNT2 EXIT. Unsupported ownership rejects unchanged.
 * Source division overflow continues; SQRT alarm stops before recovery.
 * Caller excludes every producer and owns distinct ordinary-word objects. */
enum p66_feedback_status p66_throttle_feedback_compute(p66_preparation *,p66_scale_scratch *,
    p66_elapsed_state *,p66_divide_scratch *,p66_norm_scratch *,p66_pushdown *,
    p66_throttle_feedback *,const p66_throttle_inputs *,const agc_word bias[6]);
#endif
