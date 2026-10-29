/* SPDX-License-Identifier: MIT */
#include "dsky_major_request_check.h"
#include "dsky_major_request_vectors.h"
uint16_t dsky_major_request_check(uint8_t n,apollo_overlay_state *s,apollo_overlay_state *before){uint16_t i,p,end,number=dsky_major_request_numbers[n],errors=0,status;dsky_charin_io io,old;enum dsky_major_request_route route=DSKY_MAJOR_REQUEST_END;
 dsky_major_request_input(s,&io,number);*before=*s;old=io;
 if(number<4096u)status=(uint16_t)dsky_major_request(s,&io,&route);else{cm_native_io cm={0};cm.superbank=io.control.superbank;cm.interrupt_enabled=io.control.interrupt_enabled;cm.channel11=io.channel11;status=(uint16_t)cm_major_request(s,&cm,&io.wrote_channel11,&route);io.control.superbank=cm.superbank;io.control.interrupt_enabled=cm.interrupt_enabled;io.channel11=cm.channel11;}
 if(status!=0)++errors;
 p=dsky_major_request_offsets[n];end=dsky_major_request_offsets[n+1u];
 for(i=0;i<2053u;++i){uint16_t want=i<2048u ? before->words[i] : i==2048u ? old.control.superbank : i==2049u ? old.control.interrupt_enabled : i==2050u ? old.channel11 : i==2051u ? old.wrote_channel11 : 0,actual=i<2048u ? s->words[i] : i==2048u ? io.control.superbank : i==2049u ? io.control.interrupt_enabled : i==2050u ? io.channel11 : i==2051u ? io.wrote_channel11 : (uint16_t)route;
  if(p<end&&dsky_major_request_patch[p][0]==i)want=dsky_major_request_patch[p++][1];
  if(actual!=want)++errors;
 }if(p!=end)++errors;return errors;}
