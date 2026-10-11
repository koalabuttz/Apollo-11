/* SPDX-License-Identifier: MIT */
#ifndef CM_PRIORITY_DISPLAY_H
#define CM_PRIORITY_DISPLAY_H
#include "cm_native_image.h"
/* Actual PRIODSPR through SPVAC, genuine admission, and BOTHJOBS immediate
 * caller return. No queued MAKEPLAY payload is executed. Producers excluded. */
enum exec_status cm_priority_display_step(apollo_overlay_state *,cm_native_io *,unsigned group);
#endif
