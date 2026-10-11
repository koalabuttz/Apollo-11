/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "dsky_test_noun_check.h"
static apollo_overlay_state s,before;
int main(void){uint16_t n,fail=0;unsigned m,g;for(n=0;n<256u;++n)fail=(uint16_t)(fail+dsky_test_noun_checkpoint(n,&s,&before));for(m=0;m<2u;++m)for(g=0;g<11u;++g){p66_bias_io io,old;enum dsky_nv_mission mission;enum dsky_enter_zero_route route=(enum dsky_enter_zero_route)99;dsky_test_noun_input(&s,&io,(uint16_t)(m*8192u),&mission);switch(g){case 0:mission=(enum dsky_nv_mission)2;break;case 1:s.words[5]=0;break;case 2:s.words[4]=0;break;case 3:s.words[3]=1;break;case 4:s.words[6]^=1;break;case 5:s.words[7]=1;break;case 6:io.superbank=0;break;case 7:io.interrupt_enabled=2;break;case 8:s.words[0200]=0100000;break;case 9:s.words[01002]=100;break;default:s.words[01002]=040000;break;}before=s;old=io;if(dsky_test_noun(&s,&io,mission,&route)!=P66_BIAS_OUT_OF_SCOPE||memcmp(&s,&before,sizeof s)||memcmp(&io,&old,sizeof io)||route!=(enum dsky_enter_zero_route)99)++fail;}printf("256 full original direct TESTNN states +22 unchanged invalid guards: %s\n",fail?"FAIL":"PASS");return fail?1:0;}
