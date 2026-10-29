/* SPDX-License-Identifier: MIT */
#include "dsky_mode_admit_check.h"
#include "dsky_mode_admit_vectors.h"
#if defined(DSKY_MODE_ADMIT_TARGET_LM) || defined(DSKY_MODE_ADMIT_TARGET_CM)
volatile uint16_t dsky_mode_admit_difference[4];
#endif
uint16_t dsky_mode_admit_check(uint8_t n,apollo_overlay_state *s,apollo_overlay_state *before){uint16_t i,p,end,number=dsky_mode_admit_numbers[n],errors=0,status,alarm=0;p66_bias_io io,old;enum dsky_mode_admit_route route=DSKY_MODE_ADMIT_CALLER;
 dsky_mode_admit_input(s,&io,number);*before=*s;old=io;
#if defined(DSKY_MODE_ADMIT_TARGET_LM)
 status=(uint16_t)dsky_mode_admit(s,&io,&route);
#elif defined(DSKY_MODE_ADMIT_TARGET_CM)
 {cm_native_io cm={0};cm.superbank=io.superbank;cm.interrupt_enabled=io.interrupt_enabled;status=(uint16_t)cm_mode_admit(s,&cm,&route);io.superbank=cm.superbank;io.interrupt_enabled=cm.interrupt_enabled;alarm=cm.executive_alarm;}
#else
 if(number<4096u)status=(uint16_t)dsky_mode_admit(s,&io,&route);else{cm_native_io cm={0};cm.superbank=io.superbank;cm.interrupt_enabled=io.interrupt_enabled;status=(uint16_t)cm_mode_admit(s,&cm,&route);io.superbank=cm.superbank;io.interrupt_enabled=cm.interrupt_enabled;alarm=cm.executive_alarm;}
#endif
 if(status!=0)++errors;
 p=dsky_mode_admit_offsets[n];end=dsky_mode_admit_offsets[n+1u];
 for(i=0;i<2052u;++i){uint16_t want=i<2048u ? before->words[i] : i==2048u ? old.superbank : i==2049u ? old.interrupt_enabled : 0,actual=i<2048u ? s->words[i] : i==2048u ? io.superbank : i==2049u ? io.interrupt_enabled : i==2050u ? alarm : (uint16_t)route;
  if(p<end&&dsky_mode_admit_patch[p][0]==i)want=dsky_mode_admit_patch[p++][1];
  if(actual!=want){++errors;
#if defined(DSKY_MODE_ADMIT_TARGET_LM) || defined(DSKY_MODE_ADMIT_TARGET_CM)
   if(dsky_mode_admit_difference[0]==0){dsky_mode_admit_difference[0]=(uint16_t)(number+1u);dsky_mode_admit_difference[1]=i;dsky_mode_admit_difference[2]=actual;dsky_mode_admit_difference[3]=want;}
#endif
  }
 }if(p!=end)++errors;return errors;}
