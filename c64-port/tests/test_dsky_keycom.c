/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "dsky_keycom_check.h"
static apollo_overlay_state state,before;
int main(void){uint8_t n;uint16_t failures=0;p66_bias_io io={0,0};
 for(n=0;n<DSKY_KEYCOM_TARGET_COUNT;++n)failures=(uint16_t)(failures+dsky_keycom_check(n,&state,&before));
 dsky_keycom_prepare(&state,0);state.words[0]=040;memcpy(&before,&state,sizeof state);
 if(dsky_keycom(&state,&io)!=EXEC_OUT_OF_SCOPE || memcmp(&state,&before,sizeof state)!=0)++failures;
 printf("DSKY KEYCOM: 64 complete2050word original images, %u failures\n",failures);return failures!=0;
}
