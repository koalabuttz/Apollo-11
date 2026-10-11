/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "p66_launch_cases.h"
static apollo_overlay_state image;
int main(void){unsigned i,j;for(i=0;i<32u;++i){uint16_t n=(uint16_t)((i*37u+11u)&1023u);p66_launch_prepare(&image,n);printf("%u",n);for(j=0;j<APOLLO_SHARED_WORDS;++j)printf(" %u",image.words[j]);printf(" %u %u 0 0\n",(unsigned)((n/5u)%4u*020u),n%2u);}return 0;}
