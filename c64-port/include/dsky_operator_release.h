/* SPDX-License-Identifier: MIT */
#ifndef DSKY_OPERATOR_RELEASE_H
#define DSKY_OPERATOR_RELEASE_H
#include "dsky_lamp_release.h"
enum dsky_operator_release_route {DSKY_OPERATOR_RELEASE_END,DSKY_OPERATOR_RELEASE_PIN,DSKY_OPERATOR_RELEASE_BLANK};
/* Actual VBRELDSP LM40,3461 / CM40,3366 and UNSUSPEN LM40,3477 /
 * CM40,3404. Full ordinary saved-lock/CADR/list classes; actual selected
 * RELDSP/JOBWAKE or RELDSP1, then END/PINBRNCH/BLNKSUB1 entries.
 * Any EB/selectingSUPER/IRQ/raw registers. Selected wake preconditions
 * apply only to the normal release route, not monitor unsuspension.
 * One canonical image, restoring producer exclusion; rejected no-op. */
enum p66_bias_status dsky_operator_release(apollo_overlay_state *,dsky_lamp_io *,enum dsky_operator_release_route *);
enum exec_status cm_operator_release(apollo_overlay_state *,cm_native_io *,uint16_t *wrote_channels,uint16_t *restart_light,enum dsky_operator_release_route *);
#endif
