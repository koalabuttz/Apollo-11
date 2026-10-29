/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "p66_mode_zero_check.h"
static apollo_overlay_state state,before;
int main(void){uint8_t n;uint16_t failures=0;p66_bias_io io={0100,0};for(n=0;n<P66_MODE_TARGET_COUNT;++n)failures=(uint16_t)(failures+p66_mode_zero_check(n,&state,&before));p66_mode_zero_prepare(&state,73);state.words[01011]=100;before=state;if(p66_mode_display(&state,&io)!=P66_BIAS_OUT_OF_SCOPE || memcmp(&state,&before,sizeof state))++failures;printf("P66 MODE DISPLAY: 64 FULL 2050-WORD IMAGES, %u FAILURES\n",failures);return failures!=0;}
