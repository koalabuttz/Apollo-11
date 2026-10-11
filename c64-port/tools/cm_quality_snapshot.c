/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <stdlib.h>
#include "cm_quality.h"
#include "cm_quality_cases.h"
int main(int argc,char **argv){uint32_t n,start=0,limit=65536;unsigned i;if(argc>1)start=(uint32_t)strtoul(argv[1],NULL,0);if(argc>2)limit=(uint32_t)strtoul(argv[2],NULL,0);for(n=start;n<limit;++n){apollo_overlay_state m;p66_bias_io io;unsigned phase;cm_quality_input(&m,&io,n);for(phase=0;phase<20u;++phase){uint16_t row[2054];uint8_t status=cm_quality_step(&m,&io,(uint8_t)phase);if(status==2u)return 2;row[0]=(uint16_t)n;row[1]=(uint16_t)(n>>16);row[2]=io.interrupt_enabled;row[3]=io.superbank;row[4]=(uint16_t)phase;row[5]=status;for(i=0;i<2048u;++i)row[i+6u]=m.words[i];if(fwrite(row,sizeof row,1,stdout)!=1)return 2;if(status)break;}}return 0;}
