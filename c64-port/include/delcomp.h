/* SPDX-License-Identifier: MIT */
#ifndef DELCOMP_H
#define DELCOMP_H
#include "gimbal_stage.h"
/* Native original DELCOMP matrix construction, using one canonical image.
 * Caller excludes producers; ordinary words15-bit, A/L/Q raw16-bit; source
 * NEWOPS/LOC=DELCOMP, DP MODE0 and valid VAC/FIXLOC. QPRET is a positive
 * original fixed continuation in the high-bank half; no continuation body
 * runs here. Unsupported contexts return zero with all objects unchanged. */
uint16_t delcomp_native(apollo_overlay_state *,gimbal_io *,enum apollo_fixed_mission);
#endif
