/* SPDX-License-Identifier: MIT */
#ifndef SBAND_LENGTH_H
#define SBAND_LENGTH_H
#include "sband_orbit.h"
enum sband_length_status {SBAND_LENGTH_UNSUPPORTED,SBAND_LENGTH_OK,SBAND_LENGTH_ABORT_1302};
/* Original ABVAL (vector length) PC03201/bank0, MODE-1 and five valid VACs.
 * Ordinary canonical image/Q, raw16 A/L, coherent banks/ZERO. No private
 * authoritative image and no producer admitted across three owned phases.
 * Stops before mission DANZIG (Q3, or Q02314 after long unnormalization)
 * or actual source SQRTABRT1302 body (Q03210). */
enum sband_length_status sband_length(apollo_overlay_state *,p66_bias_io *,enum apollo_fixed_mission);
#endif
