/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "dsky_scalar_output_check.h"
static apollo_overlay_state s,before;
int main(void){p66_bias_io io,old;enum dsky_nv_mission mission;enum dsky_scalar_output_entry entry;agc_word channel;uint16_t failures=0;uint8_t n,g;
 for(n=0;n<160u;++n)failures=(uint16_t)(failures+dsky_scalar_output_checkpoint(n,&s,&before));
 for(n=0;n<2u;++n)for(g=0;g<13u;++g){if(!dsky_scalar_output_checkpoint_input((uint8_t)(n*80u),&s,&io,&mission,&entry,&channel))return 2;switch(g){case 0:s.words[5]=0;break;case 1:s.words[4]=1;break;case 2:s.words[3]=1;break;case 3:s.words[6]^=1;break;case 4:s.words[7]=1;break;case 5:io.superbank=0;break;case 6:io.interrupt_enabled=2;break;case 7:mission=(enum dsky_nv_mission)2;break;case 8:entry=(enum dsky_scalar_output_entry)6;break;case 9:entry=mission==DSKY_NV_LM?DSKY_SCALAR_FRACTION:DSKY_SCALAR_LANDING_POSITION;break;case 10:io.superbank=0101;break;case 11:io.superbank=0161;break;default:io.superbank=0200;break;}before=s;old=io;if(dsky_scalar_output(&s,&io,mission,entry,channel)!=P66_BIAS_OUT_OF_SCOPE||memcmp(&s,&before,sizeof s)||memcmp(&io,&old,sizeof io))++failures;}
 for(g=0;g<6u;++g){if(!dsky_scalar_output_checkpoint_input(144,&s,&io,&mission,&entry,&channel))return 2;s.words[0140]=2;switch(g){case 0:s.words[0140]=0;break;case 1:s.words[0117]=3;break;case 2:s.words[0150u+s.words[0117]]=0777;break;case 3:s.words[0150u+s.words[0117]]=03777;break;case 4:s.words[0150u+s.words[0117]]=01377;break;default:s.words[0140]=1;s.words[0145]=02000;break;}before=s;old=io;if(dsky_scalar_output(&s,&io,mission,entry,channel)!=P66_BIAS_OUT_OF_SCOPE||memcmp(&s,&before,sizeof s)||memcmp(&io,&old,sizeof io))++failures;}
 if(!dsky_scalar_output_checkpoint_input(64,&s,&io,&mission,&entry,&channel))return 2;
 before=s;old=io;if(dsky_scalar_output(&s,&io,mission,entry,0100000)!=P66_BIAS_OUT_OF_SCOPE||memcmp(&s,&before,sizeof s)||memcmp(&io,&old,sizeof io))++failures;
 printf("160 full LM/CM scalar output source states and33 unchanged invalid guards: %s\n",failures?"FAIL":"PASS");return failures?1:0;}
