/* SPDX-License-Identifier: MIT */
#ifndef CM_LONGCALL_H
#define CM_LONGCALL_H
#include "overlay_runtime.h"
#include "executive.h"
enum cm_longcall_phase { CM_LONGCALL_ENTER=0,CM_LONGCALL_CYCLE=1,CM_LONGCALL_RETURN=2,CM_LONGCALL_GETCADR=3 };
enum cm_longcall_status { CM_LONGCALL_WAIT=0,CM_LONGCALL_TRANSFER=1,CM_LONGCALL_INVALID=2 };
/* Explicit original CM LONGCALL/LONGCYCL/LONGRTRN/GETCADR source groups.
 * ENTER receives its already resolved source task 2CADR as metadata. WAIT stops
 * at original WAITLIST entry; image Q selects LONGCYCL (03512) or GETCADR
 * (03503), and image A is the source delay. TRANSFER returns the actual target
 * in image Z/BB, after source DTCB exchange. Callers execute the native selected
 * task/service and own exclusion; no C frame crosses a dispatcher boundary.
 * ENTER accepts a nonnegative canonical DP duration after register correction.
 * CYCLE also accepts mixed-sign low words in a positive remaining duration.
 * RETURN/GETCADR require ordinary valid 12-bit location and BB encoding.
 * External SUPERBNK/IRQ latches are unchanged; the sole image owns all words. */
enum cm_longcall_status cm_longcall_phase_run(apollo_overlay_state *,enum cm_longcall_phase,exec_address);
#endif
