/* SPDX-License-Identifier: MIT */
#ifndef CM_QUALITY_H
#define CM_QUALITY_H
#include "rtb_angles.h"
/* Original QUALITY1/QUALITY2, twenty explicit compiled source boundaries.
 * phase0 begins at QUALITY1 NEWOPS; later calls resume the previous returned
 * canonical boundary. Pending EDOP is retained, never an invented job check.
 * Five legal VACs, ordinary source words, raw A/L, coherent banks and excluded
 * producers are required; the actual caller has saved QUALITY3 in QPRET.
 * Returns0 after a local phase,1 at true NBRANCH/QUALITY3 NEWOPS,2 unchanged
 * for unsupported input. No orbital integration body or interpreter decoder. */
uint8_t cm_quality_step(apollo_overlay_state *,p66_bias_io *,uint8_t phase);
#endif
