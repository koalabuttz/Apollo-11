/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "dsky_keyrupt_check.h"
static apollo_overlay_state state,before;
int main(void){uint8_t n;uint16_t failures=0;p66_bias_io io={0,0};
 for(n=0;n<DSKY_KEYRUPT_TARGET_COUNT;++n)failures=(uint16_t)(failures+dsky_keyrupt_check(n,&state,&before));
 dsky_keyrupt_prepare(&state,0);state.words[4]=02000;memcpy(&before,&state,sizeof state);
 if(dsky_keyrupt(&state,&io,0)!=EXEC_OUT_OF_SCOPE || memcmp(&state,&before,sizeof state)!=0)++failures;
 printf("DSKY KEYRUPT1: 64 complete2050word original images, %u failures\n",failures);return failures!=0;
}
