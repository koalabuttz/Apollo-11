/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "lm_powered_trig_cases.h"
#include "lm_powered_trig.h"
static int row(apollo_overlay_state *m,agc_word irq,uint16_t n,uint8_t live,uint8_t phase){uint16_t h[4];h[0]=n;h[1]=live;h[2]=phase;h[3]=irq;return fwrite(h,sizeof h,1,stdout)!=1||fwrite(m->words,sizeof m->words,1,stdout)!=1;}
int main(void){uint8_t live;for(live=0;live<2u;++live){uint32_t n;for(n=0;n<LM_POWERED_TRIG_CASES;++n){apollo_overlay_state m;agc_word irq;uint8_t phase;lm_powered_trig_input(&m,&irq,(uint16_t)n,live);if(lm_powered_cdu_start(&m,live)!=LM_POWERED_TRIG_OK||row(&m,irq,(uint16_t)n,live,0))return 2;for(phase=1;phase<10u;++phase){enum lm_powered_trig_status result=phase%3u==1u?lm_powered_cdu_cosine(&m):phase%3u==2u?lm_powered_cdu_sine(&m):lm_powered_cdu_next(&m);if(result!=LM_POWERED_TRIG_OK||row(&m,irq,(uint16_t)n,live,phase))return 2;}}}return 0;}
