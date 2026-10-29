/* SPDX-License-Identifier: MIT */
#include "p66_direct_launch_check.h"
volatile unsigned char p66_direct_launch_mailbox[5];
static apollo_overlay_state image,expected;
int main(void){uint8_t n;uint16_t errors=0;for(n=0;n<24;++n)errors=(uint16_t)(errors+p66_direct_launch_check(n,&image,&expected));p66_direct_launch_mailbox[1]=(unsigned char)errors;p66_direct_launch_mailbox[2]=(unsigned char)(errors>>8);p66_direct_launch_mailbox[3]=24;p66_direct_launch_mailbox[4]=2;p66_direct_launch_mailbox[0]=0xa5;for(;;){}}
