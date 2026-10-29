/* SPDX-License-Identifier: MIT */
#ifndef GUIDANCE_HORIZONTAL_H
#define GUIDANCE_HORIZONTAL_H
#include "guidance_unit.h"
#include "guidance_vector.h"
#include "guidance_product.h"
#include "guidance_shift.h"
#include "guidance_sum.h"
enum p66_horizontal_status {P66_HORIZONTAL_OK=0,P66_HORIZONTAL_ALARM_1302=1,P66_HORIZONTAL_OUT_OF_SCOPE=2,P66_HORIZONTAL_UNIT_OVERFLOW=3};
/* Source post-THROTTLE horizontal speed computation. Entry DP MODE+0,
 * legal owned VAC and PUSHLOCbase; updated velocity VAC24..29. Stable
 * R/DELVS/HDOTDISP are original producer-owned erasable inputs. Each explicit
 * phase ends at an original completed-pair DANZIG scheduler check. Producer
 * exclusion, distinct ordinary-word objects and exclusive ownership required.
 * Source UNIT overflow continues the following VXSC; negative SQRT stops at
 * SQRTABRT with an observation. No executive/alarm/interpreter dispatch. */
enum p66_horizontal_status p66_horizontal_load(p66_preparation *,p66_norm_scratch *,p66_pushdown *); /* LOC03516 */
enum p66_horizontal_status p66_horizontal_surface(p66_preparation *,p66_scale_scratch *,p66_elapsed_state *,p66_norm_scratch *,p66_pushdown *,const agc_word delvs[6]); /*03520*/
enum p66_horizontal_status p66_horizontal_position(p66_preparation *,p66_scale_scratch *,p66_elapsed_state *,p66_norm_scratch *,p66_pushdown *,const agc_word position[6]); /*03522*/
enum p66_horizontal_status p66_horizontal_radial(p66_preparation *,p66_scale_scratch *,p66_elapsed_state *,p66_divide_scratch *,p66_norm_scratch *,p66_pushdown *,const agc_dp *hdotdisp); /*03524*/
enum p66_horizontal_status p66_horizontal_tangential(p66_preparation *,p66_scale_scratch *,p66_elapsed_state *,p66_norm_scratch *,p66_pushdown *); /*03525*/
enum p66_horizontal_status p66_horizontal_length(p66_preparation *,p66_scale_scratch *,p66_elapsed_state *,p66_divide_scratch *,p66_norm_scratch *,p66_pushdown *); /*03526*/
enum p66_horizontal_status p66_horizontal_store(p66_preparation *,p66_norm_scratch *,p66_pushdown *,agc_dp *vhoriz); /*03527*/
/* Numeric EXIT effects at original basic continuation03531 before BANKCALL
 * DISPEXIT+3. Final source BANKSET064007 in A; MODE/MPAC/L retained. */
enum p66_horizontal_status p66_horizontal_exit(p66_preparation *,p66_pushdown *);
#endif
