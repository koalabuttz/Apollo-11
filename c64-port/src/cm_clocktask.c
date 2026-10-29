/* SPDX-License-Identifier: MIT */
#include "cm_clocktask.h"
#include "agc_multiword.h"
static enum cm_clocktask_exit finish(apollo_overlay_state *s){s->words[0]=agc_extend(agc_negate(s->words[025]));s->words[01065]=agc_correct(s->words[0]);s->words[5]=05324;return CM_CLOCK_TASKOVER;}
enum cm_clocktask_exit cm_clocktask_phase(apollo_overlay_state *s,cm_native_io *io,uint8_t phase){
 if(s==0||io==0||phase>2u||s->words[6]!=050007u||s->words[3]!=03400u||s->words[4]!=050000u||io->interrupt_enabled>1u||(io->superbank&077617u)!=0)return CM_CLOCK_INVALID;
 if(phase==2u)return finish(s);
 if(phase==1u){agc_register a;
  if(s->words[01145]==0)return finish(s);
  a=agc_add_register(agc_extend(agc_negate(01525u)),agc_extend(s->words[01145]));
  s->words[0]=027000u;
  if(a==0||a==0177777u){s->words[2]=03322;s->words[5]=05147;return CM_CLOCK_FINDVAC;}
  s->words[2]=03313;s->words[5]=05134;return CM_CLOCK_NOVAC;
 }
 if((s->words[0103]&02000u)==0){
  cm_phase_state p;static const agc_word word=6;cm_native_gather_phase(s,io,&p);p.q=03265;
  if(cm_phase_change(&p,&word,1,0)!=CM_PHASE_OK)return CM_CLOCK_INVALID;
  cm_native_commit_phase(s,io,&p);s->words[2]=03267;s->words[5]=05324;return CM_CLOCK_TASKOVER;
 }else{
  agc_register_pair r;agc_dp t={s->words[024],s->words[025]};
  r.a=agc_extend(agc_negate(s->words[03412]));r.l=agc_extend(agc_negate(s->words[03413]));agc_double_add(&r,&t);
  s->words[03660]=t.high;s->words[03661]=t.low;s->words[0]=0144;s->words[1]=0;s->words[2]=03277;s->words[5]=05245;
  return CM_CLOCK_WAITLIST;
 }
}
