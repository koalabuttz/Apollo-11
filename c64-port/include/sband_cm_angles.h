/* SPDX-License-Identifier: MIT */
#ifndef SBAND_CM_ANGLES_H
#define SBAND_CM_ANGLES_H
#include "sband_plane.h"
enum sband_cm_angles_status {SBAND_CM_ANGLES_UNSUPPORTED,SBAND_CM_ANGLES_OK,SBAND_CM_ANGLES_ALARM1302,SBAND_CM_ANGLES_ALARM1301};
/* Actual CM normal-plane returned NEWOPS through complete yaw computation
 * and source BPL/optional reversed-yaw path, then full pitch calculation. Exact VAC stack, ordinary RAM,
 * coherent original source banks, NEWJOB+0 and producer exclusion required.
 * Stops before original EXIT or before source alarm service. */
enum sband_cm_angles_status sband_cm_angles(apollo_overlay_state *,p66_bias_io *);
enum sband_cm_angles_status sband_cm_angles_null(apollo_overlay_state *,p66_bias_io *);
#endif
