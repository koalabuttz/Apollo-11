/* SPDX-License-Identifier: MIT */
#include "dsky_five_blank_check.h"
volatile unsigned char dsky_five_blank_mailbox[5];static apollo_overlay_state image,before;
int main(void){uint16_t errors=0;uint8_t n;for(n=0;n<64;++n)errors=(uint16_t)(errors+dsky_five_blank_check(n,&image,&before));dsky_five_blank_mailbox[1]=(unsigned char)errors;dsky_five_blank_mailbox[2]=(unsigned char)(errors>>8);dsky_five_blank_mailbox[3]=64;dsky_five_blank_mailbox[4]=2;dsky_five_blank_mailbox[0]=0xa5;for(;;){}}
