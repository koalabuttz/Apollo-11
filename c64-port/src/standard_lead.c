/* SPDX-License-Identifier: MIT */
#include "standard_lead.h"
static void exchange(agc_word *w,uint16_t address){
 agc_word a=w[address],l=w[address+1u];w[address]=agc_correct(w[0]);w[address+1u]=agc_correct(w[1]);w[0]=agc_extend(a);w[1]=agc_extend(l);
}
static agc_word inline_word(agc_word *w,agc_word logical,agc_word fixed){
 if(logical<01400u)return w[logical];
 if(logical<02000u)return w[(uint16_t)(w[3]+(logical&0377u))];
 return fixed;
}
bool standard_lead_stage(apollo_overlay_state *s,enum standard_lead_mission mission,enum standard_lead_phase phase,agc_word fixed){
 agc_word *w,ebuf,bank_entry,bank_return,interp_entry,interp_resume,wake_return,value,oldl,bb;
 uint16_t address;unsigned path;
 if(!s||(unsigned)mission>1u||(unsigned)phase>STANDARD_LEAD_WAKE_RETURN||fixed>AGC_MASK)return false;
 w=s->words;ebuf=mission==STANDARD_LEAD_LM?01167:0364;bank_entry=mission==STANDARD_LEAD_LM?02576:02036;bank_return=mission==STANDARD_LEAD_LM?02607:02047;interp_entry=mission==STANDARD_LEAD_LM?02615:02055;interp_resume=mission==STANDARD_LEAD_LM?02627:02067;wake_return=mission==STANDARD_LEAD_LM?02632:02607;
 if(w[7]!=0||w[3]>03400u||(w[3]&0377u)||(w[4]&~076000u)||w[6]!=(agc_word)(w[4]|(w[3]>>8)))return false;
 if(w[4]!=(phase==STANDARD_LEAD_WAKE_RETURN?010000u:mission==STANDARD_LEAD_LM?010000u:026000u))return false;
 if(phase==STANDARD_LEAD_BANK_PREPARE){
  if(w[5]!=bank_entry||w[0133]<0100u||w[0133]>07775u)return false;
  exchange(w,0133);exchange(w,ebuf);w[ebuf]=agc_add_word(w[ebuf],1);
  w[0]=agc_add_register(agc_extend((agc_word)(w[6]&01777u)),agc_extend(w[ebuf+1u]));w[ebuf+1u]=agc_correct(w[0]);
  address=(uint16_t)(agc_add_word(w[ebuf],AGC_MASK-1u)&07777u);w[0]=agc_extend(inline_word(w,address,fixed));w[2]=bank_return;w[5]=mission==STANDARD_LEAD_LM?04622:04666;return true;
 }
 if(phase>=STANDARD_LEAD_BANK_RETURN0&&phase<=STANDARD_LEAD_BANK_RETURN2){
  path=(unsigned)phase-STANDARD_LEAD_BANK_RETURN0;
  if(w[5]!=(agc_word)(bank_return+path)||w[ebuf]<0100u||w[ebuf]>07775u)return false;
  if(path<2u)w[2]=(agc_word)(bank_return+path+1u);
  if(path)w[ebuf]=agc_add_word(w[ebuf],1);
  if(path==2u)w[ebuf]=agc_add_word(w[ebuf],1);
  exchange(w,ebuf);value=agc_correct(w[0]);bb=agc_correct(w[1]);w[0]=(agc_word)(bank_return+6u);w[1]=w[6];w[5]=(agc_word)(value&07777u);w[6]=(agc_word)(bb&076007u);w[3]=(agc_word)((bb&7u)<<8);w[4]=(agc_word)(bb&076000u);return true;
 }
 if(phase==STANDARD_LEAD_INTERP_PREPARE){
  if(w[5]!=interp_entry||w[0164]<077u||w[0164]>07774u)return false;
  oldl=w[1];w[1]=agc_extend(w[0164]);w[0164]=agc_correct(oldl);
  address=(uint16_t)(agc_add_word(agc_correct(w[1]),1)&07777u);w[0]=agc_extend(inline_word(w,address,fixed));
  w[1]=agc_add_register(w[1],1);w[1]=agc_add_register(w[1],1);exchange(w,ebuf);w[2]=(agc_word)(interp_entry+7u);w[5]=mission==STANDARD_LEAD_LM?06037:06006;return true;
 }
 if(phase==STANDARD_LEAD_INTERP_RESUME){
  if(w[5]!=interp_resume)return false;
  value=w[ebuf+1u];w[ebuf+1u]=agc_correct(w[1]);w[1]=agc_extend(value);w[5]=mission==STANDARD_LEAD_LM?06041:06010;return true;
 }
 if(w[5]!=wake_return||(w[064]!=077776u&&(w[064]>(mission==STANDARD_LEAD_LM?84u:72u)||w[064]%12u)))return false;
 address=(uint16_t)(agc_add_word(0164u,w[064])&01777u);w[0]=agc_add_register(agc_extend((agc_word)(AGC_MASK^02000u)),agc_extend(w[address]));w[address]=agc_correct(w[0]);w[1]=agc_extend(agc_correct(w[1]));w[2]=(agc_word)(wake_return+4u);w[5]=072;return true;
}
