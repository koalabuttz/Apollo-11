/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "dsky_decimal_row_check.h"
static apollo_overlay_state s,before;
int main(void){p66_bias_io io,old;enum dsky_nv_mission mission;enum dsky_decimal_row_entry entry;uint16_t failures=0;uint8_t n,g;
 for(n=0;n<128u;++n)failures=(uint16_t)(failures+dsky_decimal_row_checkpoint(n,&s,&before));
 for(n=0;n<2u;++n)for(g=0;g<13u;++g){if(!dsky_decimal_row_checkpoint_input((uint8_t)(n*64u),&s,&io,&mission,&entry))return 2;switch(g){case 0:s.words[5]=0;break;case 1:s.words[4]=1;break;case 2:s.words[3]=1;break;case 3:s.words[6]^=1;break;case 4:s.words[7]=1;break;case 5:io.superbank=0;break;case 6:io.interrupt_enabled=2;break;case 7:mission=(enum dsky_nv_mission)2;break;case 8:entry=(enum dsky_decimal_row_entry)2;break;case 9:s.words[0117]=3;break;case 10:io.superbank=0101;break;case 11:io.superbank=0161;break;default:io.superbank=0200;break;}before=s;old=io;if(dsky_decimal_row(&s,&io,mission,entry)!=P66_BIAS_OUT_OF_SCOPE||memcmp(&s,&before,sizeof s)||memcmp(&io,&old,sizeof io))++failures;}
 for(n=0;n<2u;++n)for(g=0;g<2u;++g){if(!dsky_decimal_row_checkpoint_input((uint8_t)(n*64u+2u),&s,&io,&mission,&entry))return 2;s.words[g?0134u:0133u]=0;before=s;old=io;if(dsky_decimal_row(&s,&io,mission,entry)!=P66_BIAS_OUT_OF_SCOPE||memcmp(&s,&before,sizeof s)||memcmp(&io,&old,sizeof io))++failures;}
 printf("128 full LM/CM DSPDCEND call/return source states and30 unchanged invalid guards: %s\n",failures?"FAIL":"PASS");return failures?1:0;}
