/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <stdlib.h>
#include "imu_gyro_device_cases.h"
int main(int argc,char **argv){uint32_t n;unsigned mission,mode;if(argc!=3)return 2;mission=(unsigned)strtoul(argv[1],0,0);mode=(unsigned)strtoul(argv[2],0,0);while(fread(&n,sizeof n,1,stdin)==1){apollo_overlay_state m;imu_pulse_io io;imu_gyro_device d;imu_gyro_device_input(&m,&io,&d,(enum rtb_mission)mission,(enum imu_gyro_clock_mode)mode,n);if(fwrite(&io,sizeof io,1,stdout)!=1||fwrite(&d,sizeof d,1,stdout)!=1||fwrite(m.words,sizeof m.words,1,stdout)!=1)return 2;}return ferror(stdin)?2:0;}
