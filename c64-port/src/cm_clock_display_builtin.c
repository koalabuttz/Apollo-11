/* SPDX-License-Identifier: MIT */
#include "cm_clock_display_mission.h"
#include "cm_makeplay.h"
#include "cm_makeplay_sleep.h"
#include "cm_priority_complete.h"
#include "cm_basic_wake.h"
#include "cm_basic_sleep.h"
enum cm_service_status cm_clock_display_invoke(const cm_mission_call *c,apollo_overlay_state *s,cm_native_io *io,cm_task_interrupt_state *irq,exec_transfer *next,void *user){enum exec_status st;
 switch(c->group){
 case CM_CLOCK_DISPLAY_MAKEPLAY:return cm_makeplay_priority(s,io)==CM_MAKEPLAY_PRIORITY?CM_SERVICE_ADVANCED:CM_SERVICE_FAULT;
 case CM_CLOCK_DISPLAY_PRIORITY:st=cm_priority_complete(s,io,next);return st==EXEC_OK?CM_SERVICE_ADVANCED:CM_SERVICE_FAULT;
 case CM_CLOCK_DISPLAY_SELECT:return cm_makeplay_select(s,io)!=CM_MAKEPLAY_UNSUPPORTED?CM_SERVICE_ADVANCED:CM_SERVICE_FAULT;
 case CM_CLOCK_DISPLAY_WAKE:st=cm_basic_wake(s,io);return st==EXEC_OK||st==EXEC_NOT_FOUND?CM_SERVICE_ADVANCED:CM_SERVICE_FAULT;
 case CM_CLOCK_DISPLAY_WAKE_RETURN:return cm_makeplay_wake_return(s,io)==CM_MAKEPLAY_PLAY?CM_SERVICE_ADVANCED:CM_SERVICE_FAULT;
 case CM_CLOCK_DISPLAY_SLEEP_ENTER:return cm_makeplay_sleep_enter(s,io)==CM_DISPLAY_SLEEP_WAKE?CM_SERVICE_ADVANCED:CM_SERVICE_FAULT;
 case CM_CLOCK_DISPLAY_SLEEP_RETURN:return cm_makeplay_sleep_return(s,io)==CM_DISPLAY_SLEEP_EXECUTIVE?CM_SERVICE_ADVANCED:CM_SERVICE_FAULT;
 case CM_CLOCK_DISPLAY_SLEEP:st=cm_basic_sleep(s,io,next);return st==EXEC_OK||st==EXEC_IDLE?CM_SERVICE_ADVANCED:CM_SERVICE_FAULT;
 case CM_CLOCK_DISPLAY_SLEEP_RESUME:return cm_makeplay_sleep_resume(s,io)==CM_DISPLAY_SLEEP_PLAY?CM_SERVICE_ADVANCED:CM_SERVICE_FAULT;
 default:return cm_clock_mission_builtin_invoke(c,s,io,irq,next,user);}}
