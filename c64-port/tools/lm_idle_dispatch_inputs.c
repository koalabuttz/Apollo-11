/* SPDX-License-Identifier: MIT */
#include "lm_idle_dispatch_cases.h"
#include <stdio.h>
static apollo_overlay_state image;
int main(void){unsigned i,j;for(i=0;i<64u;++i){uint16_t n=(uint16_t)((i/8u)*2048u+(i%8u)*241u+29u);
 p66_bias_io io;agc_word channel;lm_idle_dispatch_input(&image,&io,&channel,n);
 printf("%u",n);for(j=0;j<2048u;++j)printf(" %u",image.words[j]);
 printf(" %u %u %u\n",io.superbank,io.interrupt_enabled,channel);}return 0;}
