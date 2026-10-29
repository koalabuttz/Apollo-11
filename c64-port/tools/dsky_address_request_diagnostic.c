/* SPDX-License-Identifier: MIT */
#include "dsky_address_request_check.h"
volatile unsigned char dsky_address_request_mailbox[5];static apollo_overlay_state image,before;
int main(void){uint16_t errors=0;uint8_t n;for(n=0;n<64;++n)errors=(uint16_t)(errors+dsky_address_request_check(n,&image,&before));dsky_address_request_mailbox[1]=(unsigned char)errors;dsky_address_request_mailbox[2]=(unsigned char)(errors>>8);dsky_address_request_mailbox[3]=64;dsky_address_request_mailbox[4]=2;dsky_address_request_mailbox[0]=0xa5;for(;;){}}
