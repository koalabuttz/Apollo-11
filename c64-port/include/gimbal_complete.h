/* SPDX-License-Identifier: MIT */
#ifndef GIMBAL_COMPLETE_H
#define GIMBAL_COMPLETE_H
#include "gimbal_stage.h"
/* Whole original gimbal file, NOGIMLOC/WCALC through SETGO NEWANGL+1.
 * Same checked entry/producer ownership as gimbal_entry; NEWANGL external. */
uint16_t gimbal_complete(apollo_overlay_state *,gimbal_io *,enum apollo_fixed_mission);
#endif
