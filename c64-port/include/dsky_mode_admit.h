/* SPDX-License-Identifier: MIT */
#ifndef DSKY_MODE_ADMIT_H
#define DSKY_MODE_ADMIT_H
#include "lm_admission_image.h"
#include "cm_admission_complete.h"
enum dsky_mode_admit_route {DSKY_MODE_ADMIT_CALLER,DSKY_MODE_ADMIT_ALARM};
/* Actual DSPMM LM04,2636 / CM04,2566 through real NOVAC request for
 * DSPMMJB{03534/03441,060101}, RELINT and TC MPAC ordinary return, or
 * genuine BAILOUT entry on resource exhaustion. Active positive core0,
 * canonical remaining nonzero core priorities/VAC markers/NEWJOB, aligned
 * banks, all retained SUPER0..160, ordinary Q0100..7777. No draw placeholder
 * or deferred consumer claim. Unsupported input/image/IO/route unchanged. */
enum p66_bias_status dsky_mode_admit(apollo_overlay_state *,p66_bias_io *,enum dsky_mode_admit_route *);
enum exec_status cm_mode_admit(apollo_overlay_state *,cm_native_io *,enum dsky_mode_admit_route *);
#endif
