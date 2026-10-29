/* SPDX-License-Identifier: MIT */
#include "dsky_charin_alarm_check.h"
#include "dsky_charin_alarm_vectors.h"
uint16_t dsky_charin_alarm_check(uint8_t n,apollo_overlay_state *s,apollo_overlay_state *before){uint16_t i,p=0,end,number=dsky_charin_alarm_numbers[n],errors=0,status;dsky_charin_io io,old;
 dsky_charin_alarm_input(s,&io,number);*before=*s;old=io;
 if(number<4096u)status=(uint16_t)dsky_charin_alarm(s,&io);else{cm_native_io cm={0};cm.superbank=io.control.superbank;cm.interrupt_enabled=io.control.interrupt_enabled;cm.channel11=io.channel11;status=(uint16_t)cm_charin_alarm(s,&cm,&io.wrote_channel11);io.control.superbank=cm.superbank;io.control.interrupt_enabled=cm.interrupt_enabled;io.channel11=cm.channel11;}
 if(status!=0)++errors;
 p=dsky_charin_alarm_offsets[n];end=dsky_charin_alarm_offsets[n+1u];
 for(i=0;i<2052u;++i){uint16_t want=i<2048u ? before->words[i] : i==2048u ? old.control.superbank : i==2049u ? old.control.interrupt_enabled : i==2050u ? old.channel11 : old.wrote_channel11,actual=i<2048u ? s->words[i] : i==2048u ? io.control.superbank : i==2049u ? io.control.interrupt_enabled : i==2050u ? io.channel11 : io.wrote_channel11;
  if(p<end&&dsky_charin_alarm_patch[p][0]==i)want=dsky_charin_alarm_patch[p++][1];
  if(actual!=want)++errors;
 }if(p!=end)++errors;return errors;}
