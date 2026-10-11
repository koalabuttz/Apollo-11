/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "dsky_relay_check.h"
static apollo_overlay_state state,before;
volatile unsigned char dsky_relay_mailbox[5];
int main(void){uint8_t n;uint16_t failures=0;for(n=0;n<DSKY_RELAY_TARGET_COUNT;++n)failures=(uint16_t)(failures+dsky_relay_check(n,&state,&before));dsky_relay_mailbox[1]=(unsigned char)failures;dsky_relay_mailbox[2]=(unsigned char)(failures>>8);dsky_relay_mailbox[3]=64;dsky_relay_mailbox[4]=2;dsky_relay_mailbox[0]=0xa5;printf("DSKY RELAY: 64 FULL IMAGES, %u FAILURES\n",failures);for(;;){}}
