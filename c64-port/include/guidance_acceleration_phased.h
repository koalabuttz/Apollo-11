/* SPDX-License-Identifier: MIT */
#ifndef GUIDANCE_ACCELERATION_PHASED_H
#define GUIDANCE_ACCELERATION_PHASED_H
#include "guidance_acceleration.h"
/* Fixed source phases for native overlay size. Producer exclusion and owned
 * physical scratch/VAC span every call; no Executive yield between phases.
 * DANZIG boundaries: LOC03414,03416,03420,03422, all EDOP0 and MODE0.
 * The first phase starts at completed LOC03411. Pointer advances from base
 * to base+2 in gravity and returns to base in sum. Unsupported ownership is
 * unchanged; SQRT alarm in gravity terminates the sequence. No decoder. */
enum p66_acceleration_status p66_acceleration_rate_phase(p66_preparation *,p66_scale_scratch *,p66_elapsed_state *,p66_divide_scratch *,p66_norm_scratch *,p66_pushdown *,const agc_dp *);
enum p66_acceleration_status p66_acceleration_gravity_phase(p66_preparation *,p66_scale_scratch *,p66_elapsed_state *,p66_divide_scratch *,p66_norm_scratch *,p66_pushdown *,const agc_word [6]);
enum p66_acceleration_status p66_acceleration_scale_phase(p66_preparation *,p66_scale_scratch *,p66_elapsed_state *,p66_divide_scratch *,p66_norm_scratch *,p66_pushdown *);
enum p66_acceleration_status p66_acceleration_sum_phase(p66_preparation *,p66_scale_scratch *,p66_elapsed_state *,p66_norm_scratch *,p66_pushdown *);
#endif
