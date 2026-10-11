/* SPDX-License-Identifier: MIT */
#ifndef GUIDANCE_FEEDBACK_PAIRS_H
#define GUIDANCE_FEEDBACK_PAIRS_H
#include "guidance_throttle.h"
/* Source hardware/control words after a completed interpreter pair. Inputs
 * are the actual source entry controls; next_pair is explicit native routing,
 * not an interpreter opcode. IRQ is retained by these numerical groups.
 * All twenty source callbacks finish at genuine DANZIG or NOIBNKSW/EDOP0; branch targets
 * that jump directly to NEWOPS are followed until the next real job check.
 * This does not grant a job check inside ABVAL's paired PDDL or basic SQRT.
 * Caller owns the same physical scratch/VAC and excludes all producers. */
typedef struct {agc_word pc,q,fb,eb,bb,loc,bankset,cyr,edop,intbit15,interrupt_enabled,polish;uint8_t next_pair;} p66_feedback_pair_control;
enum p66_feedback_status p66_feedback_pair(p66_preparation *,p66_scale_scratch *,p66_elapsed_state *,p66_divide_scratch *,p66_norm_scratch *,p66_pushdown *,p66_throttle_feedback *,const p66_throttle_inputs *,const agc_word bias[6],p66_feedback_pair_control *,uint8_t pair);
enum p66_feedback_status p66_feedback_timestamp_after_length(p66_preparation *,p66_norm_scratch *,p66_pushdown *,p66_feedback_pair_control *);
#endif
