/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "dsky_pinball_check.h"
static apollo_overlay_state state,before;
int main(void){uint8_t n;uint16_t failures=0;p66_bias_io io={0100,0};for(n=0;n<DSKY_PINBALL_TARGET_COUNT;++n)failures=(uint16_t)(failures+dsky_pinball_check(n,&state,&before));dsky_pinball_prepare(&state,73);state.words[0143]=22;before=state;if(dsky_pinball_write(&state,&io,DSKY_DSPIN)!=P66_BIAS_OUT_OF_SCOPE || memcmp(&state,&before,sizeof state))++failures;printf("DSKY PINBALL: 64 FULL 2050-WORD IMAGES, %u FAILURES\n",failures);return failures!=0;}
