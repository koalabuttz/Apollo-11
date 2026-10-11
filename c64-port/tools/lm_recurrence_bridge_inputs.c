/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "lm_idle_start_cases.h"
#include "lm_taskover_resume_cases.h"
int main(void){uint16_t n;for(n=0;n<64u;++n){apollo_overlay_state image;p66_bias_io io;agc_word c11,c14;unsigned i;uint16_t sample=(uint16_t)((n/2u*173u+29u)&8191u);if(n&1u)lm_taskover_resume_input(&image,&io,&c11,&c14,sample);else lm_idle_start_input(&image,&io,&c11,&c14,sample);printf("%u",n);for(i=0;i<2048u;++i)printf(" %u",image.words[i]);printf(" %u %u %u %u %u\n",io.superbank,io.interrupt_enabled,c11,c14,(unsigned)(n&1u));}return 0;}
