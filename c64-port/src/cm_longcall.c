/* SPDX-License-Identifier: MIT */
#include "cm_longcall.h"
#include "agc_multiword.h"
static void banks(apollo_overlay_state *s,agc_word bb){
 s->words[6]=(agc_word)(bb&076007u);s->words[3]=(agc_word)((bb&7u)<<8);s->words[4]=(agc_word)(bb&076000u);
}
static int duration(agc_word high,agc_word low){return high<=037777u&&low<=037777u;}
static int addr(agc_word loc,agc_word bank){return loc>0&&loc<=07777u&&(bank&~076167u)==0;}
static enum cm_longcall_status cycle(apollo_overlay_state *s){
 agc_register_pair r={agc_extend(077777u),agc_extend(057777u)};
 agc_dp t={s->words[01137],s->words[01140]};int more;
 agc_double_add(&r,&t);s->words[01137]=t.high;s->words[01140]=t.low;
 more=agc_classify(t.low)==AGC_POSITIVE||agc_classify(t.high)==AGC_POSITIVE;
 s->words[0]=020000u;s->words[1]=0;s->words[5]=05245;
 if(more)s->words[2]=03512;
 else{
  s->words[0]=agc_add_register(020000u,agc_extend(t.low));s->words[01140]=agc_correct(s->words[0]);s->words[2]=03503;
 }
 return CM_LONGCALL_WAIT;
}
enum cm_longcall_status cm_longcall_phase_run(apollo_overlay_state *s,enum cm_longcall_phase phase,exec_address task){
 agc_word loc,bb,oldbb;
 if(s==0||(unsigned)phase>CM_LONGCALL_GETCADR||s->words[6]!=(agc_word)(s->words[4]|(s->words[3]>>8)))return CM_LONGCALL_INVALID;
 if(phase==CM_LONGCALL_ENTER){
  if(!duration(agc_correct(s->words[0]),agc_correct(s->words[1]))||!addr(task.location,task.bank)||s->words[2]>07775u)return CM_LONGCALL_INVALID;
  s->words[01137]=agc_correct(s->words[0]);s->words[01140]=agc_correct(s->words[1]);
  s->words[01133]=task.location;s->words[01134]=task.bank;oldbb=s->words[6];banks(s,02003);
  s->words[01435]=oldbb;s->words[2]=agc_correct(agc_add_register(s->words[2],2));s->words[01434]=s->words[2];
  return cycle(s);
 }
 if(s->words[6]!=02003u)return CM_LONGCALL_INVALID;
 if(phase==CM_LONGCALL_CYCLE){if(s->words[01137]>037777u||s->words[01140]>077777u||(s->words[01137]==0&&(s->words[01140]&040000u)!=0))return CM_LONGCALL_INVALID;return cycle(s);}
 loc=s->words[phase==CM_LONGCALL_RETURN?01434:01133];bb=s->words[phase==CM_LONGCALL_RETURN?01435:01134];
 if(!addr(loc,bb))return CM_LONGCALL_INVALID;
 oldbb=s->words[6];
 if(phase==CM_LONGCALL_RETURN){s->words[01434]=05324;s->words[01435]=agc_correct(s->words[1]);s->words[0]=03510;}
 else{s->words[01133]=agc_correct(s->words[0]);s->words[01134]=agc_correct(s->words[1]);s->words[0]=03517;}
 s->words[1]=oldbb;s->words[5]=loc;banks(s,bb);return CM_LONGCALL_TRANSFER;
}
