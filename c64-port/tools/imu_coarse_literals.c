/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "imu_coarse.h"
int main(void){unsigned m;uint32_t at;for(m=0;m<2u;++m)for(at=0;at<65536u;++at){agc_word out=05555;uint16_t row[3];row[0]=(uint16_t)at;row[1]=imu_coarse_literal((enum rtb_mission)m,(uint16_t)at,&out);row[2]=out;if(fwrite(row,sizeof row,1,stdout)!=1)return 2;}return 0;}
