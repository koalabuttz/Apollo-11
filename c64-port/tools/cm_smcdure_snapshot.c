/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "cm_smcdure_cases.h"
#include "cm_smcdure.h"
static int row(apollo_overlay_state *m,agc_word irq,uint16_t n,uint16_t phase){uint16_t h[3]={n,phase,irq};return fwrite(h,sizeof h,1,stdout)!=1||fwrite(m->words,sizeof m->words,1,stdout)!=1;}
int main(int argc,char **argv){uint32_t n,limit=CM_SMCDURE_CASES;(void)argv;if(argc>1)limit=1;for(n=0;n<limit;++n){apollo_overlay_state m;agc_word irq;uint8_t phase;cm_smcdure_input(&m,&irq,(uint16_t)n);for(phase=0;phase<18u;++phase){if(cm_smcdure(&m,(enum cm_smcdure_phase)phase)!=CM_SMCDURE_OK||row(&m,irq,(uint16_t)n,phase))return 2;}}return 0;}
