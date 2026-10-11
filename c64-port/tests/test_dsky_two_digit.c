/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "dsky_two_digit_check.h"
static apollo_overlay_state state,before;
int main(void){uint8_t n;uint16_t failures=0;p66_bias_io io={0100,0};for(n=0;n<DSKY_TWO_DIGIT_TARGET_COUNT;++n)failures=(uint16_t)(failures+dsky_two_digit_check(n,&state,&before));dsky_two_digit_prepare(&state,73);state.words[0]=100;before=state;if(dsky_two_digit(&state,&io)!=P66_BIAS_OUT_OF_SCOPE || memcmp(&state,&before,sizeof state))++failures;printf("DSKY DSPDECVN: 64 FULL 2050-WORD IMAGES, %u FAILURES\n",failures);return failures!=0;}
