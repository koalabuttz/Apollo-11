/* SPDX-License-Identifier: MIT */
#ifndef CM_WAITLIST_INTERNAL_H
#define CM_WAITLIST_INTERNAL_H
#include "cm_native_image.h"
enum cm_waitlist_internal_entry {CM_WL_DLY2_MINUS1,CM_WL_VARDELAY,CM_WL_FIXDELAY};
/* Actual internal DLY2-1 05255, VARDELAY05272 or FIXDELAY05267 through
 * original WAIT2 insertion and real caller/TASKOVER or BAILOUT handoff.
 * Source positive dt1..037777; FIXDELAY literal is explicit compiled original
 * caller data. VAR/FIX run under WAITLIST producer exclusion/IRQ0 with actual
 * caller Q0100..07776. DLY2-1 carries delay in rawQ, task pair inA/L, already
 * source-selected WAITEXIT word. No delayed task or recovery body executes. */
enum cm_wl_status cm_waitlist_internal(apollo_overlay_state *,cm_native_io *,enum cm_waitlist_internal_entry,agc_word);
#endif
