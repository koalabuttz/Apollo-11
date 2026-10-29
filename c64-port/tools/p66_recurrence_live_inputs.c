/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "p66_recurrence_live_cases.h"
int main(void){uint16_t n;for(n=0;n<2u;++n){apollo_overlay_state m;executive s;p66_bias_io io;agc_word c14,c11;unsigned i;prl_prepare(&m,&s,&io,&c14,&c11,n);printf("%u %u %u %u %u",n,io.superbank,io.interrupt_enabled,c14,c11);for(i=0;i<2048u;++i)printf(" %u",m.words[i]);puts("");}return 0;}
