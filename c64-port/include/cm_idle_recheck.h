/* SPDX-License-Identifier: MIT */
#ifndef CM_IDLE_RECHECK_H
#define CM_IDLE_RECHECK_H
#include "cm_idle_dispatch.h"
/* Actual NUCHANG2 01,3032 after ADVAN observed a pending positive NEWJOB.
 * Source rechecks after INHINT; a legitimate source wake can have changed
 * NEWJOB to+0, in which case RELINT→NUDIRECT executes. Positive typed NEWJOB
 * exchanges its true basic/interpreted context. One canonical image; caller
 * excludes producers across this compiled group. No chosen job body executes.
 * Typed NEWJOB0/12..72 and authentic selected active job required; -0 or
 * reserved descriptor controls are outside this producer entry contract. */
enum exec_status cm_idle_recheck(apollo_overlay_state *,cm_native_io *,exec_transfer *);
#endif
