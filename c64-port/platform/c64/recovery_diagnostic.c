/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "recovery_check.h"
#include "recovery_vectors.h"
static recovery_case state;
volatile unsigned char recovery_mailbox[5];
int main(void) {
 unsigned n; uint16_t failures=0;
 for(n=0;n<RECOVERY_TARGET_COUNT;++n) failures=(uint16_t)(failures+recovery_check((uint8_t)n,&state));
 recovery_mailbox[1]=(unsigned char)failures; recovery_mailbox[2]=(unsigned char)(failures>>8);
 recovery_mailbox[3]=RECOVERY_TARGET_COUNT; recovery_mailbox[4]=RECOVERY_WORD_COUNT; recovery_mailbox[0]=0xa5;
 printf("RECOVERY %s: %u STATES, %u FAILURES\n",failures==0 ? "PASS" : "FAIL",RECOVERY_TARGET_COUNT,(unsigned)failures);
 for(;;) {}
}
