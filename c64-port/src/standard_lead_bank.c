/* SPDX-License-Identifier: MIT; compiled original control, no word decoder. */
#include "standard_lead_bank.h"
#include "cm_interbank_extended.h"
static bool valid(const apollo_overlay_state *s,const p66_bias_io *io){
 return s&&io&&s->words[7]==0&&s->words[2]<=07775u&&s->words[3]<=03400u&&!(s->words[3]&0377u)&&!(s->words[4]&~076000u)&&s->words[6]==(agc_word)(s->words[4]|(s->words[3]>>8))&&!(io->superbank&~0160u)&&io->interrupt_enabled<=1u;
}
static bool control(apollo_overlay_state *s,p66_bias_io *io,enum standard_lead_mission mission,bool returning){
 agc_word oldfb,cadr;cm_native_io cmio={0};
 if((unsigned)mission>1u||!valid(s,io)||s->words[5]!=(mission==STANDARD_LEAD_LM?(returning?04631u:04622u):(returning?04675u:04666u)))return false;
 if(returning&&s->words[0133]>07777u)return false;
 if(mission==STANDARD_LEAD_CM){cmio.superbank=io->superbank;cmio.interrupt_enabled=io->interrupt_enabled;
  return cm_interbank_extended(s,&cmio,returning?CM_IB_SWRETURN:CM_IB_SWCALL,0,0)==EXEC_OK;
 }
 /* E/BKCALL's CA supplies ordinary A at SWCALL. The unbounded generic TS
  * overflow branch is outside this LM caller contract, checked before stores. */
 if(!returning&&agc_overflow(s->words[0]))return false;
 oldfb=s->words[4];s->words[1]=agc_extend(agc_correct(s->words[1]));
 if(returning){s->words[4]=(agc_word)(s->words[0134]&076000u);s->words[0134]=oldfb;s->words[0]=agc_extend(agc_correct(s->words[0]));s->words[2]=0134;s->words[5]=s->words[0133];}
 else{cadr=agc_correct(s->words[0]);s->words[4]=(agc_word)(cadr&076000u);s->words[0]=agc_extend(s->words[0133]);s->words[1]=agc_extend(s->words[0134]);s->words[0133]=s->words[2];s->words[0134]=oldfb;s->words[2]=04631;s->words[5]=(agc_word)(02000u+(cadr&01777u));}
 s->words[6]=(agc_word)(s->words[4]|(s->words[3]>>8));return true;
}
bool standard_lead_bank_call(apollo_overlay_state *s,p66_bias_io *io,enum standard_lead_mission mission){return control(s,io,mission,false);}
bool standard_lead_bank_return(apollo_overlay_state *s,p66_bias_io *io,enum standard_lead_mission mission){return control(s,io,mission,true);}
