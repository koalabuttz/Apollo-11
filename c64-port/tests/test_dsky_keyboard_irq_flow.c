/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "dsky_keyboard_irq_flow_check.h"
int main(void){uint8_t n;uint16_t failures=0;for(n=0;n<DSKY_KEYBOARD_IRQ_FLOW_TARGET_COUNT;++n)failures=(uint16_t)(failures+dsky_keyboard_irq_flow_check(n));printf("DSKY KEYBOARD:16 queued native jobs/48 complete2051word source images, %u failures\n",failures);return failures!=0;}
