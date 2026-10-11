/* SPDX-License-Identifier: MIT */
#include "cm_mission_loop.h"
#include "cm_danzig.h"
#include "cm_basic_finish.h"
enum cm_service_status cm_mission_builtin_invoke(const cm_mission_call *c,apollo_overlay_state *s,cm_native_io *io,cm_task_interrupt_state *irq,exec_transfer *next,void *user){enum cm_task_interrupt_status interrupt;enum exec_status status;
 if(c==0||s==0||io==0||irq==0||next==0)return CM_SERVICE_INVALID;
 if(c->group==CM_MISSION_INTERRUPT){interrupt=cm_task_interrupt_enter(s,io,irq);return interrupt==CM_TASK_IRQ_ENTERED?CM_SERVICE_ADVANCED:interrupt==CM_TASK_IRQ_DEFERRED?CM_SERVICE_RESUME:CM_SERVICE_FAULT;}
 if(c->group==CM_MISSION_RESUME)return cm_task_interrupt_resume(s,io,irq)==CM_TASK_IRQ_RESUMED?CM_SERVICE_ADVANCED:CM_SERVICE_FAULT;
 if(c->group==CM_MISSION_DANZIG)return cm_danzig(s,io,next)==EXEC_OK?CM_SERVICE_ADVANCED:CM_SERVICE_FAULT;
 if(c->group==CM_MISSION_ENDJOB){status=cm_basic_finish(s,io,next);return status==EXEC_OK||status==EXEC_IDLE?CM_SERVICE_ADVANCED:CM_SERVICE_FAULT;}
 return cm_service_builtin_invoke(&c->service,s,io,user);}
