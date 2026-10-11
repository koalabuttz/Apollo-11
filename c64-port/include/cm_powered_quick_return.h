/* SPDX-License-Identifier: MIT */
#ifndef CM_POWERED_QUICK_RETURN_H
#define CM_POWERED_QUICK_RETURN_H
#include "overlay_runtime.h"
enum cm_powered_quick_return_status {CM_POWERED_QUICK_RETURN_OK=0,CM_POWERED_QUICK_RETURN_OUT_OF_SCOPE=1};
/* Source QUICTRIG final TC A23,3605; source A contains fixed-code savedQ.
 * Completes at PC=oldA/Q1 before executing caller code; banks/MODE/IRQ and
 * every other source word retained. Caller excludes producers and retains
 * ownership through the basic call; no source Executive check occurs. */
enum cm_powered_quick_return_status cm_powered_quick_return(apollo_overlay_state *);
#endif
