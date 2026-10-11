/* SPDX-License-Identifier: MIT */
#include <string.h>
#include "dsky_relay_check.h"
#include "dsky_relay_vectors.h"
uint16_t dsky_relay_check(uint8_t n,apollo_overlay_state *s,apollo_overlay_state *before){
 uint16_t f,patch,end,failures=0,number;dsky_relay_io io;uint16_t values[4],old[4];
 if(n>=DSKY_RELAY_TARGET_COUNT)return 1;
 number=dsky_relay_numbers[n];dsky_relay_prepare(s,number);memcpy(before,s,sizeof *s);
 io.control.superbank=(uint16_t)((number/5u)%8u*020u);io.control.interrupt_enabled=(uint16_t)(number%2u);io.wrote_channel10=0;io.channel10=0;
 old[0]=io.control.superbank;old[1]=io.control.interrupt_enabled;old[2]=0;old[3]=0;
 if(dsky_relay_drain(s,&io)!=dsky_relay_results[n])++failures;
 values[0]=io.control.superbank;values[1]=io.control.interrupt_enabled;values[2]=io.wrote_channel10;values[3]=io.channel10;
 patch=dsky_relay_offsets[n];end=dsky_relay_offsets[n+1u];
 for(f=0;f<APOLLO_SHARED_WORDS+4u;++f){uint16_t expected=f<APOLLO_SHARED_WORDS ? before->words[f] : old[f-APOLLO_SHARED_WORDS],actual=f<APOLLO_SHARED_WORDS ? s->words[f] : values[f-APOLLO_SHARED_WORDS];
  if(patch<end && dsky_relay_patch[patch][0]==f)expected=dsky_relay_patch[patch++][1];
  if(actual!=expected)++failures;
 }
 if(patch!=end)++failures;
 return failures;
}
