/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "rtb_unit_cases.h"
#ifndef RTBU_MISSION
#define RTBU_MISSION 0
#endif
#ifndef RTBU_CALLER
#define RTBU_CALLER 0
#endif
#ifndef RTBU_PHASE
#define RTBU_PHASE 0
#endif
#define RTB_UNIT_FROZEN_IMPLEMENTATION
#include "rtb_unit_vectors.h"
static apollo_overlay_state image,expected;
static uint8_t patch(frozen_words *r,apollo_overlay_state *m){uint16_t count,i,address,value;if(!frozen_words_next(r,&count)||count>2048u)return 0;for(i=0;i<count;++i){if(!frozen_words_next(r,&address)||!frozen_words_next(r,&value)||address>=2048u)return 0;m->words[address]=value;}return 1;}
unsigned rtb_unit_check(void){unsigned row,fail=0;
for(row=0;row<32u;++row){frozen_words r;uint16_t low,high,irq,sup,status,pad;uint32_t n;p66_bias_io io;unsigned p;enum rtb_unit_status result=RTB_UNIT_OK;
 if(!rtb_unit_reference_begin(&r,row)||!frozen_words_next(&r,&low)||!frozen_words_next(&r,&high)){++fail;continue;}n=(uint32_t)low+((uint32_t)high<<16);
 rtb_unit_input(&image,&io,(enum rtb_mission)RTBU_MISSION,RTBU_CALLER,n);expected=image;
 if(!frozen_words_next(&r,&irq)||!frozen_words_next(&r,&sup)||!frozen_words_next(&r,&status)||!patch(&r,&expected)){++fail;continue;}
 while(frozen_words_next(&r,&pad))if(pad)++fail;
 if(RTBU_CALLER==2){if(rtb_unit_begin(&image,&io,(enum rtb_mission)RTBU_MISSION)!=RTB_UNIT_OK)++fail;}
 else if(rtb_vector(&image,&io,(enum rtb_mission)RTBU_MISSION,RTBU_CALLER?RTB_NORMUNX1_FRONT:RTB_NORMUNIT_FRONT)!=RTB_OK)++fail;
 for(p=0;p<=RTBU_PHASE&&result==RTB_UNIT_OK;++p)result=rtb_unit_phase(&image,&io,(enum rtb_mission)RTBU_MISSION,(uint8_t)p);
 if(!frozen_words_finished(&r)||(unsigned)result!=status||io.interrupt_enabled!=irq||io.superbank!=sup||memcmp(&image,&expected,sizeof image))++fail;
}return fail;}
#ifndef RTB_UNIT_CHECK_ONLY
static unsigned rejected(void){unsigned f=0;p66_bias_io io,old;rtb_unit_input(&image,&io,(enum rtb_mission)RTBU_MISSION,2,660);image.words[5]=03024;image.words[7]=1;expected=image;old=io;if(rtb_unit_phase(&image,&io,(enum rtb_mission)RTBU_MISSION,0)!=RTB_UNIT_OUT_OF_SCOPE||memcmp(&image,&expected,sizeof image)||memcmp(&io,&old,sizeof io))++f;return f;}
int main(void){unsigned f=rtb_unit_check()+rejected();printf("32 complete UNIT/NORM original states mission%d caller%d phase%d: %s\n",RTBU_MISSION,RTBU_CALLER,RTBU_PHASE,f?"FAIL":"PASS");return f?1:0;}
#endif
