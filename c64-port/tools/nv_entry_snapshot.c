/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "nv_entry_cases.h"
static apollo_overlay_state s;
int main(void){uint16_t n,i;for(n=0;n<NV_ENTRY_CASES;++n){p66_bias_io io;enum p66_nv_boundary status;nv_entry_input(&s,&io,n);status=p66_nv_enter(&s,&io);printf("%u %u",n,(unsigned)status);for(i=0;i<APOLLO_SHARED_WORDS;++i)printf(" %u",s.words[i]);printf(" %u %u\n",io.superbank,io.interrupt_enabled);}return 0;}
