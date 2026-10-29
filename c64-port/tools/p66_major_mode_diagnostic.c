/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "p66_major_mode_check.h"
static apollo_overlay_state state,before;
volatile unsigned char p66_major_mode_mailbox[5];
int main(void){uint8_t n;uint16_t failures=0;for(n=0;n<P66_MAJOR_TARGET_COUNT;++n)failures=(uint16_t)(failures+p66_major_mode_check(n,&state,&before));p66_major_mode_mailbox[1]=(unsigned char)failures;p66_major_mode_mailbox[2]=(unsigned char)(failures>>8);p66_major_mode_mailbox[3]=64;p66_major_mode_mailbox[4]=2;p66_major_mode_mailbox[0]=0xa5;printf("P66 MAJOR MODE: 64 FULL IMAGES, %u FAILURES\n",failures);for(;;){}}
