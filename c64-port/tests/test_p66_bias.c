/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "p66_bias_check.h"
static apollo_overlay_state state,before;
int main(void){uint8_t n;uint16_t failures=0; p66_bias_io io={0,0};
 for(n=0;n<P66_BIAS_TARGET_COUNT;++n)failures=(uint16_t)(failures+p66_bias_check(n,&state,&before));
 for(n=0;n<4;++n){p66_bias_prepare(&state,73);if(n==0)state.words[067]=12;else if(n==1)state.words[state.words[0120]-1u]=(uint16_t)(state.words[0120]-1u);else if(n==2)++state.words[0166];else io.superbank=0100;before=state;if(p66_bias_start(&state,&io)!=P66_BIAS_OUT_OF_SCOPE || memcmp(&state,&before,sizeof state))++failures;}
 printf("P66 BIAS: 64 FULL 2050-WORD IMAGES, %u FAILURES\n",failures);return failures!=0;
}
