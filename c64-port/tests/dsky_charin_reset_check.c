/* SPDX-License-Identifier: MIT */
#include "dsky_charin_reset_check.h"
#include "dsky_charin_reset_vectors.h"
uint16_t dsky_charin_reset_check(uint8_t n,apollo_overlay_state *s,apollo_overlay_state *before){uint16_t i,p=0,end,number=dsky_charin_reset_numbers[n],errors=0,status;dsky_reset_io io,old;
 dsky_charin_reset_input(s,&io,number);*before=*s;old=io;
 if(number<4096u)status=(uint16_t)dsky_charin_reset(s,&io);else{cm_native_io cm={0};cm.superbank=io.control.superbank;cm.interrupt_enabled=io.control.interrupt_enabled;cm.channel11=io.channel11;cm.channel13=io.channel13;status=(uint16_t)cm_charin_reset(s,&cm,&io.wrote_channels,&io.restart_light);io.control.superbank=cm.superbank;io.control.interrupt_enabled=cm.interrupt_enabled;io.channel11=cm.channel11;io.channel13=cm.channel13;}
 if(status!=0)++errors;
 p=dsky_charin_reset_offsets[n];end=dsky_charin_reset_offsets[n+1u];
 for(i=0;i<2054u;++i){uint16_t want=i<2048u ? before->words[i] : i==2048u ? old.control.superbank : i==2049u ? old.control.interrupt_enabled : i==2050u ? old.channel11 : i==2051u ? old.channel13 : i==2052u ? old.wrote_channels : old.restart_light,actual=i<2048u ? s->words[i] : i==2048u ? io.control.superbank : i==2049u ? io.control.interrupt_enabled : i==2050u ? io.channel11 : i==2051u ? io.channel13 : i==2052u ? io.wrote_channels : io.restart_light;
  if(p<end&&dsky_charin_reset_patch[p][0]==i)want=dsky_charin_reset_patch[p++][1];
  if(actual!=want)++errors;
 }if(p!=end)++errors;return errors;}
