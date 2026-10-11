/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "dsky_two_digit_check.h"
static apollo_overlay_state state,before;
volatile unsigned char dsky_two_digit_mailbox[5];
int main(void){uint8_t n;uint16_t failures=0;for(n=0;n<DSKY_TWO_DIGIT_TARGET_COUNT;++n)failures=(uint16_t)(failures+dsky_two_digit_check(n,&state,&before));dsky_two_digit_mailbox[1]=(unsigned char)failures;dsky_two_digit_mailbox[2]=(unsigned char)(failures>>8);dsky_two_digit_mailbox[3]=64;dsky_two_digit_mailbox[4]=2;dsky_two_digit_mailbox[0]=0xa5;printf("DSKY DSPDECVN: 64 FULL IMAGES, %u FAILURES\n",failures);for(;;){}}
