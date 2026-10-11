/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "imu_mode_control_cases.h"
#ifndef IMUM_MISSION
#define IMUM_MISSION 0
#endif
#ifndef IMUM_ENTRY
#define IMUM_ENTRY 0
#endif
#define IMU_MODE_CONTROL_FROZEN_IMPLEMENTATION
#include "imu_mode_control_vectors.h"
static apollo_overlay_state image,expected;
static uint8_t patch(frozen_words *r,apollo_overlay_state *m){uint16_t count,i,address,value;if(!frozen_words_next(r,&count)||count>2048u)return 0;for(i=0;i<count;++i){if(!frozen_words_next(r,&address)||!frozen_words_next(r,&value)||address>=2048u)return 0;m->words[address]=value;}return 1;}
unsigned imu_mode_control_check(void){unsigned row,fail=0;for(row=0;row<32u;++row){frozen_words r;uint16_t low,high,irq,sup,ch11,ch12,ch14,status,pad;uint32_t n;imu_pulse_io io;enum imu_mode_status result;
 if(!imu_mode_control_reference_begin(&r,row)||!frozen_words_next(&r,&low)||!frozen_words_next(&r,&high)){++fail;continue;}n=(uint32_t)low+((uint32_t)high<<16);
 imu_mode_control_input(&image,&io,(enum rtb_mission)IMUM_MISSION,(enum imu_mode_stage)IMUM_ENTRY,n);expected=image;
 if(!frozen_words_next(&r,&irq)||!frozen_words_next(&r,&sup)||!frozen_words_next(&r,&ch11)||!frozen_words_next(&r,&ch12)||!frozen_words_next(&r,&ch14)||!frozen_words_next(&r,&status)||!patch(&r,&expected)){++fail;continue;}
 while(frozen_words_next(&r,&pad))if(pad)++fail;
 result=imu_mode_control(&image,&io,(enum rtb_mission)IMUM_MISSION,(enum imu_mode_stage)IMUM_ENTRY);
 if(!frozen_words_finished(&r)||(unsigned)result!=status||io.latch.interrupt_enabled!=irq||io.latch.superbank!=sup||io.channel11!=ch11||io.channel12!=ch12||io.channel14!=ch14||memcmp(&image,&expected,sizeof image))++fail;
 }
 {unsigned i;for(i=0;i<12u;++i){imu_pulse_io io,old;imu_mode_control_input(&image,&io,(enum rtb_mission)IMUM_MISSION,(enum imu_mode_stage)IMUM_ENTRY,2);
 switch(i){case 0:image.words[5]=0;break;case 1:image.words[3]=0;break;case 2:image.words[4]=0;break;case 3:image.words[6]=0;break;case 4:image.words[7]=1;break;case 5:image.words[2]=1;break;case 6:image.words[050]=1;break;case 7:image.words[047]=1;break;case 8:io.channel14|=070000u;break;case 9:io.latch.interrupt_enabled=2;break;case 10:io.latch.superbank=1;break;default:image.words[01300]=0100000;break;}
 expected=image;old=io;if(imu_mode_control(&image,&io,(enum rtb_mission)IMUM_MISSION,(enum imu_mode_stage)IMUM_ENTRY)!=IMU_MODE_UNSUPPORTED||memcmp(&image,&expected,sizeof image)||memcmp(&io,&old,sizeof io))++fail;}
 }return fail;}
#ifndef IMU_MODE_CONTROL_CHECK_ONLY
int main(void){unsigned f=imu_mode_control_check();printf("32 complete coarse original states + 12 immutable guards mission%d entry%d: %s\n",IMUM_MISSION,IMUM_ENTRY,f?"FAIL":"PASS");return f?1:0;}
#endif
