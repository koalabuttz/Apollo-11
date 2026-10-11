/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <stdlib.h>
#include "imu_pulse_cases.h"
int main(int argc,char **argv){uint32_t n;unsigned mission,stage;if(argc!=3)return 2;mission=(unsigned)strtoul(argv[1],0,0);stage=(unsigned)strtoul(argv[2],0,0);while(fread(&n,sizeof n,1,stdin)==1){apollo_overlay_state m;imu_pulse_io io;if(stage)imu_pulse_service_input(&m,&io,(enum rtb_mission)mission,n);else imu_pulse_input(&m,&io,(enum rtb_mission)mission,n);if(fwrite(&io,sizeof io,1,stdout)!=1||fwrite(m.words,sizeof m.words,1,stdout)!=1)return 2;}return ferror(stdin)?2:0;}
