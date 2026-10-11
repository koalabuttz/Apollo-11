/* SPDX-License-Identifier: MIT */
#include "cm_task_transfer.h"
static void banks(apollo_overlay_state *s,agc_word bb){s->words[6]=(agc_word)(bb&076007u);s->words[3]=(agc_word)((bb&7u)<<8);s->words[4]=(agc_word)(bb&076000u);}
static enum cm_task_transfer_exit next_task(apollo_overlay_state *s,cm_native_io *io){cm_waitlist w;exec_address task;
 cm_native_gather_waitlist(s,io,&w);if(cm_wl_begin(&w,&task)!=CM_WL_OK)return CM_TASK_INVALID;
 cm_native_commit_waitlist(s,io,&w);s->words[0]=03461;s->words[1]=02003;s->words[5]=task.location;banks(s,task.bank);io->superbank=(agc_word)(task.bank&0160u);return CM_TASK_SELECTED;
}
enum cm_task_transfer_exit cm_task_transfer(apollo_overlay_state *s,cm_native_io *io,enum cm_task_transfer_entry entry){agc_word oldbb,oldq;agc_register oldl;
 if(s==0||io==0||(unsigned)entry>CM_TASK_FINISH||s->words[6]!=(agc_word)(s->words[4]|(s->words[3]>>8))||s->words[2]>07777u||s->words[012]>07777u||io->interrupt_enabled>1u||(io->superbank&077617u)!=0)return CM_TASK_INVALID;
 if(entry==CM_TASK_DISPATCH){if(s->words[6]!=02003u)return CM_TASK_INVALID;return next_task(s,io);}
 if(s->words[0734]==1u){banks(s,02003);return next_task(s,io);}
 if(s->words[0734]!=077777u||(s->words[016]&~076167u)!=0)return CM_TASK_INVALID;
 oldbb=s->words[6];oldq=s->words[2];oldl=s->words[1];s->words[2]=s->words[012];s->words[012]=oldq;
 banks(s,s->words[016]);io->superbank=(agc_word)(s->words[016]&0160u);
 s->words[0]=agc_extend(s->words[010]);s->words[1]=agc_extend(s->words[011]);s->words[010]=oldbb;s->words[011]=agc_correct(oldl);
 s->words[5]=05341;io->interrupt_enabled=1;return CM_TASK_RESUME;
}
