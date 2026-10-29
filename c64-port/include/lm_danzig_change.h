/* SPDX-License-Identifier: MIT */
#ifndef LM_DANZIG_CHANGE_H
#define LM_DANZIG_CHANGE_H
#include "lm_interpreter_change.h"
/* Source completed-pair scheduler check at DANZIG06061 or NOIBNKSW06063,
 * positive NEWJOB, through frozen canonical CHANG2. No next order fetch.
 * All source/control guards precede mutation; producer exclusion is owned
 * by the compiled caller. Other EDOP/NEWJOB paths are explicitly excluded. */
enum exec_status lm_danzig_change(apollo_overlay_state *,p66_bias_io *,exec_transfer *);
#endif
