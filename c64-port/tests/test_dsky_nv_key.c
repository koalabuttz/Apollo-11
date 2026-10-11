/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "dsky_nv_key_check.h"
static apollo_overlay_state state,before;
int main(void){uint8_t n;uint16_t failures=0;p66_bias_io io={0100,0};for(n=0;n<DSKY_NV_KEY_TARGET_COUNT;++n)failures=(uint16_t)(failures+dsky_nv_key_check(n,&state,&before));dsky_nv_key_prepare(&state,73);before=state;if(dsky_nv_key(&state,&io,(enum dsky_nv_key)2)!=P66_BIAS_OUT_OF_SCOPE || memcmp(&state,&before,sizeof state))++failures;printf("DSKY VERB/NOUN: 64 FULL 2050-WORD IMAGES, %u FAILURES\n",failures);return failures!=0;}
