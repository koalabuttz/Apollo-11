/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "dsky_scale_check.h"
static apollo_overlay_state state,before;
volatile unsigned char dsky_scale_mailbox[5];
int main(void){uint8_t n;uint16_t failures=0;for(n=0;n<DSKY_SCALE_TARGET_COUNT;++n)failures=(uint16_t)(failures+dsky_scale_check(n,&state,&before));dsky_scale_mailbox[1]=(unsigned char)failures;dsky_scale_mailbox[2]=(unsigned char)(failures>>8);dsky_scale_mailbox[3]=64;dsky_scale_mailbox[4]=2;dsky_scale_mailbox[0]=0xa5;printf("P66 MAJOR MODE: 64 FULL IMAGES, %u FAILURES\n",failures);for(;;){}}
