/* SPDX-License-Identifier: MIT */
#ifndef CM_IDLE_DISPATCH_H
#define CM_IDLE_DISPATCH_H
#include "cm_dispatch.h"
#include "cm_native_image.h"
/* Actual idle ADVAN01,3231 through NUDIRECT/NUCHANG2 and genuine basic,
 * interpreted or SELFRET selfcheck continuation. No callback body executes. */
enum exec_status cm_idle_dispatch(apollo_overlay_state *,cm_native_io *,exec_transfer *);
#endif
