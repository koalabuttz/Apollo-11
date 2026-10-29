/* SPDX-License-Identifier: MIT */
#include "cm_keyboard_irq_check.h"
static apollo_overlay_state image,before;
volatile unsigned char cm_keyboard_irq_mailbox[5];
int main(void){uint8_t n;uint16_t failures=0;for(n=0;n<CM_KEYBOARD_IRQ_TARGET_COUNT;++n)failures=(uint16_t)(failures+cm_keyboard_irq_check(n,&image,&before));cm_keyboard_irq_mailbox[1]=(unsigned char)failures;cm_keyboard_irq_mailbox[2]=(unsigned char)(failures>>8);cm_keyboard_irq_mailbox[3]=64;cm_keyboard_irq_mailbox[4]=2;cm_keyboard_irq_mailbox[0]=0xa5;for(;;){}}
