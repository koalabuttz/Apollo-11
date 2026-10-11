/* SPDX-License-Identifier: MIT */
#include "dsky_machine_cadr_check.h"
#include "dsky_machine_cadr_vectors.h"
uint16_t dsky_machine_cadr_check(uint8_t n,apollo_overlay_state *s,apollo_overlay_state *before){uint16_t i,p,end,number=dsky_machine_cadr_numbers[n],errors=0,status;p66_bias_io io,old;enum dsky_machine_route route=DSKY_MACHINE_VERB_FAN;
 dsky_machine_cadr_input(s,&io,number);*before=*s;old=io;
 if(number<4096u)status=(uint16_t)dsky_machine_cadr(s,&io,&route);else{cm_native_io cm={0};cm.superbank=io.superbank;cm.interrupt_enabled=io.interrupt_enabled;status=(uint16_t)cm_machine_cadr(s,&cm,&route);io.superbank=cm.superbank;io.interrupt_enabled=cm.interrupt_enabled;}
 if(status!=0)++errors;
 p=dsky_machine_cadr_offsets[n];end=dsky_machine_cadr_offsets[n+1u];
 for(i=0;i<2051u;++i){uint16_t want=i<2048u ? before->words[i] : i==2048u ? old.superbank : i==2049u ? old.interrupt_enabled : 0,actual=i<2048u ? s->words[i] : i==2048u ? io.superbank : i==2049u ? io.interrupt_enabled : (uint16_t)route;
  if(p<end&&dsky_machine_cadr_patch[p][0]==i)want=dsky_machine_cadr_patch[p++][1];
  if(actual!=want)++errors;
 }if(p!=end)++errors;return errors;}
