/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "nv_entry_cases.h"
static apollo_overlay_state s;
int main(void){uint16_t n,i;for(n=0;n<NV_ENTRY_CASES;++n){p66_bias_io io;nv_entry_input(&s,&io,n);printf("%u 0",n);for(i=0;i<APOLLO_SHARED_WORDS;++i)printf(" %u",s.words[i]);printf(" %u %u\n",io.superbank,io.interrupt_enabled);}return 0;}
