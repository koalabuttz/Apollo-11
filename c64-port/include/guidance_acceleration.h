/* SPDX-License-Identifier: MIT */
#ifndef GUIDANCE_ACCELERATION_H
#define GUIDANCE_ACCELERATION_H
#include "guidance_unit.h"
enum p66_acceleration_status { P66_ACCELERATION_OK=0, P66_ACCELERATION_ALARM_1302=1, P66_ACCELERATION_OUT_OF_SCOPE=2 };
/* P66 BDSU VDGVERT through DDV TAUROD, PDVL GDT/2, ABVAL, DDV GSCALE,
 * SR2, STORE20D and implicit-pop DAD. Entry DP MODE0/MPAC HDOTDISP,
 * legal owned VAC with PUSHLOC=FIXLOC. Uses rate.desired as VDGVERT;
 * TAUROD and GDT/2 are stable ordinary-word inputs. GSCALE is the exact
 * fixed pair {01440,0}. Exit DP desired plus gravity acceleration, VAC0/1
 * desired acceleration and20/21 gravity acceleration retained, pointerbase.
 * Preserve complete source scratch, registers, signed zeros and overflow
 * continuation. SQRT failure stops before alarm dispatch with pointerbase+2.
 * Invalid MODE/base/pointer rejects all objects unchanged. Distinct objects,
 * ordinary words and producer exclusion are caller obligations. Stop before
 * PDVL UNITX/CALL CDU*NBSM; no interpreter or Executive dispatch. */
enum p66_acceleration_status p66_acceleration(p66_preparation *,p66_scale_scratch *,
    p66_elapsed_state *,p66_divide_scratch *,p66_norm_scratch *,p66_pushdown *,
    const agc_dp *taurod,const agc_word gravity[6]);
#endif
