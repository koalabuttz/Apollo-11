/* SPDX-License-Identifier: MIT */
#ifndef CM_CLOCKJOB_CASES_H
#define CM_CLOCKJOB_CASES_H
#include "cm_clockjob.h"
#define CM_CLOCKJOB_CASES 24576u
void cm_clockjob_input(apollo_overlay_state *,cm_native_io *,uint16_t);
enum cm_clockjob_boundary cm_clockjob_apply(apollo_overlay_state *,cm_native_io *,uint16_t);
#endif
