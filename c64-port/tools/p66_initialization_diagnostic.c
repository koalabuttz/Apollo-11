/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "p66_initialization_check.h"
static p66_init_case state;
volatile unsigned char p66_initialization_mailbox[5];
int main(void){uint8_t n;uint16_t failures=0;for(n=0;n<P66_INIT_TARGET_COUNT;++n)failures=(uint16_t)(failures+p66_init_check(n,&state));p66_initialization_mailbox[1]=(unsigned char)failures;p66_initialization_mailbox[2]=(unsigned char)(failures>>8);p66_initialization_mailbox[3]=128;p66_initialization_mailbox[4]=17;p66_initialization_mailbox[0]=0xa5;printf("P66 INITIALIZATION: 128 STATES, %u FAILURES\n",failures);for(;;){}}
