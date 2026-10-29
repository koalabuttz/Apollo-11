/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <stdlib.h>
#include "lm_sps_rcs_cases.h"
int main(int argc,char **argv){uint32_t n,limit=LM_SPS_RCS_CASES;if(argc>1)limit=(uint32_t)strtoul(argv[1],NULL,0);for(n=0;n<limit;++n){apollo_overlay_state m;agc_word irq;uint16_t row[2051];unsigned i;lm_sps_rcs_input(&m,&irq,n);if(lm_sps_rcs(&m))return 2;row[0]=(uint16_t)n;row[1]=(uint16_t)(n>>16);row[2]=irq;for(i=0;i<2048u;++i)row[i+3u]=m.words[i];if(fwrite(row,sizeof row,1,stdout)!=1)return 2;}return 0;}
