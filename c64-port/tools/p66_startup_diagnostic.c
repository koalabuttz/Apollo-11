/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "p66_startup_check.h"
static p66_startup_case state;
volatile unsigned char p66_startup_mailbox[5];
int main(void) { unsigned n; uint16_t failures=0; for(n=0;n<P66_STARTUP_TARGET_COUNT;++n) failures=(uint16_t)(failures+p66_startup_check((uint8_t)n,&state)); p66_startup_mailbox[1]=(unsigned char)failures;p66_startup_mailbox[2]=(unsigned char)(failures>>8);p66_startup_mailbox[3]=P66_STARTUP_TARGET_COUNT;p66_startup_mailbox[4]=3;p66_startup_mailbox[0]=0xa5;printf("P66 STARTUP: 128 STATES, %u FAILURES\n",failures);for(;;){} }
