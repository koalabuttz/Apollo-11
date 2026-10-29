/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "lm_waitlist_dispatch_cases.h"
int main(void){uint16_t n;for(n=0;n<64u;++n){apollo_overlay_state image;p66_bias_io io;agc_word c11,c14;unsigned i;lm_waitlist_dispatch_input(&image,&io,&c11,&c14,(uint16_t)((n*173u+29u)&8191u));printf("%u",n);for(i=0;i<2048u;++i)printf(" %u",image.words[i]);printf(" %u %u %u %u %u\n",io.superbank,io.interrupt_enabled,c11,c14,1u);}return 0;}
