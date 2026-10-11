/* SPDX-License-Identifier: MIT */
#include "p66_direct_pending_check.h"
volatile unsigned char p66_direct_pending_mailbox[5];static apollo_overlay_state image,expected;
int main(void){uint16_t errors=p66_direct_pending_check(&image,&expected);p66_direct_pending_mailbox[1]=(unsigned char)errors;p66_direct_pending_mailbox[2]=(unsigned char)(errors>>8);p66_direct_pending_mailbox[3]=1;p66_direct_pending_mailbox[4]=2;p66_direct_pending_mailbox[0]=0xa5;for(;;){}}
