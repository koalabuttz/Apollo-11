/* SPDX-License-Identifier: MIT */
#include "cm_idle_wake.h"
#include "cm_basic_sleep.h"
#include "cm_wake_complete.h"
#include "cm_task_transfer.h"
#include "cm_idle_dispatch.h"
#include "service_routines.h"
bool cm_idle_wake_step(apollo_overlay_state *s,cm_native_io *io,cm_idle_interrupt_state *irq,exec_transfer *next,uint16_t phase){p66_bias_io basic;bool ok;
 if(s==0||io==0||irq==0||next==0||phase>8u)return false;
 switch(phase){
 case 0:return cm_basic_sleep(s,io,next)==EXEC_IDLE;
 case 1:return cm_idle_interrupt_enter(s,io,irq)==CM_IDLE_IRQ_ENTERED;
 case 2:return cm_task_transfer(s,io,CM_TASK_DISPATCH)==CM_TASK_SELECTED;
 case 3:case 5:basic.superbank=io->superbank;basic.interrupt_enabled=io->interrupt_enabled;ok=service_routines_step(s,&basic,SERVICE_ROUTINES_CM,phase==3u?SERVICE_WAKER:SERVICE_WAKER_FINISH,0,0,0);if(ok){io->superbank=basic.superbank;io->interrupt_enabled=basic.interrupt_enabled;}return ok;
 case 4:return cm_wake_complete(s,io,next)==EXEC_OK;
 case 6:return cm_task_transfer(s,io,CM_TASK_FINISH)==CM_TASK_RESUME;
 case 7:return cm_idle_interrupt_resume(s,io,irq)==CM_IDLE_IRQ_RESUMED;
 default:if(!irq->pending_idle||irq->in_isr)return false;ok=cm_idle_dispatch(s,io,next)==EXEC_OK;if(ok)irq->pending_idle=0;return ok;
 }
}
