/* SPDX-License-Identifier: MIT */
#ifndef SBAND_EXIT_H
#define SBAND_EXIT_H
#include "sband_control.h"
/* Original antenna NEWOPS/EXIT to first basic display instruction.
 * Producer excluded; ordinary image/Q, raw A/L and coherent source banks.
 * No display or scheduler body executes. */
int sband_exit(apollo_overlay_state *,p66_bias_io *,enum apollo_fixed_mission);
#endif
