/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "lm_fleshpot_interface_cases.h"
#include "lm_powered_fixed_return.h"
static int row(const apollo_overlay_state *m,agc_word irq,uint16_t n,uint16_t phase){uint16_t h[3]={n,phase,irq};return fwrite(h,sizeof h,1,stdout)!=1||fwrite(m->words,sizeof m->words,1,stdout)!=1;}
int main(int argc,char **argv){uint32_t n,limit=LM_FLESHPOT_INTERFACE_CASES;(void)argv;if(argc>1)limit=1;for(n=0;n<limit;++n){apollo_overlay_state m;agc_word irq;uint8_t step;lm_fleshpot_interface_input(&m,&irq,(uint16_t)n);if(lm_fleshpot_interface_start(&m)!=LM_FLESHPOT_OK||row(&m,irq,(uint16_t)n,0))return 2;for(step=0;step<4u;++step)if(lm_fleshpot(&m,(enum lm_fleshpot_phase)step)!=LM_FLESHPOT_OK||row(&m,irq,(uint16_t)n,(uint16_t)(step+1u)))return 2;if(lm_fleshpot_bank_return(&m)!=LM_FLESHPOT_OK||row(&m,irq,(uint16_t)n,5)||lm_fleshpot_interface_reentry(&m,&irq)!=LM_FLESHPOT_OK||row(&m,irq,(uint16_t)n,6)||lm_powered_fixed_return(&m)!=LM_POWERED_FIXED_RETURN_OK||row(&m,irq,(uint16_t)n,7))return 2;}return 0;}
