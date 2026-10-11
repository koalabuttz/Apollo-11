/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "dsky_nv_return_check.h"
static apollo_overlay_state s,before;
int main(void){p66_bias_io io,old;enum dsky_nv_mission mission;uint16_t failures=0;uint8_t n,g;for(n=0;n<64u;++n)failures=(uint16_t)(failures+dsky_nv_return_checkpoint(n,&s,&before));for(n=0;n<2u;++n)for(g=0;g<11u;++g){if(!dsky_nv_return_checkpoint_input((uint8_t)(n*32u),&s,&io,&mission))return 2;switch(g){case 0:s.words[5]=0;break;case 1:s.words[4]=1;break;case 2:s.words[3]=1;break;case 3:s.words[6]^=1;break;case 4:s.words[7]=1;break;case 5:io.superbank=1;break;case 6:io.interrupt_enabled=2;break;case 7:mission=(enum dsky_nv_mission)2;break;case 8:s.words[01037]=01777;break;case 9:s.words[01037]=010000;break;default:s.words[01040]|=010;break;}before=s;old=io;if(dsky_nv_return(&s,&io,mission)!=P66_BIAS_OUT_OF_SCOPE||memcmp(&s,&before,sizeof s)||memcmp(&io,&old,sizeof io))++failures;}printf("64 full LM/CM NVSUBEND source states and22 unchanged invalid guards: %s\n",failures?"FAIL":"PASS");return failures?1:0;}
