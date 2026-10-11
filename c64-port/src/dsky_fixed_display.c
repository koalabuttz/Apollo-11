/* SPDX-License-Identifier: MIT */
#include "dsky_fixed_display.h"
#include "lm_interbank.h"
#include "cm_fixed_callers.h"
#include "dsky_octal_display.h"
enum p66_bias_status dsky_fixed_display(apollo_overlay_state *s,p66_bias_io *io,enum dsky_nv_mission mission,enum dsky_fixed_display_stage stage,agc_word value,enum dsky_fixed_display_route *route)
{
 uint16_t *w,i;uint8_t cm;enum dsky_octal_route octal;cm_native_io c={0};
 if(!s||!io||!route||(unsigned)mission>1u||(unsigned)stage>2u)return P66_BIAS_OUT_OF_SCOPE;
#if defined(DSKY_FIXED_DISPLAY_ONLY_STAGE)
 if(stage!=DSKY_FIXED_DISPLAY_ONLY_STAGE)return P66_BIAS_OUT_OF_SCOPE;
#endif
 w=s->words;cm=(uint8_t)(mission==DSKY_NV_CM);
 if(w[5]!=(stage==DSKY_FIXED_ENTER?03353u:stage==DSKY_FIXED_DATA?(cm?04715u:04651u):03361u)||w[4]!=062000u||w[3]>03400u||(w[3]&0377u)||w[6]!=(agc_word)(062000u|(w[3]>>8))||w[7]||io->superbank<0100u||io->superbank>0160u||(io->superbank&017u)||io->interrupt_enabled>1u)return P66_BIAS_OUT_OF_SCOPE;
 for(i=2;i<2048u;++i)if(w[i]>AGC_MASK)return P66_BIAS_OUT_OF_SCOPE;
 if(stage==DSKY_FIXED_ENTER){
  if(cm){c.superbank=io->superbank;c.interrupt_enabled=io->interrupt_enabled;if((unsigned)cm_fixed_caller_enter(s,&c,0)!=(unsigned)EXEC_OK)return P66_BIAS_OUT_OF_SCOPE;io->superbank=c.superbank;io->interrupt_enabled=c.interrupt_enabled;}
  else{w[0777]=14;w[1]=agc_extend(w[01047]);w[0]=agc_extend(w[01017]);w[2]=03361;w[5]=04651;}
  *route=DSKY_FIXED_DATA_ENTRY;
 }else if(stage==DSKY_FIXED_DATA){uint16_t address;
  if(value>AGC_MASK||!dsky_fixed_display_address(s,io,mission,&address))return P66_BIAS_OUT_OF_SCOPE;
  (void)address;
  if(cm){c.superbank=io->superbank;c.interrupt_enabled=io->interrupt_enabled;if((unsigned)cm_supdacal_data(s,&c,value)!=(unsigned)EXEC_OK)return P66_BIAS_OUT_OF_SCOPE;io->superbank=c.superbank;io->interrupt_enabled=c.interrupt_enabled;}
  else if(!lm_interbank(s,io,LM_IB_SUPDACAL,0,value))return P66_BIAS_OUT_OF_SCOPE;
  *route=DSKY_FIXED_READ_READY;
 }else{
  if(w[0]!=agc_extend(agc_correct(w[0]))||w[0777]<4u||w[0777]>21u)return P66_BIAS_OUT_OF_SCOPE;
  w[2]=03362;w[5]=03363;
  if(cm){c.superbank=io->superbank;c.interrupt_enabled=io->interrupt_enabled;if((unsigned)cm_octal_display(s,&c,&octal)!=(unsigned)EXEC_OK)return P66_BIAS_OUT_OF_SCOPE;io->superbank=c.superbank;io->interrupt_enabled=c.interrupt_enabled;}
  else if(dsky_octal_display(s,io,&octal)!=P66_BIAS_OK)return P66_BIAS_OUT_OF_SCOPE;
  if(octal==DSKY_OCTAL_CCS_HOLE)*route=DSKY_FIXED_CCS_HOLE;
  else{w[2]=03363;w[5]=cm?05217u:05155u;*route=DSKY_FIXED_END;}
 }
 return P66_BIAS_OK;
}

bool dsky_fixed_display_address(const apollo_overlay_state *s,const p66_bias_io *io,enum dsky_nv_mission mission,uint16_t *out)
{cm_native_io c={0};if((unsigned)mission>1u||!s||!io||!out||s->words[2]!=03361u||agc_overflow(s->words[0]))return false;if(mission==DSKY_NV_LM)return lm_interbank_data_address(s,io,out);c.superbank=io->superbank;c.interrupt_enabled=io->interrupt_enabled;return (unsigned)cm_supdacal_address(s,&c,out)==(unsigned)EXEC_OK;}
