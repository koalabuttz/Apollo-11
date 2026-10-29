/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "dsky_scale_check.h"
static apollo_overlay_state state,before;
int main(void){uint8_t n;uint16_t failures=0;p66_bias_io io={0100,0};for(n=0;n<DSKY_SCALE_TARGET_COUNT;++n)failures=(uint16_t)(failures+dsky_scale_check(n,&state,&before));dsky_scale_prepare(&state,73);state.words[0140]=2;state.words[0117]=0;state.words[0150]=01777;before=state;if(dsky_scale_output(&state,&io,DSKY_DP1OUT)!=P66_BIAS_OUT_OF_SCOPE || memcmp(&state,&before,sizeof state))++failures;printf("DSKY SCALE: 64 FULL 2050-WORD IMAGES, %u FAILURES\n",failures);return failures!=0;}
