/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <stdlib.h>
#include "inverse_trig_cases.h"
static void row(const apollo_overlay_state *m,agc_word irq,uint32_t n,uint16_t phase,enum inverse_trig_status status){uint16_t w[2054];unsigned i;w[0]=(uint16_t)n;w[1]=(uint16_t)(n>>16);w[2]=phase;w[3]=(uint16_t)status;w[4]=irq;w[5]=0;for(i=0;i<2048u;++i)w[i+6u]=m->words[i];if(fwrite(w,sizeof w,1,stdout)!=1)exit(2);}
int main(int argc,char **argv){uint32_t n,limit=INVERSE_TRIG_CASES;enum inverse_trig_mission mission;if(argc<2)return 2;mission=(enum inverse_trig_mission)strtoul(argv[1],NULL,0);if(argc>2)limit=(uint32_t)strtoul(argv[2],NULL,0);for(n=0;n<limit;++n){apollo_overlay_state m;agc_word irq;enum inverse_trig_status status;inverse_trig_input(&m,&irq,mission,n);status=inverse_trig_prepare(&m,mission,(uint8_t)(n&1u));row(&m,irq,n,0,status);if(status!=INVERSE_TRIG_OK)continue;status=inverse_trig_root(&m,mission);row(&m,irq,n,1,status);if(status!=INVERSE_TRIG_OK)continue;status=inverse_trig_polynomial(&m,mission);row(&m,irq,n,2,status);if(status!=INVERSE_TRIG_OK)continue;status=inverse_trig_finish(&m,mission);row(&m,irq,n,3,status);}return 0;}
