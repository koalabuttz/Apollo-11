/* SPDX-License-Identifier: MIT */
#include "dsky_operator_release_check.h"
#include "dsky_operator_release_vectors.h"
#if defined(DSKY_OPERATOR_RELEASE_TARGET_LM) || defined(DSKY_OPERATOR_RELEASE_TARGET_CM)
volatile uint16_t dsky_operator_release_difference[4];
#endif
uint16_t dsky_operator_release_check(uint8_t n,apollo_overlay_state *s,apollo_overlay_state *before){uint16_t i,p,end,number=dsky_operator_release_numbers[n],errors=0,status;dsky_lamp_io io,old;enum dsky_operator_release_route route=DSKY_OPERATOR_RELEASE_END;
 dsky_operator_release_input(s,&io,number);*before=*s;old=io;
#if defined(DSKY_OPERATOR_RELEASE_TARGET_LM)
 status=(uint16_t)dsky_operator_release(s,&io,&route);
#elif defined(DSKY_OPERATOR_RELEASE_TARGET_CM)
 {cm_native_io cm={0};cm.superbank=io.control.superbank;cm.interrupt_enabled=io.control.interrupt_enabled;cm.channel11=io.channel11;cm.channel12=io.channel12;cm.channel13=io.channel13;status=(uint16_t)cm_operator_release(s,&cm,&io.wrote_channels,&io.restart_light,&route);io.control.superbank=cm.superbank;io.control.interrupt_enabled=cm.interrupt_enabled;io.channel11=cm.channel11;io.channel13=cm.channel13;}
#else
 if(number<4096u)status=(uint16_t)dsky_operator_release(s,&io,&route);else{cm_native_io cm={0};cm.superbank=io.control.superbank;cm.interrupt_enabled=io.control.interrupt_enabled;cm.channel11=io.channel11;cm.channel12=io.channel12;cm.channel13=io.channel13;status=(uint16_t)cm_operator_release(s,&cm,&io.wrote_channels,&io.restart_light,&route);io.control.superbank=cm.superbank;io.control.interrupt_enabled=cm.interrupt_enabled;io.channel11=cm.channel11;io.channel13=cm.channel13;}
#endif
 if(status!=0)++errors;
 p=dsky_operator_release_offsets[n];end=dsky_operator_release_offsets[n+1u];
 for(i=0;i<2056u;++i){uint16_t want=i<2048u ? before->words[i] : i==2048u ? old.control.superbank : i==2049u ? old.control.interrupt_enabled : i==2050u ? old.channel11 : i==2051u ? old.channel12 : i==2052u ? old.channel13 : i==2053u ? old.wrote_channels : i==2054u ? old.restart_light : 0,actual=i<2048u ? s->words[i] : i==2048u ? io.control.superbank : i==2049u ? io.control.interrupt_enabled : i==2050u ? io.channel11 : i==2051u ? io.channel12 : i==2052u ? io.channel13 : i==2053u ? io.wrote_channels : i==2054u ? io.restart_light : (uint16_t)route;
  if(p<end&&dsky_operator_release_patch[p][0]==i)want=dsky_operator_release_patch[p++][1];
  if(actual!=want){++errors;
#if defined(DSKY_OPERATOR_RELEASE_TARGET_LM) || defined(DSKY_OPERATOR_RELEASE_TARGET_CM)
   if(dsky_operator_release_difference[0]==0){dsky_operator_release_difference[0]=(uint16_t)(number+1u);dsky_operator_release_difference[1]=i;dsky_operator_release_difference[2]=actual;dsky_operator_release_difference[3]=want;}
#endif
  }
 }if(p!=end)++errors;return errors;}
