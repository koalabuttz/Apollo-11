/* SPDX-License-Identifier: MIT; original Luminary099 SPECTEST callers. */
#include "lm_spectest_handoff.h"
static uint16_t erasable(agc_word q,agc_word eb){return q<01400u?q:(uint16_t)((eb>>8)*0400u+(q&0377u));}
enum exec_status lm_spectest_handoff(apollo_overlay_state *s,p66_bias_io *io,
 enum lm_spectest_entry entry,exec_address job){
 static const agc_word pc[6]={05072,05105,05116,05112,05137,02651};
 agc_word *w,priority,oldfb,l,packed,wake,savedbank;agc_register delta;
 uint8_t vac=0;
 if(!s||!io||(unsigned)entry>5u)return EXEC_OUT_OF_SCOPE;
 w=s->words;
 if(w[5]!=pc[entry]||w[7]||w[3]>03400u||(w[3]&0377u)||
  (w[4]&~076000u)||w[6]!=(agc_word)(w[4]|(w[3]>>8))||
  (io->superbank&~0160u)||io->interrupt_enabled>1u)return EXEC_OUT_OF_SCOPE;
 if(entry==LM_SPECTEST_DIRECT){enum agc_ccs_class flag=agc_classify(w[067]);
  if(io->interrupt_enabled||w[4]!=02000u||
     (flag!=AGC_PLUS_ZERO&&flag!=AGC_NEGATIVE))return EXEC_OUT_OF_SCOPE;
  w[0]=agc_extend(agc_diminished_abs(w[067]));w[1]=agc_extend(agc_correct(w[1]));
  w[2]=(agc_word)(flag==AGC_PLUS_ZERO?02654u:02655u);w[5]=05705;
  return EXEC_BAILOUT_PENDING;
 }
 if(w[067]!=0||w[2]<0100u||w[2]>07775u)return EXEC_OUT_OF_SCOPE;
 oldfb=w[4];l=agc_correct(w[1]);
 if(entry==LM_SPECTEST_WAKE){
  wake=agc_correct(w[0]);savedbank=w[0165];
  if(w[0]!=agc_extend(wake)||agc_classify(w[0167])!=AGC_NEGATIVE||
     (savedbank&~0167u))return EXEC_OUT_OF_SCOPE;
  delta=agc_add_register(agc_extend(agc_negate(wake)),agc_extend(w[0164]));
  if(delta!=0&&delta!=0177777u)return EXEC_OUT_OF_SCOPE;
  w[063]=agc_negate(w[0167]);w[0167]=w[063];
  w[065]=agc_add_word((agc_word)(wake&01777u),02000);
  w[066]=agc_add_word((agc_word)(wake&076000u),savedbank);
 }else{
  if(w[0167]!=AGC_NEG_ZERO)return EXEC_OUT_OF_SCOPE;
  if(entry>=LM_SPECTEST_SPVAC){
   if(io->interrupt_enabled)return EXEC_OUT_OF_SCOPE;
   priority=w[063];job.location=agc_correct(w[0]);job.bank=agc_correct(w[1]);
  }else{
   if(agc_overflow(w[0]))return EXEC_OUT_OF_SCOPE;
   priority=agc_correct(w[0]);
   if(w[2]<02000u){job.location=w[erasable(w[2],w[3])];job.bank=w[erasable((agc_word)(w[2]+1u),w[3])];}
  }
  if(priority>037000u||job.location>AGC_MASK||job.bank>AGC_MASK)return EXEC_OUT_OF_SCOPE;
  if(entry!=LM_SPECTEST_NOVAC){
   for(vac=0;vac<5u;++vac)if(w[0400u+44u*vac]&&w[0400u+44u*vac]!=(agc_word)(0400u+44u*vac))return EXEC_OUT_OF_SCOPE;
   for(vac=0;vac<5u&&w[0400u+44u*vac]==0;++vac){}
   if(vac==5u)return EXEC_OUT_OF_SCOPE;
   l=w[0400u+44u*vac];packed=agc_add_word(priority,(agc_word)(l+1u));
   w[0400u+44u*vac]=0;
  }else{l=w[066];packed=agc_add_word(priority,EXEC_FAKEPRET);}
  w[063]=packed;w[065]=job.location;w[066]=job.bank;
  w[0167]=packed;w[0166]=(agc_word)(packed&0777u);
  w[0121]=0;w[0120]=w[0166];
 }
 w[061]=oldfb;w[062]=7;w[064]=0;w[4]=02000;
 w[6]=(agc_word)(02000u|(w[3]>>8));w[0]=0;w[1]=agc_extend(l);
 w[2]=02654;w[5]=05705;io->interrupt_enabled=0;
 return EXEC_BAILOUT_PENDING;
}
