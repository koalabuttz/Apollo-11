/* SPDX-License-Identifier: MIT */
#ifndef CM_MISSION_LOOP_H
#define CM_MISSION_LOOP_H
#include "cm_service_loop.h"
#include "cm_task_interrupt.h"
#include "cm_dispatch.h"
enum cm_mission_group {CM_MISSION_INTERRUPT=100,CM_MISSION_RESUME,CM_MISSION_DANZIG,CM_MISSION_ENDJOB};
typedef struct {unsigned group;cm_service_call service;} cm_mission_call;
typedef enum cm_service_status (*cm_mission_invoke)(const cm_mission_call *,apollo_overlay_state *,cm_native_io *,cm_task_interrupt_state *,exec_transfer *,void *);
typedef struct {apollo_overlay_state *image;cm_native_io *io;cm_mission_invoke invoke;void *user;cm_task_interrupt_state irq;exec_transfer transfer;uint16_t completed_groups;uint8_t pending_t3,in_callback;} cm_mission_loop;
/* Resident metadata only. Each invocation returns before the next group or
 * overlay load. Source IRQ producer delivery occurs at certified DANZIG. */
enum cm_service_status cm_mission_identify(const cm_mission_loop *,cm_mission_call *);
enum cm_service_status cm_mission_step(cm_mission_loop *);
enum cm_service_status cm_mission_builtin_invoke(const cm_mission_call *,apollo_overlay_state *,cm_native_io *,cm_task_interrupt_state *,exec_transfer *,void *);
#endif
