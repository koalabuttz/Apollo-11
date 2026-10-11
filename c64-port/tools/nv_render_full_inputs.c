/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "nv_render_cases.h"
static apollo_overlay_state s;
int main(void){uint16_t n,i;for(n=0;n<1024u;++n){p66_bias_io io;nv_rows_input(&s,&io,(uint16_t)(n*4u));(void)p66_nv_data_prepare(&s,&io);printf("%u 0 0",n);for(i=0;i<APOLLO_SHARED_WORDS;++i)printf(" %u",s.words[i]);printf(" %u %u\n",io.superbank,io.interrupt_enabled);}return 0;}
