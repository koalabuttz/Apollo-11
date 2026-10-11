/* SPDX-License-Identifier: MIT */
#ifndef CM_PRIOLARM_H
#define CM_PRIOLARM_H
#include "cm_native_image.h"
enum cm_priolarm_result {CM_PRIOLARM_DISPLAY,CM_PRIOLARM_POODOO,CM_PRIOLARM_OUT_OF_SCOPE};
/* Actual PRIOLARM10,3766 through source-selected PRIODSPR10,2726 or MULTFAIL
 * overflow POODOO05721. Full raw A/Q register bits are retained at entry;
 * TS L overflow follows its real skip, preserving the resulting ALMCADR.
 * No downstream priority display or recovery completion is fabricated. */
enum cm_priolarm_result cm_priolarm(apollo_overlay_state *,cm_native_io *);
#endif
