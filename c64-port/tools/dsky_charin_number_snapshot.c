/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "dsky_charin_number_cases.h"
int main(void){uint16_t n,i;static apollo_overlay_state s;for(n=0;n<DSKY_CHARIN_NUMBER_CASE_COUNT;++n){dsky_charin_io io={{0100,(uint16_t)(n%2u)},dsky_charin_number_channel11(n),0};dsky_charin_number_prepare(&s,n);printf("%u %u",n,(unsigned)dsky_charin_number(&s,&io));for(i=0;i<APOLLO_SHARED_WORDS;++i)printf(" %u",s.words[i]);printf(" %u %u %u %u\n",io.control.superbank,io.control.interrupt_enabled,io.channel11,io.wrote_channel11);}return 0;}
