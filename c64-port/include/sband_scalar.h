/* SPDX-License-Identifier: MIT */
#ifndef SBAND_SCALAR_H
#define SBAND_SCALAR_H
#include "fixed_data.h"
#include "overlay_runtime.h"
#include "agc_word.h"
enum sband_scalar_operation { SBAND_DMP, SBAND_DAD, SBAND_DSU };
/* Original compiled DP interpreter entries, MODE+0, coherent banks,
 * ordinary RAM/Q and raw16 A/L. ADDRWD names a live ordinary erasable pair
 * (no CPU/editing registers), or fixed_dp supplies exactly two immutable,
 * nonaliasing ordinary fixed words without crossing the selected bank.
 * Producer exclusion is required across this call. Stops before mission
 * DANZIG; preserves pending LOC/EDOP/VAC/PUSH and external SUPER/IRQ.
 * DMP keeps the unrounded triple product. No opcode decoding/scheduling. */
uint8_t sband_scalar(apollo_overlay_state *, enum apollo_fixed_mission,
                     enum sband_scalar_operation, const agc_word fixed_dp[2]);
#endif
