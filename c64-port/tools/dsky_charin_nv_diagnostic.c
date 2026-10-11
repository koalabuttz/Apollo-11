/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "dsky_charin_nv_check.h"
static apollo_overlay_state state,before;
volatile unsigned char dsky_charin_nv_mailbox[5];
int main(void){uint8_t n;uint16_t failures=0;for(n=0;n<DSKY_CHARIN_NV_TARGET_COUNT;++n)failures=(uint16_t)(failures+dsky_charin_nv_check(n,&state,&before));dsky_charin_nv_mailbox[1]=(unsigned char)failures;dsky_charin_nv_mailbox[2]=(unsigned char)(failures>>8);dsky_charin_nv_mailbox[3]=64;dsky_charin_nv_mailbox[4]=2;dsky_charin_nv_mailbox[0]=0xa5;printf("DSKY VERB/NOUN: 64 FULL IMAGES, %u FAILURES\n",failures);for(;;){}}
