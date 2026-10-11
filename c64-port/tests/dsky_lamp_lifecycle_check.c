/* SPDX-License-Identifier: MIT */
#include "dsky_lamp_lifecycle_check.h"
#include "dsky_lamp_lifecycle_vectors.h"
#if defined(DSKY_LAMP_LIFECYCLE_TARGET_LM) || defined(DSKY_LAMP_LIFECYCLE_TARGET_CM)
volatile uint16_t dsky_lamp_lifecycle_difference[4];
#endif
uint16_t dsky_lamp_lifecycle_check(uint8_t n,apollo_overlay_state *s,apollo_overlay_state *before){uint16_t i,p,end,number=dsky_lamp_lifecycle_numbers[n],errors=0,status,alarm=0,waitalarm=0;dsky_lamp_io io,old;enum dsky_lamp_lifecycle_route route=DSKY_LAMP_LIFECYCLE_END;
 dsky_lamp_lifecycle_input(s,&io,number);*before=*s;old=io;
#if defined(DSKY_LAMP_LIFECYCLE_TARGET_LM)
 status=(uint16_t)dsky_lamp_lifecycle(s,&io,&route);
#elif defined(DSKY_LAMP_LIFECYCLE_TARGET_CM)
 {cm_native_io cm={0};cm.superbank=io.control.superbank;cm.interrupt_enabled=io.control.interrupt_enabled;cm.channel11=io.channel11;cm.channel12=io.channel12;cm.channel13=io.channel13;status=(uint16_t)cm_lamp_lifecycle(s,&cm,&io.wrote_channels,&io.restart_light,&route);io.control.superbank=cm.superbank;io.control.interrupt_enabled=cm.interrupt_enabled;io.channel11=cm.channel11;io.channel13=cm.channel13;alarm=cm.executive_alarm;waitalarm=cm.waitlist_alarm;}
#else
 if(number<4096u)status=(uint16_t)dsky_lamp_lifecycle(s,&io,&route);else{cm_native_io cm={0};cm.superbank=io.control.superbank;cm.interrupt_enabled=io.control.interrupt_enabled;cm.channel11=io.channel11;cm.channel12=io.channel12;cm.channel13=io.channel13;status=(uint16_t)cm_lamp_lifecycle(s,&cm,&io.wrote_channels,&io.restart_light,&route);io.control.superbank=cm.superbank;io.control.interrupt_enabled=cm.interrupt_enabled;io.channel11=cm.channel11;io.channel13=cm.channel13;alarm=cm.executive_alarm;waitalarm=cm.waitlist_alarm;}
#endif
 if(status!=0)++errors;
 p=dsky_lamp_lifecycle_offsets[n];end=dsky_lamp_lifecycle_offsets[n+1u];
 for(i=0;i<2058u;++i){uint16_t want=i<2048u ? before->words[i] : i==2048u ? old.control.superbank : i==2049u ? old.control.interrupt_enabled : i==2050u ? old.channel11 : i==2051u ? old.channel12 : i==2052u ? old.channel13 : i==2053u ? old.wrote_channels : i==2054u ? old.restart_light : 0,actual=i<2048u ? s->words[i] : i==2048u ? io.control.superbank : i==2049u ? io.control.interrupt_enabled : i==2050u ? io.channel11 : i==2051u ? io.channel12 : i==2052u ? io.channel13 : i==2053u ? io.wrote_channels : i==2054u ? io.restart_light : i==2055u ? alarm : i==2056u ? waitalarm : (uint16_t)route;
  if(p<end&&dsky_lamp_lifecycle_patch[p][0]==i)want=dsky_lamp_lifecycle_patch[p++][1];
  if(actual!=want){++errors;
#if defined(DSKY_LAMP_LIFECYCLE_TARGET_LM) || defined(DSKY_LAMP_LIFECYCLE_TARGET_CM)
   if(dsky_lamp_lifecycle_difference[0]==0){dsky_lamp_lifecycle_difference[0]=(uint16_t)(number+1u);dsky_lamp_lifecycle_difference[1]=i;dsky_lamp_lifecycle_difference[2]=actual;dsky_lamp_lifecycle_difference[3]=want;}
#endif
  }
 }if(p!=end)++errors;return errors;}
