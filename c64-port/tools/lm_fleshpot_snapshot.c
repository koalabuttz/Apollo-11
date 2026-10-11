/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "lm_fleshpot_cases.h"
int main(int argc,char **argv){uint32_t n,limit=LM_FLESHPOT_CASES;(void)argv;if(argc>1)limit=1;for(n=0;n<limit;++n){apollo_overlay_state m;agc_word irq;uint8_t phase;lm_fleshpot_input(&m,&irq,(uint16_t)n);for(phase=0;phase<(argc>1?2u:4u);++phase){uint16_t h[3]={(uint16_t)n,phase,irq};if(lm_fleshpot(&m,(enum lm_fleshpot_phase)phase)!=LM_FLESHPOT_OK||fwrite(h,sizeof h,1,stdout)!=1||fwrite(m.words,sizeof m.words,1,stdout)!=1)return 2;}}return 0;}
