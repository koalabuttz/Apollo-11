/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "dsky_read_pair_check.h"
static apollo_overlay_state s,before;
int main(void){uint16_t failures=0,guards=0;uint8_t n,g;enum dsky_nv_mission mission;p66_bias_io io,old;
 for(n=0;n<128u;++n)failures=(uint16_t)(failures+dsky_read_pair_checkpoint(n,&s,&before));
 for(n=0;n<2u;++n)for(g=0;g<18u;++g){dsky_read_pair_input(&s,&io,(uint16_t)(n*8192u),&mission);
 switch(g){case 0:s.words[5]=0;break;case 1:s.words[4]=0;break;case 2:s.words[3]=1;break;case 3:s.words[6]^=1;break;case 4:s.words[7]=1;break;case 5:io.superbank=0;break;case 6:io.superbank=0101;break;case 7:io.superbank=0200;break;case 8:io.interrupt_enabled=2;break;case 9:mission=(enum dsky_nv_mission)2;break;case 10:s.words[0117]=3;break;case 11:s.words[2]=077;break;case 12:s.words[2]=010000;break;case 13:s.words[0140]=0;break;case 14:s.words[030]=0100000;break;case 15:s.words[0145]=0100-1;break;case 16:s.words[0145]=0377;break;default:s.words[0145]=02000;break;}
 before=s;old=io;++guards;if(dsky_read_pair(&s,&io,mission)!=P66_BIAS_OUT_OF_SCOPE||memcmp(&s,&before,sizeof s)||memcmp(&io,&old,sizeof io))++failures;
 }
 printf("128 complete original fresh READLO pair states + %u unchanged invalid guards: %s\n",guards,failures?"FAIL":"PASS");return failures?1:0;
}
