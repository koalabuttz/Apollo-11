/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "dsky_octal_verb_cases.h"
static apollo_overlay_state s;
int main(void){p66_bias_io io;enum dsky_nv_mission mission;uint8_t entry;uint16_t n,i;enum dsky_octal_verb_route route;for(n=0;n<DSKY_OCTAL_VERB_CASES;++n){dsky_octal_verb_input(&s,&io,n,&mission,&entry);if((entry==2u?dsky_octal_verb_channel(&s,&io,mission,s.words[01020],&route):entry?dsky_octal_verb_return(&s,&io,mission,&route):dsky_octal_verb_begin(&s,&io,mission,&route))!=P66_BIAS_OK)return 2;printf("%u",n);for(i=0;i<2048u;++i)printf(" %u",s.words[i]);printf(" %u %u\n",io.superbank,io.interrupt_enabled);}return 0;}
