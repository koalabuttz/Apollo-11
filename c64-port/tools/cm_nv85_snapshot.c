/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "cm_nv85_cases.h"
static apollo_overlay_state s;
int main(void){cm_native_io io;uint16_t n,i;uint8_t p;for(n=0;n<CM_NV85_CASES;++n){cm_nv85_input(&s,&io,n);for(p=0;p<26u;++p){if(!cm_nv85_apply(&s,&io,p)){fprintf(stderr,"nativeNV case%u p%u unsupportedPC%oBB%o\n",n,p,s.words[5],s.words[6]);return 2;}printf("%u 1",n*26u+p);for(i=0;i<2048u;++i)printf(" %u",s.words[i]);printf(" %u %u %u\n",io.superbank,io.interrupt_enabled,io.channel11);}}return 0;}
