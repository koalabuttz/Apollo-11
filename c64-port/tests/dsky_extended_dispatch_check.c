/* SPDX-License-Identifier: MIT */
#include "dsky_extended_dispatch_check.h"
#include "dsky_extended_dispatch_vectors.h"
uint16_t dsky_extended_dispatch_check(uint8_t n,apollo_overlay_state *s,apollo_overlay_state *before){uint16_t i,p,end,number=dsky_extended_dispatch_numbers[n],errors=0,status;p66_bias_io io,old;
 dsky_extended_dispatch_input(s,&io,number);*before=*s;old=io;
 if(number<4096u)status=(uint16_t)dsky_extended_dispatch(s,&io);else{cm_native_io cm={0};cm.superbank=io.superbank;cm.interrupt_enabled=io.interrupt_enabled;status=(uint16_t)cm_extended_dispatch(s,&cm);io.superbank=cm.superbank;io.interrupt_enabled=cm.interrupt_enabled;}
 if(status!=0)++errors;
 p=dsky_extended_dispatch_offsets[n];end=dsky_extended_dispatch_offsets[n+1u];
 for(i=0;i<2050u;++i){uint16_t want=i<2048u ? before->words[i] : i==2048u ? old.superbank : old.interrupt_enabled,actual=i<2048u ? s->words[i] : i==2048u ? io.superbank : io.interrupt_enabled;
  if(p<end&&dsky_extended_dispatch_patch[p][0]==i)want=dsky_extended_dispatch_patch[p++][1];
  if(actual!=want)++errors;
 }if(p!=end)++errors;return errors;}
