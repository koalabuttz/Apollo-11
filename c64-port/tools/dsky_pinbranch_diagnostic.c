/* SPDX-License-Identifier: MIT */
#include "dsky_pinbranch_check.h"
volatile unsigned char dsky_pinbranch_mailbox[5];static apollo_overlay_state image,before;
int main(void){uint16_t errors=0;uint8_t n;for(n=0;n<64;++n)errors=(uint16_t)(errors+dsky_pinbranch_check(n,&image,&before));dsky_pinbranch_mailbox[1]=(unsigned char)errors;dsky_pinbranch_mailbox[2]=(unsigned char)(errors>>8);dsky_pinbranch_mailbox[3]=64;dsky_pinbranch_mailbox[4]=2;dsky_pinbranch_mailbox[0]=0xa5;for(;;){}}
