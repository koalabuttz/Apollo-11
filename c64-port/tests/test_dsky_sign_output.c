/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "dsky_sign_output_check.h"
static apollo_overlay_state s,before;
int main(void){uint16_t failures=0,guards=0;uint8_t n,g,negative;enum dsky_nv_mission mission;p66_bias_io io,old;enum dsky_sign_output_route route,prior;
 for(n=0;n<128u;++n)failures=(uint16_t)(failures+dsky_sign_output_checkpoint(n,&s,&before));
 for(n=0;n<2u;++n)for(g=0;g<16u;++g){dsky_sign_output_input(&s,&io,(uint16_t)(n*8192u),&mission,&negative);route=prior=(enum dsky_sign_output_route)2;
 switch(g){case 0:s.words[5]=0;break;case 1:s.words[4]=0;break;case 2:s.words[3]=1;break;case 3:s.words[6]^=1;break;case 4:s.words[7]=1;break;case 5:io.superbank=0;break;case 6:io.superbank=0101;break;case 7:io.superbank=0200;break;case 8:io.interrupt_enabled=2;break;case 9:mission=(enum dsky_nv_mission)2;break;case 10:negative=2;break;case 11:s.words[2]=077;break;case 12:s.words[2]=010000;break;case 13:s.words[0777]=15;break;case 14:s.words[030]=0100000;break;default:s.words[3]=04000;break;}
 before=s;old=io;++guards;if(dsky_sign_output(&s,&io,mission,negative,&route)!=P66_BIAS_OUT_OF_SCOPE||memcmp(&s,&before,sizeof s)||memcmp(&io,&old,sizeof io)||route!=prior)++failures;
 }
 printf("128 complete original data-row sign output states + %u unchanged invalid guards: %s\n",guards,failures?"FAIL":"PASS");return failures?1:0;
}
