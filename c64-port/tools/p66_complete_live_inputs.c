/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "p66_complete_live_cases.h"
int main(void){uint16_t n;for(n=0;n<2u;++n){apollo_overlay_state image;executive state;p66_bias_io io;agc_word channel14,channel11;unsigned i;pcl_prepare(&image,&state,&io,&channel14,&channel11,n);printf("%u %u %u %u %u",n,io.superbank,io.interrupt_enabled,channel14,channel11);for(i=0;i<2048u;++i)printf(" %u",image.words[i]);puts("");}return 0;}
