/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "lm_danzig_change_cases.h"
int main(void){uint16_t n;for(n=0;n<64u;++n){unsigned i;apollo_overlay_state image;p66_bias_io io;uint16_t number=(uint16_t)((n*257u+29u)%LM_DANZIG_CHANGE_CASES);lm_danzig_change_input(&image,&io,number);printf("%u %u",number,(unsigned)(number&1u));for(i=0;i<2048u;++i)printf(" %u",image.words[i]);printf(" %u %u\n",io.superbank,io.interrupt_enabled);}return 0;}
