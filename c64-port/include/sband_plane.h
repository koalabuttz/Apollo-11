/* SPDX-License-Identifier: MIT */
#ifndef SBAND_PLANE_H
#define SBAND_PLANE_H
#include "sband_navframe.h"
enum sband_plane_status {SBAND_PLANE_UNSUPPORTED,SBAND_PLANE_OK,SBAND_PLANE_ABORT_1302,SBAND_PLANE_NULL_EXIT};
/* Actual pending PDVL, antenna-plane projection/reverse subtraction, UNIT
 * and BOV source controls. Null/overflow exits stop at true source NEWOPS
 * before the selected next operator; no display/alarm handler is executed.
 * Five VAC/exact stack, ordinary canonical data/coherent banks, NEWJOB+0,
 * returned UNIT tuple and producer exclusion across all pairs are required. */
enum sband_plane_status sband_plane(apollo_overlay_state *,p66_bias_io *,enum apollo_fixed_mission);
#endif
