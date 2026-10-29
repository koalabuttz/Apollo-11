/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "cm_charin_number_check.h"
static apollo_overlay_state state,before;
volatile unsigned char cm_charin_number_mailbox[5];
int main(void){uint8_t n;uint16_t failures=0;for(n=0;n<CM_CHARIN_NUMBER_TARGET_COUNT;++n)failures=(uint16_t)(failures+cm_charin_number_check(n,&state,&before));cm_charin_number_mailbox[1]=(unsigned char)failures;cm_charin_number_mailbox[2]=(unsigned char)(failures>>8);cm_charin_number_mailbox[3]=64;cm_charin_number_mailbox[4]=2;cm_charin_number_mailbox[0]=0xa5;printf("DSKY numeric entry: 64 FULL IMAGES, %u FAILURES\n",failures);for(;;){}}
