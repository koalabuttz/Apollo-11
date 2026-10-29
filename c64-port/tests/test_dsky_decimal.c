/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "dsky_decimal_check.h"
static apollo_overlay_state state,before;
int main(void){uint8_t n;uint16_t failures=0;p66_bias_io io={0100,0};for(n=0;n<DSKY_DECIMAL_TARGET_COUNT;++n)failures=(uint16_t)(failures+dsky_decimal_check(n,&state,&before));dsky_decimal_prepare(&state,73);state.words[0154]=0;state.words[0155]=077777;before=state;if(dsky_decimal_display(&state,&io,0)!=P66_BIAS_OUT_OF_SCOPE || memcmp(&state,&before,sizeof state))++failures;printf("DSKY DECIMAL: 64 FULL 2050-WORD IMAGES, %u FAILURES\n",failures);return failures!=0;}
