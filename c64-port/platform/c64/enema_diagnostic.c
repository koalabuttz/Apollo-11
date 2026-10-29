/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "enema_check.h"
#include "enema_vectors.h"
static enema_case state;
volatile unsigned char enema_mailbox[5];
int main(void) {
 unsigned n; uint16_t failures=0;
 for(n=0;n<ENEMA_TARGET_COUNT;++n) failures=(uint16_t)(failures+enema_check((uint8_t)n,&state));
 enema_mailbox[1]=(unsigned char)failures; enema_mailbox[2]=(unsigned char)(failures>>8);
 enema_mailbox[3]=ENEMA_TARGET_COUNT; enema_mailbox[4]=(unsigned char)(ENEMA_WORD_COUNT&255); enema_mailbox[0]=0xa5;
 printf("ENEMA %s: %u STATES, %u FAILURES\n",failures==0 ? "PASS" : "FAIL",ENEMA_TARGET_COUNT,(unsigned)failures);
 for(;;) {}
}
