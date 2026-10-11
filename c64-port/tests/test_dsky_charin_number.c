/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "dsky_charin_number_check.h"
static apollo_overlay_state state,before;
int main(void){uint8_t n;uint16_t failures=0;dsky_charin_io io,old;
 for(n=0;n<DSKY_CHARIN_NUMBER_TARGET_COUNT;++n)failures=(uint16_t)(failures+dsky_charin_number_check(n,&state,&before));
 for(n=0;n<7;++n){dsky_charin_number_prepare(&state,73);io.control.superbank=0100;io.control.interrupt_enabled=0;io.channel11=077777;io.wrote_channel11=1;
  switch(n){case 0:state.words[0154]=034;break;case 1:state.words[0777]=15;break;case 2:state.words[0777]=20;break;case 3:state.words[01000]=3;break;case 4:state.words[0777]=19;state.words[01001]=100;break;case 5:state.words[0777]=0;state.words[01005]=10000;break;default:state.words[0154]=8;state.words[01000]=0;break;}
  before=state;old=io;if(dsky_charin_number(&state,&io)!=P66_BIAS_OUT_OF_SCOPE || memcmp(&state,&before,sizeof state) || memcmp(&io,&old,sizeof io))++failures;
 }
 printf("DSKY numeric entry:64 complete2052-field states, %u failures\n",failures);return failures!=0;}
