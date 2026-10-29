/* SPDX-License-Identifier: MIT */
#include "cm_tig5.h"
enum cm_tig5_exit cm_tig5_phase(apollo_overlay_state *s,cm_native_io *io,uint8_t phase){
 if(s==0||io==0||phase>2u||s->words[6]!=050007u||s->words[3]!=03400u||s->words[4]!=050000u||io->interrupt_enabled>1u||(io->superbank&077617u)!=0)return CM_TIG5_INVALID;
 if(phase==0u){s->words[0]=0764;s->words[2]=02576;s->words[5]=05245;return CM_TIG5_WAITLIST;}
 if(phase==1u){cm_phase_state p;static const agc_word words[2]={040074,033};
  s->words[0]=agc_extend(agc_negate(0400));s->words[01145]=077377;
  cm_native_gather_phase(s,io,&p);p.q=02603;
  if(cm_phase_change(&p,words,2,1)!=CM_PHASE_OK)return CM_TIG5_INVALID;
  cm_native_commit_phase(s,io,&p);s->words[0]=020000;s->words[2]=02607;s->words[5]=05147;return CM_TIG5_FINDVAC;
 }
 s->words[5]=05324;return CM_TIG5_TASKOVER;
}
