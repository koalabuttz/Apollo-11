/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "dsky_fixed_display_check.h"
static apollo_overlay_state s,before;
int main(void){uint16_t n,fail=0;unsigned m,g;for(n=0;n<96u;++n)fail=(uint16_t)(fail+dsky_fixed_display_checkpoint(n,&s,&before));for(m=0;m<2u;++m)for(g=0;g<16u;++g){p66_bias_io io,old;enum dsky_nv_mission mission;enum dsky_fixed_display_stage stage=DSKY_FIXED_ENTER;enum dsky_fixed_display_route route=(enum dsky_fixed_display_route)99;agc_word value=0;uint16_t address=05555;dsky_fixed_display_input(&s,&io,m*40960u,&mission);
 if(g>=11u){if(dsky_fixed_display(&s,&io,mission,DSKY_FIXED_ENTER,0,&route)!=P66_BIAS_OK)return 2;stage=DSKY_FIXED_DATA;route=(enum dsky_fixed_display_route)99;}
 switch(g){case 0:mission=(enum dsky_nv_mission)2;break;case 1:stage=(enum dsky_fixed_display_stage)3;break;case 2:s.words[5]=0;break;case 3:s.words[4]=0;break;case 4:s.words[3]=1;break;case 5:s.words[6]^=1;break;case 6:s.words[7]=1;break;case 7:io.superbank=0;break;case 8:io.interrupt_enabled=2;break;case 9:s.words[0200]=0100000;break;case 10:s.words[2]=0100000;break;case 11:value=0100000;break;case 12:s.words[2]=03360;break;case 13:s.words[0]=040000;break;case 14:stage=DSKY_FIXED_FORMAT;s.words[5]=03361;s.words[0]=0;s.words[0777]=3;break;default:stage=DSKY_FIXED_FORMAT;s.words[5]=03361;s.words[0]=040000;s.words[0777]=14;break;}
 before=s;old=io;if(dsky_fixed_display(&s,&io,mission,stage,value,&route)!=P66_BIAS_OUT_OF_SCOPE||memcmp(&s,&before,sizeof s)||memcmp(&io,&old,sizeof io)||route!=(enum dsky_fixed_display_route)99)++fail;
 if(g==12u||g==13u){if(dsky_fixed_display_address(&s,&io,mission,&address)||address!=05555u)++fail;}
 }printf("96 full original fixed-memory display states +32 unchanged invalid guards: %s\n",fail?"FAIL":"PASS");return fail?1:0;}
