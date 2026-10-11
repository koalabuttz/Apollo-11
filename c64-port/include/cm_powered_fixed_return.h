/* SPDX-License-Identifier: MIT */
#ifndef CM_POWERED_FIXED_RETURN_H
#define CM_POWERED_FIXED_RETURN_H
#include "overlay_runtime.h"
enum cm_powered_fixed_return_status { CM_POWERED_FIXED_RETURN_OK=0, CM_POWERED_FIXED_RETURN_OUT_OF_SCOPE=1 };
/* Source RVQ from the four POWERED interfaces/SMCDURES, fixed FCADR all banks. */
enum cm_powered_fixed_return_status cm_powered_fixed_return(apollo_overlay_state *);
#endif
