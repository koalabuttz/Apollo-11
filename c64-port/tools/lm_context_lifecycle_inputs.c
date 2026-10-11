/* SPDX-License-Identifier: MIT */
#include "lm_context_lifecycle_cases.h"
#include <stdio.h>
static apollo_overlay_state image;
int main(void){unsigned i,j;for(i=0;i<64u;++i){uint16_t n=(uint16_t)(i<16u?i:
 i<24u?16384u+i-16u:i<40u?24576u+i-24u:(i*613u+29u)&32767u);
 p66_bias_io io;agc_word channel;lm_context_lifecycle_input(&image,&io,&channel,n);
 printf("%u",n);for(j=0;j<2048u;++j)printf(" %u",image.words[j]);
 printf(" %u %u %u\n",io.superbank,io.interrupt_enabled,channel);}return 0;}
