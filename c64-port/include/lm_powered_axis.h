/* SPDX-License-Identifier: MIT */
#ifndef LM_POWERED_AXIS_H
#define LM_POWERED_AXIS_H
#include "overlay_runtime.h"
#include "agc_word.h"
enum lm_powered_axis_status {LM_POWERED_AXIS_OK=0,LM_POWERED_AXIS_OUT_OF_SCOPE=1};
/* Original basic CM AX*SR*T23,3650. A is signextended +3/-3; vector is
 * VBUF, half-amplitude SINCDU/COSCDU are stable explicit source inputs.
 * Stops before TC RTNSAVER atPC0145, then return executes only that source
 * return. No Executive handoff or private continuation state. Caller excludes
 * producers through both calls. All ordinary MODE words retained. */
enum lm_powered_axis_status lm_powered_axis(apollo_overlay_state *);
enum lm_powered_axis_status lm_powered_axis_return(apollo_overlay_state *);
#endif
