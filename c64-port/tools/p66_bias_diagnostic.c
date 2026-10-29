/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "p66_bias_check.h"
static apollo_overlay_state state,before;
volatile unsigned char p66_bias_mailbox[5];
int main(void){uint8_t n;uint16_t failures=0;for(n=0;n<P66_BIAS_TARGET_COUNT;++n)failures=(uint16_t)(failures+p66_bias_check(n,&state,&before));p66_bias_mailbox[1]=(unsigned char)failures;p66_bias_mailbox[2]=(unsigned char)(failures>>8);p66_bias_mailbox[3]=64;p66_bias_mailbox[4]=2;p66_bias_mailbox[0]=0xa5;printf("P66 BIAS: 64 FULL IMAGES, %u FAILURES\n",failures);for(;;){}}
