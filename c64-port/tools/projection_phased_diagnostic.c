/* SPDX-License-Identifier: MIT */
#include <stdint.h>
unsigned projection_phased_check(void);
int main(void){volatile uint8_t *mail=(volatile uint8_t *)0xc080;unsigned f;mail[0]=0;f=projection_phased_check();mail[1]=(uint8_t)f;mail[2]=(uint8_t)(f>>8);mail[3]=32;mail[4]=GUIDANCE_PHASE_SELECTED;mail[0]=0xa5;return f?1:0;}
