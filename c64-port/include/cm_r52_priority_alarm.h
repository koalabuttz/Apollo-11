/* SPDX-License-Identifier: MIT */
#ifndef CM_R52_PRIORITY_ALARM_H
#define CM_R52_PRIORITY_ALARM_H
#include "cm_native_image.h"
#include "cm_dispatch.h"
/* Actual P51/R52 source caller CAF40414,2174→BANKCALL PRIOLARM; groups0..6
 * compose real alarm, PRIODSPR admission, immediate2202return, and ENDOFJOB.
 * Resource failure stops at genuine BAILOUT. Queued MAKEPLAY remains its actual
 * selected compiled entry. No decoder/callback or extra persistent image. */
enum exec_status cm_r52_priority_alarm_step(apollo_overlay_state *,cm_native_io *,unsigned group,exec_transfer *next);
#endif
