/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "imu_attitude_admission_cases.h"
#ifndef IMUA_MISSION
#define IMUA_MISSION 0
#endif
#ifndef IMUA_ENTRY
#define IMUA_ENTRY 0
#endif
#define IMU_ATTITUDE_ADMISSION_FROZEN_IMPLEMENTATION
#include "imu_attitude_admission_vectors.h"
static apollo_overlay_state image,expected;
static uint8_t patch(frozen_words *r,apollo_overlay_state *m){uint16_t count,i,address,value;if(!frozen_words_next(r,&count)||count>2048u)return 0;for(i=0;i<count;++i){if(!frozen_words_next(r,&address)||!frozen_words_next(r,&value)||address>=2048u)return 0;m->words[address]=value;}return 1;}
unsigned imu_attitude_admission_check(void){unsigned row,fail=0;for(row=0;row<32u;++row){frozen_words r;uint16_t low,high,irq,sup,ch11,ch12,ch14,status,pad;uint32_t n;imu_pulse_io io;enum imu_attitude_status result;
 if(!imu_attitude_admission_reference_begin(&r,row)||!frozen_words_next(&r,&low)||!frozen_words_next(&r,&high)){++fail;continue;}n=(uint32_t)low+((uint32_t)high<<16);
 imu_attitude_admission_input(&image,&io,(enum rtb_mission)IMUA_MISSION,(enum imu_attitude_stage)IMUA_ENTRY,n);expected=image;
 if(!frozen_words_next(&r,&irq)||!frozen_words_next(&r,&sup)||!frozen_words_next(&r,&ch11)||!frozen_words_next(&r,&ch12)||!frozen_words_next(&r,&ch14)||!frozen_words_next(&r,&status)||!patch(&r,&expected)){++fail;continue;}
 while(frozen_words_next(&r,&pad))if(pad)++fail;
 result=imu_attitude_admission(&image,&io,(enum rtb_mission)IMUA_MISSION,(enum imu_attitude_stage)IMUA_ENTRY);
 if(!frozen_words_finished(&r)||(unsigned)result!=status||io.latch.interrupt_enabled!=irq||io.latch.superbank!=sup||io.channel11!=ch11||io.channel12!=ch12||io.channel14!=ch14||memcmp(&image,&expected,sizeof image))++fail;
 }
 {unsigned i;for(i=0;i<12u;++i){imu_pulse_io io,old;imu_attitude_admission_input(&image,&io,(enum rtb_mission)IMUA_MISSION,(enum imu_attitude_stage)IMUA_ENTRY,2);
 switch(i){case 0:image.words[5]=0;break;case 1:image.words[3]=0;break;case 2:image.words[4]=0;break;case 3:image.words[6]=0;break;case 4:image.words[7]=1;break;case 5:image.words[2]=1;break;case 6:io.channel11=0100000;break;case 7:io.channel12=0100000;break;case 8:io.channel14|=070000u;break;case 9:io.latch.interrupt_enabled=2;break;case 10:io.latch.superbank=1;break;default:image.words[01300]=0100000;break;}
 expected=image;old=io;if(imu_attitude_admission(&image,&io,(enum rtb_mission)IMUA_MISSION,(enum imu_attitude_stage)IMUA_ENTRY)!=IMU_ATTITUDE_UNSUPPORTED||memcmp(&image,&expected,sizeof image)||memcmp(&io,&old,sizeof io))++fail;}
 }
 {static const uint16_t at[2][6]={{017754,017771,017772,011253,011271,011273},{017633,017650,017651,017652,017670,017672}};static const agc_word value[2][6]={{01210,044004,03276,063,7,0220},{01210,044000,03261,063,7,0220}};unsigned k;for(k=0;k<6u;++k){agc_word result=05555;if(!imu_attitude_literal((enum rtb_mission)IMUA_MISSION,at[IMUA_MISSION][k],&result)||result!=value[IMUA_MISSION][k])++fail;}}
 return fail;}
#ifndef IMU_ATTITUDE_ADMISSION_CHECK_ONLY
int main(void){unsigned f=imu_attitude_admission_check();printf("32 complete coarse original states + 12 immutable guards mission%d entry%d: %s\n",IMUA_MISSION,IMUA_ENTRY,f?"FAIL":"PASS");return f?1:0;}
#endif
