/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "dsky_nv_key_check.h"
static apollo_overlay_state state,before;
volatile unsigned char dsky_nv_key_mailbox[5];
int main(void){uint8_t n;uint16_t failures=0;for(n=0;n<DSKY_NV_KEY_TARGET_COUNT;++n)failures=(uint16_t)(failures+dsky_nv_key_check(n,&state,&before));dsky_nv_key_mailbox[1]=(unsigned char)failures;dsky_nv_key_mailbox[2]=(unsigned char)(failures>>8);dsky_nv_key_mailbox[3]=64;dsky_nv_key_mailbox[4]=2;dsky_nv_key_mailbox[0]=0xa5;printf("DSKY VERB/NOUN: 64 FULL IMAGES, %u FAILURES\n",failures);for(;;){}}
