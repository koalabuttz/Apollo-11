/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "dsky_decimal_general_check.h"
static apollo_overlay_state s,before;
int main(void){p66_bias_io io,old;enum dsky_nv_mission mission;uint8_t rounded,n,g;enum dsky_decimal_general_route route;uint16_t failures=0;
for(n=0;n<128u;++n)failures=(uint16_t)(failures+dsky_decimal_general_checkpoint(n,&s,&before));
for(n=0;n<2u;++n)for(g=0;g<16u;++g){if(!dsky_decimal_general_checkpoint_input((uint8_t)(n*64u),&s,&io,&mission,&rounded))return 2;switch(g){case 0:s.words[5]=0;break;case 1:s.words[4]=1;break;case 2:s.words[3]=1;break;case 3:s.words[6]^=1;break;case 4:s.words[7]=1;break;case 5:io.superbank=0;break;case 6:io.interrupt_enabled=2;break;case 7:mission=(enum dsky_nv_mission)2;break;case 8:rounded=2;break;case 9:s.words[0777]=3;break;case 10:io.superbank=0101;break;case 11:io.superbank=0161;break;case 12:io.superbank=0200;break;case 13:s.words[2]=077;break;case 14:s.words[2]=010000;break;default:s.words[0155]^=040000;break;}before=s;old=io;route=(enum dsky_decimal_general_route)2;if(dsky_decimal_general(&s,&io,mission,rounded,&route)!=P66_BIAS_OUT_OF_SCOPE||memcmp(&s,&before,sizeof s)||memcmp(&io,&old,sizeof io)||route!=(enum dsky_decimal_general_route)2)++failures;}
printf("128 full original decimal caller/CCSHOLE states and32 unchanged invalid guards: %s\n",failures?"FAIL":"PASS");return failures?1:0;}
