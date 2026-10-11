/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "lm_taskover_resume_cases.h"
int main(void){uint16_t n;for(n=0;n<LM_TASKOVER_RESUME_COUNT;++n){apollo_overlay_state image;p66_bias_io io;agc_word c11,c14;uint16_t in_interrupt=1;unsigned i;lm_taskover_resume_input(&image,&io,&c11,&c14,n);if(lm_taskover_resume(&image,&io,&in_interrupt)!=EXEC_OK)return 2;printf("%u",n);for(i=0;i<2048u;++i)printf(" %u",image.words[i]);printf(" %u %u %u %u %u\n",io.superbank,io.interrupt_enabled,c11,c14,in_interrupt);}return 0;}
