/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "cm_charin_nv_check.h"
static apollo_overlay_state state,before;
volatile unsigned char cm_charin_nv_mailbox[5];
int main(void){uint8_t n;uint16_t failures=0;for(n=0;n<CM_CHARIN_NV_TARGET_COUNT;++n)failures=(uint16_t)(failures+cm_charin_nv_check(n,&state,&before));cm_charin_nv_mailbox[1]=(unsigned char)failures;cm_charin_nv_mailbox[2]=(unsigned char)(failures>>8);cm_charin_nv_mailbox[3]=64;cm_charin_nv_mailbox[4]=2;cm_charin_nv_mailbox[0]=0xa5;printf("DSKY VERB/NOUN entry: 64 FULL IMAGES, %u FAILURES\n",failures);for(;;){}}
