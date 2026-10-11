/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "cm_dyndisp_cases.h"
static apollo_overlay_state s;
int main(void){cm_native_io io;uint16_t n,i;uint8_t phase;for(n=0;n<CM_DYNDISP_CASES;++n){cm_dyndisp_input(&s,&io,n);for(phase=0;phase<27u;++phase){if(!cm_dyndisp_apply(&s,&io,phase)){fprintf(stderr,"nativeunsupported case%u phase%u PC%o LOC%o BB%o\n",n,phase,s.words[5],s.words[0164],s.words[6]);return 2;}printf("%u 1",n*27u+phase);for(i=0;i<2048u;++i)printf(" %u",s.words[i]);printf(" %u %u\n",io.superbank,io.interrupt_enabled);}}return 0;}
