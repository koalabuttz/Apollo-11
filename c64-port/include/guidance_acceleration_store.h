/* SPDX-License-Identifier: MIT */
#ifndef GUIDANCE_ACCELERATION_STORE_H
#define GUIDANCE_ACCELERATION_STORE_H
#include "guidance_acceleration.h"
/* Source STORE20 and the following implicit-pushdown DAD are separate EDOP0
 * DANZIG continuations, LOC03421 and03422. Caller owns scratch and VAC at
 * each call. Unsupported MODE/VAC/push-pointer inputs remain unchanged. */
enum p66_acceleration_status p66_acceleration_store_phase(p66_preparation *,p66_norm_scratch *,p66_pushdown *);
enum p66_acceleration_status p66_acceleration_add_phase(p66_preparation *,p66_scale_scratch *,p66_elapsed_state *,p66_norm_scratch *,p66_pushdown *);
#endif
