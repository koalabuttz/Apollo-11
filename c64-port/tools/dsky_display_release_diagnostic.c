/* SPDX-License-Identifier: MIT */
#include "dsky_display_release_check.h"
volatile unsigned char dsky_display_release_mailbox[5];static apollo_overlay_state image,before;
int main(void){uint16_t errors=0;uint8_t n;for(n=0;n<64;++n)errors=(uint16_t)(errors+dsky_display_release_check(n,&image,&before));dsky_display_release_mailbox[1]=(unsigned char)errors;dsky_display_release_mailbox[2]=(unsigned char)(errors>>8);dsky_display_release_mailbox[3]=64;dsky_display_release_mailbox[4]=2;dsky_display_release_mailbox[0]=0xa5;for(;;){}}
