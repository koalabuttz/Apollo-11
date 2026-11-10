/* SPDX-License-Identifier: MIT */
/* KALMAN_FILTER.agc, the LM's (Luminary099 pages 1470-1471, bank 16):
 * RATELOOP, which P-AXIS's T5RUPT reaches by TCF before the rate filter
 * (BACKP). For each jet axis, V, U then P, it takes the jet time this pass
 * fires (all of TJV, TJU or TJP if under 100 ms, else 100 ms of it, the
 * rest left for the next pass), the torque that gives with the axis's jet
 * count, kept in DAPTEMP1, 2 or 3 for the filter and summed into the downlist's
 * DOWNTORK counters; then the jet rates about R and Q (ROTORQUE) from the
 * U and V torques, and TCF BACKP. The faithful C is
 * c64-port/src/lm_kalman.c. */
#ifndef READABLE_KALMAN_FILTER_H
#define READABLE_KALMAN_FILTER_H

#include "../agc/names.h"

#if LUMINARY099
void rateloop(void); /* RATELOOP: from P-AXIS */
#endif

#endif
