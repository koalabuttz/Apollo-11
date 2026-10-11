/* SPDX-License-Identifier: MIT */
#include "dsky_pinbranch_check.h"
#include "dsky_pinbranch_vectors.h"
uint16_t dsky_pinbranch_check(uint8_t n,apollo_overlay_state *s,apollo_overlay_state *before){uint16_t i,p,end,number=dsky_pinbranch_numbers[n],errors=0,status;p66_bias_io io,old;enum dsky_pinbranch_route route=DSKY_PINBRANCH_ERASE;
 dsky_pinbranch_input(s,&io,number);*before=*s;old=io;
 if(number<8192u)status=(uint16_t)(((number/4096u)%2u) ? dsky_pinbranch_resume(s,&io,&route) : dsky_pinbranch(s,&io,&route));else{cm_native_io cm={0};cm.superbank=io.superbank;cm.interrupt_enabled=io.interrupt_enabled;status=(uint16_t)(((number/4096u)%2u) ? cm_pinbranch_resume(s,&cm,&route) : cm_pinbranch(s,&cm,&route));io.superbank=cm.superbank;io.interrupt_enabled=cm.interrupt_enabled;}
 if(status!=0)++errors;
 p=dsky_pinbranch_offsets[n];end=dsky_pinbranch_offsets[n+1u];
 for(i=0;i<2051u;++i){uint16_t want=i<2048u ? before->words[i] : i==2048u ? old.superbank : i==2049u ? old.interrupt_enabled : 0,actual=i<2048u ? s->words[i] : i==2048u ? io.superbank : i==2049u ? io.interrupt_enabled : (uint16_t)route;
  if(p<end&&dsky_pinbranch_patch[p][0]==i)want=dsky_pinbranch_patch[p++][1];
  if(actual!=want)++errors;
 }if(p!=end)++errors;return errors;}
