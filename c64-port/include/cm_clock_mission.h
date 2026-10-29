/* SPDX-License-Identifier: MIT */
#ifndef CM_CLOCK_MISSION_H
#define CM_CLOCK_MISSION_H
#include "cm_mission_loop.h"
enum cm_clock_mission_group {CM_FLIGHT_CLOCK_ENTER=200,CM_FLIGHT_CLOCK_TRIG,CM_FLIGHT_CLOCK_RETURN,CM_FLIGHT_CLOCK_SELECT,CM_FLIGHT_DISPLAY_CONTROL,CM_FLIGHT_IDLE};
/* Resident extension selecting genuine compiled CLOCKJOB/display/idle groups.
 * One canonical image and the existing native IRQ/runtime metadata remain
 * authoritative; all callbacks return before another group/slot load. */
enum cm_service_status cm_clock_mission_identify(const cm_mission_loop *,cm_mission_call *);
enum cm_service_status cm_clock_mission_step(cm_mission_loop *);
enum cm_service_status cm_clock_mission_builtin_invoke(const cm_mission_call *,apollo_overlay_state *,cm_native_io *,cm_task_interrupt_state *,exec_transfer *,void *);
#endif
