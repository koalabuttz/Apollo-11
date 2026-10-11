/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "imu_pulse_task_cases.h"
#ifndef IMUG_MISSION
#define IMUG_MISSION 0
#endif
#ifndef IMUG_AUGMENT
#define IMUG_AUGMENT 0
#endif
#ifndef IMUG_STAGE
#define IMUG_STAGE 0
#endif
#define IMU_PULSE_TASK_FROZEN_IMPLEMENTATION
#include "imu_pulse_task_vectors.h"
static apollo_overlay_state image,expected;
static uint8_t patch(frozen_words *r,apollo_overlay_state *m){uint16_t count,i,address,value;if(!frozen_words_next(r,&count)||count>2048u)return 0;for(i=0;i<count;++i){if(!frozen_words_next(r,&address)||!frozen_words_next(r,&value)||address>=2048u)return 0;m->words[address]=value;}return 1;}
unsigned imu_pulse_task_check(void){unsigned row,fail=0;for(row=0;row<32u;++row){frozen_words r;uint16_t low,high,irq,sup,ch11,ch12,ch14,status,pad;uint32_t n;imu_pulse_io io;enum imu_gyro_task_status result;
 if(!imu_pulse_task_reference_begin(&r,row)||!frozen_words_next(&r,&low)||!frozen_words_next(&r,&high)){++fail;continue;}n=(uint32_t)low+((uint32_t)high<<16);
 imu_pulse_task_input(&image,&io,(enum rtb_mission)IMUG_MISSION,IMUG_AUGMENT,n);expected=image;
 if(!frozen_words_next(&r,&irq)||!frozen_words_next(&r,&sup)||!frozen_words_next(&r,&ch11)||!frozen_words_next(&r,&ch12)||!frozen_words_next(&r,&ch14)||!frozen_words_next(&r,&status)||!patch(&r,&expected)){++fail;continue;}
 while(frozen_words_next(&r,&pad))if(pad)++fail;
 result=imu_gyro_task(&image,&io,(enum rtb_mission)IMUG_MISSION,IMUG_AUGMENT);
 if(IMUG_STAGE&&result==IMU_GYRO_WAITLIST)result=imu_gyro_register(&image,&io,(enum rtb_mission)IMUG_MISSION);
 if(!frozen_words_finished(&r)||(unsigned)result!=status||io.latch.interrupt_enabled!=irq||io.latch.superbank!=sup||io.channel11!=ch11||io.channel12!=ch12||io.channel14!=ch14||memcmp(&image,&expected,sizeof image))++fail;
 }return fail;}
#ifndef IMU_PULSE_TASK_CHECK_ONLY
int main(void){unsigned f=imu_pulse_task_check();printf("32 complete IMUPULSE task original states mission%d stage%d: %s\n",IMUG_MISSION,IMUG_STAGE,f?"FAIL":"PASS");return f?1:0;}
#endif
