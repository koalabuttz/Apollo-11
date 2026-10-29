/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "imu_standby_cases.h"
#ifndef IMUS_MISSION
#define IMUS_MISSION 0
#endif
#ifndef IMUS_ENTRY
#define IMUS_ENTRY 0
#endif
#define IMU_STANDBY_FROZEN_IMPLEMENTATION
#include "imu_standby_vectors.h"
static apollo_overlay_state image,expected;
static uint8_t patch(frozen_words *r,apollo_overlay_state *m){uint16_t count,i,address,value;if(!frozen_words_next(r,&count)||count>2048u)return 0;for(i=0;i<count;++i){if(!frozen_words_next(r,&address)||!frozen_words_next(r,&value)||address>=2048u)return 0;m->words[address]=value;}return 1;}
unsigned imu_standby_check(void){unsigned row,fail=0;for(row=0;row<32u;++row){frozen_words r;uint16_t low,high,irq,sup,ch11,ch12,ch14,ch13,status,pad;uint32_t n;imu_standby_io io;enum imu_standby_status result;
 if(!imu_standby_reference_begin(&r,row)||!frozen_words_next(&r,&low)||!frozen_words_next(&r,&high)){++fail;continue;}n=(uint32_t)low+((uint32_t)high<<16);
 imu_standby_input(&image,&io,(enum rtb_mission)IMUS_MISSION,(enum imu_standby_stage)IMUS_ENTRY,n);expected=image;
 if(!frozen_words_next(&r,&irq)||!frozen_words_next(&r,&sup)||!frozen_words_next(&r,&ch11)||!frozen_words_next(&r,&ch12)||!frozen_words_next(&r,&ch14)||!frozen_words_next(&r,&ch13)||!frozen_words_next(&r,&status)||!patch(&r,&expected)){++fail;continue;}
 while(frozen_words_next(&r,&pad))if(pad)++fail;
 result=imu_standby(&image,&io,(enum rtb_mission)IMUS_MISSION,(enum imu_standby_stage)IMUS_ENTRY);
 if(!frozen_words_finished(&r)||(unsigned)result!=status||io.pulse.latch.interrupt_enabled!=irq||io.pulse.latch.superbank!=sup||io.pulse.channel11!=ch11||io.pulse.channel12!=ch12||io.pulse.channel14!=ch14||io.channel13!=ch13||memcmp(&image,&expected,sizeof image))++fail;
 }
 {unsigned i;for(i=0;i<12u;++i){imu_standby_io io,old;imu_standby_input(&image,&io,(enum rtb_mission)IMUS_MISSION,(enum imu_standby_stage)IMUS_ENTRY,2);
 switch(i){case 0:image.words[5]=0;break;case 1:image.words[3]=1;break;case 2:image.words[4]=0;break;case 3:image.words[6]=0;break;case 4:image.words[7]=1;break;case 5:image.words[2]=1;break;case 6:io.pulse.channel11=0100000;break;case 7:io.pulse.channel12=0100000;break;case 8:io.pulse.channel14|=070000u;break;case 9:io.pulse.latch.interrupt_enabled=2;break;case 10:io.pulse.latch.superbank=1;break;default:image.words[01300]=0100000;break;}
 expected=image;old=io;if(imu_standby(&image,&io,(enum rtb_mission)IMUS_MISSION,(enum imu_standby_stage)IMUS_ENTRY)!=IMU_STANDBY_UNSUPPORTED||memcmp(&image,&expected,sizeof image)||memcmp(&io,&old,sizeof io))++fail;}
 }
 {static const uint16_t at[2][10]={{077656,077671,077673,077675,077702,077703,077704,077705,077710,077772},{055661,055674,055676,055700,055705,055706,055707,055710,055713,055775}};static const agc_word value[2][10]={{054,017175,07,010,07024,020000,03734,076060,020623,054},{054,017060,07,010,07024,020000,03737,054062,021045,054}};unsigned k;for(k=0;k<10u;++k){agc_word result=05555;if(!imu_standby_literal((enum rtb_mission)IMUS_MISSION,at[IMUS_MISSION][k],&result)||result!=value[IMUS_MISSION][k])++fail;}}
 return fail;}
#ifndef IMU_STANDBY_CHECK_ONLY
int main(void){unsigned f=imu_standby_check();printf("32 complete coarse original states + 12 immutable guards mission%d entry%d: %s\n",IMUS_MISSION,IMUS_ENTRY,f?"FAIL":"PASS");return f?1:0;}
#endif
