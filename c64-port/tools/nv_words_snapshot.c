/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "nv_words_cases.h"
static apollo_overlay_state s;
int main(void){uint16_t n,i;uint8_t phase;for(n=0;n<NV_WORDS_CASES;++n){p66_bias_io io;nv_words_input(&s,&io,n);for(phase=0;phase<2u;++phase){enum p66_bias_status status=phase?p66_nv_noun(&s,&io):p66_nv_verb(&s,&io);printf("%u %u %u",n,phase,(unsigned)status);for(i=0;i<APOLLO_SHARED_WORDS;++i)printf(" %u",s.words[i]);printf(" %u %u\n",io.superbank,io.interrupt_enabled);}}return 0;}
