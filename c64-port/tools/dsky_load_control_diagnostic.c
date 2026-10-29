/* SPDX-License-Identifier: MIT */
#include "dsky_load_control_check.h"
volatile unsigned char dsky_load_control_mailbox[5];static apollo_overlay_state image,before;
int main(void){uint16_t errors=0;uint8_t n;for(n=0;n<64;++n)errors=(uint16_t)(errors+dsky_load_control_check(n,&image,&before));dsky_load_control_mailbox[1]=(unsigned char)errors;dsky_load_control_mailbox[2]=(unsigned char)(errors>>8);dsky_load_control_mailbox[3]=64;dsky_load_control_mailbox[4]=2;dsky_load_control_mailbox[0]=0xa5;for(;;){}}
