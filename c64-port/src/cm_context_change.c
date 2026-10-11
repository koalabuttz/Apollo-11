/* SPDX-License-Identifier: MIT; original CM basic/interpretive exchanges. */
#include "cm_context_change.h"
enum exec_status cm_context_change(apollo_overlay_state *s,cm_native_io *io,uint8_t interpretive,exec_transfer *next){cm_executive e;exec_interpreter control;exec_address incoming,outgoing;enum exec_status status;uint8_t slot,selected_interpreter;agc_word oldl;
 if(!s||!io||!next||interpretive>1u||s->words[5]!=(interpretive?05170u:05164u)||s->words[7]||s->words[3]>03400u||(s->words[3]&0377u)||(s->words[4]&~076000u)||s->words[6]!=(agc_word)(s->words[4]|(s->words[3]>>8))||(io->superbank&~0160u)||io->interrupt_enabled>1u||io->executive_alarm)return EXEC_OUT_OF_SCOPE;
 cm_native_gather_executive(s,io,&e);if(!e.newjob||e.newjob>72u||e.newjob%12u)return EXEC_OUT_OF_SCOPE;slot=(uint8_t)(e.newjob/12u);
 incoming.location=e.core[slot][CM_EXEC_LOC];incoming.bank=e.core[slot][CM_EXEC_BANK];selected_interpreter=(uint8_t)((incoming.location&AGC_SIGN)!=0);
 if(!e.core[slot][CM_EXEC_PRIORITY]||e.core[slot][CM_EXEC_PRIORITY]>=040000u||(incoming.bank&(agc_word)~076167u)||(!selected_interpreter&&(!incoming.location||incoming.location>07777u))||(selected_interpreter&&agc_negate(incoming.location)>=07777u))return EXEC_OUT_OF_SCOPE;
 oldl=s->words[1];control.edop=0;control.intbit15=s->words[0115];
 if(interpretive){status=cm_exec_change_interpretive(&e,io->superbank,&control,next);if(status!=EXEC_OK)return status;}
 else{outgoing.location=s->words[2];outgoing.bank=(agc_word)(s->words[6]|io->superbank);if(!outgoing.location||outgoing.location>07777u)return EXEC_OUT_OF_SCOPE;
  if(selected_interpreter)e.core[slot][CM_EXEC_LOC]=agc_add_word(agc_negate(incoming.location),1);
  status=cm_exec_change_basic(&e,outgoing,&incoming);if(status!=EXEC_OK)return status;next->interpretive=selected_interpreter;next->address=incoming;
  if(selected_interpreter){e.core[0][CM_EXEC_LOC]=incoming.location;e.core[0][CM_EXEC_BANK]=(agc_word)(incoming.bank&076007u);control.intbit15=(agc_word)(incoming.bank&040000u);}
 }
 cm_native_commit_executive(s,io,&e);if(!interpretive)s->words[2]=oldl;s->words[6]=(agc_word)(next->address.bank&076007u);s->words[3]=(agc_word)((s->words[6]&7u)*0400u);s->words[4]=(agc_word)(s->words[6]&076000u);io->superbank=(agc_word)(next->address.bank&0160u);io->interrupt_enabled=1;s->words[1]=02006;s->words[0]=03013;s->words[5]=next->address.location;
 if(next->interpretive){s->words[0115]=control.intbit15;s->words[023]=0;s->words[0]=agc_extend(control.intbit15);s->words[5]=06037;}
 return EXEC_OK;
}
