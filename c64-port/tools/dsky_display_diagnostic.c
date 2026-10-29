/* SPDX-License-Identifier: MIT */
#include "dsky_display_check.h"
volatile unsigned char dsky_display_mailbox[5];
int main(void){uint16_t failures=dsky_display_check((volatile uint8_t *)0x0400,(volatile uint8_t *)0xd800);dsky_display_mailbox[1]=(unsigned char)failures;dsky_display_mailbox[2]=(unsigned char)(failures>>8);dsky_display_mailbox[3]=1;dsky_display_mailbox[4]=1;dsky_display_mailbox[0]=0xa5;for(;;){}}
