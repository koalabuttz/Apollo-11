/* SPDX-License-Identifier: MIT */

#include "p66_basic_check.h"
volatile unsigned char p66_basic_mailbox[5];
int main(void){uint16_t errors;__asm__ volatile("cli" : : : "memory");errors=p66_basic_check();p66_basic_mailbox[1]=(unsigned char)errors;p66_basic_mailbox[2]=(unsigned char)(errors>>8);p66_basic_mailbox[3]=8;p66_basic_mailbox[4]=2;p66_basic_mailbox[0]=0xa5;for(;;){}}
