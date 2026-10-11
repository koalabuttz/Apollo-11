/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "waitlist_zero_check.h"
static waitlist state;
volatile unsigned char waitlist_zero_mailbox[5];
int main(void) {
 uint16_t n,failures=0;
 for(n=0;n<WL_ZERO_CASE_COUNT;++n) failures=(uint16_t)(failures+wl_zero_check(n,&state));
 waitlist_zero_mailbox[1]=(unsigned char)failures;waitlist_zero_mailbox[2]=(unsigned char)(failures>>8);
 waitlist_zero_mailbox[3]=0;waitlist_zero_mailbox[4]=WL_ZERO_WORD_COUNT;waitlist_zero_mailbox[0]=0xa5;
 printf("WAITLIST ZERO: 256 STATES, %u FAILURES\n",failures); for(;;) {}
}
