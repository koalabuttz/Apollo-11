/* SPDX-License-Identifier: MIT */
#ifndef CM_INIT_DISPLAY_H
#define CM_INIT_DISPLAY_H
#include "cm_basic_priority.h"
/* Actual CM INITDSP10,3261: saved EB/SUPER restoration and PRIOCHNG.
 * Returns selected source pair. A preempting job suspends INITDSP+6 in the
 * original saved core; its later actual3267 mapping calls _return below.
 * If this job remains selected, executes that continuation immediately.
 * Valid saved priority01000..037000 and ordinary target TEMPFLSH>=4.
 * The caller owns exclusion; no native callbacks or rope are fetched. */
enum exec_status cm_init_display(apollo_overlay_state *,cm_native_io *,exec_address *);
/* Source INITDSP+6 through BANKJUMP4704 to requested caller continuation.
 * The target CADR is original display state, not a native instruction word. */
enum exec_status cm_init_display_return(apollo_overlay_state *,cm_native_io *,exec_address *);
#endif
