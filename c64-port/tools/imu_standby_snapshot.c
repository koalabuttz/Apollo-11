/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <stdlib.h>
#include "imu_standby_cases.h"
int main(int argc,char **argv){uint32_t n,limit=65536;unsigned mission,stage;if(argc<3)return 2;mission=(unsigned)strtoul(argv[1],NULL,0);stage=(unsigned)strtoul(argv[2],NULL,0);if(argc>3)limit=(uint32_t)strtoul(argv[3],NULL,0);for(n=0;n<limit;++n){apollo_overlay_state m;imu_standby_io io;unsigned i;uint16_t row[2057];imu_standby_input(&m,&io,(enum rtb_mission)mission,(enum imu_standby_stage)stage,n);row[8]=(uint16_t)imu_standby(&m,&io,(enum rtb_mission)mission,(enum imu_standby_stage)stage);row[0]=(uint16_t)n;row[1]=(uint16_t)(n>>16);row[2]=io.pulse.latch.interrupt_enabled;row[3]=io.pulse.latch.superbank;row[4]=io.pulse.channel11;row[5]=io.pulse.channel12;row[6]=io.pulse.channel14;row[7]=io.channel13;for(i=0;i<2048u;++i)row[i+9u]=m.words[i];if(fwrite(row,sizeof row,1,stdout)!=1)return 2;}return 0;}
