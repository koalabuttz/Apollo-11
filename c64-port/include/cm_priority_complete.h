/* SPDX-License-Identifier: MIT */
#ifndef CM_PRIORITY_COMPLETE_H
#define CM_PRIORITY_COMPLETE_H
#include "cm_native_image.h"
#include "cm_dispatch.h"
/* Actual PRIOCHNG05210 through current/selected basic DTCB or interpreted
 * INTRSM NEWOPS. Positive upper priority in A; caller Q compiled ordinary.
 * Source CCSHOLE +0 candidates remain outside scope. One canonical image,
 * caller-owned exclusion, no selected application body/callback execution. */
enum exec_status cm_priority_complete(apollo_overlay_state *,cm_native_io *,exec_transfer *);
#endif
