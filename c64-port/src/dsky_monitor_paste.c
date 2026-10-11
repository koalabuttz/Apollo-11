/* SPDX-License-Identifier: MIT */
#include "dsky_monitor_paste.h"
#include "lm_interbank.h"
#include "cm_interbank_complete.h"
enum p66_bias_status dsky_monitor_paste(apollo_overlay_state *s,p66_bias_io *io,enum dsky_nv_mission mission,enum dsky_monitor_paste_stage stage)
{uint16_t *w,i;agc_word paste,after,blank,packed;cm_native_io c={0};uint8_t cm;
 if(!s||!io||(unsigned)mission>1u||(unsigned)stage>2u)return P66_BIAS_OUT_OF_SCOPE;
#if defined(DSKY_MONITOR_PASTE_ONLY_STAGE)
 if(stage!=DSKY_MONITOR_PASTE_ONLY_STAGE)return P66_BIAS_OUT_OF_SCOPE;
#endif
 w=s->words;cm=(uint8_t)(mission==DSKY_NV_CM);paste=cm?04140u:04124u;after=cm?04152u:04136u;blank=cm?04156u:04142u;
 if(w[4]!=062000u||w[3]>03400u||(w[3]&0377u)||w[6]!=(agc_word)(062000u|(w[3]>>8))||w[7]||io->superbank<0100u||io->superbank>0160u||(io->superbank&017u)||io->interrupt_enabled>1u)return P66_BIAS_OUT_OF_SCOPE;
 for(i=2;i<2048u;++i)if(w[i]>AGC_MASK)return P66_BIAS_OUT_OF_SCOPE;
 if(stage==DSKY_MONITOR_PASTE_BEGIN){
  if((w[5]!=paste&&(w[5]!=0136u||w[0136]!=paste)))return P66_BIAS_OUT_OF_SCOPE;
  packed=(agc_word)(w[01022]&037600u);if(!packed)packed=w[01020];if(((packed>>7)&0177u)>99u)return P66_BIAS_OUT_OF_SCOPE;
  w[023]=0;w[0]=(agc_word)((packed>>7)&0177u);w[2]=(agc_word)(after-1u);w[5]=cm?04662u:04616u;
  if(cm){c.superbank=io->superbank;c.interrupt_enabled=io->interrupt_enabled;if(!cm_interbank_complete(s,&c,CM_IC_BANKCALL,062337,0))return P66_BIAS_OUT_OF_SCOPE;io->superbank=c.superbank;io->interrupt_enabled=c.interrupt_enabled;}
  else if(!lm_interbank(s,io,LM_IB_BANKCALL,062337,0))return P66_BIAS_OUT_OF_SCOPE;
 }else if(stage==DSKY_MONITOR_PASTE_UPDATED){
  if(w[5]!=(cm?04675u:04631u)||w[0133]!=after||w[0134]!=062000u)return P66_BIAS_OUT_OF_SCOPE;
  if(cm){c.superbank=io->superbank;c.interrupt_enabled=io->interrupt_enabled;if(!cm_interbank_complete(s,&c,CM_IC_SWRETURN,0,0))return P66_BIAS_OUT_OF_SCOPE;io->superbank=c.superbank;io->interrupt_enabled=c.interrupt_enabled;}
  else if(!lm_interbank(s,io,LM_IB_SWRETURN,0,0))return P66_BIAS_OUT_OF_SCOPE;
  w[01013]=0;w[0]=agc_extend(w[01022]);w[2]=blank;w[5]=cm?04271u:04255u;
 }else{
  if(w[5]!=blank&&w[5]!=blank+1u)return P66_BIAS_OUT_OF_SCOPE;
  w[1]=agc_extend(agc_correct(w[1]));w[2]=(agc_word)(blank+2u);w[5]=cm?05217u:05155u;
 }
 return P66_BIAS_OK;
}
