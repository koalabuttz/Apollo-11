/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "cm_formatter_check.h"
static apollo_overlay_state s,before;
volatile unsigned char cm_formatter_mailbox[5];
int main(void){uint8_t n;uint16_t failures=0;for(n=0;n<CM_FORMATTER_TARGET_COUNT;++n)failures=(uint16_t)(failures+cm_formatter_check(n,&s,&before));cm_formatter_mailbox[1]=(unsigned char)failures;cm_formatter_mailbox[2]=(unsigned char)(failures>>8);cm_formatter_mailbox[3]=64;cm_formatter_mailbox[4]=2;cm_formatter_mailbox[0]=0xa5;printf("CM FORMATTER: 64 FULL STATES, %u FAILURES\n",failures);for(;;){}}
