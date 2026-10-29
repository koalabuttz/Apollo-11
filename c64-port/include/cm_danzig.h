/* SPDX-License-Identifier: MIT */
#ifndef CM_DANZIG_H
#define CM_DANZIG_H
#include "cm_dispatch.h"
#include "cm_native_image.h"
/* Actual DANZIG06030 through NEWOPS06037 or a genuine selected context.
 * EDOP=0 and NEWJOB=+0 or an admitted positive candidate displacement.
 * No pending order decoder or application callback is executed. */
enum exec_status cm_danzig(apollo_overlay_state *,cm_native_io *,exec_transfer *);
#endif
