/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "dsky_relay_check.h"
static apollo_overlay_state state,before;
int main(void){uint8_t n;uint16_t failures=0;dsky_relay_io io={{0,0},0,0};for(n=0;n<DSKY_RELAY_TARGET_COUNT;++n)failures=(uint16_t)(failures+dsky_relay_check(n,&state,&before));dsky_relay_prepare(&state,73);state.words[01023]=0;before=state;if(dsky_relay_drain(&state,&io)!=P66_BIAS_OUT_OF_SCOPE || memcmp(&state,&before,sizeof state))++failures;printf("DSKY RELAY: 64 FULL 2052-WORD IMAGES, %u FAILURES\n",failures);return failures!=0;}
