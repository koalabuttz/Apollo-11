/* SPDX-License-Identifier: MIT */
#ifndef GUIDANCE_PROJECTION_PHASED_H
#define GUIDANCE_PROJECTION_PHASED_H
#include "guidance_projection.h"
/* Internal native overlay returns. Ownership/exclusion must span all three
 * calls, with no Executive handoff at either basic subroutine return. State
 * is entirely the original owned scratch/VAC; no persistent private context.
 * First call includes PDVL UNITX/CALL; second MPACVBUF and AX*SR*T; third
 * VLOAD VBUF/RVQ/DOT14 and reaches the genuine P66 LOC03427 check. */
enum p66_projection_status p66_projection_trig_phase(p66_preparation *,p66_scale_scratch *,p66_divide_scratch *,p66_norm_scratch *,p66_pushdown *,p66_trig_scratch *,const agc_word cdu_xyz[3]); /* BasicPC03663 */
enum p66_projection_status p66_projection_rotation_phase(p66_preparation *,p66_scale_scratch *,p66_elapsed_state *,p66_divide_scratch *,p66_norm_scratch *,p66_pushdown *,p66_trig_scratch *,p66_rotation_scratch *); /* BasicPC03656 */
enum p66_projection_status p66_projection_dot_phase(p66_preparation *,p66_scale_scratch *,p66_elapsed_state *,p66_divide_scratch *,p66_norm_scratch *,p66_pushdown *,p66_trig_scratch *); /* DANZIG LOC03427 */
#endif
