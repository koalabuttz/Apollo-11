/* SPDX-License-Identifier: MIT */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "gimbal_cases.h"
int main(void){static apollo_overlay_state s,before;gimbal_io io,saved;uint16_t n,out;unsigned mission,address,hits;
 for(n=0;n<20u;++n){gimbal_prepare(&s,&io,17);switch(n){
 case 0:s.words[3]=03401;break;case 1:s.words[4]=042000;break;case 2:s.words[6]^=1;break;
 case 3:s.words[5]++;break;case 4:s.words[0165]^=1;break;case 5:s.words[0115]=0;break;
 case 6:s.words[0164]=02745;break;case 7:s.words[023]=1;break;case 8:s.words[067]=077777;break;
 case 9:s.words[7]=1;break;case 10:s.words[0120]=0400;break;case 11:s.words[0120]=0662;break;
 case 12:s.words[01325]=1;break;case 13:s.words[01325]=8;break;
 case 14:io.interrupt_enabled=2;break;case 15:io.superbank=1;break;
 default:s.words[01000u+n]=0100000;break;}
 before=s;saved=io;assert(!gimbal_entry(&s,&io,APOLLO_FIXED_LM));
 assert(!memcmp(&s,&before,sizeof s));assert(!memcmp(&io,&saved,sizeof io));}
 gimbal_prepare(&s,&io,0);before=s;saved=io;
 assert(!gimbal_entry(0,&io,APOLLO_FIXED_LM));assert(!gimbal_entry(&s,0,APOLLO_FIXED_LM));
 assert(!gimbal_entry(&s,&io,(enum apollo_fixed_mission)2));
 assert(!memcmp(&s,&before,sizeof s));assert(!memcmp(&io,&saved,sizeof io));
 for(mission=0;mission<2u;++mission){hits=0;for(address=0;address<65536u;++address){
 out=012345;if(gimbal_fixed_read((enum apollo_fixed_mission)mission,(uint16_t)address,&out))++hits;
 else assert(out==012345);}assert(hits==(mission==0?10u:30u));}
 out=012345;assert(!gimbal_fixed_read((enum apollo_fixed_mission)2,0,&out));assert(out==012345);
 assert(!gimbal_fixed_read(APOLLO_FIXED_LM,18937,0));
 puts("PASS: unsupported entries preserve image/latches; 131072 fixed-read probes expose exactly40 qualified words");return 0;}
