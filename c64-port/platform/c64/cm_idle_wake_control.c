/* SPDX-License-Identifier: MIT */
#include "cm_idle_wake_binding.h"
#include "cm_basic_sleep.h"
#include "cm_wake_complete.h"
#include "cm_task_transfer.h"
#include "cm_idle_dispatch.h"
#include "service_routines.h"
uint16_t apollo_overlay_main(apollo_overlay_state *s,uint16_t selector){cm_idle_wake_fixture *f=CM_IDLE_WAKE_FIXTURE;cm_native_io *io=CM_IDLE_WAKE_IO;
 if(selector!=f->phase)return 1;
#if CM_IDLE_WAKE_GROUP == 0
 {exec_transfer next;if(selector!=0u)return 1;return cm_basic_sleep(s,io,&next)!=EXEC_IDLE;}
#elif CM_IDLE_WAKE_GROUP == 1
 if(selector==1u)return cm_idle_interrupt_enter(s,io,&f->irq)!=CM_IDLE_IRQ_ENTERED;if(selector==7u)return cm_idle_interrupt_resume(s,io,&f->irq)!=CM_IDLE_IRQ_RESUMED;return 1;
#elif CM_IDLE_WAKE_GROUP == 2
 if(selector==2u)return cm_task_transfer(s,io,CM_TASK_DISPATCH)!=CM_TASK_SELECTED;if(selector==6u)return cm_task_transfer(s,io,CM_TASK_FINISH)!=CM_TASK_RESUME;return 1;
#elif CM_IDLE_WAKE_GROUP == 3
 {p66_bias_io basic;bool ok;if(selector!=3u&&selector!=5u)return 1;basic.superbank=io->superbank;basic.interrupt_enabled=io->interrupt_enabled;ok=service_routines_step(s,&basic,SERVICE_ROUTINES_CM,selector==3u?SERVICE_WAKER:SERVICE_WAKER_FINISH,0,0,0);if(ok){io->superbank=basic.superbank;io->interrupt_enabled=basic.interrupt_enabled;}return !ok;}
#elif CM_IDLE_WAKE_GROUP == 4
 {exec_transfer next;if(selector!=4u)return 1;return cm_wake_complete(s,io,&next)!=EXEC_OK;}
#else
 {exec_transfer next;if(selector!=8u||!f->irq.pending_idle||f->irq.in_isr)return 1;if(cm_idle_dispatch(s,io,&next)!=EXEC_OK)return 1;f->irq.pending_idle=0;return next.interpretive||next.address.location!=03332u||next.address.bank!=050066u;}
#endif
}
