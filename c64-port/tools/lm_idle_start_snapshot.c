/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "lm_idle_start_cases.h"
int main(void){uint16_t n;for(n=0;n<LM_IDLE_START_COUNT;++n){apollo_overlay_state image;p66_bias_io io;agc_word c11,c14;exec_address next;unsigned i;lm_idle_start_input(&image,&io,&c11,&c14,n);if(lm_idle_start(&image,&io,&c11,&next)!=EXEC_OK)return 2;printf("%u",n);for(i=0;i<2048u;++i)printf(" %u",image.words[i]);printf(" %u %u %u %u\n",io.superbank,io.interrupt_enabled,c11,c14);}return 0;}
