/* SPDX-License-Identifier: MIT */
#ifndef GUIDANCE_RADIAL_H
#define GUIDANCE_RADIAL_H
#include "guidance_unit.h"
typedef struct {agc_dp hdotdisp,hcalc1;} p66_radial_state;
enum p66_radial_status {P66_RADIAL_OK=0,P66_RADIAL_ALARM_1302=1,P66_RADIAL_OUT_OF_SCOPE=2,P66_RADIAL_UNIT_OVERFLOW=3};
/* P66 UNIT R through STODL HCALC1,HDOTDISP, before BDSU VDGVERT.
 * Entry vector MPAC R, owned legal VAC with pointer at its base, velocity
 * at24..29 and elapsed time30/31. LAND is stable raw erasable radius input.
 * UNIT retains HALF unit R14..19, square34/35 and length36/37. HDOTDISP
 * is radial velocity B7; HCALC1 is predicted altitude B24. Original A/L,
 * numerical scratch and signed zeros are retained. Unsupported ownership
 * shape/MODE rejects unchanged; SQRT abort stops at its boundary. UNIT overflow follows the original
 * continuation and returns an observation after the final store.
 * Distinct ordinary-word objects, excluded producers and exclusive ownership
 * are caller obligations. No interpreter, alarm or executive dispatch. */
enum p66_radial_status p66_radial_position(p66_preparation *,p66_scale_scratch *,
    p66_elapsed_state *,p66_divide_scratch *,p66_norm_scratch *,p66_pushdown *,
    p66_radial_state *,const agc_dp *landing_radius);
#endif
