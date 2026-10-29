/* SPDX-License-Identifier: MIT */
#include "cm_waitlist_image.h"
static agc_register add(agc_register a,agc_word b){return agc_add_register(a,agc_extend(b));}
static agc_register neg(agc_register a){return (agc_register)(a^0177777u);}
static int kind(agc_register a){int v=agc_overflow(a);return v>0?(int)AGC_POSITIVE:v<0?(int)AGC_NEGATIVE:(int)agc_classify(agc_correct(a));}
static agc_register dabs(agc_register a){a=(a&0100000u)?neg(a):a;return a==0?0:(agc_register)(a-1u);}
enum cm_wl_status cm_waitlist_image(apollo_overlay_state *s,cm_native_io *io,exec_address task){
 cm_waitlist w;agc_register a;agc_word delay,caller,bank,temp=0;uint8_t slot=0;int cls;enum cm_wl_status status;
 if(s==0||io==0||s->words[0]>037777u||s->words[0]==0||s->words[2]>07775u||s->words[6]!=(agc_word)(s->words[4]|(s->words[3]>>8))||io->interrupt_enabled>1u||(io->superbank&077617u)!=0)return CM_WL_OUT_OF_SCOPE;
 delay=s->words[0];caller=s->words[2];bank=s->words[6];cm_native_gather_waitlist(s,io,&w);
 if(w.alarm||w.time3>037777u)return CM_WL_OUT_OF_SCOPE;
 a=add(agc_extend(agc_negate(w.time3)),0200);cls=kind(a);a=dabs(a);
 if(cls==AGC_POSITIVE)a=neg(add(a,040001));else if(cls==AGC_PLUS_ZERO)a=neg(a);
 if(cls!=AGC_MINUS_ZERO)a=add(a,040201);
 a=add(a,delay);cls=kind(a);a=dabs(a);
 if(cls==AGC_POSITIVE){a=add(a,w.interval[0]);
  for(slot=1;slot<CM_WL_TASK_COUNT;++slot){cls=kind(a);a=dabs(a);if(cls==AGC_NEGATIVE||cls==AGC_MINUS_ZERO||slot==CM_WL_INTERVAL_COUNT)break;if(cls==AGC_POSITIVE)a=add(a,w.interval[slot]);}
  if(slot==CM_WL_TASK_COUNT||(slot==CM_WL_INTERVAL_COUNT&&cls==AGC_POSITIVE))return CM_WL_OUT_OF_SCOPE;
  if(cls==AGC_NEGATIVE||cls==AGC_PLUS_ZERO)a=add(a,1);
  temp=agc_correct(a);
 }
 status=cm_wl_schedule(&w,delay,task);if(status!=CM_WL_OK)return status;
 cm_native_commit_waitlist(s,io,&w);
 if(slot)s->words[064]=temp;
 s->words[061]=077777;s->words[062]=CM_WL_END_BANK;s->words[063]=task.location;
 s->words[0]=05266;s->words[1]=02003;s->words[2]=slot;s->words[5]=agc_correct(add(caller,2));
 s->words[6]=bank;s->words[3]=(agc_word)((bank&7u)<<8);s->words[4]=(agc_word)(bank&076000u);io->interrupt_enabled=0;
 return CM_WL_OK;
}
