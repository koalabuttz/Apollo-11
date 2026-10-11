/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <stdlib.h>
#include "imu_pulse_task_cases.h"
int main(int argc,char **argv){uint32_t n;unsigned mission,augment;if(argc!=3)return 2;mission=(unsigned)strtoul(argv[1],0,0);augment=(unsigned)strtoul(argv[2],0,0);while(fread(&n,sizeof n,1,stdin)==1){apollo_overlay_state m;imu_pulse_io io;imu_pulse_task_input(&m,&io,(enum rtb_mission)mission,(uint8_t)augment,n);if(fwrite(&io,sizeof io,1,stdout)!=1||fwrite(m.words,sizeof m.words,1,stdout)!=1)return 2;}return ferror(stdin)?2:0;}
