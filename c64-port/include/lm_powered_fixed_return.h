/* SPDX-License-Identifier: MIT */
#ifndef LM_POWERED_FIXED_RETURN_H
#define LM_POWERED_FIXED_RETURN_H
#include "overlay_runtime.h"
enum lm_powered_fixed_return_status { LM_POWERED_FIXED_RETURN_OK=0, LM_POWERED_FIXED_RETURN_OUT_OF_SCOPE=1 };
/* Source RVQ from the four POWERED interfaces/CALCSMSC, fixed FCADR all banks. */
enum lm_powered_fixed_return_status lm_powered_fixed_return(apollo_overlay_state *);
#endif
