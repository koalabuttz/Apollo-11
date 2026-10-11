/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <stdlib.h>
#include "rtb_pulse_return_cases.h"
int main(int argc,char **argv){uint32_t n,limit=RTB_PULSE_RETURN_CASES;unsigned mission;if(argc<2)return 2;mission=(unsigned)strtoul(argv[1],0,0);if(argc>2)limit=(uint32_t)strtoul(argv[2],0,0);for(n=0;n<limit;++n){apollo_overlay_state m;p66_bias_io io;uint16_t row[2053];unsigned i;enum rtb_status status;rtb_pulse_return_input(&m,&io,(enum rtb_mission)mission,n);status=rtb_pulse_return(&m,&io,(enum rtb_mission)mission);row[0]=(uint16_t)n;row[1]=(uint16_t)(n>>16);row[2]=io.interrupt_enabled;row[3]=io.superbank;row[4]=(uint16_t)status;for(i=0;i<2048u;++i)row[i+5u]=m.words[i];if(fwrite(row,sizeof row,1,stdout)!=1)return 2;}return 0;}
