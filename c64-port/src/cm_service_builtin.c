/* SPDX-License-Identifier: MIT */
#include "cm_service_loop.h"
#include "cm_task_transfer.h"
#include "cm_entry_tasks.h"
#include "cm_clocktask.h"
#include "cm_tig5.h"
#include "cm_svct3.h"
#include "cm_waitlist_image.h"
#include "cm_admission_image.h"
#include "cm_delay_image.h"
#include "cm_longcall.h"
#include "cm_restart_select.h"
#include "cm_init_display.h"
enum cm_service_status cm_service_builtin_invoke(const cm_service_call *c,apollo_overlay_state *s,cm_native_io *io,void *user){unsigned result;
#ifndef CM_SERVICE_TASK_PROOF_ONLY
 exec_address next;
#endif
 (void)user;
 if(c==0||s==0||io==0)return CM_SERVICE_INVALID;
 switch(c->group){
 case CM_SERVICE_T3_BEGIN:case CM_SERVICE_TASK_FINISH:
  result=(unsigned)cm_task_transfer(s,io,c->group==CM_SERVICE_T3_BEGIN?CM_TASK_DISPATCH:CM_TASK_FINISH);return result<=CM_TASK_RESUME?CM_SERVICE_ADVANCED:CM_SERVICE_FAULT;
 case CM_SERVICE_SETJTAG:return cm_setjtag_image(s,io)==CM_WL_OK?CM_SERVICE_ADVANCED:CM_SERVICE_FAULT;
 case CM_SERVICE_CLOCK_ENTRY:case CM_SERVICE_CLOCK_RETURN:case CM_SERVICE_CLOCK_FINISH:
  return cm_clocktask_phase(s,io,(uint8_t)(c->group-CM_SERVICE_CLOCK_ENTRY))!=CM_CLOCK_INVALID?CM_SERVICE_ADVANCED:CM_SERVICE_FAULT;
 case CM_SERVICE_TIG5_ENTRY:case CM_SERVICE_TIG5_RETURN:case CM_SERVICE_TIG5_FINISH:
  return cm_tig5_phase(s,io,(uint8_t)(c->group-CM_SERVICE_TIG5_ENTRY))!=CM_TIG5_INVALID?CM_SERVICE_ADVANCED:CM_SERVICE_FAULT;
 case CM_SERVICE_SVCT3_ENTRY:case CM_SERVICE_SVCT3_RETURN:case CM_SERVICE_SVCT3_RECHECK:
  return cm_svct3_phase(s,io,(uint8_t)(c->group-CM_SERVICE_SVCT3_ENTRY))!=CM_SVCT3_INVALID?CM_SERVICE_ADVANCED:CM_SERVICE_FAULT;
 case CM_SERVICE_WAITLIST:return cm_waitlist_image(s,io,c->argument)==CM_WL_OK?CM_SERVICE_ADVANCED:CM_SERVICE_FAULT;
 case CM_SERVICE_FINDVAC:case CM_SERVICE_NOVAC:return cm_admission_image(s,io,c->argument,(uint8_t)(c->group==CM_SERVICE_FINDVAC))==EXEC_OK?CM_SERVICE_ADVANCED:CM_SERVICE_FAULT;
 case CM_SERVICE_FIXDELAY:{cm_native_io task_io=*io;agc_word irq=io->interrupt_enabled;
  /* Source InIsr excludes nested interrupts even when AllowInterrupt is one.
   * FIXDELAY contains no INHINT; preserve that latch across insertion. */
  task_io.interrupt_enabled=0;if(cm_delay_image(s,&task_io,c->literal,1)!=CM_WL_OK)return CM_SERVICE_FAULT;
  *io=task_io;io->interrupt_enabled=irq;return CM_SERVICE_ADVANCED;}
#ifndef CM_SERVICE_TASK_PROOF_ONLY
 case CM_SERVICE_LONG_ENTER:case CM_SERVICE_LONG_CYCLE:case CM_SERVICE_LONG_RETURN:case CM_SERVICE_LONG_TARGET:
  return cm_longcall_phase_run(s,(enum cm_longcall_phase)(c->group-CM_SERVICE_LONG_ENTER),c->argument)!=CM_LONGCALL_INVALID?CM_SERVICE_ADVANCED:CM_SERVICE_FAULT;
 case CM_SERVICE_RESTART_GROUP:case CM_SERVICE_RESTART_PART2:case CM_SERVICE_RESTART_TYPEB:
  return cm_restart_select(s,(enum cm_restart_select_entry)(c->group-CM_SERVICE_RESTART_GROUP))!=CM_RS_SELECT_INVALID?CM_SERVICE_ADVANCED:CM_SERVICE_FAULT;
 case CM_SERVICE_RESTART_CALL:s->words[2]=0705;s->words[5]=c->literal;return CM_SERVICE_ADVANCED;
 case CM_SERVICE_INIT_DISPLAY:return cm_init_display(s,io,&next)==EXEC_OK?CM_SERVICE_ADVANCED:CM_SERVICE_FAULT;
 case CM_SERVICE_INIT_DISPLAY_RETURN:return cm_init_display_return(s,io,&next)==EXEC_OK?CM_SERVICE_ADVANCED:CM_SERVICE_FAULT;
#endif
 default:return CM_SERVICE_UNMAPPED;
 }
}
