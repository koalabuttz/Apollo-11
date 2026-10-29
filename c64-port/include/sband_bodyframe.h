/* SPDX-License-Identifier: MIT */
#ifndef SBAND_BODYFRAME_H
#define SBAND_BODYFRAME_H
#include "sband_orbit.h"
/* Actual post-frame-shift antenna setup and complete *SMNB* transform.
 * Canonical valid source VAC/PUSH and stable half-amplitude CDU trig inputs;
 * producer exclusion across every internal basic/interpreted return. */
bool sband_bodyframe(apollo_overlay_state *,p66_bias_io *,enum apollo_fixed_mission);
#endif
