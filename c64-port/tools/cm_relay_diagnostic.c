/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "cm_relay_check.h"
static apollo_overlay_state s,before;
volatile unsigned char cm_relay_mailbox[5];
int main(void){uint8_t n;uint16_t failures=0;for(n=0;n<CM_RELAY_TARGET_COUNT;++n)failures=(uint16_t)(failures+cm_relay_check(n,&s,&before));failures=(uint16_t)(failures+cm_relay_display_check(&s,(volatile uint8_t *)0x0400,(volatile uint8_t *)0xd800));cm_relay_mailbox[1]=(unsigned char)failures;cm_relay_mailbox[2]=(unsigned char)(failures>>8);cm_relay_mailbox[3]=64;cm_relay_mailbox[4]=2;cm_relay_mailbox[0]=0xa5;printf("CM RELAY:64 FULL STATES/SOURCE MODE89 SCREEN, %u FAILURES\n",failures);for(;;){}}
