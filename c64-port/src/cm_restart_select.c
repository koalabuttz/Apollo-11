/* SPDX-License-Identifier: MIT */
#include "cm_restart_select.h"
#include "cm_restart_table_data.h"
#include "agc_word.h"
static agc_register com(agc_register a){return (agc_register)(a^0177777u);}
static agc_word cyr(agc_word a){return (agc_word)((a>>1)|((a&1u)<<14));}
static agc_word phys(agc_word address,agc_word bank){return address<01400u?address:(agc_word)((bank&7u)*0400u+(address&0377u));}
static void job(apollo_overlay_state *s,agc_word priority){
 if(priority==0||(priority&AGC_SIGN)!=0){s->words[0704]=05134;s->words[0]=com(agc_extend(priority));s->words[2]=03643;}
 else{s->words[0704]=05147;s->words[0]=agc_extend(priority);s->words[2]=03637;}
 s->words[5]=0704;
}
static enum cm_restart_select_exit timed(apollo_overlay_state *s,uint8_t group,agc_word delta,int lng){
 agc_word bank=s->words[0706];
 if(lng){
  if(delta==02654u&&(bank&076000u)==026000u){s->words[0]=3;s->words[1]=025140;}
  else {agc_word address=phys(delta,bank);if(delta>=02000u||address>=03777u)return CM_RS_SELECT_INVALID;s->words[0]=agc_extend(s->words[address]);s->words[1]=agc_extend(s->words[address+1u]);}
  s->words[0706]=(agc_word)(bank&076007u);
  return cm_restart_long_delay(s)==CM_RSTIME_LONGCALL?CM_RS_SELECTED_LONGCALL:CM_RS_SELECTED_TASK;
 }
 if(delta==077777u){s->words[0]=1;s->words[2]=03614;s->words[5]=0704;return CM_RS_SELECTED_TASK;}
 if((delta&AGC_SIGN)!=0){agc_word address=agc_negate(delta);if(address>=02000u)return CM_RS_SELECT_INVALID;s->words[0]=agc_extend(s->words[phys(address,bank)]);s->words[0706]=(agc_word)(bank&076007u);}
 else s->words[0]=agc_extend(delta);
 (void)cm_restart_wait_delay(s,group);return CM_RS_SELECTED_TASK;
}
enum cm_restart_select_exit cm_restart_select(apollo_overlay_state *s,enum cm_restart_select_entry entry){
 uint8_t group;agc_word phase,priority,pointer,code,bank,oldloc,oldbank;
 if(s==0||(unsigned)entry>CM_RS_SELECT_TYPEB_TABLE||s->words[6]!=02003u||s->words[3]!=01400u||s->words[4]!=02000u||s->words[0161]>=6u)return CM_RS_SELECT_INVALID;
 group=(uint8_t)s->words[0161];phase=s->words[0154];
 if(entry==CM_RS_SELECT_PART2){pointer=(agc_word)(s->words[0156]+3u);if(pointer>0366u)return CM_RS_SELECT_INVALID;
  s->words[0156]=pointer;s->words[0707]=04675;}
 else{
  if(phase==0||phase==077777u||phase>01777u)return CM_RS_SELECT_INVALID;
  if(entry==CM_RS_SELECT_GROUP){s->words[0155]=(agc_word)(group*2u);s->words[0157]=03755;s->words[0707]=04675;}
  if((phase&01400u)!=0&&entry==CM_RS_SELECT_GROUP){
   oldloc=s->words[0705];oldbank=s->words[0706];s->words[0705]=s->words[01436+group*2u];s->words[0706]=s->words[01437+group*2u];
   s->words[0]=agc_extend(oldloc);s->words[1]=agc_extend(oldbank);priority=s->words[01054+group*2u];
   if((phase&01000u)!=0){s->words[0157]=04675;s->words[0707]=03533;s->words[0154]=(agc_word)(phase&0177u);job(s,priority);return CM_RS_SELECTED_JOB;}
   if((phase&7u)==2u){job(s,priority);return CM_RS_SELECTED_JOB;}
   s->words[0704]=05245;
   return timed(s,group,(phase&7u)>=3u?agc_negate(priority):priority,(phase&7u)>=3u);
  }
  if(phase==1u){s->words[0]=014000;s->words[2]=03540;s->words[5]=05147;return CM_RS_SELECTED_DISPLAY;}
  if((phase&1u)!=0){
   s->words[020]=cyr(cyr((agc_word)(phase-2u)));s->words[021]=(agc_word)(phase>>2);
   pointer=(agc_word)(cm_restart_table_data[group*2u+1u]+phase+(phase>>1));s->words[0707]=04675;
  }else{
   s->words[020]=cyr(cyr((agc_word)(phase-2u)));s->words[0707]=s->words[0157];
   pointer=(agc_word)(cm_restart_table_data[group*2u]+3u*phase);
  }
  if(pointer>0366u)return CM_RS_SELECT_INVALID;
  s->words[0156]=pointer;
 }
 code=cm_restart_table_data[pointer+1u];bank=cm_restart_table_data[pointer+2u];priority=cm_restart_table_data[pointer];oldbank=s->words[0706];s->words[0706]=bank;s->words[1]=agc_extend(oldbank);
 if(code!=0&&(code&AGC_SIGN)==0){s->words[0]=agc_extend(s->words[0705]);s->words[0705]=code;job(s,priority);return CM_RS_SELECTED_JOB;}
 s->words[0705]=agc_negate(code);s->words[0704]=05245;
 if((bank&01000u)!=0){s->words[0706]=agc_negate(bank);return timed(s,group,priority,0);}
 return timed(s,group,priority,1);
}
