/* SPDX-License-Identifier: MIT */
#include "cm_clock_mission.h"
#include "cm_clockjob.h"
#include "cm_quictrig.h"
#include "cm_display_control.h"
#include "cm_idle_dispatch.h"
enum cm_service_status cm_clock_mission_builtin_invoke(const cm_mission_call *c,apollo_overlay_state *s,cm_native_io *io,cm_task_interrupt_state *irq,exec_transfer *next,void *user){enum exec_status status;
 switch(c->group){
 case CM_FLIGHT_CLOCK_ENTER:return cm_clockjob_enter(s,io)==CM_CLOCKJOB_TRIG?CM_SERVICE_ADVANCED:CM_SERVICE_FAULT;
 case CM_FLIGHT_CLOCK_TRIG:return cm_quictrig(s,&io->interrupt_enabled)==CM_TRIG_OK?CM_SERVICE_ADVANCED:CM_SERVICE_FAULT;
 case CM_FLIGHT_CLOCK_RETURN:return cm_clockjob_return(s,io)==CM_CLOCKJOB_CHECK?CM_SERVICE_ADVANCED:CM_SERVICE_FAULT;
 case CM_FLIGHT_CLOCK_SELECT:return cm_clockjob_select(s,io)!=CM_CLOCKJOB_UNSUPPORTED?CM_SERVICE_ADVANCED:CM_SERVICE_FAULT;
 case CM_FLIGHT_DISPLAY_CONTROL:return cm_display_control_enter(s,io)==CM_DISPLAY_CONTROL_OK?CM_SERVICE_ADVANCED:CM_SERVICE_FAULT;
 case CM_FLIGHT_IDLE:status=cm_idle_dispatch(s,io,next);return status==EXEC_OK||status==EXEC_IDLE?CM_SERVICE_ADVANCED:CM_SERVICE_FAULT;
 default:return cm_mission_builtin_invoke(c,s,io,irq,next,user);}
}
