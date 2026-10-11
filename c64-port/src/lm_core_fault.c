/* SPDX-License-Identifier: MIT; original Luminary099 Executive fault aliases. */
#include "lm_core_fault.h"
static uint16_t erasable(agc_word q,agc_word eb){return q<01400u?q:(uint16_t)((eb>>8)*0400u+(q&0377u));}
enum exec_status lm_core_fault(apollo_overlay_state *s,p66_bias_io *io,
 enum lm_core_fault_entry entry,exec_address job){
 static const agc_word pc[5]={05072,05105,05116,05112,05137};
 agc_word *w,priority=0,l,wake=0,oldfb;uint8_t i,slot=8,vac=5;
 if(!s||!io||(unsigned)entry>4u)return EXEC_OUT_OF_SCOPE;
 w=s->words;
 if(w[5]!=pc[entry]||w[7]||w[2]<0100u||w[2]>07775u||
  w[3]>03400u||(w[3]&0377u)||(w[4]&~076000u)||
  w[6]!=(agc_word)(w[4]|(w[3]>>8))||
  (io->superbank&~0160u)||io->interrupt_enabled>1u)return EXEC_OUT_OF_SCOPE;
 oldfb=w[4];l=agc_correct(w[1]);
 if(entry==LM_CORE_FAULT_WAKE){
  wake=agc_correct(w[0]);if(w[0]!=agc_extend(wake))return EXEC_OUT_OF_SCOPE;
 }else{
  if(entry>=LM_CORE_FAULT_SPVACIN){
   if(io->interrupt_enabled)return EXEC_OUT_OF_SCOPE;
   priority=w[063];job.location=agc_correct(w[0]);job.bank=agc_correct(w[1]);
  }else{
   if(agc_overflow(w[0]))return EXEC_OUT_OF_SCOPE;
   priority=agc_correct(w[0]);
   if(w[2]<02000u){job.location=w[erasable(w[2],w[3])];job.bank=w[erasable((agc_word)(w[2]+1u),w[3])];}
  }
  if(priority>037000u||job.location>AGC_MASK||job.bank>AGC_MASK)return EXEC_OUT_OF_SCOPE;
  if(entry!=LM_CORE_FAULT_NOVAC){
   for(i=0;i<5u;++i){agc_word use=w[0400u+44u*i];
    if(use&&use!=(agc_word)(0400u+44u*i))return EXEC_OUT_OF_SCOPE;
    if(vac==5u&&use)vac=i;}
   if(vac==5u)return EXEC_OUT_OF_SCOPE;
  }
 }
 for(i=0;i<8u;++i){agc_word p=w[0167u+12u*i];
  if(p==0){slot=i;break;}
  if(entry!=LM_CORE_FAULT_WAKE){if(p==AGC_NEG_ZERO)return EXEC_OUT_OF_SCOPE;}
  else if(agc_classify(p)==AGC_NEGATIVE){
   agc_register d=agc_add_register(agc_extend(agc_negate(wake)),agc_extend(w[0164u+12u*i]));
   if(d==0||d==0177777u)return EXEC_OUT_OF_SCOPE;
  }
 }
 if(slot==8u)return EXEC_OUT_OF_SCOPE;
 if(entry==LM_CORE_FAULT_WAKE)w[065]=wake;
 else{
  if(entry==LM_CORE_FAULT_NOVAC){l=w[066];priority=agc_add_word(priority,EXEC_FAKEPRET);}
  else{l=w[0400u+44u*vac];priority=agc_add_word(priority,(agc_word)(l+1u));w[0400u+44u*vac]=0;}
  w[063]=priority;w[065]=job.location;w[066]=job.bank;
 }
 w[061]=oldfb;w[062]=(agc_word)(7u-slot);w[064]=(agc_word)(12u*slot);
 w[0]=0;w[1]=agc_extend(l);w[4]=02000;
 w[6]=(agc_word)(02000u|(w[3]>>8));io->interrupt_enabled=0;
 w[2]=entry==LM_CORE_FAULT_WAKE?03034:02635;
 w[5]=entry==LM_CORE_FAULT_WAKE?014:7;
 return EXEC_UNMAPPED;
}
