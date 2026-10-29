/* SPDX-License-Identifier: MIT */
#include <stdint.h>
unsigned cm_difeq_check(void);
int main(void){volatile uint8_t *mail=(volatile uint8_t *)0xc080;unsigned f;mail[0]=0;f=cm_difeq_check();mail[1]=(uint8_t)f;mail[2]=(uint8_t)(f>>8);mail[3]=32;mail[4]=CM_DIFEQ_SELECTOR;mail[0]=0xa5;return f?1:0;}
