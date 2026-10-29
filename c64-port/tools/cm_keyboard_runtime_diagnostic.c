/* SPDX-License-Identifier: MIT */
#include "cm_keyboard_runtime_check.h"
volatile unsigned char cm_keyboard_runtime_mailbox[5];
static apollo_overlay_state s,w;
int main(void){uint16_t errors=0;uint8_t n;__asm__ volatile("cli" : : : "memory");for(n=0;n<8;++n)errors=(uint16_t)(errors+cm_keyboard_runtime_check(n,&s,&w));cm_keyboard_runtime_mailbox[1]=(unsigned char)errors;cm_keyboard_runtime_mailbox[2]=(unsigned char)(errors>>8);cm_keyboard_runtime_mailbox[3]=8;cm_keyboard_runtime_mailbox[4]=4;cm_keyboard_runtime_mailbox[0]=0xa5;for(;;){}}
