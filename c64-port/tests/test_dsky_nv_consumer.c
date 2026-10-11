/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "dsky_nv_consumer_check.h"
static apollo_overlay_state s,before;
int main(void){p66_bias_io io,old;agc_word channel11,oldchannel;enum dsky_nv_mission mission;enum dsky_nv_consumer_stage stage;uint16_t failures=0;uint8_t n,g;for(n=0;n<28u;++n)failures=(uint16_t)(failures+dsky_nv_consumer_checkpoint(n,&s,&before));for(n=0;n<4u;++n)for(g=0;g<8u;++g){dsky_nv_consumer_input(&s,&io,&channel11,0,&mission);stage=(enum dsky_nv_consumer_stage)n;switch(g){case 0:s.words[5]=0;break;case 1:channel11=0100000;break;case 2:mission=(enum dsky_nv_mission)2;break;case 3:stage=(enum dsky_nv_consumer_stage)4;break;case 4:s.words[7]=1;break;case 5:s.words[6]^=1;break;case 6:io.interrupt_enabled=2;break;default:io.superbank=1;break;}before=s;old=io;oldchannel=channel11;if(dsky_nv_consumer_step(&s,&io,&channel11,mission,stage)!=P66_BIAS_OUT_OF_SCOPE||memcmp(&s,&before,sizeof s)||memcmp(&io,&old,sizeof io)||channel11!=oldchannel)++failures;}printf("28 continuous LM/CM NVSUB sequences/92 complete checkpoints and32 unchanged invalid guards: %s\n",failures?"FAIL":"PASS");return failures?1:0;}
