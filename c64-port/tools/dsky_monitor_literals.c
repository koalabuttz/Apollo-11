/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "dsky_monitor.h"
int main(void){unsigned m,at;for(m=0;m<2u;++m)for(at=0;at<65536u;++at){agc_word value=05555;uint16_t row[3];row[0]=(uint16_t)at;row[1]=dsky_monitor_literal((enum dsky_nv_mission)m,(uint16_t)at,&value);row[2]=value;if(fwrite(row,sizeof row,1,stdout)!=1)return 2;}return 0;}
