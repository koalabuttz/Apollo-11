/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "p66_guidance_live_cases.h"
int main(void){uint16_t n;for(n=0;n<PGL_CASES;++n){uint16_t w[PGL_WORDS];unsigned i;pgl_input(w,n);printf("%u",n);for(i=0;i<PGL_WORDS;++i)printf(" %u",w[i]);puts("");}return 0;}
