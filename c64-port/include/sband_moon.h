/* SPDX-License-Identifier: MIT */
#ifndef SBAND_MOON_H
#define SBAND_MOON_H
#include "sband_orbit.h"
/* Actual original LUNPOS-return NEWOPS tuple, MODE-1, original bank42
 * selected by SUPER100 forms, five valid VACs, NEWJOB+0, ordinary RAM/Q
 * and raw16 A/L. Retains producer exclusion until true LM UNIT or CM MXV
 * entry, before their bodies. Unsupported image/latches remain unchanged.
 * This does not execute LUNPOS or claim its output numerical calculation. */
bool sband_moon(apollo_overlay_state *,p66_bias_io *,enum apollo_fixed_mission);
#endif
