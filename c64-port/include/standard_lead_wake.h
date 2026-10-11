/* SPDX-License-Identifier: MIT */
#ifndef STANDARD_LEAD_WAKE_H
#define STANDARD_LEAD_WAKE_H
#include "standard_lead.h"
#include "lm_basic_wake.h"
#include "cm_basic_wake.h"
/* Actual E/JOBWAK entry through native JOBWAKE, original LOC bit clearing,
 * and saved RUPTREG3 TC caller return. Source interrupt exclusion required;
 * ordinary wake address and the underlying native JOBWAKE core domain apply.
 * Includes source no-match side effect at LOC-1. Unsupported image unchanged. */
enum exec_status standard_lead_wake_lm(apollo_overlay_state *,p66_bias_io *);
enum exec_status standard_lead_wake_cm(apollo_overlay_state *,cm_native_io *);
#endif
