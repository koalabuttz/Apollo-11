/* SPDX-License-Identifier: MIT */
#ifndef CM_CLOCK_DISPLAY_MISSION_H
#define CM_CLOCK_DISPLAY_MISSION_H
#include "cm_clock_mission.h"
enum cm_clock_display_group {CM_CLOCK_DISPLAY_MAKEPLAY=210,CM_CLOCK_DISPLAY_PRIORITY,CM_CLOCK_DISPLAY_SELECT,CM_CLOCK_DISPLAY_WAKE,CM_CLOCK_DISPLAY_WAKE_RETURN,CM_CLOCK_DISPLAY_SLEEP_ENTER,CM_CLOCK_DISPLAY_SLEEP_RETURN,CM_CLOCK_DISPLAY_SLEEP,CM_CLOCK_DISPLAY_SLEEP_RESUME};
/* Actual clock service extension through source MAKEPLAY/priority/wake/sleep.
 * Stops at genuine GOPLAY: noun40 conversion remains a separate consumer.
 * One image; no frame persists across explicit native group invocations. */
enum cm_service_status cm_clock_display_identify(const cm_mission_loop *,cm_mission_call *);
enum cm_service_status cm_clock_display_step(cm_mission_loop *);
enum cm_service_status cm_clock_display_invoke(const cm_mission_call *,apollo_overlay_state *,cm_native_io *,cm_task_interrupt_state *,exec_transfer *,void *);
#endif
