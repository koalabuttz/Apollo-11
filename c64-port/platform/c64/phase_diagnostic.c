/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "phase_check.h"
#include "phase_vectors.h"
static phase_case state;
volatile unsigned char phase_mailbox[5];
int main(void) {
 unsigned n; uint16_t failures=0;
 for(n=0;n<PH_TARGET_COUNT;++n) failures=(uint16_t)(failures+phase_check((uint8_t)n,&state));
 phase_mailbox[1]=(unsigned char)failures; phase_mailbox[2]=(unsigned char)(failures>>8);
 phase_mailbox[3]=PH_TARGET_COUNT; phase_mailbox[4]=PH_WORD_COUNT; phase_mailbox[0]=0xa5;
 printf("PHASE %s: %u STATES, %u FAILURES\n",failures==0 ? "PASS" : "FAIL",PH_TARGET_COUNT,(unsigned)failures);
 for(;;) {}
}
