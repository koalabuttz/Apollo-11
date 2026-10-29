/* SPDX-License-Identifier: MIT */
#ifndef DSKY_OPERATOR_REQUEST_H
#define DSKY_OPERATOR_REQUEST_H
#include "dsky_request_control.h"
#include "dsky_display_release.h"
/* Uninterrupted actual VBRQEXEC3466 / VBRQWAIT3512 through real RELDSP,
 * selected Executive wake and source request continuation to literal
 * NOVAC/FINDVAC/WAITLIST entry. Service/recovery bodies remain continuations.
 * Selecting wake requires released JOBWAKE domain; unused wake fields are
 * opaque. One canonical image, restoring producer exclusion, no extra poll. */
enum p66_bias_status dsky_operator_request(apollo_overlay_state *,dsky_charin_io *,enum dsky_request_control_route *);
enum exec_status cm_operator_request(apollo_overlay_state *,cm_native_io *,uint16_t *wrote_channel11,enum dsky_request_control_route *);
#endif
