/* SPDX-License-Identifier: MIT */
#include "cm_service_overlay_binding.h"
#if CM_SERVICE_MODULE == 1
#include "cm_task_transfer.h"
#elif CM_SERVICE_MODULE == 2
#include "cm_entry_tasks.h"
#include "cm_clocktask.h"
#include "cm_tig5.h"
#include "cm_svct3.h"
#elif CM_SERVICE_MODULE == 3
#include "cm_waitlist_image.h"
#include "cm_delay_image.h"
#elif CM_SERVICE_MODULE == 4
#include "cm_admission_image.h"
#else
#error unknown CM service body module
#endif
uint16_t apollo_overlay_main(apollo_overlay_state *s,uint16_t selector){const cm_service_call *c=CM_SERVICE_CALL;cm_native_io *io=CM_SERVICE_IO;
 if(selector!=(uint16_t)c->group||s->words[5]!=c->entry.location||(s->words[6]|io->superbank)!=c->entry.bank)return 1;
 switch(c->group){
#if CM_SERVICE_MODULE == 1
 case CM_SERVICE_T3_BEGIN:case CM_SERVICE_TASK_FINISH:return cm_task_transfer(s,io,c->group==CM_SERVICE_T3_BEGIN?CM_TASK_DISPATCH:CM_TASK_FINISH)==CM_TASK_INVALID;
#elif CM_SERVICE_MODULE == 2
 case CM_SERVICE_SETJTAG:return cm_setjtag_image(s,io)!=CM_WL_OK;
 case CM_SERVICE_CLOCK_ENTRY:case CM_SERVICE_CLOCK_RETURN:case CM_SERVICE_CLOCK_FINISH:return cm_clocktask_phase(s,io,(uint8_t)(c->group-CM_SERVICE_CLOCK_ENTRY))==CM_CLOCK_INVALID;
 case CM_SERVICE_TIG5_ENTRY:case CM_SERVICE_TIG5_RETURN:case CM_SERVICE_TIG5_FINISH:return cm_tig5_phase(s,io,(uint8_t)(c->group-CM_SERVICE_TIG5_ENTRY))==CM_TIG5_INVALID;
 case CM_SERVICE_SVCT3_ENTRY:case CM_SERVICE_SVCT3_RETURN:case CM_SERVICE_SVCT3_RECHECK:return cm_svct3_phase(s,io,(uint8_t)(c->group-CM_SERVICE_SVCT3_ENTRY))==CM_SVCT3_INVALID;
#elif CM_SERVICE_MODULE == 3
 case CM_SERVICE_WAITLIST:return cm_waitlist_image(s,io,c->argument)!=CM_WL_OK;
 case CM_SERVICE_FIXDELAY:{cm_native_io task_io=*io;agc_word irq=io->interrupt_enabled;
  task_io.interrupt_enabled=0;if(cm_delay_image(s,&task_io,c->literal,1)!=CM_WL_OK)return 1;
  *io=task_io;io->interrupt_enabled=irq;return 0;}
#elif CM_SERVICE_MODULE == 4
 case CM_SERVICE_FINDVAC:case CM_SERVICE_NOVAC:return cm_admission_image(s,io,c->argument,(uint8_t)(c->group==CM_SERVICE_FINDVAC))!=EXEC_OK;
#endif
 default:return 1;
 }
}
