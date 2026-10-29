/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "dsky_relay_cases.h"
int main(void){uint16_t n,i;static apollo_overlay_state s;for(n=0;n<DSKY_RELAY_CASE_COUNT;++n){dsky_relay_io io={{(uint16_t)((n/5u)%8u*020u),(uint16_t)(n%2u)},0,0};dsky_relay_prepare(&s,n);printf("%u %u",n,(unsigned)dsky_relay_drain(&s,&io));for(i=0;i<APOLLO_SHARED_WORDS;++i)printf(" %u",s.words[i]);printf(" %u %u %u %u\n",io.control.superbank,io.control.interrupt_enabled,io.wrote_channel10,io.channel10);}return 0;}
