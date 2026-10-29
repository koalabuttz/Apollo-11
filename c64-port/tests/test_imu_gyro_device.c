/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "imu_gyro_device_cases.h"
#ifndef IMUD_MISSION
#define IMUD_MISSION 0
#endif
#ifndef IMUD_MODE
#define IMUD_MODE 0
#endif
#define IMU_GYRO_DEVICE_FROZEN_IMPLEMENTATION
#include "imu_gyro_device_vectors.h"
static apollo_overlay_state image,expected;
static uint8_t patch(frozen_words *r,apollo_overlay_state *m){uint16_t count,i,address,value;if(!frozen_words_next(r,&count)||count>2048u)return 0;for(i=0;i<count;++i){if(!frozen_words_next(r,&address)||!frozen_words_next(r,&value)||address>=2048u)return 0;m->words[address]=value;}return 1;}
static unsigned rejected(void){unsigned k,fail=0;for(k=0;k<6u;++k){imu_pulse_io io,oldio;imu_gyro_device d,old;agc_word events[18],before[18];uint8_t count=071;unsigned i;enum imu_gyro_clock_mode mode=(enum imu_gyro_clock_mode)IMUD_MODE;imu_gyro_device_input(&image,&io,&d,(enum rtb_mission)IMUD_MISSION,mode,65535);if(k==0)image.words[047]=040000;else if(k==1)d.phase=IMUD_MODE?160UL:128000UL;else if(k==2)mode=(enum imu_gyro_clock_mode)2;else if(k==3)d.selection=1;else if(k==4)io.channel14=0100000;else d.pending=IMUD_MODE?800u:040000u;expected=image;oldio=io;old=d;for(i=0;i<18u;++i)events[i]=before[i]=(agc_word)(i+010000u);if(imu_gyro_device_step(&image,&io,&d,mode,events,&count)||memcmp(&image,&expected,sizeof image)||memcmp(&io,&oldio,sizeof io)||memcmp(&d,&old,sizeof d)||memcmp(events,before,sizeof events)||count!=071)++fail;}return fail;}
unsigned imu_gyro_device_check(void){unsigned row,fail=0;for(row=0;row<32u;++row){frozen_words r;uint16_t low,high,out[28],pad;uint32_t n;imu_pulse_io io;imu_gyro_device d;agc_word events[18];uint8_t count=0;unsigned i;if(!imu_gyro_device_reference_begin(&r,row)||!frozen_words_next(&r,&low)||!frozen_words_next(&r,&high)){++fail;continue;}n=(uint32_t)low+((uint32_t)high<<16);imu_gyro_device_input(&image,&io,&d,(enum rtb_mission)IMUD_MISSION,(enum imu_gyro_clock_mode)IMUD_MODE,n);expected=image;for(i=0;i<28u;++i)if(!frozen_words_next(&r,&out[i])){++fail;break;}if(!patch(&r,&expected)){++fail;continue;}while(frozen_words_next(&r,&pad))if(pad)++fail;for(i=0;i<18u;++i)events[i]=(agc_word)(010000u+i);if(!imu_gyro_device_step(&image,&io,&d,(enum imu_gyro_clock_mode)IMUD_MODE,events,&count)||!frozen_words_finished(&r)||io.latch.interrupt_enabled!=out[0]||io.latch.superbank!=out[1]||io.channel11!=out[2]||io.channel12!=out[3]||io.channel14!=out[4]||count!=out[5]||d.phase!=((uint32_t)out[6]+((uint32_t)out[7]<<16))||d.pending!=out[8]||d.selection!=out[9]||memcmp(events,&out[10],sizeof events)||memcmp(&image,&expected,sizeof image))++fail;}return fail+rejected();}
#ifndef IMU_GYRO_DEVICE_CHECK_ONLY
int main(void){unsigned f=imu_gyro_device_check();printf("32 complete gyro peripheral states mission%d mode%d: %s\n",IMUD_MISSION,IMUD_MODE,f?"FAIL":"PASS");return f?1:0;}
#endif
