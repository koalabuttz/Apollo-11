/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "imu_attitude_admission.h"
int main(void){unsigned mission,at;for(mission=0;mission<2u;++mission)for(at=0;at<65536u;++at){agc_word value=05555;uint16_t row[3];row[0]=(uint16_t)at;row[1]=imu_attitude_literal((enum rtb_mission)mission,(uint16_t)at,&value);row[2]=value;if(fwrite(row,sizeof row,1,stdout)!=1)return 2;}return 0;}
