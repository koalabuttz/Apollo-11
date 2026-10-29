/* SPDX-License-Identifier: MIT */
#ifndef SBAND_LM_YAW_H
#define SBAND_LM_YAW_H
#include "sband_plane.h"
enum sband_lm_yaw_status {SBAND_LM_YAW_UNSUPPORTED,SBAND_LM_YAW_OK,SBAND_LM_YAW_ALARM1302,SBAND_LM_YAW_ALARM1301};
/* Actual original LM NOADJUST through complete yaw; fiveVAC/exact stack,
 * ordinary RAM/coherent original banks, NEWJOB+0 and producer exclusion.
 * Ends before original EXIT or true alarm services. */
enum sband_lm_yaw_status sband_lm_yaw(apollo_overlay_state *,p66_bias_io *);
#endif
