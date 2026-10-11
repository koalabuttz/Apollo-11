/* SPDX-License-Identifier: MIT */
#ifndef GUIDANCE_GRAVITY_H
#define GUIDANCE_GRAVITY_H
#include "guidance_push.h"
/* DP PDVL GDT/2: push two quotient words and load six stable gravity
 * words, retaining MPAC+2. Caller owns distinct state/push/vector objects
 * and excludes producers. Stop before the following VSU.
 * Unsupported mode or VAC pointer rejects without mutation. */
enum p66_push_status p66_push_gravity(p66_preparation *state,
    p66_pushdown *push,const agc_word gravity[6]);
#endif
