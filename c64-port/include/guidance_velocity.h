/* SPDX-License-Identifier: MIT */
#ifndef GUIDANCE_VELOCITY_H
#define GUIDANCE_VELOCITY_H
#include "guidance_gravity.h"
#include "guidance_vector.h"
#include "guidance_product.h"
#include "guidance_shift.h"
enum p66_velocity_status { P66_VELOCITY_OK=0, P66_VELOCITY_OUT_OF_SCOPE=1 };
/* P66 group 2 after PDVL GDT/2: subtract stable VBIAS, pop quotient and
 * multiply, VSL2, add stable V, pop/add saved six-word scaled delta vector,
 * STADR/STOVL 24D and load stable R. Requires vector MODE=-1 and PUSHLOC
 * exactly FIXLOC+8 in a legal historical VAC. Writes VAC24..29 and reduces
 * PUSHLOC to FIXLOC; VAC30/31 and all popped payload words are retained.
 * Retains original arithmetic scratch and final VLOAD A/L exchange debris;
 * stop at DANZIG before UNIT. All objects and three six-word input vectors
 * must be distinct, exclusively owned, ordinary words, producers excluded.
 * Unsupported MODE/base/pointer rejects every object without mutation. */
enum p66_velocity_status p66_update_velocity(p66_preparation *state,
    p66_scale_scratch *scale,p66_elapsed_state *elapsed,p66_pushdown *push,
    const agc_word bias[6],const agc_word velocity[6],const agc_word position[6]);
#endif
