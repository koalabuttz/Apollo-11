/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "cm_phase_check.h"
#include "cm_phase_vectors.h"
static cm_phase_case state;
volatile unsigned char cm_phase_mailbox[5];
int main(void) {
 unsigned n; uint16_t failures=0;
 for(n=0;n<CM_PH_TARGET_COUNT;++n) failures=(uint16_t)(failures+cm_phase_check((uint8_t)n,&state));
 cm_phase_mailbox[1]=(unsigned char)failures; cm_phase_mailbox[2]=(unsigned char)(failures>>8);
 cm_phase_mailbox[3]=CM_PH_TARGET_COUNT; cm_phase_mailbox[4]=CM_PH_WORD_COUNT; cm_phase_mailbox[0]=0xa5;
 printf("PHASE %s: %u STATES, %u FAILURES\n",failures==0 ? "PASS" : "FAIL",CM_PH_TARGET_COUNT,(unsigned)failures);
 for(;;) {}
}
