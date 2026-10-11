/* SPDX-License-Identifier: MIT */
#ifndef MATRIX_NATIVE_H
#define MATRIX_NATIVE_H
#include "overlay_runtime.h"
#include "agc_word.h"
#include "fixed_data.h"
/* Source MXV (column0) and VXM (column1), ordinary DP3x3 matrix row order.
 * Explicit compiled phases0=prologue,1=first dot/exchanges,2=second,
 * 3=third/final exchanges through the entry of DANZIG. No dispatch/poll.
 * Matrix is an ordinary18-word source operand, stable during each phase,
 * outside modified scratch; caller must pass the actual selected source
 * matrix and own producer exclusion. MODE is vector-1; legal aligned banks,
 * ordinary caller RAM, logical matrix base01000..07757. Raw A/L/Q accepted.
 * All canonical CPU/scratch effects retained. Unsupported inputs preserve
 * image and operand. External IRQ/SUPER/device state is unchanged.
 * LM and CM use their original source return addresses; no source decoding. */
uint8_t matrix_native_phase(apollo_overlay_state *,enum apollo_fixed_mission,
                           uint8_t column,const agc_word matrix[18],uint8_t phase);
#endif
