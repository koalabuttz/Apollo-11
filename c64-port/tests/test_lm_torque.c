/* SPDX-License-Identifier: MIT */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "lm_torque_cases.h"
int main(void){static apollo_overlay_state s,before;lm_torque_io io,saved;uint16_t n;
 for(n=0;n<16u;++n){lm_torque_prepare(&s,&io,123);switch(n){
 case 0:s.words[3]=03401;break;case 1:s.words[3]=04000;break;
 case 2:s.words[4]=036000;break;case 3:s.words[6]^=1;break;
 case 4:s.words[5]++;break;case 5:s.words[7]=1;break;
 case 6:io.interrupt_enabled=1;break;case 7:io.superbank=1;break;
 case 8:io.superbank=0200;break;default:s.words[0400u+n]=0100000;break;}
 before=s;saved=io;assert(!lm_torque(&s,&io));assert(!memcmp(&s,&before,sizeof s));assert(!memcmp(&io,&saved,sizeof io));}
 lm_torque_prepare(&s,&io,0);before=s;saved=io;assert(!lm_torque(0,&io));assert(!lm_torque(&s,0));
 assert(!memcmp(&s,&before,sizeof s));assert(!memcmp(&io,&saved,sizeof io));
 puts("PASS: rejected contexts preserve the full image and external latches");return 0;}
