/* SPDX-License-Identifier: MIT */
#ifndef LM_RADAR_LEADIN_H
#define LM_RADAR_LEADIN_H
#include "lm_admission_image.h"
#include "lm_waitlist_image.h"
#include "agc_multiword.h"
#define LM_RADAR_STALL_CADR UINT16_C(017714)
enum lm_radar_result {LM_RADAR_TASKOVER,LM_RADAR_MEASURE,LM_RADAR_STALL,
 LM_RADAR_END,LM_RADAR_ALARM,LM_RADAR_WAITLIST,LM_RADAR_REENTER,LM_RADAR_REJECT};
/* Original bank25 local entries. Native callers retain exclusion and route
 * exact source frontiers. Radar measurement/stall bodies remain external.
 * Cyclic/source table indices use their explicit operational domains; all
 * raw RSAMPDT signs and zero encodings retain their source branches. */
enum lm_radar_result lm_radar_leadin(apollo_overlay_state *,p66_bias_io *);
#endif
