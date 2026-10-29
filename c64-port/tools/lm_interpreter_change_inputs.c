/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "lm_interpreter_change_cases.h"
static apollo_overlay_state image;
int main(void){unsigned i,j;for(i=0;i<64u;++i){uint16_t n=(uint16_t)((i*613u+29u)&16383u);p66_bias_io io;lm_interpreter_change_input(&image,&io,n);printf("%u",n);for(j=0;j<APOLLO_SHARED_WORDS;++j)printf(" %u",image.words[j]);printf(" %u %u %u %u\n",io.superbank,io.interrupt_enabled,lm_interpreter_change_channel11(n),lm_interpreter_change_channel14(n));}return 0;}
